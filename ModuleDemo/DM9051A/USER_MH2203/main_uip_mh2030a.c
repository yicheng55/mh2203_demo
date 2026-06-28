#include "hal_mh2030a.h"
#include "netconf_mh2030a.h"
#include "app_call.h"
#include <stdio.h>

int main(void)
{
    mh2030a_uip_board_init(115200);
    mh2030a_uip_tick_init();

    printf("[MH2030A uIP] DM9051 + uIP bring-up\r\n");
    printf("[MH2030A uIP] target mode: %s\r\n", mh2030a_uip_target_mode());
    mh2030a_uip_net_init();

#if WEB_EN
    httpd_init();
#endif

    while (1) {
        mh2030a_uip_net_loop();
    }
}


