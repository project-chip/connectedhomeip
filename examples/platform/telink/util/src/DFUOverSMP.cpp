/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "DFUOverSMP.h"

#if !defined(CONFIG_MCUMGR_TRANSPORT_BT) || !defined(CONFIG_MCUMGR_GRP_IMG) || !defined(CONFIG_MCUMGR_GRP_OS)
#error "DFUOverSMP requires MCUMGR module configs enabled"
#endif

#include "OTAUtil.h"

#include <lib/core/OTAImageHeader.h>
#include <platform/CHIPDeviceLayer.h>

#include <lib/support/logging/CHIPLogging.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::DeviceLayer;
namespace {
enum mgmt_cb_return UploadProgressHandler(uint32_t event, enum mgmt_cb_return prev_status, int32_t * rc, uint16_t * group,
                                         bool * abort_more, void * data, size_t data_size)
{
    const img_mgmt_upload_check & imgData = *static_cast<img_mgmt_upload_check *>(data);

    LOG_INF("[DFU] DFU over SMP progress: %u/%u B of image %u", static_cast<unsigned>(imgData.req->off),
            static_cast<unsigned>(imgData.action->size), static_cast<unsigned>(imgData.req->image));

    return MGMT_CB_OK;
}

void GetOTAHeaderFromImage()
{
    const struct flash_area *fa;
    struct image_header hdr;
    struct image_tlv_info tlv_info;
    int rc;

    rc = flash_area_open(FIXED_PARTITION_ID(slot1_partition), &fa);
    if (rc) {
        LOG_INF("Failed to open image area\n");
        return;
    }

    rc = flash_area_read(fa, 0, &hdr, sizeof(hdr));
    if (rc) {
        LOG_INF("Failed to read header\n");
        return;
    }

    if (hdr.ih_magic != IMAGE_MAGIC) {
        LOG_INF("Not a valid MCUboot image (magic=0x%x)\n", hdr.ih_magic);
        return;
    }

    off_t tlv_off = hdr.ih_hdr_size + hdr.ih_img_size;
    flash_area_read(fa, tlv_off, &tlv_info, sizeof(tlv_info));
    off_t appended_offset = tlv_off + tlv_info.it_tlv_tot;
    // LOG_INF("TLV offset: 0x%08x", (unsigned)tlv_info.it_tlv_tot);
    // LOG_INF("Partition offset: 0x%08x", (unsigned)fa->fa_off);
    // LOG_INF("appended_offset: 0x%08x", (unsigned)appended_offset);
    LOG_INF("Appended data starts at flash offset 0x%08x", // debug
           (unsigned)(fa->fa_off + appended_offset));
}

enum mgmt_cb_return UploadConfirmHandler(uint32_t event, enum mgmt_cb_return prev_status, int32_t * rc, uint16_t * group,
                                         bool * abort_more, void * data, size_t data_size)
{
    const img_mgmt_upload_check & imgData = *static_cast<img_mgmt_upload_check *>(data);
    IgnoreUnusedVariable(imgData);

    LOG_INF("[DFU] Image Uploaded!");

    GetOTAHeaderFromImage();

    return MGMT_CB_OK;
}

mgmt_callback sUploadProgressCallback = {
    .callback = UploadProgressHandler,
    .event_id = MGMT_EVT_OP_IMG_MGMT_DFU_CHUNK,
};

mgmt_callback sUploadFinishCallback = {
    .callback = UploadConfirmHandler,
    .event_id = MGMT_EVT_OP_IMG_MGMT_DFU_PENDING,
};

} // namespace

DFUOverSMP DFUOverSMP::sDFUOverSMP;

void DFUOverSMP::Init()
{
    mgmt_callback_register(&sUploadProgressCallback);
    mgmt_callback_register(&sUploadFinishCallback);
}
