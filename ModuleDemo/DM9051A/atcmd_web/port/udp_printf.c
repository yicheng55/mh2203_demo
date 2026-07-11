#include <stdio.h>
#include <string.h>

#include "uip.h"
#include "udp_printf.h"

#define UDP_PRINTF_LINE_SIZE   256
#define UDP_PRINTF_QUEUE_LINES 10
#define UDP_PRINTF_ACK_PREFIX  "ack:"
#define UDP_PRINTF_ACK_PREFIX_LEN 4
#define UDP_PRINTF_ACK_SUFFIX  "\r\n"
#define UDP_PRINTF_ACK_SUFFIX_LEN 2

#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

extern int SER_PutChar(int ch);

static struct uip_udp_conn *udp_printf_conn;
static int udp_printf_link_up;
static int udp_printf_peer_active;
static int udp_printf_output_active;
static int udp_printf_output_mode;
static uip_ipaddr_t udp_printf_peer_addr;
static u16_t udp_printf_peer_port;

static char udp_printf_line[UDP_PRINTF_LINE_SIZE];
static unsigned short udp_printf_line_len;
static char udp_printf_queue[UDP_PRINTF_QUEUE_LINES][UDP_PRINTF_LINE_SIZE];
static unsigned short udp_printf_queue_len[UDP_PRINTF_QUEUE_LINES];
static unsigned char udp_printf_queue_head;
static unsigned char udp_printf_queue_tail;
static unsigned char udp_printf_queue_count;

struct udp_printf_reserved_port_entry {
    uint16_t port;
    const char *name;
};

static const struct udp_printf_reserved_port_entry udp_printf_reserved_ports[] = {
    /* Add reserved UDP service ports here so AT bridge cleanup keeps them. */
    { UDP_PRINTF_PORT, "udp_printf" },
};

#define UDP_PRINTF_RESERVED_PORT_COUNT \
    (sizeof(udp_printf_reserved_ports) / sizeof(udp_printf_reserved_ports[0]))

static struct uip_udp_conn *udp_printf_new_reserved_conn(uint16_t port)
{
    struct uip_udp_conn *conn;

    conn = uip_udp_new(0, 0);
    if (conn != 0) {
        uip_udp_bind(conn, HTONS(port));
    }

    return conn;
}

static void udp_printf_reset_conn_peer(void)
{
    if (udp_printf_conn != 0) {
        memset(udp_printf_conn->ripaddr, 0, sizeof(uip_ipaddr_t));
        udp_printf_conn->rport = 0;
    }
}

static void udp_printf_clear_peer(void)
{
    udp_printf_peer_active = 0;
    udp_printf_peer_port = 0;
    memset(udp_printf_peer_addr, 0, sizeof(udp_printf_peer_addr));
    udp_printf_reset_conn_peer();
}

static void udp_printf_clear_buffers(void)
{
    udp_printf_line_len = 0;
    udp_printf_queue_head = 0;
    udp_printf_queue_tail = 0;
    udp_printf_queue_count = 0;
}

static void udp_printf_register_peer(void)
{
    int was_peer_active = udp_printf_peer_active;
    uip_ipaddr_t old_peer_addr;
    u16_t old_peer_port = udp_printf_peer_port;

    if (!udp_printf_link_up || udp_printf_conn == 0) {
        return;
    }

    uip_ipaddr_copy(old_peer_addr, udp_printf_peer_addr);
    uip_ipaddr_copy(udp_printf_peer_addr, UDPBUF->srcipaddr);
    udp_printf_peer_port = UDPBUF->srcport;
    udp_printf_peer_active = 1;

    uip_ipaddr_copy(udp_printf_conn->ripaddr, udp_printf_peer_addr);
    udp_printf_conn->rport = udp_printf_peer_port;

    if (!was_peer_active ||
        old_peer_port != udp_printf_peer_port ||
        !uip_ipaddr_cmp(old_peer_addr, udp_printf_peer_addr)) {
        char msg[96];
        snprintf(msg, sizeof(msg),
                 "[UDP printf] client connected %u.%u.%u.%u:%u\r\n",
                 (unsigned)uip_ipaddr1(udp_printf_peer_addr),
                 (unsigned)uip_ipaddr2(udp_printf_peer_addr),
                 (unsigned)uip_ipaddr3(udp_printf_peer_addr),
                 (unsigned)uip_ipaddr4(udp_printf_peer_addr),
                 (unsigned)htons(udp_printf_peer_port));
        udp_printf_status_uart(msg);
    }
}

static void udp_printf_queue_line(void)
{
    if (udp_printf_line_len == 0) {
        return;
    }

    if (udp_printf_queue_count < UDP_PRINTF_QUEUE_LINES) {
        memcpy(udp_printf_queue[udp_printf_queue_tail],
               udp_printf_line,
               udp_printf_line_len);
        udp_printf_queue_len[udp_printf_queue_tail] = udp_printf_line_len;
        udp_printf_queue_tail++;
        if (udp_printf_queue_tail >= UDP_PRINTF_QUEUE_LINES) {
            udp_printf_queue_tail = 0;
        }
        udp_printf_queue_count++;
    }

    udp_printf_line_len = 0;
}

static void udp_printf_send_one_line(void)
{
    unsigned short len;

    if (!udp_printf_link_up || !udp_printf_peer_active ||
        udp_printf_conn == 0 || udp_printf_queue_count == 0) {
        return;
    }

    len = udp_printf_queue_len[udp_printf_queue_head];
    if (len > 0) {
        memcpy(uip_appdata, udp_printf_queue[udp_printf_queue_head], len);
        uip_ipaddr_copy(udp_printf_conn->ripaddr, udp_printf_peer_addr);
        udp_printf_conn->rport = udp_printf_peer_port;
        udp_printf_output_active = 1;
        uip_udp_send(len);
    }

    udp_printf_queue_head++;
    if (udp_printf_queue_head >= UDP_PRINTF_QUEUE_LINES) {
        udp_printf_queue_head = 0;
    }
    udp_printf_queue_count--;
}

static int udp_printf_send_ack(void)
{
    unsigned short rx_len;
    unsigned short ack_payload_len;
    unsigned short ack_len;

    if (!udp_printf_is_ready()) {
        return 0;
    }

    rx_len = uip_datalen();
    if (rx_len == 0) {
        return 0;
    }

    ack_payload_len = rx_len;
    if (ack_payload_len > (UDP_PRINTF_LINE_SIZE - UDP_PRINTF_ACK_PREFIX_LEN - UDP_PRINTF_ACK_SUFFIX_LEN)) {
        ack_payload_len = UDP_PRINTF_LINE_SIZE - UDP_PRINTF_ACK_PREFIX_LEN - UDP_PRINTF_ACK_SUFFIX_LEN;
    }

    memmove((char *)uip_appdata + UDP_PRINTF_ACK_PREFIX_LEN,
            uip_appdata,
            ack_payload_len);
    memcpy(uip_appdata, UDP_PRINTF_ACK_PREFIX, UDP_PRINTF_ACK_PREFIX_LEN);
    memcpy((char *)uip_appdata + UDP_PRINTF_ACK_PREFIX_LEN + ack_payload_len,
           UDP_PRINTF_ACK_SUFFIX,
           UDP_PRINTF_ACK_SUFFIX_LEN);
    ack_len = (unsigned short)(UDP_PRINTF_ACK_PREFIX_LEN + ack_payload_len + UDP_PRINTF_ACK_SUFFIX_LEN);
    uip_ipaddr_copy(udp_printf_conn->ripaddr, udp_printf_peer_addr);
    udp_printf_conn->rport = udp_printf_peer_port;
    udp_printf_output_active = 1;
    uip_udp_send((u16_t)ack_len);

    return 1;
}

void udp_printf_init(void)
{
    udp_printf_clear_buffers();
    udp_printf_clear_peer();

    udp_printf_conn = udp_printf_new_reserved_conn(UDP_PRINTF_PORT);
    if (udp_printf_conn != 0) {
        udp_printf_reset_conn_peer();
    }
    printf("[UDP printf] initialized on port %u\r\n", UDP_PRINTF_PORT);
}

void udp_printf_appcall(void)
{
    int sent_ack = 0;

    if (!udp_printf_link_up) {
        udp_printf_clear_peer();
        udp_printf_clear_buffers();
        return;
    }

    if (uip_newdata()) {
        udp_printf_register_peer();
        sent_ack = udp_printf_send_ack();
    }

    if (uip_poll() || (!sent_ack && uip_newdata())) {
        udp_printf_send_one_line();
    }
}

int udp_printf_putchar(int ch)
{
    if (udp_printf_output_active) {
        return ch;
    }

    if (!udp_printf_is_ready()) {
        udp_printf_line_len = 0;
        return ch;
    }

    if (udp_printf_line_len >= (UDP_PRINTF_LINE_SIZE - 1)) {
        udp_printf_line_len = 0;
        return ch;
    }

    udp_printf_line[udp_printf_line_len++] = (char)ch;
    if (ch == '\n') {
        udp_printf_queue_line();
    }

    return ch;
}

void udp_printf_set_link(int link_up)
{
    if (link_up) {
        udp_printf_link_up = 1;
        return;
    }

    udp_printf_link_up = 0;
    udp_printf_clear_peer();
    udp_printf_clear_buffers();
}

int udp_printf_is_link_up(void)
{
    return udp_printf_link_up;
}

int udp_printf_is_ready(void)
{
    return (udp_printf_link_up &&
            udp_printf_peer_active &&
            udp_printf_conn != 0) ? 1 : 0;
}

int udp_printf_has_pending(void)
{
    return (udp_printf_is_ready() &&
            udp_printf_queue_count > 0) ? 1 : 0;
}

struct uip_udp_conn *udp_printf_get_conn(void)
{
    return udp_printf_conn;
}

void udp_printf_output_done(void)
{
    udp_printf_output_active = 0;
}

void udp_printf_set_output_mode(int enable)
{
    udp_printf_output_mode = enable ? 1 : 0;
}

int udp_printf_is_output_mode_enabled(void)
{
    return udp_printf_output_mode;
}

int udp_printf_is_reserved_port(uint16_t port)
{
    unsigned int i;

    for (i = 0; i < UDP_PRINTF_RESERVED_PORT_COUNT; i++) {
        if (udp_printf_reserved_ports[i].port == port) {
            return 1;
        }
    }

    return 0;
}

const char *udp_printf_reserved_port_name(uint16_t port)
{
    unsigned int i;

    for (i = 0; i < UDP_PRINTF_RESERVED_PORT_COUNT; i++) {
        if (udp_printf_reserved_ports[i].port == port) {
            return udp_printf_reserved_ports[i].name;
        }
    }

    return "";
}

void udp_printf_status_uart(const char *msg)
{
    char prev = '\0';

    if (msg == 0) {
        return;
    }

    while (*msg != '\0') {
        if (*msg == '\n' && prev != '\r') {
            SER_PutChar('\r');
        }
        SER_PutChar((int)*msg);
        prev = *msg;
        msg++;
    }
}
