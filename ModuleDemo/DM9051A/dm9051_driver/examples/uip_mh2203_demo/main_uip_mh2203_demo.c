#include "dm9051_uip_mh2203_smoke.h"
#include <stdio.h>

int main(void)
{
    int status;

    status = dm9051_uip_mh2203_smoke_init();
    if (status != DM9051_OK) {
        printf("Smoke test failed: %d\r\n", status);
    }

    while (1) {
    }
}
