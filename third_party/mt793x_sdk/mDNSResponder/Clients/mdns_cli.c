// ==== FreeRTOS ====
#include "FreeRTOS.h"
#include "task.h"
#include "os.h"

#include <stdio.h>
#include "mDNSDebug.h"
#include "bct.h"
#include "cli.h"
#include "mdns.h"

static void mdnsd_task(void* p)
{
    mdnslogInfo("MDNS mDNSResponder!\n");
    mdnsd_start();
    vTaskDelete(NULL);
}

static void mdnsd_refresh(void* p)
{
    mdnslogInfo("Refresh mDNSResponder\n");
    mdns_update_interface();
}

// mdnsd start (fork 10240 1)
uint8_t _cli_mdnsd_start(uint8_t len, char *param[])
{
    mdnslogInfo("MDNS _cli_mdnsd_start!\n");
    mdnsd_task(NULL);
    return pdPASS;
}

// mdnsd stop
uint8_t _cli_mdnsd_stop(uint8_t len, char *param[])
{
    mdnslogInfo("MDNS _cli_mdnsd_stop!\n");
    mdnsd_stop();
    return pdPASS;
}

// fork 3096 1 mdnsd reg asdfg _http._tcp
uint8_t _cli_reg(uint8_t len, char *param[])
{
    mdnslogInfo("MDNS _cli_reg!\n");
    reg_service_cmd(param);
    return pdPASS;
}

// fork 3096 1 mdnsd rename
uint8_t _cli_rename(uint8_t len, char *param[])
{
    mdnslogInfo("MDNS _cli_rename!\n");
    rename_service_cmd(param);
    return pdPASS;
}

uint8_t _cli_refresh(uint8_t len, char *param[])
{
    mdnslogInfo("MDNS _cli_refresh!\n");
    mdnsd_refresh(NULL);
    return pdPASS;
}

cmd_t mdns_cli[] = {
    {"start", "mdnsd daemon task", _cli_mdnsd_start},
    {"stop", "stop mdnsd", _cli_mdnsd_stop},
    {"reg", "reg", _cli_reg},
    {"rename", "rename service", _cli_rename},
    {"refresh", "refresh mdnsd", _cli_refresh},
    {NULL}
};

