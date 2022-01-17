/** @file wifi.c
 *
 *  @brief  This file provides WiFi Core API
 *
 *  Copyright 2008-2020 NXP
 *
 *  NXP CONFIDENTIAL
 *  The source code contained or described herein and all documents related to
 *  the source code ("Materials") are owned by NXP, its
 *  suppliers and/or its licensors. Title to the Materials remains with NXP,
 *  its suppliers and/or its licensors. The Materials contain
 *  trade secrets and proprietary and confidential information of NXP, its
 *  suppliers and/or its licensors. The Materials are protected by worldwide copyright
 *  and trade secret laws and treaty provisions. No part of the Materials may be
 *  used, copied, reproduced, modified, published, uploaded, posted,
 *  transmitted, distributed, or disclosed in any way without NXP's prior
 *  express written permission.
 *
 *  No license under any patent, copyright, trade secret or other intellectual
 *  property right is granted to or conferred upon you by disclosure or delivery
 *  of the Materials, either expressly, by implication, inducement, estoppel or
 *  otherwise. Any license under such intellectual property rights must be
 *  express and approved by NXP in writing.
 *
 */

#include <mlan_api.h>
#include <mlan_sdio_api.h>

#include <stdio.h>
#include <string.h>
#include <wifi.h>
#include <wm_os.h>

#include "mlan_sdio.h"
#include "wifi-internal.h"
#include "wifi-sdio.h"

#define WIFI_COMMAND_RESPONSE_WAIT_MS 20000
#define WIFI_CORE_STACK_SIZE (350)
/* We don't see events coming in quick succession,
 * MAX_EVENTS = 10 is fairly big value */
#define MAX_EVENTS 20
#define MAX_MCAST_LEN (MLAN_MAX_MULTICAST_LIST_SIZE * MLAN_MAC_ADDR_LENGTH)
#define MAX_WAIT_TIME 35

#ifndef USB_SUPPORT_ENABLE
#define _T(x) x
#endif

static char wifi_init_done;
static char wifi_core_init_done;

bool sta_ampdu_tx_enable = true;

bool sta_ampdu_rx_enable = true;

static int retry_attempts;
wm_wifi_t wm_wifi;
static bool xfer_pending;

typedef enum __mlan_status
{
    MLAN_CARD_NOT_DETECTED = 3,
    MLAN_STATUS_FW_DNLD_FAILED,
    MLAN_STATUS_FW_NOT_DETECTED,
    MLAN_STATUS_FW_NOT_READY,
    MLAN_CARD_CMD_TIMEOUT
} __mlan_status;

static os_thread_stack_define(wifi_core_stack, WIFI_CORE_STACK_SIZE * sizeof(portSTACK_TYPE));
static os_thread_stack_define(wifi_drv_stack, 1024);
static os_queue_pool_define(g_io_events_queue_data, sizeof(struct bus_message) * MAX_EVENTS);

int wifi_set_mac_multicast_addr(const char * mlist, uint32_t num_of_addr);
int wrapper_get_wpa_ie_in_assoc(uint8_t * wpa_ie);
KEY_TYPE_ID get_sec_info();
mlan_status wlan_process_int_status(void * pmadapter);
void handle_data_packet(t_u8 interface, t_u8 * rcvdata, t_u16 datalen);

unsigned wifi_get_last_cmd_sent_ms()
{
    return wm_wifi.last_sent_cmd_msec;
}

uint32_t wifi_get_value1()
{
    return wifi_get_device_value1();
}

/* Wake up Wi-Fi card */
void wifi_wake_up_card(uint32_t * resp)
{
    sdio_drv_creg_write(0x0, 1, 0x02, resp);
}

/* When Wi-Fi card is in IEEE PS and sleeping
 * CMD or Data cannot be transmitted.
 * The card must be woken up.
 * So data or command transfer is temporarily kept
 * in pending state. This function returns value
 * of pending flag true/false.
 */
bool wifi_get_xfer_pending()
{
    return xfer_pending;
}
/*
 * This function sets the flag value
 */
void wifi_set_xfer_pending(bool xfer_val)
{
    xfer_pending = xfer_val;
}

void wifi_update_last_cmd_sent_ms()
{
    wm_wifi.last_sent_cmd_msec = os_ticks_to_msec(os_ticks_get());
}

static int wifi_get_command_resp_sem(unsigned long wait)
{
    return os_semaphore_get(&wm_wifi.command_resp_sem, wait);
}

static int wifi_put_command_resp_sem(void)
{
    return os_semaphore_put(&wm_wifi.command_resp_sem);
}

#define WL_ID_WIFI_CMD "wifi_cmd"

int wifi_get_command_lock()
{
    int rv; // = wakelock_get(WL_ID_WIFI_CMD);
            //	if (rv == WM_SUCCESS)
    rv = os_mutex_get(&wm_wifi.command_lock, OS_WAIT_FOREVER);

    return rv;
}

int wifi_get_mcastf_lock(void)
{
    return os_mutex_get(&wm_wifi.mcastf_mutex, OS_WAIT_FOREVER);
}

int wifi_put_mcastf_lock(void)
{
    return os_mutex_put(&wm_wifi.mcastf_mutex);
}

int wifi_put_command_lock()
{
    int rv = WM_SUCCESS;
    //	rv = wakelock_put(WL_ID_WIFI_CMD);
    //	if (rv == WM_SUCCESS)
    rv = os_mutex_put(&wm_wifi.command_lock);

    return rv;
}

#ifdef CONFIG_WIFI_FW_DEBUG

void wifi_register_fw_dump_cb(int (*wifi_usb_mount_cb)(), int (*wifi_usb_file_open_cb)(char * test_file_name),
                              int (*wifi_usb_file_write_cb)(uint8_t * data, size_t data_len), int (*wifi_usb_file_close_cb)())
{
    wm_wifi.wifi_usb_mount_cb      = wifi_usb_mount_cb;
    wm_wifi.wifi_usb_file_open_cb  = wifi_usb_file_open_cb;
    wm_wifi.wifi_usb_file_write_cb = wifi_usb_file_write_cb;
    wm_wifi.wifi_usb_file_close_cb = wifi_usb_file_close_cb;
}

#ifdef SD8801

#define DEBUG_HOST_READY 0xEE
#define DEBUG_FW_DONE 0xFF
#define DEBUG_MEMDUMP_FINISH 0xFE
#define SDIO_SCRATCH_REG 0x60
#define DEBUG_ITCM_DONE 0xaa
#define DEBUG_DTCM_DONE 0xbb
#define DEBUG_SQRAM_DONE 0xcc

#define DEBUG_DUMP_CTRL_REG 0x63
#define DEBUG_DUMP_FIRST_REG 0x62
#define DEBUG_DUMP_START_REG 0x64
#define DEBUG_DUMP_END_REG 0x6a
#define ITCM_SIZE 0x60000

#define SQRAM_SIZE 0x33500

#define DTCM_SIZE 0x14000

char itcm_dump_file_name[]  = _T("1:/itcm.bin");
char dtcm_dump_file_name[]  = _T("1:/dtcm.bin");
char sqram_dump_file_name[] = _T("1:/sqram.bin");

/**
 *  @brief This function dump firmware memory to file
 *
 *  @return         N/A
 */
void wifi_dump_firmware_info()
{
    int ret = 0;
    unsigned int reg, reg_start, reg_end;
    t_u8 ctrl_data = 0;
    int tries;
    t_u8 data[8], i;
    uint32_t resp;
    wifi_d("==== DEBUG MODE OUTPUT START: %d ====", os_get_timestamp());
    if (wm_wifi.wifi_usb_file_open_cb != NULL)
    {
        ret = wm_wifi.wifi_usb_file_open_cb(itcm_dump_file_name);
        if (ret != WM_SUCCESS)
        {
            wifi_e("File opening failed");
            goto done;
        }
    }
    else
    {
        wifi_e("File open callback is not registered");
        goto done;
    }
    wifi_d("Start ITCM output %d, please wait...", os_get_timestamp());
    reg_start = DEBUG_DUMP_START_REG;
    reg_end   = DEBUG_DUMP_END_REG;
    do
    {
        ret = sdio_drv_creg_write(DEBUG_DUMP_CTRL_REG, 1, DEBUG_HOST_READY, &resp);
        if (!ret)
        {
            wifi_e("SDIO Write ERR");
            goto done;
        }

        for (tries = 0; tries < MAX_POLL_TRIES; tries++)
        {
            ret = sdio_drv_creg_read(DEBUG_DUMP_CTRL_REG, 1, &resp);
            if (!ret)
            {
                wifi_e("SDIO READ ERR");
                goto done;
            }
            ctrl_data = resp & 0xff;

            if ((ctrl_data == DEBUG_FW_DONE) || (ctrl_data == DEBUG_ITCM_DONE) || (ctrl_data == DEBUG_DTCM_DONE) ||
                (ctrl_data == DEBUG_SQRAM_DONE))
                break;
            if (ctrl_data != DEBUG_HOST_READY)
            {
                ret = sdio_drv_creg_write(DEBUG_DUMP_CTRL_REG, 1, DEBUG_HOST_READY, &resp);
                if (!ret)
                {
                    wifi_e("SDIO Write ERR");
                    goto done;
                }
            }
            os_thread_sleep(os_msec_to_ticks(10));
        }
        if (ctrl_data == DEBUG_HOST_READY)
        {
            wifi_e("Fail to pull ctrl_data");
            goto done;
        }
        reg = DEBUG_DUMP_FIRST_REG;
        ret = sdio_drv_creg_read(reg, 1, &resp);
        if (!ret)
        {
            wifi_e("SDIO READ ERR");
            goto done;
        }

        i = 0;
        for (reg = reg_start; reg <= reg_end; reg++)
        {
            ret = sdio_drv_creg_read(reg, 1, &resp);
            if (!ret)
            {
                wifi_e("SDIO READ ERR");
                goto done;
            }
            data[i++] = resp & 0xff;
        }

        dump_hex(data, sizeof(data));

        if (wm_wifi.wifi_usb_file_write_cb != NULL)
        {
            ret = wm_wifi.wifi_usb_file_write_cb(data, sizeof(data));
            if (ret != WM_SUCCESS)
            {
                wifi_e("File writing failed");
                goto done;
            }
        }
        else
        {
            wifi_e("File write callback is not registered");
            goto done;
        }
        switch (ctrl_data)
        {
        case DEBUG_ITCM_DONE:
            if (wm_wifi.wifi_usb_file_close_cb != NULL)
            {
                ret = wm_wifi.wifi_usb_file_close_cb();
                if (ret != WM_SUCCESS)
                {
                    wifi_e("File closing failed");
                    goto done;
                }
            }
            else
            {
                wifi_e("File close callback is not registered");
                goto done;
            }
            if (wm_wifi.wifi_usb_file_open_cb != NULL)
            {
                ret = wm_wifi.wifi_usb_file_open_cb(dtcm_dump_file_name);
                if (ret != WM_SUCCESS)
                {
                    wifi_e("File opening failed");
                    goto done;
                }
                wifi_d("Start DTCM output %d, please wait...", os_get_timestamp());
            }
            else
            {
                wifi_e("USB open callback is not registered");
                goto done;
            }
            break;
        case DEBUG_DTCM_DONE:
            if (wm_wifi.wifi_usb_file_close_cb != NULL)
            {
                ret = wm_wifi.wifi_usb_file_close_cb();
                if (ret != WM_SUCCESS)
                {
                    wifi_e("File closing failed");
                    goto done;
                }
            }
            else
            {
                wifi_e("File close callback is not registered");
                goto done;
            }
            if (wm_wifi.wifi_usb_file_open_cb != NULL)
            {
                ret = wm_wifi.wifi_usb_file_open_cb(sqram_dump_file_name);
                if (ret != WM_SUCCESS)
                {
                    wifi_e("File opening failed");
                    goto done;
                }
                wifi_d("Start SQRAM output %u.%06u, please wait...", os_get_timestamp());
            }
            else
            {
                wifi_e("USB open cb is not registered");
                goto done;
            }
            break;
        case DEBUG_SQRAM_DONE:
            if (wm_wifi.wifi_usb_file_close_cb != NULL)
            {
                ret = wm_wifi.wifi_usb_file_close_cb();
                if (ret != WM_SUCCESS)
                {
                    wifi_e("File closing failed");
                    goto done;
                }
                wifi_d("End output!");
            }
            else
            {
                wifi_e("File close callback is not registered");
                goto done;
            }
            break;
        default:
            break;
        }
    } while (ctrl_data != DEBUG_SQRAM_DONE);

    wifi_d("The output ITCM/DTCM/SQRAM have been saved to files successfully!");
    /* end dump fw memory */
done:
    wifi_d("==== DEBUG MODE OUTPUT END: %d ====\n", os_get_timestamp());

    while (1)
        ;
}

/**
 *  @brief This function reads and displays SDIO registers for debugging
 *
 *  @return         N/A
 */
void wifi_sdio_reg_dbg()
{
    int ret = 0;
    t_u8 loop, index = 0, func, data;
    unsigned int reg, reg_start, reg_end;
    unsigned int scratch_reg = SDIO_SCRATCH_REG;
    unsigned int reg_table[] = { 0x28, 0x30, 0x34, 0x38, 0x3c };
    char buf[256], *ptr;
    uint32_t resp;

    for (loop = 0; loop < 5; loop++)
    {
        memset(buf, 0, sizeof(buf));
        ptr = buf;
        if (loop == 0)
        {
            /* Read the registers of SDIO function0 */
            func      = loop;
            reg_start = 0;
            reg_end   = 9;
        }
        else if (loop == 1)
        {
            /* Read the registers of SDIO function1 */
            func      = loop;
            reg_start = 4;
            reg_end   = 9;
        }
        else if (loop == 2)
        {
            /* Read specific registers of SDIO function1 */
            index     = 0;
            func      = 1;
            reg_start = reg_table[index++];
            reg_end   = reg_table[ARRAY_SIZE(reg_table) - 1];
        }
        else
        {
            /* Read the scratch registers of SDIO function1 */
            if (loop == 4)
                os_thread_sleep(os_msec_to_ticks(1));
            func      = 1;
            reg_start = scratch_reg;
            reg_end   = scratch_reg + 10;
        }
        if (loop != 2)
            ptr += sprintf(ptr, "SDIO Func%d (%#x-%#x): ", func, reg_start, reg_end);
        else
            ptr += sprintf(ptr, "SDIO Func%d: ", func);
        for (reg = reg_start; reg <= reg_end;)
        {
            ret  = sdio_drv_creg_read(reg, func, &resp);
            data = resp & 0xff;
            if (loop == 2)
                ptr += sprintf(ptr, "(%#x) ", reg);
            if (!ret)
                ptr += sprintf(ptr, "%02x ", data);
            else
            {
                ptr += sprintf(ptr, "ERR");
                break;
            }
            if (loop == 2 && reg < reg_end)
                reg = reg_table[index++];
            else
                reg++;
        }
        wifi_d("%s", buf);
    }
}

#elif defined(SD8977) || defined(SD8978) || defined(SD8987) || defined(SD8997) || defined(SD9097) || defined(SD9098)

#define DEBUG_HOST_READY 0xCC
#define DEBUG_FW_DONE 0xFF
#define DEBUG_MEMDUMP_FINISH 0xFE

#define DEBUG_DUMP_CTRL_REG 0xF9
#define DEBUG_DUMP_START_REG 0xF1
#define DEBUG_DUMP_END_REG 0xF8
#define SDIO_SCRATCH_REG 0xE8

char fw_dump_file_name[] = _T("1:/fw_dump.bin");

typedef enum
{
    DUMP_TYPE_ITCM        = 0,
    DUMP_TYPE_DTCM        = 1,
    DUMP_TYPE_SQRAM       = 2,
    DUMP_TYPE_APU_REGS    = 3,
    DUMP_TYPE_CIU_REGS    = 4,
    DUMP_TYPE_ICU_REGS    = 5,
    DUMP_TYPE_MAC_REGS    = 6,
    DUMP_TYPE_EXTEND_7    = 7,
    DUMP_TYPE_EXTEND_8    = 8,
    DUMP_TYPE_EXTEND_9    = 9,
    DUMP_TYPE_EXTEND_10   = 10,
    DUMP_TYPE_EXTEND_11   = 11,
    DUMP_TYPE_EXTEND_12   = 12,
    DUMP_TYPE_EXTEND_13   = 13,
    DUMP_TYPE_EXTEND_LAST = 14
} dumped_mem_type;

#define MAX_NAME_LEN 8
#define MAX_FULL_NAME_LEN 32

typedef struct
{
    t_u8 mem_name[MAX_NAME_LEN];
    t_u8 * mem_Ptr;
    struct file * pfile_mem;
    t_u8 done_flag;
    t_u8 type;
} memory_type_mapping;

memory_type_mapping mem_type_mapping_tbl = { "DUMP", NULL, NULL, 0xDD };

typedef enum
{
    RDWR_STATUS_SUCCESS = 0,
    RDWR_STATUS_FAILURE = 1,
    RDWR_STATUS_DONE    = 2
} rdwr_status;

/**
 *  @brief This function read/write firmware via cmd52
 *
 *  @param doneflag  A flag
 *
 *  @return         MLAN_STATUS_SUCCESS
 */
rdwr_status wifi_cmd52_rdwr_firmware(t_u8 doneflag)
{
    int ret                = 0;
    int tries              = 0;
    t_u8 ctrl_data         = 0;
    t_u8 dbg_dump_ctrl_reg = 0;
    t_u8 debug_host_ready  = 0;
    uint32_t resp;

    dbg_dump_ctrl_reg = DEBUG_DUMP_CTRL_REG;
    debug_host_ready  = DEBUG_HOST_READY;

    ret = sdio_drv_creg_write(dbg_dump_ctrl_reg, 1, debug_host_ready, &resp);
    if (!ret)
    {
        wifi_e("SDIO Write ERR");
        return RDWR_STATUS_FAILURE;
    }
    for (tries = 0; tries < MAX_POLL_TRIES; tries++)
    {
        ret = sdio_drv_creg_read(dbg_dump_ctrl_reg, 1, &resp);
        if (!ret)
        {
            wifi_e("SDIO READ ERR");
            return RDWR_STATUS_FAILURE;
        }
        ctrl_data = resp & 0xff;
        if (ctrl_data == DEBUG_FW_DONE)
            break;
        if (doneflag && ctrl_data == doneflag)
            return RDWR_STATUS_DONE;
        if (ctrl_data != debug_host_ready)
        {
            ret = sdio_drv_creg_write(dbg_dump_ctrl_reg, 1, debug_host_ready, &resp);
            if (!ret)
            {
                wifi_e("SDIO Write ERR");
                return RDWR_STATUS_FAILURE;
            }
        }
        os_thread_sleep(os_msec_to_ticks(1));
    }
    if (ctrl_data == debug_host_ready)
    {
        wifi_e("Fail to pull ctrl_data");
        return RDWR_STATUS_FAILURE;
    }

    return RDWR_STATUS_SUCCESS;
}

/**
 *  @brief This function dump firmware memory to file
 *
 *  @return         N/A
 */
void wifi_dump_firmware_info()
{
    int ret   = 0;
    int tries = 0;
    unsigned int reg, reg_start, reg_end;
    t_u8 start_flag = 0;
    t_u8 doneflag   = 0;
    rdwr_status stat;
    t_u8 dbg_dump_start_reg                     = 0;
    t_u8 dbg_dump_end_reg                       = 0;
    memory_type_mapping * pmem_type_mapping_tbl = &mem_type_mapping_tbl;
    t_u8 data[8], i;
    uint32_t resp;

    dbg_dump_start_reg = DEBUG_DUMP_START_REG;
    dbg_dump_end_reg   = DEBUG_DUMP_END_REG;

    wifi_d("==== DEBUG MODE OUTPUT START: %d.%06u ====", os_get_timestamp());
    /* read the number of the memories which will dump */
    if (RDWR_STATUS_FAILURE == wifi_cmd52_rdwr_firmware(doneflag))
        goto done;

    /** check the reg which indicate dump starting */
    for (reg = dbg_dump_start_reg; reg <= dbg_dump_end_reg; reg++)
    {
        for (tries = 0; tries < MAX_POLL_TRIES; tries++)
        {
            ret = sdio_drv_creg_read(reg, 1, &resp);
            if (!ret)
            {
                wifi_e("SDIO READ ERR");
                goto done;
            }
            start_flag = resp & 0xff;
            /** 0 means dump starting*/
            if (start_flag == 0)
                break;
            os_thread_sleep(os_msec_to_ticks(1));
        }
        if (tries == MAX_POLL_TRIES)
        {
            wifi_d("FW not ready to dump");
            goto done;
        }
    }
    if (wm_wifi.wifi_usb_file_open_cb != NULL)
    {
        ret = wm_wifi.wifi_usb_file_open_cb(fw_dump_file_name);
        if (ret != WM_SUCCESS)
        {
            wifi_e("File opening failed");
            goto done;
        }
    }
    else
    {
        wifi_e("File open callback is not registered");
        goto done;
    }

    doneflag = pmem_type_mapping_tbl->done_flag;
    wifi_d("Start %s output %d, please wait...", pmem_type_mapping_tbl->mem_name, os_get_timestamp());
    do
    {
        stat = wifi_cmd52_rdwr_firmware(doneflag);
        if (RDWR_STATUS_FAILURE == stat)
            goto done;

        reg_start = dbg_dump_start_reg;
        reg_end   = dbg_dump_end_reg;
        i         = 0;
        for (reg = reg_start; reg <= reg_end; reg++)
        {
            ret = sdio_drv_creg_read(reg, 1, &resp);
            if (!ret)
            {
                wifi_e("SDIO READ ERR");
                goto done;
            }
            data[i++] = (resp & 0xff);
        }
        if (wm_wifi.wifi_usb_file_write_cb != NULL)
        {
            ret = wm_wifi.wifi_usb_file_write_cb(data, sizeof(data));
            if (ret != WM_SUCCESS)
            {
                wifi_e("File writing failed");
                goto done;
            }
        }
        else
        {
            wifi_e("File write callback is not registered");
            goto done;
        }

        if (RDWR_STATUS_DONE == stat)
        {
            if (wm_wifi.wifi_usb_file_close_cb != NULL)
            {
                ret = wm_wifi.wifi_usb_file_close_cb();
                if (ret != WM_SUCCESS)
                {
                    wifi_e("File closing failed");
                    goto done;
                }
            }
            else
            {
                wifi_e("File close callback is not registered");
                goto done;
            }
            break;
        }
    } while (1);

    wifi_d("==== DEBUG MODE OUTPUT END: %d ====\n", os_get_timestamp());
    /* end dump fw memory */
done:
    while (1)
        ;
}

/**
 *  @brief This function reads and displays SDIO registers for debugging
 *
 *  @return         N/A
 */
void wifi_sdio_reg_dbg()
{
    int ret = 0;
    t_u8 loop, index = 0, func, data;
    unsigned int reg, reg_start, reg_end;
    unsigned int scratch_reg = SDIO_SCRATCH_REG;
    unsigned int reg_table[] = { 0x08, 0x58, 0x5C, 0x5D, 0x60, 0x61, 0x62, 0x64, 0x65, 0x66, 0x68, 0x69, 0x6a };
    char buf[256], *ptr;
    uint32_t resp;

    for (loop = 0; loop < 5; loop++)
    {
        memset(buf, 0, sizeof(buf));
        ptr = buf;
        if (loop == 0)
        {
            /* Read the registers of SDIO function0 */
            func      = loop;
            reg_start = 0;
            reg_end   = 9;
        }
        else if (loop == 1)
        {
            /* Read the registers of SDIO function1 */
            func      = loop;
            reg_start = 0x10;
            reg_end   = 0x17;
        }
        else if (loop == 2)
        {
            /* Read specific registers of SDIO function1 */
            index     = 0;
            func      = 1;
            reg_start = reg_table[index++];
            reg_end   = reg_table[ARRAY_SIZE(reg_table) - 1];
        }
        else
        {
            /* Read the scratch registers of SDIO function1 */
            if (loop == 4)
                os_thread_sleep(os_msec_to_ticks(1));
            func      = 1;
            reg_start = scratch_reg;
            reg_end   = scratch_reg + 10;
        }
        if (loop != 2)
            ptr += sprintf(ptr, "SDIO Func%d (%#x-%#x): ", func, reg_start, reg_end);
        else
            ptr += sprintf(ptr, "SDIO Func%d: ", func);
        for (reg = reg_start; reg <= reg_end;)
        {
            ret  = sdio_drv_creg_read(reg, func, &resp);
            data = resp & 0xff;
            if (loop == 2)
                ptr += sprintf(ptr, "(%#x) ", reg);
            if (ret)
                ptr += sprintf(ptr, "%02x ", data);
            else
            {
                ptr += sprintf(ptr, "ERR");
                break;
            }
            if (loop == 2 && reg < reg_end)
                reg = reg_table[index++];
            else
                reg++;
        }
        wifi_d("%s", buf);
    }
}
#endif
#endif

int wifi_wait_for_cmdresp(void * cmd_resp_priv)
{
    int ret;
    HostCmd_DS_COMMAND * cmd = wifi_get_command_buffer();
    t_u32 buf_len            = MLAN_SDIO_BLOCK_SIZE;
    t_u32 tx_blocks;

#if defined(CONFIG_ENABLE_WARNING_LOGS) || defined(CONFIG_WIFI_CMD_RESP_DEBUG)

    wcmdr_d("CMD --- : 0x%x Size: %d Seq: %d", cmd->command, cmd->size, cmd->seq_num);
#endif /* CONFIG_ENABLE_WARNING_LOGS || CONFIG_WIFI_CMD_RESP_DEBUG*/

    if (cmd->size > WIFI_FW_CMDBUF_SIZE)
    {
        /*
         * This is a error added to be flagged during
         * development cycle. It is not expected to
         * occur in production. The legacy code below
         * only sents out MLAN_SDIO_BLOCK_SIZE or 2 *
         * MLAN_SDIO_BLOCK_SIZE sized packet. If ever
         * in future greater packet size generated then
         * this error will help to localize the problem.
         */
        wifi_e("cmd size greater than WIFI_FW_CMDBUF_SIZE\r\n");
        return -WM_FAIL;
    }

    tx_blocks = (cmd->size + MLAN_SDIO_BLOCK_SIZE - 1) / MLAN_SDIO_BLOCK_SIZE;

    if (cmd->size < 512)
    {
        buf_len   = tx_blocks * MLAN_SDIO_BLOCK_SIZE;
        tx_blocks = 1;
    }

    /*
     * This is the private pointer. Only the command response handler
     * function knows what it means or where it points to. It can be
     * NULL.
     */
    wm_wifi.cmd_resp_priv = cmd_resp_priv;

    wifi_send_cmdbuffer(tx_blocks, buf_len);
    /* Wait max 10 sec for the command response */
    ret = wifi_get_command_resp_sem(WIFI_COMMAND_RESPONSE_WAIT_MS);
    if (ret != WM_SUCCESS)
    {
#ifdef CONFIG_ENABLE_WARNING_LOGS
        wifi_w("Command response timed out. command = 0x%x", cmd->command);
#endif /* CONFIG_ENABLE_WARNING_LOGS */
#ifdef CONFIG_WIFI_FW_DEBUG
        wifi_sdio_reg_dbg();

        if (wm_wifi.wifi_usb_mount_cb != NULL)
        {
            ret = wm_wifi.wifi_usb_mount_cb();
            if (ret == WM_SUCCESS)
                wifi_dump_firmware_info();
            else
            {
                wifi_e("USB mounting failed");
            }
        }
        else
            wifi_e("USB mount callback is not registered");
#endif
    }

    wm_wifi.cmd_resp_priv = NULL;
    wifi_put_command_lock();
    return ret;
}

void wifi_event_completion(int event, enum wifi_event_reason result, void * data)
{
    struct wifi_message msg;
    if (!wm_wifi.wlc_mgr_event_queue)
        return;

    msg.data   = data;
    msg.reason = result;
    msg.event  = event;
    os_queue_send(wm_wifi.wlc_mgr_event_queue, &msg, OS_NO_WAIT);
}

static int cmp_mac_addr(uint8_t * mac_addr1, uint8_t * mac_addr2)
{
    int i = 0;

    if (!mac_addr1 || !mac_addr2)
        return 1;

    for (i = 0; i < MLAN_MAC_ADDR_LENGTH; i++)
        if (mac_addr1[i] != mac_addr2[i])
            return 1;
    return 0;
}

static int add_mcast_ip(uint8_t * mac_addr)
{
    mcast_filter *node_t, *new_node;
    wifi_get_mcastf_lock();
    node_t = wm_wifi.start_list;
    if (wm_wifi.start_list == NULL)
    {
        new_node = os_mem_alloc(sizeof(mcast_filter));
        if (new_node == NULL)
        {
            wifi_put_mcastf_lock();
            return -WM_FAIL;
        }
        memcpy(new_node->mac_addr, mac_addr, MLAN_MAC_ADDR_LENGTH);
        new_node->next     = NULL;
        wm_wifi.start_list = new_node;
        wifi_put_mcastf_lock();
        return WM_SUCCESS;
    }
    while (node_t->next != NULL && cmp_mac_addr(node_t->mac_addr, mac_addr))
        node_t = node_t->next;

    if (!cmp_mac_addr(node_t->mac_addr, mac_addr))
    {
        wifi_put_mcastf_lock();
        return -WM_FAIL;
    }
    new_node = os_mem_alloc(sizeof(mcast_filter));
    if (new_node == NULL)
    {
        wifi_put_mcastf_lock();
        return -WM_FAIL;
    }
    memcpy(new_node->mac_addr, mac_addr, MLAN_MAC_ADDR_LENGTH);
    new_node->next = NULL;
    node_t->next   = new_node;
    wifi_put_mcastf_lock();
    return WM_SUCCESS;
}

static int remove_mcast_ip(uint8_t * mac_addr)
{
    mcast_filter *curr_node, *prev_node;
    wifi_get_mcastf_lock();
    curr_node = wm_wifi.start_list->next;
    prev_node = wm_wifi.start_list;
    if (wm_wifi.start_list == NULL)
    {
        wifi_put_mcastf_lock();
        return -WM_FAIL;
    }
    if (curr_node == NULL && cmp_mac_addr(prev_node->mac_addr, mac_addr))
    {
        os_mem_free(prev_node);
        wm_wifi.start_list = NULL;
        wifi_put_mcastf_lock();
        return WM_SUCCESS;
    }
    /* If search element is at first location */
    if (!cmp_mac_addr(prev_node->mac_addr, mac_addr))
    {
        wm_wifi.start_list = prev_node->next;
        os_mem_free(prev_node);
        wifi_put_mcastf_lock();
        return WM_SUCCESS;
    }
    /* Find node in linked list */
    while (cmp_mac_addr(curr_node->mac_addr, mac_addr) && curr_node->next != NULL)
    {
        prev_node = curr_node;
        curr_node = curr_node->next;
    }
    if (!cmp_mac_addr(curr_node->mac_addr, mac_addr))
    {
        prev_node->next = curr_node->next;
        os_mem_free(curr_node);
        wifi_put_mcastf_lock();
        return WM_SUCCESS;
    }
    wifi_put_mcastf_lock();
    return -WM_FAIL;
}

static int make_filter_list(char * mlist, int maxlen)
{
    mcast_filter * node_t;
    uint8_t maddr_cnt = 0;
    wifi_get_mcastf_lock();
    node_t = wm_wifi.start_list;
    while (node_t != NULL)
    {
        memcpy(mlist, node_t->mac_addr, MLAN_MAC_ADDR_LENGTH);
        node_t = (struct mcast_filter *) node_t->next;
        mlist  = mlist + MLAN_MAC_ADDR_LENGTH;
        maddr_cnt++;
        if (maddr_cnt > (maxlen / 6))
            break;
    }
    wifi_put_mcastf_lock();
    return maddr_cnt;
}

void wifi_get_ipv4_multicast_mac(uint32_t ipaddr, uint8_t * mac_addr)
{
    int i = 0, j = 0;
    uint32_t mac_addr_r = 0x01005E;
    ipaddr              = ipaddr & 0x7FFFFF;
    /* Generate Multicast Mapped Mac Address for IPv4
     * To get Multicast Mapped MAC address,
     * To calculate 6 byte Multicast-Mapped MAC Address.
     * 1) Fill higher 24-bits with IANA Multicast OUI (01-00-5E)
     * 2) Set 24th bit as Zero
     * 3) Fill lower 23-bits with from IP address (ignoring higher
     * 9bits).
     */
    for (i = 2; i >= 0; i--, j++)
        mac_addr[j] = (char) (mac_addr_r >> 8 * i) & 0xFF;

    for (i = 2; i >= 0; i--, j++)
        mac_addr[j] = (char) (ipaddr >> 8 * i) & 0xFF;
}

#ifdef CONFIG_IPV6
void wifi_get_ipv6_multicast_mac(uint32_t ipaddr, uint8_t * mac_addr)
{
    int i = 0, j = 0;
    uint32_t mac_addr_r = 0x3333;
    /* Generate Multicast Mapped Mac Address for IPv6
     * To get Multicast Mapped MAC address,
     * To calculate 6 byte Multicast-Mapped MAC Address.
     * 1) Fill higher 16-bits with IANA Multicast OUI (33-33)
     * 2) Fill lower 24-bits with from IP address
     */
    for (i = 1; i >= 0; i--, j++)
        mac_addr[j] = (char) (mac_addr_r >> 8 * i) & 0xFF;

    for (i = 3; i >= 0; i--, j++)
        mac_addr[j] = (char) (ipaddr >> 8 * i) & 0xFF;
}
#endif /* CONFIG_IPV6 */

int wifi_add_mcast_filter(uint8_t * mac_addr)
{
    char mlist[MAX_MCAST_LEN];
    int len, ret;
    /* If MAC address is 00:11:22:33:44:55,
     * then pass mac_addr array in following format:
     * mac_addr[0] = 00
     * mac_addr[1] = 11
     * mac_addr[2] = 22
     * mac_addr[3] = 33
     * mac_addr[4] = 44
     * mac_addr[5] = 55
     */
    ret = add_mcast_ip(mac_addr);
    if (ret != WM_SUCCESS)
        return ret;
    len = make_filter_list(mlist, MAX_MCAST_LEN);
    return wifi_set_mac_multicast_addr(mlist, len);
}

int wifi_remove_mcast_filter(uint8_t * mac_addr)
{
    char mlist[MAX_MCAST_LEN];
    int len, ret;
    /* If MAC address is 00:11:22:33:44:55,
     * then pass mac_addr array in following format:
     * mac_addr[0] = 00
     * mac_addr[1] = 11
     * mac_addr[2] = 22
     * mac_addr[3] = 33
     * mac_addr[4] = 44
     * mac_addr[5] = 55
     */
    ret = remove_mcast_ip(mac_addr);
    if (ret != WM_SUCCESS)
        return ret;
    len = make_filter_list(mlist, MAX_MCAST_LEN);
    ret = wifi_set_mac_multicast_addr(mlist, len);
    return ret;
}

/* Since we do not have the descriptor list we will using this adaptor function */
int wrapper_bssdesc_first_set(int bss_index, uint8_t * BssId, bool * is_ibss_bit_set, int * ssid_len, uint8_t * ssid,
                              uint8_t * Channel, uint8_t * RSSI, uint16_t * beacon_period, uint8_t * dtim_period,
                              _SecurityMode_t * WPA_WPA2_WEP, _Cipher_t * wpa_mcstCipher, _Cipher_t * wpa_ucstCipher,
                              _Cipher_t * rsn_mcstCipher, _Cipher_t * rsn_ucstCipher, bool * is_pmf_required);

int wrapper_bssdesc_second_set(int bss_index, bool * phtcap_ie_present, bool * phtinfo_ie_present, bool * wmm_ie_present,
                               uint8_t * band, bool * wps_IE_exist, uint16_t * wps_session, bool * wpa2_entp_IE_exist,
                               uint8_t * trans_mode, uint8_t * trans_bssid, int * trans_ssid_len, uint8_t * trans_ssid);

static struct wifi_scan_result common_desc;
int wifi_get_scan_result(unsigned int index, struct wifi_scan_result ** desc)
{
    memset(&common_desc, 0x00, sizeof(struct wifi_scan_result));
    int rv =
        wrapper_bssdesc_first_set(index, common_desc.bssid, &common_desc.is_ibss_bit_set, &common_desc.ssid_len, common_desc.ssid,
                                  &common_desc.Channel, &common_desc.RSSI, &common_desc.beacon_period, &common_desc.dtim_period,
                                  &common_desc.WPA_WPA2_WEP, &common_desc.wpa_mcstCipher, &common_desc.wpa_ucstCipher,
                                  &common_desc.rsn_mcstCipher, &common_desc.rsn_ucstCipher, &common_desc.is_pmf_required);
    if (rv != WM_SUCCESS)
    {
        wifi_e("wifi_get_scan_result failed");
        return rv;
    }

    /* Country info not populated */
    rv = wrapper_bssdesc_second_set(index, &common_desc.phtcap_ie_present, &common_desc.phtinfo_ie_present,
                                    &common_desc.wmm_ie_present, &common_desc.band, &common_desc.wps_IE_exist,
                                    &common_desc.wps_session, &common_desc.wpa2_entp_IE_exist, &common_desc.trans_mode,
                                    common_desc.trans_bssid, &common_desc.trans_ssid_len, common_desc.trans_ssid);

    if (rv != WM_SUCCESS)
    {
        wifi_e("wifi_get_scan_result failed");
        return rv;
    }

    *desc = &common_desc;

    return WM_SUCCESS;
}

int wifi_register_event_queue(os_queue_t * event_queue)
{
    if (!event_queue)
        return -WM_E_INVAL;

    if (wm_wifi.wlc_mgr_event_queue)
        return -WM_FAIL;

    wm_wifi.wlc_mgr_event_queue = event_queue;
    return WM_SUCCESS;
}

int wifi_unregister_event_queue(os_queue_t * event_queue)
{
    if (!wm_wifi.wlc_mgr_event_queue || wm_wifi.wlc_mgr_event_queue != event_queue)
        return -WM_FAIL;

    wm_wifi.wlc_mgr_event_queue = NULL;
    return WM_SUCCESS;
}

int wifi_get_wpa_ie_in_assoc(uint8_t * wpa_ie)
{
    return wrapper_get_wpa_ie_in_assoc(wpa_ie);
}

#define WL_ID_WIFI_MAIN_LOOP "wifi_main_loop"

static void wifi_driver_main_loop(void * argv)
{
    int ret;
    struct bus_message msg;

    /* Main Loop */
    while (1)
    {
        ret = os_queue_recv(&wm_wifi.io_events, &msg, OS_WAIT_FOREVER);
        if (ret == WM_SUCCESS)
        {
            // wakelock_get(WL_ID_WIFI_MAIN_LOOP);

            if (msg.event == MLAN_TYPE_EVENT)
            {
                wifi_handle_fw_event(&msg);
                /*
                 * Free the buffer after the event is
                 * handled.
                 */
                wifi_free_eventbuf(msg.data);
            }
            else if (msg.event == MLAN_TYPE_CMD)
            {
                wifi_process_cmd_response((HostCmd_DS_COMMAND *) ((uint8_t *) msg.data + INTF_HEADER_LEN));
                wifi_update_last_cmd_sent_ms();
                wifi_put_command_resp_sem();
            }

            // wakelock_put(WL_ID_WIFI_MAIN_LOOP);
        }
    }
}

#define WL_ID_WIFI_CORE_INPUT "wifi_core_input"
/**
 * This function should be called when a packet is ready to be read
 * from the interface.
 */
static void wifi_core_input()
{
    int sta;

    for (;;)
    {
        sta = os_enter_critical_section();
        /* Allow interrupt handler to deliver us a packet */
        g_txrx_flag = true;
        //		SDIOC_IntMask(SDIOC_INT_CDINT, UNMASK);
        //		SDIOC_IntSigMask(SDIOC_INT_CDINT, UNMASK);
        sdio_enable_interrupt();

        os_exit_critical_section(sta);

        /* Wait till we receive a packet from SDIO */
        os_event_notify_get(OS_WAIT_FOREVER);
        // wakelock_get(WL_ID_WIFI_CORE_INPUT);

        /* Protect the SDIO from other parallel activities */
        wifi_sdio_lock();

        wlan_process_int_status(mlan_adap);

        wifi_sdio_unlock();
        // wakelock_put(WL_ID_WIFI_CORE_INPUT);
    } /* for ;; */
}

static void wifi_core_deinit();
static int wifi_low_level_input(const uint8_t interface, const uint8_t * buffer, const uint16_t len);

static int wifi_core_init(void)
{
    int ret;

    if (wifi_core_init_done)
        return WM_SUCCESS;

    ret = os_mutex_create(&wm_wifi.command_lock, "command lock", OS_MUTEX_INHERIT);

    if (ret != WM_SUCCESS)
    {
        wifi_e("Create command_lock failed");
        goto fail;
    }

    ret = os_semaphore_create(&wm_wifi.command_resp_sem, "command resp sem");

    if (ret != WM_SUCCESS)
    {
        wifi_e("Create command resp sem failed");
        goto fail;
    }

    ret = os_mutex_create(&wm_wifi.mcastf_mutex, "mcastf-mutex", OS_MUTEX_INHERIT);
    if (ret != WM_SUCCESS)
    {
        wifi_e("Create mcastf mutex failed");
        goto fail;
    }

    /*
     * Take the cmd resp lock immediately so that we can later block on
     * it.
     */
    wifi_get_command_resp_sem(OS_WAIT_FOREVER);
    wm_wifi.io_events_queue_data = g_io_events_queue_data;

    ret = os_queue_create(&wm_wifi.io_events, "io-events", sizeof(struct bus_message), &wm_wifi.io_events_queue_data);
    if (ret != WM_SUCCESS)
    {
        wifi_e("Create io events queue failed");
        goto fail;
    }

    ret = bus_register_event_queue(&wm_wifi.io_events);
    if (ret != WM_SUCCESS)
    {
        wifi_e("Register io events queue failed");
        goto fail;
    }

    ret = os_thread_create(&wm_wifi.wm_wifi_main_thread, "wifi_driver", wifi_driver_main_loop, NULL, &wifi_drv_stack, OS_PRIO_3);
    if (ret != WM_SUCCESS)
    {
        wifi_e("Create wifi driver thread failed");
        goto fail;
    }

    ret = bus_register_data_input_function(&wifi_low_level_input);
    if (ret != WM_SUCCESS)
    {
        wifi_e("Register wifi low level input failed");
        goto fail;
    }

    ret = os_thread_create(&wm_wifi.wm_wifi_core_thread, "stack_dispatcher", (void (*)(os_thread_arg_t)) wifi_core_input, NULL,
                           &wifi_core_stack, OS_PRIO_1);

    if (ret != WM_SUCCESS)
    {
        wifi_e("Create stack dispatcher thread failed");
        goto fail;
    }

    wifi_core_thread    = wm_wifi.wm_wifi_core_thread;
    wifi_core_init_done = 1;

    return WM_SUCCESS;

fail:

    wifi_core_deinit();

    return -WM_FAIL;
}

static void wifi_core_deinit()
{
    wifi_core_init_done = 0;

    bus_deregister_event_queue();
    bus_deregister_data_input_function();

    if (wm_wifi.io_events)
    {
        os_queue_delete(&wm_wifi.io_events);
        wm_wifi.io_events = NULL;
    }
    if (wm_wifi.mcastf_mutex)
    {
        os_mutex_delete(&wm_wifi.mcastf_mutex);
        wm_wifi.mcastf_mutex = NULL;
    }
    if (wm_wifi.command_resp_sem)
    {
        os_semaphore_delete(&wm_wifi.command_resp_sem);
        wm_wifi.command_resp_sem = NULL;
    }
    if (wm_wifi.command_lock)
    {
        os_mutex_delete(&wm_wifi.command_lock);
        wm_wifi.command_lock = NULL;
    }
    if (wm_wifi.wm_wifi_main_thread)
    {
        os_thread_delete(&wm_wifi.wm_wifi_main_thread);
        wm_wifi.wm_wifi_main_thread = NULL;
    }
    if (wm_wifi.wm_wifi_core_thread)
    {
        os_thread_delete(&wm_wifi.wm_wifi_core_thread);
        wm_wifi.wm_wifi_core_thread = NULL;
        wifi_core_thread            = NULL;
    }
}

int wifi_init(const uint8_t * fw_ram_start_addr, const size_t size)
{
    if (wifi_init_done)
        return WM_SUCCESS;

    int ret = sd_wifi_init(WLAN_TYPE_NORMAL, WLAN_FW_IN_RAM, fw_ram_start_addr, size);
    if (ret)
    {
        wifi_e("sd_wifi_init failed. status code %d", ret);
        switch (ret)
        {
        case MLAN_CARD_CMD_TIMEOUT:
        case MLAN_CARD_NOT_DETECTED:
            ret = -WIFI_ERROR_CARD_NOT_DETECTED;
            break;
        case MLAN_STATUS_FW_DNLD_FAILED:
            ret = -WIFI_ERROR_FW_DNLD_FAILED;
            break;
        case MLAN_STATUS_FW_NOT_DETECTED:
            ret = -WIFI_ERROR_FW_NOT_DETECTED;
            break;
        case MLAN_STATUS_FW_NOT_READY:
            ret = -WIFI_ERROR_FW_NOT_READY;
            break;
        }
        return ret;
    }

    ret = wifi_core_init();
    if (ret)
    {
        wifi_e("wifi_core_init failed. status code %d", ret);
    }

    if (ret == WM_SUCCESS)
        wifi_init_done = 1;

    return ret;
}

void wifi_deinit(void)
{
    wifi_init_done = 0;

    sd_wifi_deinit();
    wifi_core_deinit();
}

void wifi_set_packet_retry_count(const int count)
{
    retry_attempts = count;
}

void wifi_sta_ampdu_tx_enable(void)
{
    sta_ampdu_tx_enable = true;
}

void wifi_sta_ampdu_tx_disable(void)
{
    sta_ampdu_tx_enable = false;
}

void wifi_sta_ampdu_rx_enable(void)
{
    sta_ampdu_rx_enable = true;
}

void wifi_sta_ampdu_rx_disable(void)
{
    sta_ampdu_rx_enable = false;
}

int wifi_register_data_input_callback(void (*data_intput_callback)(const uint8_t interface, const uint8_t * buffer,
                                                                   const uint16_t len))
{
    if (wm_wifi.data_intput_callback)
        return -WM_FAIL;

    wm_wifi.data_intput_callback = data_intput_callback;

    return WM_SUCCESS;
}

void wifi_deregister_data_input_callback()
{
    wm_wifi.data_intput_callback = NULL;
}

int wifi_register_amsdu_data_input_callback(void (*amsdu_data_intput_callback)(uint8_t interface, uint8_t * buffer, uint16_t len))
{
    if (wm_wifi.amsdu_data_intput_callback)
        return -WM_FAIL;

    wm_wifi.amsdu_data_intput_callback = amsdu_data_intput_callback;

    return WM_SUCCESS;
}

void wifi_deregister_amsdu_data_input_callback()
{
    wm_wifi.amsdu_data_intput_callback = NULL;
}

int wifi_register_deliver_packet_above_callback(void (*deliver_packet_above_callback)(uint8_t interface, void * lwip_pbuf))
{
    if (wm_wifi.deliver_packet_above_callback)
        return -WM_FAIL;

    wm_wifi.deliver_packet_above_callback = deliver_packet_above_callback;

    return WM_SUCCESS;
}

void wifi_deregister_deliver_packet_above_callback()
{
    wm_wifi.deliver_packet_above_callback = NULL;
}

int wifi_register_wrapper_net_is_ip_or_ipv6_callback(bool (*wrapper_net_is_ip_or_ipv6_callback)(const t_u8 * buffer))
{
    if (wm_wifi.wrapper_net_is_ip_or_ipv6_callback)
        return -WM_FAIL;

    wm_wifi.wrapper_net_is_ip_or_ipv6_callback = wrapper_net_is_ip_or_ipv6_callback;

    return WM_SUCCESS;
}

void wifi_deregister_wrapper_net_is_ip_or_ipv6_callback()
{
    wm_wifi.wrapper_net_is_ip_or_ipv6_callback = NULL;
}

static int wifi_low_level_input(const uint8_t interface, const uint8_t * buffer, const uint16_t len)
{
    if (wm_wifi.data_intput_callback)
    {
        wm_wifi.data_intput_callback(interface, buffer, len);
        return WM_SUCCESS;
    }

    return -WM_FAIL;
}

#define ERR_INPROGRESS -5

#define WL_ID_LL_OUTPUT "wifi_low_level_output"

int wifi_low_level_output(const uint8_t interface, const uint8_t * buffer, const uint16_t len)
{
    int i, ret, retry = retry_attempts;
    unsigned long pkt_len;

    // wakelock_get(WL_ID_LL_OUTPUT);
    ret = os_rwlock_read_lock(&ps_rwlock, MAX_WAIT_TIME);
    if (ret != WM_SUCCESS)
    {
        // wakelock_put(WL_ID_LL_OUTPUT);
        return ERR_INPROGRESS;
    }

    wifi_sdio_lock();

    pkt_len = sizeof(TxPD) + INTF_HEADER_LEN;

retry_xmit:
    i = wlan_xmit_pkt(pkt_len + len, interface);
    wifi_sdio_unlock();
    if (i != MLAN_STATUS_SUCCESS)
    {
        if (i == MLAN_STATUS_FAILURE)
        {
            ret = -WM_E_NOMEM;
            goto exit_fn;
        }
        else if (i == MLAN_STATUS_RESOURCE)
        {
            if (!retry)
            {
                ret = -WM_E_BUSY;
                goto exit_fn;
            }
            else
            {
                retry--;
                /* Allow the other thread to run and hence
                 * update the write bitmap so that pkt
                 * can be sent to FW */
                os_thread_sleep(1);
                wifi_sdio_lock();
                goto retry_xmit;
            }
        }
    }

    if (interface == BSS_TYPE_STA && sta_ampdu_tx_enable)
    {
        if (wm_wifi.wrapper_net_is_ip_or_ipv6_callback(buffer))
            wrapper_wlan_sta_ampdu_enable();
    }

    if (interface == BSS_TYPE_UAP)
    {
        if (wm_wifi.wrapper_net_is_ip_or_ipv6_callback(buffer))
            wrapper_wlan_upa_ampdu_enable((uint8_t *) buffer);
    }

    ret = WM_SUCCESS;

exit_fn:
    os_rwlock_read_unlock(&ps_rwlock);
    wifi_set_xfer_pending(false);
    // wakelock_put(WL_ID_LL_OUTPUT);

    return ret;
}

uint8_t * wifi_get_outbuf(uint32_t * outbuf_len)
{
    return wifi_get_sdio_outbuf(outbuf_len);
}
