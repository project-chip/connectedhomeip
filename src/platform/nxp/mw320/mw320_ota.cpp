/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include "mw320_ota.h"
#include "fsl_debug_console.h"
#include "task.h"
#if (MW320_OTA_TEST == 1)
#include "gpio_fw.h"
unsigned char * mw320_ota_mcufw = gpio_fw_bin;
unsigned int mw320_ota_len      = gpio_fw_bin_len;
#endif // MW320_OTA_TEST
extern "C" {
#include "crc32.h"
#include "mflash_drv.h"
#include "partition.h"
typedef int (*data_fetch)(unsigned char * buf, size_t len);
struct partition_entry * part_get_passive_partition(struct partition_entry * p1, struct partition_entry * p2);
}

#define FIRMWARE_UPDATE_BUF_SIZE 256 // Max buffer size can be written in 1 transaction
#define SEC_FW_SIG_LEN 284

#define FW_MAGIC_STR (('M' << 0) | ('R' << 8) | ('V' << 16) | ('L' << 24))
#define FW_MAGIC_SIG ((0x7BUL << 0) | (0xF1UL << 8) | (0x9CUL << 16) | (0x2EUL << 24))
#define SEC_FW_MAGIC_SIG (('S' << 0) | ('B' << 8) | ('F' << 16) | ('H' << 24))
/*
 * Firmware magic signature
 *
 * First word is the string "MRVL" and is endian invariant.
 * Second word = magic value 0x2e9cf17b.
 * Third word = time stamp (seconds since 00:00:00 UTC, January 1, 1970).
 */
struct img_hdr
{
    uint32_t magic_str;
    uint32_t magic_sig;
    uint32_t time;
    uint32_t seg_cnt;
    uint32_t entry;
};

struct seg_hdr
{
    uint32_t type;
    uint32_t offset;
    uint32_t len;
    uint32_t laddr;
    uint32_t crc;
};

struct tlv_hdr
{
    uint32_t magic;
    uint32_t len;
    uint32_t crc;
};
typedef struct tlv_hdr img_sec_hdr;

static uint32_t calculate_image_crc(uint32_t flash_addr, uint32_t size)
{
    int32_t result;
    uint32_t buf[32];
    uint32_t addr = flash_addr;
    uint32_t crc  = 0U;

    PRINTF("Calculate image CRC\r\n");
    for (addr = flash_addr; addr < flash_addr + size - sizeof(buf); addr += sizeof(buf))
    {
        result = mflash_drv_read(addr, buf, sizeof(buf));
        if (result != WM_SUCCESS)
        {
            assert(false);
        }
        crc = soft_crc32(buf, sizeof(buf), crc);
    }

    /* Remaining data */
    result = mflash_drv_read(addr, buf, flash_addr + size - addr);
    if (result != WM_SUCCESS)
    {
        assert(false);
    }
    crc = soft_crc32(buf, flash_addr + size - addr, crc);

    return crc;
}

static struct partition_entry * get_passive_firmware(void)
{
    short history = 0;
    struct partition_entry *f1, *f2;

    f1 = part_get_layout_by_id(FC_COMP_FW, &history);
    f2 = part_get_layout_by_id(FC_COMP_FW, &history);

    if (f1 == NULL || f2 == NULL)
    {
        PRINTF("Unable to retrieve flash layout\r\n");
        return NULL;
    }

    return (struct partition_entry *) part_get_passive_partition(f1, f2);
}

static int fw_update_begin(struct partition_entry * p)
{
    /* Erase the passive partition before updating it */
    if (mflash_drv_erase(p->start, p->size))
    {
        PRINTF("Failed to erase partition\r\n");
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

static int fw_update_data(struct partition_entry * p, const char * data, size_t datalen)
{
    int32_t result;

    result = mflash_drv_write(p->start, (uint32_t *) data, datalen);
    if (result != WM_SUCCESS)
    {
        PRINTF("Failed to write partition\r\n");
        return -WM_FAIL;
    }

    p->start += datalen;

    return WM_SUCCESS;
}

static int verify_load_firmware(uint32_t flash_addr, uint32_t size)
{
    struct img_hdr ih;
    struct seg_hdr sh;
    int32_t result;

    if (size < (sizeof(ih) + sizeof(sh)))
    {
        return -WM_FAIL;
    }

    result = mflash_drv_read(flash_addr, (uint32_t *) &ih, sizeof(ih));
    if (result != WM_SUCCESS)
    {
        assert(false);
    }

    /* MCUXpresso SDK image has only 1 segment */
    if ((ih.magic_str != FW_MAGIC_STR) || (ih.magic_sig != FW_MAGIC_SIG) || ih.seg_cnt != 1U)
    {
        return -WM_FAIL;
    }

    result = mflash_drv_read(flash_addr + sizeof(ih), (uint32_t *) &sh, sizeof(sh));
    if (result != kStatus_Success)
    {
        assert(false);
    }

    /* Image size should just cover segment end. */
    if (sh.len + sh.offset != size)
    {
        return -WM_FAIL;
    }

    if (calculate_image_crc(flash_addr + sh.offset, sh.len) != sh.crc)
    {
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

static void mw320_dev_reset(unsigned int delay_ms)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    NVIC_SystemReset();
    return;
}
// ============================================================================
typedef struct _fw_update_param
{
    // Accumulated size
    uint32_t pump_size; // How many bytes has been written
    uint32_t p_start;   // Start address of the partition
    img_sec_hdr ish;    // ish from downloaded firmware buffer

    // int hdr_size;
    // Pointer to passive firmware
    struct partition_entry part_arg;

    // Callback function to write the data
    // data_fetch data_fetch_cb;

} fw_update_param_t;
static fw_update_param_t g_fmup_param;

/*
        Initialize the ota function by getting an id
*/
fw_update_id_t mw320_fw_update_begin(void)
{
    int error;
    struct partition_entry * p;

    if (g_fmup_param.pump_size != 0)
    {
        PRINTF("Warning, wt_addr is not 0. last fw_abort may be missing \r\n");
    }
    memset(&g_fmup_param, 0, sizeof(g_fmup_param));

    // ----------------------------------------------------
    // Initialize the firmware update parameters
    p = get_passive_firmware();
    memcpy(&g_fmup_param.part_arg, p, sizeof(struct partition_entry));
    g_fmup_param.p_start = p->start;

    // ----------------------------------------------------
    // Erase the partition
    error = fw_update_begin(&g_fmup_param.part_arg);
    if (error != WM_SUCCESS)
    {
        return NULL;
    }

    return &g_fmup_param;
}

/*
        Write the received chunk to the partition
*/
int mw320_fw_update_wrblock(fw_update_id_t fwup_id, unsigned char * pblock, unsigned int blksize)
{
    fw_update_param_t * pfwup_parm = (fw_update_param_t *) fwup_id;
    unsigned int wr_size           = 0; // How many bytes has been written
    unsigned char * buf            = pblock;
    int error;

    if (pfwup_parm != &g_fmup_param)
    {
        PRINTF("Incorrect fw update id \r\n");
        return -WM_FAIL;
    }
    // Check if the size is available
    if ((pfwup_parm->pump_size + blksize) > pfwup_parm->part_arg.size)
    {
        PRINTF("[Error], Firmware size is too big. (limit, current, new)=(%u, %u, %u) \r\n", pfwup_parm->part_arg.size,
               pfwup_parm->pump_size, blksize);
        // purge_stream_bytes(pfwup_parm->data_fetch_cb, blksize, pblock, FIRMWARE_UPDATE_BUF_SIZE);
        return -WM_FAIL;
    }

    // If it's the 1st block
    if (pfwup_parm->p_start == pfwup_parm->part_arg.start)
    {
        memcpy(&pfwup_parm->ish, buf, sizeof(img_sec_hdr));
    }
    /*
            Write the data chunk to firmware partition
    */
    while (wr_size < blksize)
    {
        unsigned int blk_left = blksize - wr_size;
        unsigned int chk_size = (blk_left < FIRMWARE_UPDATE_BUF_SIZE) ? blk_left : FIRMWARE_UPDATE_BUF_SIZE;

        error = fw_update_data(&pfwup_parm->part_arg, (char *) buf, chk_size);
        if (error)
        {
            PRINTF("Failed to write firmware data \r\n");
            return -WM_FAIL;
        }
        wr_size += chk_size;
        pfwup_parm->pump_size += chk_size;
        buf += chk_size;
    }

    return WM_SUCCESS;
}

/*
        Finalize the firmware update.
                - rst_delay_sec: Delay to reset the device
                        >= 0: Will reset the device after rst_delay_sec seconds
                        <0: No reset
*/
int mw320_fw_update_end(fw_update_id_t fwup_id, int rst_delay_sec)
{
    fw_update_param_t * pfwup_parm = (fw_update_param_t *) fwup_id;
    int error;

    if (pfwup_parm != &g_fmup_param)
    {
        PRINTF("Incorrect fw update id \r\n");
        return -WM_FAIL;
    }

    // Validate the firmware data in flash
    PRINTF("Firmware verification start ... filesize = %d\r\n", pfwup_parm->pump_size);
    if (pfwup_parm->ish.magic == SEC_FW_MAGIC_SIG)
    {

        error = verify_load_firmware((pfwup_parm->p_start + SEC_FW_SIG_LEN), (pfwup_parm->pump_size - SEC_FW_SIG_LEN));
    }
    else
    {
        error = verify_load_firmware(pfwup_parm->p_start, pfwup_parm->pump_size);
    }

    if (error != WM_SUCCESS)
    {
        PRINTF("FW verification fail! Keep the same firmware \r\n");
        return -WM_FAIL;
    }
    PRINTF("FW verification pass, switch the firmware partition \r\n");
    // Restore the start
    pfwup_parm->part_arg.start = pfwup_parm->p_start;

    // Switch the active partition to the new one
    part_set_active_partition(&pfwup_parm->part_arg);

    // Reset the device if requested
    if (rst_delay_sec > 0)
    {
        mw320_dev_reset(rst_delay_sec * 1000);
    }

    return WM_SUCCESS;
}

/*
        Abort the ota function
*/
int mw320_fw_update_abort(fw_update_id_t fwup_id)
{
    fw_update_param_t * pfwup_parm = (fw_update_param_t *) fwup_id;
    if (pfwup_parm != &g_fmup_param)
    {
        PRINTF("Incorrect fw update id \r\n");
        return -WM_FAIL;
    }
    memcpy(&g_fmup_param, 0, sizeof(g_fmup_param));
    return WM_SUCCESS;
}

#if (MW320_OTA_TEST == 1)
// Module Test function
// This is also an example that how to use the interface
void mw320_fw_update_test(void)
{
    unsigned int wr_size = 0;
    int error;

    fw_update_id_t fwupid = mw320_fw_update_begin();
    while (wr_size < mw320_ota_len)
    {
        unsigned int chk_size = ((mw320_ota_len - wr_size) < 1024) ? (mw320_ota_len - wr_size) : 1024;
        PRINTF("===> Wt (%d, %d)\r\n", wr_size, chk_size);
        error = mw320_fw_update_wrblock(fwupid, &mw320_ota_mcufw[wr_size], chk_size);
        if (error != WM_SUCCESS)
        {
            break;
        }
        wr_size += chk_size;
    }
    if (error == WM_SUCCESS)
    {
        mw320_fw_update_end(fwupid, -1);
    }

    return;
}
#endif // MW320_OTA_TEST
