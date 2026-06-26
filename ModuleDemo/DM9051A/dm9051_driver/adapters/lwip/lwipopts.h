#ifndef LWIP_HDR_LWIPOPTS_H
#define LWIP_HDR_LWIPOPTS_H

/*
 * lwIP options for MH2030 + DM9051A + bare-metal HTTP server.
 *
 * This target uses the lwIP raw API with NO_SYS=1.  DM9051A is a simple
 * Ethernet controller and does not offload IP/TCP/UDP checksums, so all
 * checksums stay enabled in software.
 */

/* ---------- System mode ---------- */
#define NO_SYS                          1
#define SYS_LIGHTWEIGHT_PROT            0
#define LWIP_TIMERS                     1
#define LWIP_TIMERS_CUSTOM              0

/* Raw API only.  Do not enable socket/netconn in NO_SYS mode. */
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0
#define LWIP_NETCONN_FULLDUPLEX         0
#define LWIP_NETBUF_RECVINFO            0

/* ---------- Protocol selection ---------- */
#define LWIP_IPV4                       1
#define LWIP_IPV6                       0
#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_TCP                        1
#define LWIP_UDP                        1
#define LWIP_ICMP                       1
#define LWIP_DHCP                       0
#define LWIP_AUTOIP                     0
#define LWIP_DNS                        0
#define LWIP_IGMP                       0
#define LWIP_MDNS_RESPONDER             0
#define LWIP_PTP                        0

/* HTTPD uses lwIP callback/raw API and this app uses altcp wrappers. */
#define LWIP_CALLBACK_API               1
#define LWIP_EVENT_API                  0
#define LWIP_ALTCP                      1
#define LWIP_ALTCP_TLS                  0

/* ---------- Memory ---------- */
#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        (12 * 1024)

#define MEMP_NUM_PBUF                   8
#define MEMP_NUM_TCP_PCB                4
#define MEMP_NUM_TCP_PCB_LISTEN         4
#define MEMP_NUM_TCP_SEG                16
#define MEMP_NUM_UDP_PCB                4
#define MEMP_NUM_SYS_TIMEOUT            LWIP_NUM_SYS_TIMEOUT_INTERNAL

#define PBUF_POOL_SIZE                  8
#define PBUF_POOL_BUFSIZE               1536

/* ---------- TCP tuning ---------- */
#define TCP_MSS                         (1500 - 40)
#define TCP_SND_BUF                     (4 * TCP_MSS)
#define TCP_SND_QUEUELEN                ((4 * TCP_SND_BUF + TCP_MSS - 1) / TCP_MSS)
#define TCP_WND                         (2 * TCP_MSS)
#define TCP_TTL                         255
#define TCP_QUEUE_OOSEQ                 0
#define LWIP_WND_SCALE                  0
#define TCP_RCV_SCALE                   0

/* ---------- netif / Ethernet ---------- */
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_STATUS_CALLBACK      0
#define LWIP_NETIF_EXT_STATUS_CALLBACK  0
#define LWIP_NETIF_HOSTNAME             0
#define LWIP_NETIF_LOOPBACK             0
#define LWIP_HAVE_LOOPIF                0
#define LWIP_NUM_NETIF_CLIENT_DATA      0
#define ETHARP_SUPPORT_STATIC_ENTRIES   1

/* ---------- Statistics / diagnostics ---------- */
#define LWIP_STATS                      0
#define MIB2_STATS                      0
#define LWIP_DEBUG                      0
#define LWIP_NOASSERT                   0
#define LWIP_MEM_ILLEGAL_FREE(msg)      do { } while (0)

/* ---------- Checksum ----------
 * DM9051A does not provide IP/TCP/UDP checksum offload.  Keep these enabled.
 */
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_GEN_ICMP               1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_UDP              1
#define CHECKSUM_CHECK_TCP              1
#define CHECKSUM_CHECK_ICMP             1

/* ---------- HTTP server ----------
 * The project uses apps/lwip_web2403v2_freelw instead of lwIP's stock httpd.c.
 */
#define HTTPD_INIT_CODE                 1
#define HTTPD_CALL_FS                   1
#define HTTPD_USE_CUSTOM_FSDATA         1
#define LWIP_HTTPD_DYNAMIC_HEADERS      1
#define LWIP_HTTPD_SUPPORT_V09          0
#define LWIP_HTTPD_SUPPORT_11_KEEPALIVE 0
#define LWIP_HTTPD_SUPPORT_REQUESTLIST  1
#define LWIP_HTTPD_REQ_QUEUELEN         5
#define LWIP_HTTPD_MAX_REQ_LENGTH       512
#define LWIP_HTTPD_MAX_REQUEST_URI_LEN  96
#define LWIP_HTTPD_SUPPORT_POST         0
#define LWIP_HTTPD_CGI                  0
#define LWIP_HTTPD_SSI                  0
#define LWIP_HTTPD_CUSTOM_FILES         0
#define LWIP_HTTPD_DYNAMIC_FILE_READ    0
#define LWIP_HTTPD_FILE_STATE           0
#define HTTPD_SERVER_PORT               80
#define HTTPD_SERVER_AGENT              "MH2030-DM9051A/lwIP"

#endif /* LWIP_HDR_LWIPOPTS_H */
