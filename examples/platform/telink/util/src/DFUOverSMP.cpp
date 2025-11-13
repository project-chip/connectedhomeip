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

#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>

#include <lib/support/logging/CHIPLogging.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::DeviceLayer;
namespace {
#ifndef CONFIG_ZEPHYR_VERSION_3_3
enum mgmt_cb_return UploadProgressHandler(uint32_t event, enum mgmt_cb_return prev_status, int32_t * rc, uint16_t * group,
                                          bool * abort_more, void * data, size_t data_size)
#else
int UploadProgressHandler(uint32_t event, int32_t rc, bool * abort_more, void * data, size_t data_size)
#endif

{
    const img_mgmt_upload_check & imgData = *static_cast<img_mgmt_upload_check *>(data);

    LOG_INF("[DFU] DFU over SMP progress: %u/%u B of image %u", static_cast<unsigned>(imgData.req->off),
            static_cast<unsigned>(imgData.action->size), static_cast<unsigned>(imgData.req->image));

#ifndef CONFIG_ZEPHYR_VERSION_3_3
    return MGMT_CB_OK;
#else
    return MGMT_ERR_EOK;
#endif
}

#ifndef CONFIG_ZEPHYR_VERSION_3_3
enum mgmt_cb_return UploadConfirmHandler(uint32_t event, enum mgmt_cb_return prev_status, int32_t * rc, uint16_t * group,
                                         bool * abort_more, void * data, size_t data_size)
#else
int32_t UploadConfirmHandler(uint32_t event, int32_t rc, bool * abort_more, void * data, size_t data_size)
#endif
{
    const img_mgmt_upload_check & imgData = *static_cast<img_mgmt_upload_check *>(data);
    IgnoreUnusedVariable(imgData);

    LOG_INF("[DFU] Image Uploaded!");
    if (GetDFUOverSMP().ProcessImageFooter() != CHIP_NO_ERROR)
    {
        LOG_ERR("[DFU] Image footer verification failed!");
    }
#ifndef CONFIG_ZEPHYR_VERSION_3_3
    return MGMT_CB_OK;
#else
    return MGMT_ERR_EOK;
#endif
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

void DFUOverSMP::SetFailCallback(verificationFailCallback cb)
{
    failCallback = cb;
}

void DFUOverSMP::Init()
{
    mgmt_callback_register(&sUploadProgressCallback);
    mgmt_callback_register(&sUploadFinishCallback);
}

CHIP_ERROR DFUOverSMP::GetDFUImageFooterOffset(unsigned int & footer_offset,
                                               const struct flash_area * fa) // check if reference is working correctly
{
    struct image_header hdr;
    struct image_tlv_info tlv_info;
    int rc;

    rc = flash_area_read(fa, 0, &hdr, sizeof(hdr));
    if (rc)
    {
        LOG_ERR("[DFU] Failed to read image header\n");
        return CHIP_ERROR_READ_FAILED;
    }

    if (hdr.ih_magic != IMAGE_MAGIC)
    {
        LOG_ERR("[DFU] Not a valid MCUboot image (magic=0x%x)\n", hdr.ih_magic);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Calculate offset to the image footer
    unsigned int tlv_off = hdr.ih_hdr_size + hdr.ih_img_size;
    flash_area_read(fa, tlv_off, &tlv_info, sizeof(tlv_info));
    footer_offset = tlv_off + tlv_info.it_tlv_tot;

    LOG_DBG("[DFU] TLV offset: 0x%08x", (unsigned) tlv_info.it_tlv_tot);
    LOG_DBG("[DFU] Partition offset: 0x%08x", (unsigned) fa->fa_off);
    LOG_DBG("[DFU] Offset to footer 0x%08x", (unsigned) footer_offset);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DFUOverSMP::GetDFUImageFooter(OTAImageHeader & footer, const struct flash_area * fa)
{
    OTAImageHeaderParser mHeaderParser;
    unsigned int footer_offset = 0;
    int rc;

    mHeaderParser.Init();

    if (GetDFUImageFooterOffset(footer_offset, fa) != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    unsigned char footer_size = 0;
    rc                        = flash_area_read(fa, footer_offset, &footer_size, sizeof(footer_size));
    if (rc)
    {
        LOG_ERR("[DFU] Failed to read DFU footer size\n");
        return CHIP_ERROR_READ_FAILED;
    }
    footer_offset += sizeof(footer_size);

    LOG_DBG("[DFU] Footer size is %u", footer_size);
    std::vector<uint8_t> buffer(footer_size);
    rc = flash_area_read(fa, footer_offset, &buffer[0], footer_size);
    if (rc)
    {
        LOG_ERR("[DFU] Failed to read DFU footer");
        return CHIP_ERROR_READ_FAILED;
    }
    chip::ByteSpan footer_raw(buffer.data(), buffer.size());
    mHeaderParser.AccumulateAndDecode(footer_raw, footer);
    mHeaderParser.Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR DFUOverSMP::CheckDFUImageFooter(OTAImageHeader * imageHeader)
{
    uint16_t vendorId        = 0;
    uint16_t productId       = 0;
    uint32_t softwareVersion = 0;
    char newSoftwareVersionString[VERSION_STRING_MAX_LENGTH];
    char activeSoftwareVersionString[VERSION_STRING_MAX_LENGTH];

    if (GetDeviceInstanceInfoProvider()->GetVendorId(vendorId) != CHIP_NO_ERROR)
    {
        LOG_ERR("[DFU] Failed to retrieve local Vendor ID for DFU validation");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (GetDeviceInstanceInfoProvider()->GetProductId(productId) != CHIP_NO_ERROR)
    {
        LOG_ERR("[DFU] Failed to retrieve local Product ID for DFU validation");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (ConfigurationMgr().GetSoftwareVersion(softwareVersion) != CHIP_NO_ERROR)
    {
        LOG_ERR("[DFU] Failed to retrieve local Software version for DFU validation");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    uint8_t checkResult = 0;
    if (imageHeader->mVendorId != vendorId)
    {
        LOG_ERR("[DFU] The argument is invalid, mVendorId: 0x%x - \
            \t vendorId : 0x%x",
                imageHeader->mVendorId, vendorId);
        checkResult |= VerificationFailReason::WRONG_VENDOR_ID;
    }
    if (imageHeader->mProductId != productId)
    {
        LOG_ERR("[DFU] The argument is invalid, mProductId: 0x%x - \
                        \t productId : 0x%x",
                imageHeader->mProductId, productId);
        checkResult |= VerificationFailReason::WRONG_PRODUCT_ID;
    }
    if (imageHeader->mSoftwareVersion <= softwareVersion)
    {
        ConfigurationMgr().GetSoftwareVersionString(activeSoftwareVersionString, VERSION_STRING_MAX_LENGTH);
        memcpy(newSoftwareVersionString, imageHeader->mSoftwareVersionString.data(), imageHeader->mSoftwareVersionString.size());
        LOG_ERR("[DFU] Incorrect version of the update image!\nActive firmware version:\t %s\nUpdate version:\t %s",
                newSoftwareVersionString, activeSoftwareVersionString);
        checkResult |= VerificationFailReason::WRONG_VERSION;
    }
    if (checkResult != VerificationFailReason::NO_FAIL)
    {
        failCallback(static_cast<VerificationFailReason>(checkResult));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        LOG_INF("[DFU] Image verified successfully!");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DFUOverSMP::ProcessImageFooter()
{
    OTAImageHeader imageFooter;
    const struct flash_area * fa;
    int rc;

    // Open partition with downloaded image
    rc = flash_area_open(FIXED_PARTITION_ID(slot1_partition), &fa);
    if (rc)
    {
        LOG_ERR("[DFU] Failed to open image area\n");
        return CHIP_ERROR_OPEN_FAILED;
    }

    if (GetDFUImageFooter(imageFooter, fa) != CHIP_NO_ERROR)
    {
        flash_area_close(fa);
        return CHIP_ERROR_READ_FAILED;
    }
    if (CheckDFUImageFooter(&imageFooter) != CHIP_NO_ERROR)
    {
        flash_area_close(fa);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    flash_area_close(fa);
    return CHIP_NO_ERROR;
}
