/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "FactoryDataProvider.h"

#include "FactoryDataDecoder.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>
#include <platform/realtek/BEE/CHIPDevicePlatformConfig.h>

#if CONFIG_FACTORY_DATA
#define FACTORY_DATA_BUFFER_LEN (2560)
#endif

#define FACTORY_TEST 0

using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

#if FACTORY_TEST
void buf_dump(const char * title, uint8_t * buf, uint32_t data_len)
{
    const uint32_t bat_num = 8;
    uint32_t times         = data_len / bat_num;
    uint32_t residue       = data_len % bat_num;
    uint8_t * residue_buf  = buf + times * bat_num;

    ChipLogProgress(DeviceLayer, "buf_dump: data_len %d, times %d, residue %d", data_len, times, residue);
    ChipLogProgress(DeviceLayer, "buf_dump: buf is 0x%08x, residue_buf is 0x%08x\r\n", (uint32_t) buf, (uint32_t) residue_buf);

    ChipLogProgress(DeviceLayer, "@@@@@@@@@@@@@@@@@@@@@%s@@@@@@@@@@@@@@@@@@@@@@@@@@@", title);

    for (int32_t i = 0; i < times; i++)
    {
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",
                        buf[i * bat_num], buf[i * bat_num + 1], buf[i * bat_num + 2], buf[i * bat_num + 3], buf[i * bat_num + 4],
                        buf[i * bat_num + 5], buf[i * bat_num + 6], buf[i * bat_num + 7]);
    }

    switch (residue)
    {
    case 1:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x\r\n", residue_buf[0]);
        break;
    case 2:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1]);
        break;
    case 3:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1], residue_buf[2]);
        break;
    case 4:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1], residue_buf[2],
                        residue_buf[3]);
        break;
    case 5:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1],
                        residue_buf[2], residue_buf[3], residue_buf[4]);
        break;
    case 6:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1],
                        residue_buf[2], residue_buf[3], residue_buf[4], residue_buf[5]);
        break;
    case 7:
        ChipLogProgress(DeviceLayer, "buf_dump: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0],
                        residue_buf[1], residue_buf[2], residue_buf[3], residue_buf[4], residue_buf[5], residue_buf[6]);
        break;

    default:
        break;
    }

    ChipLogProgress(DeviceLayer, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
}
#endif

// TODO: This should be moved to a method of P256Keypair
CHIP_ERROR LoadKeypairFromRaw(ByteSpan private_key, ByteSpan public_key, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(private_key.size() + public_key.size()));
    memcpy(serialized_keypair.Bytes(), public_key.data(), public_key.size());
    memcpy(serialized_keypair.Bytes() + public_key.size(), private_key.data(), private_key.size());
    return keypair.Deserialize(serialized_keypair);
}

CHIP_ERROR FactoryDataProvider::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CONFIG_FACTORY_DATA
    uint8_t * buffer         = (uint8_t *) malloc(FACTORY_DATA_BUFFER_LEN);
    uint16_t factorydata_len = 0x5A5A;

    if (buffer)
    {
        FactoryDataDecoder decoder = FactoryDataDecoder::GetInstance();
        err                        = decoder.ReadFactoryData(buffer, FACTORY_DATA_BUFFER_LEN, &factorydata_len);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData factorydata_len %d!", factorydata_len);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ReadFactoryData failed!");
            free(buffer);
            return err;
        }
        err = decoder.DecodeFactoryData(buffer, &mFactoryData, factorydata_len);
#if FACTORY_TEST
        ChipLogDetail(DeviceLayer, "DecodeFactoryData passcode %d!", mFactoryData.cdata.passcode);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData discriminator %d!", mFactoryData.cdata.discriminator);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData vendor_id %d!", mFactoryData.dii.vendor_id);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData product_id %d!", mFactoryData.dii.product_id);
        buf_dump("DecodeFactoryData cd", mFactoryData.dac.cd.value, mFactoryData.dac.cd.len);
        buf_dump("DecodeFactoryData dac_cert", mFactoryData.dac.dac_cert.value, mFactoryData.dac.dac_cert.len);
        buf_dump("DecodeFactoryData dac_key", mFactoryData.dac.dac_key.value, mFactoryData.dac.dac_key.len);
        buf_dump("DecodeFactoryData pai_cert", mFactoryData.dac.pai_cert.value, mFactoryData.dac.pai_cert.len);
        ChipLogDetail(DeviceLayer, "DecodeFactoryData spake2_it %d", mFactoryData.cdata.spake2_it);
        buf_dump("DecodeFactoryData rd_id_uid", mFactoryData.dii.rd_id_uid.value, mFactoryData.dii.rd_id_uid.len);
        buf_dump("DecodeFactoryData vendor_name", mFactoryData.dii.vendor_name.value, mFactoryData.dii.vendor_name.len);
        buf_dump("DecodeFactoryData product_name", mFactoryData.dii.product_name.value, mFactoryData.dii.product_name.len);
        buf_dump("DecodeFactoryData serial_num", mFactoryData.dii.serial_num.value, mFactoryData.dii.serial_num.len);
#endif
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "DecodeFactoryData failed!");
            free(buffer);
            return err;
        }

        ChipLogProgress(DeviceLayer, "FactoryData of length: %d retrieved successfully", factorydata_len);

        free(buffer);
    }

#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & outBuffer)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CHIP_USE_DEVICE_CONFIG_CERTIFICATION_DECLARATION
    constexpr uint8_t kCdForAllExamples[] = CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION;
    err                                   = CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, outBuffer);
#elif CONFIG_FACTORY_DATA
    VerifyOrReturnError(outBuffer.size() >= mFactoryData.dac.cd.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dac.cd.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(outBuffer.data(), mFactoryData.dac.cd.value, mFactoryData.dac.cd.len);

    outBuffer.reduce_size(mFactoryData.dac.cd.len);
    err                      = CHIP_NO_ERROR;
#else
    const uint8_t kCdForAllExamples[] = {
        0x30, 0x82, 0x02, 0x19, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x82, 0x02, 0x0a, 0x30,
        0x82, 0x02, 0x06, 0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02,
        0x01, 0x30, 0x82, 0x01, 0x71, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x82, 0x01, 0x62,
        0x04, 0x82, 0x01, 0x5e, 0x15, 0x24, 0x00, 0x01, 0x25, 0x01, 0xf1, 0xff, 0x36, 0x02, 0x05, 0x00, 0x80, 0x05, 0x01, 0x80,
        0x05, 0x02, 0x80, 0x05, 0x03, 0x80, 0x05, 0x04, 0x80, 0x05, 0x05, 0x80, 0x05, 0x06, 0x80, 0x05, 0x07, 0x80, 0x05, 0x08,
        0x80, 0x05, 0x09, 0x80, 0x05, 0x0a, 0x80, 0x05, 0x0b, 0x80, 0x05, 0x0c, 0x80, 0x05, 0x0d, 0x80, 0x05, 0x0e, 0x80, 0x05,
        0x0f, 0x80, 0x05, 0x10, 0x80, 0x05, 0x11, 0x80, 0x05, 0x12, 0x80, 0x05, 0x13, 0x80, 0x05, 0x14, 0x80, 0x05, 0x15, 0x80,
        0x05, 0x16, 0x80, 0x05, 0x17, 0x80, 0x05, 0x18, 0x80, 0x05, 0x19, 0x80, 0x05, 0x1a, 0x80, 0x05, 0x1b, 0x80, 0x05, 0x1c,
        0x80, 0x05, 0x1d, 0x80, 0x05, 0x1e, 0x80, 0x05, 0x1f, 0x80, 0x05, 0x20, 0x80, 0x05, 0x21, 0x80, 0x05, 0x22, 0x80, 0x05,
        0x23, 0x80, 0x05, 0x24, 0x80, 0x05, 0x25, 0x80, 0x05, 0x26, 0x80, 0x05, 0x27, 0x80, 0x05, 0x28, 0x80, 0x05, 0x29, 0x80,
        0x05, 0x2a, 0x80, 0x05, 0x2b, 0x80, 0x05, 0x2c, 0x80, 0x05, 0x2d, 0x80, 0x05, 0x2e, 0x80, 0x05, 0x2f, 0x80, 0x05, 0x30,
        0x80, 0x05, 0x31, 0x80, 0x05, 0x32, 0x80, 0x05, 0x33, 0x80, 0x05, 0x34, 0x80, 0x05, 0x35, 0x80, 0x05, 0x36, 0x80, 0x05,
        0x37, 0x80, 0x05, 0x38, 0x80, 0x05, 0x39, 0x80, 0x05, 0x3a, 0x80, 0x05, 0x3b, 0x80, 0x05, 0x3c, 0x80, 0x05, 0x3d, 0x80,
        0x05, 0x3e, 0x80, 0x05, 0x3f, 0x80, 0x05, 0x40, 0x80, 0x05, 0x41, 0x80, 0x05, 0x42, 0x80, 0x05, 0x43, 0x80, 0x05, 0x44,
        0x80, 0x05, 0x45, 0x80, 0x05, 0x46, 0x80, 0x05, 0x47, 0x80, 0x05, 0x48, 0x80, 0x05, 0x49, 0x80, 0x05, 0x4a, 0x80, 0x05,
        0x4b, 0x80, 0x05, 0x4c, 0x80, 0x05, 0x4d, 0x80, 0x05, 0x4e, 0x80, 0x05, 0x4f, 0x80, 0x05, 0x50, 0x80, 0x05, 0x51, 0x80,
        0x05, 0x52, 0x80, 0x05, 0x53, 0x80, 0x05, 0x54, 0x80, 0x05, 0x55, 0x80, 0x05, 0x56, 0x80, 0x05, 0x57, 0x80, 0x05, 0x58,
        0x80, 0x05, 0x59, 0x80, 0x05, 0x5a, 0x80, 0x05, 0x5b, 0x80, 0x05, 0x5c, 0x80, 0x05, 0x5d, 0x80, 0x05, 0x5e, 0x80, 0x05,
        0x5f, 0x80, 0x05, 0x60, 0x80, 0x05, 0x61, 0x80, 0x05, 0x62, 0x80, 0x05, 0x63, 0x80, 0x18, 0x24, 0x03, 0x16, 0x2c, 0x04,
        0x13, 0x5a, 0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x32, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x33, 0x2d, 0x32, 0x34,
        0x24, 0x05, 0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18, 0x31, 0x7d, 0x30, 0x7b, 0x02, 0x01,
        0x03, 0x80, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04,
        0xf3, 0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08,
        0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x47, 0x30, 0x45, 0x02, 0x20, 0x24, 0xe5, 0xd1, 0xf4, 0x7a, 0x7d,
        0x7b, 0x0d, 0x20, 0x6a, 0x26, 0xef, 0x69, 0x9b, 0x7c, 0x97, 0x57, 0xb7, 0x2d, 0x46, 0x90, 0x89, 0xde, 0x31, 0x92, 0xe6,
        0x78, 0xc7, 0x45, 0xe7, 0xf6, 0x0c, 0x02, 0x21, 0x00, 0xf8, 0xaa, 0x2f, 0xa7, 0x11, 0xfc, 0xb7, 0x9b, 0x97, 0xe3, 0x97,
        0xce, 0xda, 0x66, 0x7b, 0xae, 0x46, 0x4e, 0x2b, 0xd3, 0xff, 0xdf, 0xc3, 0xcc, 0xed, 0x7a, 0xa8, 0xca, 0x5f, 0x4c, 0x1a,
        0x7c,
    };

    err = CopySpanToMutableSpan(ByteSpan(kCdForAllExamples), outBuffer);
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(outBuffer.size() >= mFactoryData.dac.dac_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dac.dac_cert.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(outBuffer.data(), mFactoryData.dac.dac_cert.value, mFactoryData.dac.dac_cert.len);

    outBuffer.reduce_size(mFactoryData.dac.dac_cert.len);
    err = CHIP_NO_ERROR;
#else
    const uint8_t kDacCert[] = {
        0x30, 0x82, 0x01, 0xe7, 0x30, 0x82, 0x01, 0x8e, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x69, 0xcd, 0xf1, 0x0d, 0xe9,
        0xe5, 0x4e, 0xd1, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x3d, 0x31, 0x25, 0x30,
        0x23, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x1c, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x44, 0x65, 0x76, 0x20, 0x50,
        0x41, 0x49, 0x20, 0x30, 0x78, 0x46, 0x46, 0x46, 0x31, 0x20, 0x6e, 0x6f, 0x20, 0x50, 0x49, 0x44, 0x31, 0x14, 0x30, 0x12,
        0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46, 0x46, 0x31, 0x30, 0x20,
        0x17, 0x0d, 0x32, 0x32, 0x30, 0x32, 0x30, 0x35, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x18, 0x0f, 0x39, 0x39, 0x39,
        0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x53, 0x31, 0x25, 0x30, 0x23, 0x06, 0x03,
        0x55, 0x04, 0x03, 0x0c, 0x1c, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x44, 0x65, 0x76, 0x20, 0x44, 0x41, 0x43, 0x20,
        0x30, 0x78, 0x46, 0x46, 0x46, 0x31, 0x2f, 0x30, 0x78, 0x38, 0x30, 0x30, 0x31, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b,
        0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46, 0x46, 0x31, 0x31, 0x14, 0x30, 0x12, 0x06,
        0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x02, 0x0c, 0x04, 0x38, 0x30, 0x30, 0x31, 0x30, 0x59, 0x30,
        0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07,
        0x03, 0x42, 0x00, 0x04, 0x46, 0x3a, 0xc6, 0x93, 0x42, 0x91, 0x0a, 0x0e, 0x55, 0x88, 0xfc, 0x6f, 0xf5, 0x6b, 0xb6, 0x3e,
        0x62, 0xec, 0xce, 0xcb, 0x14, 0x8f, 0x7d, 0x4e, 0xb0, 0x3e, 0xe5, 0x52, 0x60, 0x14, 0x15, 0x76, 0x7d, 0x16, 0xa5, 0xc6,
        0x63, 0xf7, 0x93, 0xe4, 0x91, 0x23, 0x26, 0x0b, 0x82, 0x97, 0xa7, 0xcd, 0x7e, 0x7c, 0xfc, 0x7b, 0x31, 0x6b, 0x39, 0xd9,
        0x8e, 0x90, 0xd2, 0x93, 0x77, 0x73, 0x8e, 0x82, 0xa3, 0x60, 0x30, 0x5e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01,
        0x01, 0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02,
        0x07, 0x80, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x88, 0xdd, 0xe7, 0xb3, 0x00, 0x38, 0x29,
        0x32, 0xcf, 0xf7, 0x34, 0xc0, 0x46, 0x24, 0x81, 0x0f, 0x44, 0x16, 0x8a, 0x6f, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23,
        0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x63, 0x54, 0x0e, 0x47, 0xf6, 0x4b, 0x1c, 0x38, 0xd1, 0x38, 0x84, 0xa4, 0x62, 0xd1,
        0x6c, 0x19, 0x5d, 0x8f, 0xfb, 0x3c, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x47,
        0x00, 0x30, 0x44, 0x02, 0x20, 0x01, 0x27, 0xa2, 0x7b, 0x4b, 0x44, 0x61, 0x0e, 0xe2, 0xfc, 0xdc, 0x4d, 0x2b, 0x78, 0x85,
        0x56, 0x36, 0x60, 0xbc, 0x0f, 0x76, 0xf1, 0x72, 0x19, 0xed, 0x6a, 0x08, 0xdf, 0xb2, 0xb3, 0xc1, 0xcd, 0x02, 0x20, 0x6b,
        0x59, 0xe0, 0xaf, 0x45, 0xf3, 0xeb, 0x2a, 0x85, 0xb9, 0x19, 0xd3, 0x57, 0x31, 0x52, 0x8c, 0x60, 0x28, 0xc4, 0x15, 0x23,
        0x95, 0x45, 0xe1, 0x08, 0xe4, 0xe5, 0x4e, 0x70, 0x97, 0x13, 0x53,
    };

    err                      = CopySpanToMutableSpan(ByteSpan(kDacCert), outBuffer);
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(outBuffer.size() >= mFactoryData.dac.pai_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dac.pai_cert.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    memcpy(outBuffer.data(), mFactoryData.dac.pai_cert.value, mFactoryData.dac.pai_cert.len);

    outBuffer.reduce_size(mFactoryData.dac.pai_cert.len);
    err = CHIP_NO_ERROR;
#else
    const uint8_t kPaiCert[] = {
        0x30, 0x82, 0x01, 0xcb, 0x30, 0x82, 0x01, 0x71, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x56, 0xad, 0x82, 0x22, 0xad,
        0x94, 0x5b, 0x64, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x30, 0x31, 0x18, 0x30,
        0x16, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0f, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20,
        0x50, 0x41, 0x41, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c,
        0x04, 0x46, 0x46, 0x46, 0x31, 0x30, 0x20, 0x17, 0x0d, 0x32, 0x32, 0x30, 0x32, 0x30, 0x35, 0x30, 0x30, 0x30, 0x30, 0x30,
        0x30, 0x5a, 0x18, 0x0f, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30,
        0x3d, 0x31, 0x25, 0x30, 0x23, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x1c, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x44,
        0x65, 0x76, 0x20, 0x50, 0x41, 0x49, 0x20, 0x30, 0x78, 0x46, 0x46, 0x46, 0x31, 0x20, 0x6e, 0x6f, 0x20, 0x50, 0x49, 0x44,
        0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46,
        0x46, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48,
        0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x41, 0x9a, 0x93, 0x15, 0xc2, 0x17, 0x3e, 0x0c, 0x8c, 0x87, 0x6d,
        0x03, 0xcc, 0xfc, 0x94, 0x48, 0x52, 0x64, 0x7f, 0x7f, 0xec, 0x5e, 0x50, 0x82, 0xf4, 0x05, 0x99, 0x28, 0xec, 0xa8, 0x94,
        0xc5, 0x94, 0x15, 0x13, 0x09, 0xac, 0x63, 0x1e, 0x4c, 0xb0, 0x33, 0x92, 0xaf, 0x68, 0x4b, 0x0b, 0xaf, 0xb7, 0xe6, 0x5b,
        0x3b, 0x81, 0x62, 0xc2, 0xf5, 0x2b, 0xf9, 0x31, 0xb8, 0xe7, 0x7a, 0xaa, 0x82, 0xa3, 0x66, 0x30, 0x64, 0x30, 0x12, 0x06,
        0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xff, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x06,
        0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e,
        0x04, 0x16, 0x04, 0x14, 0x63, 0x54, 0x0e, 0x47, 0xf6, 0x4b, 0x1c, 0x38, 0xd1, 0x38, 0x84, 0xa4, 0x62, 0xd1, 0x6c, 0x19,
        0x5d, 0x8f, 0xfb, 0x3c, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x6a, 0xfd, 0x22,
        0x77, 0x1f, 0x51, 0x1f, 0xec, 0xbf, 0x16, 0x41, 0x97, 0x67, 0x10, 0xdc, 0xdc, 0x31, 0xa1, 0x71, 0x7e, 0x30, 0x0a, 0x06,
        0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x21, 0x00, 0xb2, 0xef, 0x27,
        0xf4, 0x9a, 0xe9, 0xb5, 0x0f, 0xb9, 0x1e, 0xea, 0xc9, 0x4c, 0x4d, 0x0b, 0xdb, 0xb8, 0xd7, 0x92, 0x9c, 0x6c, 0xb8, 0x8f,
        0xac, 0xe5, 0x29, 0x36, 0x8d, 0x12, 0x05, 0x4c, 0x0c, 0x02, 0x20, 0x65, 0x5d, 0xc9, 0x2b, 0x86, 0xbd, 0x90, 0x98, 0x82,
        0xa6, 0xc6, 0x21, 0x77, 0xb8, 0x25, 0xd7, 0xd0, 0x5e, 0xdb, 0xe7, 0xc2, 0x2f, 0x9f, 0xea, 0x71, 0x22, 0x0e, 0x7e, 0xa7,
        0x03, 0xf8, 0x91,
    };

    err                             = CopySpanToMutableSpan(ByteSpan{ kPaiCert }, outBuffer);
#endif // CONFIG_FACTORY_DATA

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(mFactoryData.dac.dac_cert.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    VerifyOrReturnError(mFactoryData.dac.dac_key.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    // Extract public key from DAC cert.
    ByteSpan dacCertSpan{ reinterpret_cast<uint8_t *>(mFactoryData.dac.dac_cert.value), mFactoryData.dac.dac_cert.len };
    chip::Crypto::P256PublicKey dacPublicKey;

    ReturnErrorOnFailure(chip::Crypto::ExtractPubkeyFromX509Cert(dacCertSpan, dacPublicKey));
    ReturnErrorOnFailure(
        LoadKeypairFromRaw(ByteSpan(reinterpret_cast<uint8_t *>(mFactoryData.dac.dac_key.value), mFactoryData.dac.dac_key.len),
                           ByteSpan(dacPublicKey.Bytes(), dacPublicKey.Length()), keypair));
#else
    const uint8_t kDacPublicKey[65] = {
        0x04, 0x46, 0x3a, 0xc6, 0x93, 0x42, 0x91, 0x0a, 0x0e, 0x55, 0x88, 0xfc, 0x6f, 0xf5, 0x6b, 0xb6, 0x3e,
        0x62, 0xec, 0xce, 0xcb, 0x14, 0x8f, 0x7d, 0x4e, 0xb0, 0x3e, 0xe5, 0x52, 0x60, 0x14, 0x15, 0x76, 0x7d,
        0x16, 0xa5, 0xc6, 0x63, 0xf7, 0x93, 0xe4, 0x91, 0x23, 0x26, 0x0b, 0x82, 0x97, 0xa7, 0xcd, 0x7e, 0x7c,
        0xfc, 0x7b, 0x31, 0x6b, 0x39, 0xd9, 0x8e, 0x90, 0xd2, 0x93, 0x77, 0x73, 0x8e, 0x82,
    };

    const uint8_t kDacPrivateKey[32] = {
        0xaa, 0xb6, 0x00, 0xae, 0x8a, 0xe8, 0xaa, 0xb7, 0xd7, 0x36, 0x27, 0xc2, 0x17, 0xb7, 0xc2, 0x04,
        0x70, 0x9c, 0xa6, 0x94, 0x6a, 0xf5, 0xf2, 0xf7, 0x53, 0x08, 0x33, 0xa5, 0x2b, 0x44, 0xfb, 0xff,
    };

    ReturnErrorOnFailure(LoadKeypairFromRaw(ByteSpan(kDacPrivateKey), ByteSpan(kDacPublicKey), keypair));
#endif

    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    setupDiscriminator = mFactoryData.cdata.discriminator;
    err                = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
    uint32_t val;
    val                = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
    setupDiscriminator = static_cast<uint16_t>(val);
    err                = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#endif // CONFIG_FACTORY_DATA
    //
    return err;
}

CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(mFactoryData.cdata.spake2_it != 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    iterationCount = mFactoryData.cdata.spake2_it;
    err            = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
    iterationCount     = CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT;
    err                = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT) && CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    CHIP_ERROR err                          = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    size_t saltB64Len                       = 0;

#if CONFIG_FACTORY_DATA
    saltB64Len = mFactoryData.cdata.spake2_salt.len;
    VerifyOrReturnError(saltB64Len <= sizeof(saltB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltB64, mFactoryData.cdata.spake2_salt.value, saltB64Len);
    err = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
    saltB64Len         = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT);
    VerifyOrReturnError(saltB64Len <= sizeof(saltB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltB64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT, saltB64Len);
    err            = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
#endif // CONFIG_FACTORY_DATA

    ReturnErrorOnFailure(err);
    size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, reinterpret_cast<uint8_t *>(saltB64));

    VerifyOrReturnError(saltLen <= saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    static constexpr size_t kSpake2pSerializedVerifier_MaxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;

    CHIP_ERROR err                                            = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    char verifierB64[kSpake2pSerializedVerifier_MaxBase64Len] = { 0 };
    size_t verifierB64Len                                     = 0;

#if CONFIG_FACTORY_DATA
    verifierB64Len = mFactoryData.cdata.spake2_verifier.len;
    VerifyOrReturnError(verifierB64Len <= sizeof(verifierB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierB64, mFactoryData.cdata.spake2_verifier.value, verifierB64Len);
    err = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
    verifierB64Len = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER);
    VerifyOrReturnError(verifierB64Len <= sizeof(verifierB64), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierB64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER, verifierB64Len);
    err = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
#endif // CONFIG_FACTORY_DATA

    ReturnErrorOnFailure(err);
    verifierLen = chip::Base64Decode32(verifierB64, verifierB64Len, reinterpret_cast<uint8_t *>(verifierB64));
    VerifyOrReturnError(verifierLen <= verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), verifierB64, verifierLen);
    verifierBuf.reduce_size(verifierLen);

    return err;
}

CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(mFactoryData.cdata.passcode != 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    setupPasscode = mFactoryData.cdata.passcode;
    err           = CHIP_NO_ERROR;
#else
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    // setupPasscode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
    // err           = CHIP_NO_ERROR;
#endif // defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(bufSize >= mFactoryData.dii.vendor_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dii.vendor_name.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, mFactoryData.dii.vendor_name.value, mFactoryData.dii.vendor_name.len);
    buf[mFactoryData.dii.vendor_name.len] = 0;
    err                                   = CHIP_NO_ERROR;
#else
    VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
    err      = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    vendorId = mFactoryData.dii.vendor_id;
    err      = CHIP_NO_ERROR;
#else
    vendorId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID);
    err      = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA
    return err;
}

CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(bufSize >= mFactoryData.dii.product_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dii.product_name.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, mFactoryData.dii.product_name.value, mFactoryData.dii.product_name.len);
    buf[mFactoryData.dii.product_name.len] = 0;
    err                                    = CHIP_NO_ERROR;
#else
    VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
    err                 = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    productId = mFactoryData.dii.product_id;
    err       = CHIP_NO_ERROR;
#else
    productId           = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    err                 = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(bufSize >= mFactoryData.dii.serial_num.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dii.serial_num.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, mFactoryData.dii.serial_num.value, mFactoryData.dii.serial_num.len);
    buf[mFactoryData.dii.serial_num.len] = 0;
    err                                  = CHIP_NO_ERROR;
#else
    size_t serialNumLen = 0; // without counting null-terminator
#ifdef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
    if (CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER[0] != 0)
    {
        VerifyOrReturnError(sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) <= bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(buf, CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER, sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER));
        serialNumLen = sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER) - 1;
        err          = CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER

    VerifyOrReturnError(serialNumLen < bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(buf[serialNumLen] == 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    err             = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
#if FACTORY_TEST
    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else
    enum
    {
        kDateStringLength = 10 // YYYY-MM-DD
    };
    char * parseEnd;
    size_t dateLen;
    dateLen = mFactoryData.dii.mfg_date.len;
    VerifyOrExit(dateLen == kDateStringLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 4 digits, so our number can't be bigger than 9999.
    year = static_cast<uint16_t>(strtoul((char *) mFactoryData.dii.mfg_date.value, &parseEnd, 10));
    VerifyOrExit(parseEnd == (char *) mFactoryData.dii.mfg_date.value + 4, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    month = static_cast<uint8_t>(strtoul((char *) mFactoryData.dii.mfg_date.value + 5, &parseEnd, 10));
    VerifyOrExit(parseEnd == (char *) mFactoryData.dii.mfg_date.value + 7, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    day = static_cast<uint8_t>(strtoul((char *) mFactoryData.dii.mfg_date.value + 8, &parseEnd, 10));
    VerifyOrExit(parseEnd == (char *) mFactoryData.dii.mfg_date.value + 10, err = CHIP_ERROR_INVALID_ARGUMENT);

    err             = CHIP_NO_ERROR;
#endif
#else
    err             = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_FACTORY_DATA

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND && err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", mFactoryData.dii.mfg_date.value);
    }
    return err;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
#if FACTORY_TEST
    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else
    hardwareVersion = mFactoryData.dii.hw_ver;
    err             = CHIP_NO_ERROR;
#endif
#else
    hardwareVersion = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION);
    err             = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(bufSize >= mFactoryData.dii.hw_ver_string.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dii.hw_ver_string.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, mFactoryData.dii.hw_ver_string.value, mFactoryData.dii.hw_ver_string.len);
    buf[mFactoryData.dii.hw_ver_string.len] = 0;
    err                                     = CHIP_NO_ERROR;
#else
    VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
    err = CHIP_NO_ERROR;
#endif // CONFIG_FACTORY_DATA

    return err;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

#if CONFIG_FACTORY_DATA
    VerifyOrReturnError(uniqueIdSpan.size() >= mFactoryData.dii.rd_id_uid.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(mFactoryData.dii.rd_id_uid.value, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(uniqueIdSpan.data(), mFactoryData.dii.rd_id_uid.value, mFactoryData.dii.rd_id_uid.len);
    err = CHIP_NO_ERROR;
#else
    err = CHIP_ERROR_WRONG_KEY_TYPE;
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");
    constexpr uint8_t uniqueId[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;

    VerifyOrReturnError(sizeof(uniqueId) <= uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(sizeof(uniqueId) == ConfigurationManager::kRotatingDeviceIDUniqueIDLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(uniqueIdSpan.data(), uniqueId, sizeof(uniqueId));
    uniqueIdSpan.reduce_size(sizeof(uniqueId));
    err = CHIP_NO_ERROR;
#endif
#endif // CONFIG_FACTORY_DATA

    return err;
}

} // namespace DeviceLayer
} // namespace chip
