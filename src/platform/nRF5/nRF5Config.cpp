/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the Nordic nRF5 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/nRF5/nRF5Config.h>

#include <core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "FreeRTOS.h"
#include "fds.h"
#include "mem_manager.h"
#include "semphr.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

NRF5Config::FDSAsyncOp * volatile NRF5Config::sActiveAsyncOp;
SemaphoreHandle_t NRF5Config::sAsyncOpCompletionSem;

CHIP_ERROR NRF5Config::Init()
{
    CHIP_ERROR err;
    ret_code_t fdsRes;

    // Create a semaphore to signal the completion of async FDS operations.
    sAsyncOpCompletionSem = xSemaphoreCreateBinary();
    VerifyOrExit(sAsyncOpCompletionSem != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Register an FDS event handler.
    fdsRes = fds_register(HandleFDSEvent);
    SuccessOrExit(err = MapFDSError(fdsRes));

    // Initialize the FDS module.
    {
        FDSAsyncOp initOp(FDSAsyncOp::kInit);
        err = DoAsyncFDSOp(initOp);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR NRF5Config::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;
    fds_record_desc_t recDesc;
    fds_flash_record_t rec;
    uint32_t wordVal;
    bool needClose = false;

    err = OpenRecord(key, recDesc, rec);
    SuccessOrExit(err);
    needClose = true;

    VerifyOrExit(rec.p_header->length_words == 1, err = CHIP_ERROR_INVALID_ARGUMENT);

    wordVal = Encoding::LittleEndian::Get32((const uint8_t *) rec.p_data);
    val     = (wordVal != 0);

exit:
    if (needClose)
    {
        fds_record_close(&recDesc);
    }
    return err;
}

CHIP_ERROR NRF5Config::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err;
    fds_record_desc_t recDesc;
    fds_flash_record_t rec;
    bool needClose = false;

    err = OpenRecord(key, recDesc, rec);
    SuccessOrExit(err);
    needClose = true;

    VerifyOrExit(rec.p_header->length_words == 1, err = CHIP_ERROR_INVALID_ARGUMENT);

    val = Encoding::LittleEndian::Get32((const uint8_t *) rec.p_data);

exit:
    if (needClose)
    {
        fds_record_close(&recDesc);
    }
    return err;
}

CHIP_ERROR NRF5Config::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err;
    fds_record_desc_t recDesc;
    fds_flash_record_t rec;
    bool needClose = false;

    err = OpenRecord(key, recDesc, rec);
    SuccessOrExit(err);
    needClose = true;

    VerifyOrExit(rec.p_header->length_words == 2, err = CHIP_ERROR_INVALID_ARGUMENT);

    val = Encoding::LittleEndian::Get64((const uint8_t *) rec.p_data);

exit:
    if (needClose)
    {
        fds_record_close(&recDesc);
    }
    return err;
}

CHIP_ERROR NRF5Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    fds_flash_record_t rec;
    fds_record_desc_t recDesc;
    bool needClose = false;
    const uint8_t * strEnd;

    err = OpenRecord(key, recDesc, rec);
    SuccessOrExit(err);
    needClose = true;

    strEnd = (const uint8_t *) memchr(rec.p_data, 0, rec.p_header->length_words * kFDSWordSize);
    VerifyOrExit(strEnd != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    outLen = strEnd - (const uint8_t *) rec.p_data;

    // NOTE: the caller is allowed to pass NULL for buf to query the length of the stored
    // value.

    if (buf != NULL)
    {
        VerifyOrExit(bufSize > outLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

        memcpy(buf, rec.p_data, outLen + 1);
    }

exit:
    if (needClose)
    {
        fds_record_close(&recDesc);
    }
    return err;
}

CHIP_ERROR NRF5Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    fds_flash_record_t rec;
    fds_record_desc_t recDesc;
    bool needClose = false;

    err = OpenRecord(key, recDesc, rec);
    SuccessOrExit(err);
    needClose = true;

    VerifyOrExit(rec.p_header->length_words >= 1, err = CHIP_ERROR_INVALID_ARGUMENT);

    // First two bytes are length.
    outLen = Encoding::LittleEndian::Get16((const uint8_t *) rec.p_data);

    VerifyOrExit((rec.p_header->length_words * kFDSWordSize) >= (outLen + 2), err = CHIP_ERROR_INVALID_ARGUMENT);

    // NOTE: the caller is allowed to pass NULL for buf to query the length of the stored
    // value.

    if (buf != NULL)
    {
        VerifyOrExit(bufSize >= outLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

        memcpy(buf, ((const uint8_t *) rec.p_data) + 2, outLen);
    }

exit:
    if (needClose)
    {
        fds_record_close(&recDesc);
    }
    return err;
}

CHIP_ERROR NRF5Config::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err;
    uint32_t storedVal = (val) ? 1 : 0;

    FDSAsyncOp addOrUpdateOp(FDSAsyncOp::kAddOrUpdateRecord);
    addOrUpdateOp.FileId                = GetFileId(key);
    addOrUpdateOp.RecordKey             = GetRecordKey(key);
    addOrUpdateOp.RecordData            = (const uint8_t *) &storedVal;
    addOrUpdateOp.RecordDataLengthWords = 1;

    err = DoAsyncFDSOp(addOrUpdateOp);
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "FDS set: %04" PRIX16 "/%04" PRIX16 " = %s", GetFileId(key), GetRecordKey(key),
                    val ? "true" : "false");

exit:
    return err;
}

CHIP_ERROR NRF5Config::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err;

    FDSAsyncOp addOrUpdateOp(FDSAsyncOp::kAddOrUpdateRecord);
    addOrUpdateOp.FileId                = GetFileId(key);
    addOrUpdateOp.RecordKey             = GetRecordKey(key);
    addOrUpdateOp.RecordData            = (const uint8_t *) &val;
    addOrUpdateOp.RecordDataLengthWords = 1;

    err = DoAsyncFDSOp(addOrUpdateOp);
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "FDS set: 0x%04" PRIX16 "/0x%04" PRIX16 " = %" PRIu32 " (0x%" PRIX32 ")", GetFileId(key),
                    GetRecordKey(key), val, val);

exit:
    return err;
}

CHIP_ERROR NRF5Config::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err;

    FDSAsyncOp addOrUpdateOp(FDSAsyncOp::kAddOrUpdateRecord);
    addOrUpdateOp.FileId                = GetFileId(key);
    addOrUpdateOp.RecordKey             = GetRecordKey(key);
    addOrUpdateOp.RecordData            = (const uint8_t *) &val;
    addOrUpdateOp.RecordDataLengthWords = 2;

    err = DoAsyncFDSOp(addOrUpdateOp);
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "FDS set: 0x%04" PRIX16 "/0x%04" PRIX16 " = %" PRIu64 " (0x%" PRIX64 ")", GetFileId(key),
                    GetRecordKey(key), val, val);

exit:
    return err;
}

CHIP_ERROR NRF5Config::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR NRF5Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;
    uint8_t * storedVal = NULL;

    if (str != NULL)
    {
        uint32_t storedValWords = FDSWords(strLen + 1);

        storedVal = (uint8_t *) pvPortMalloc(storedValWords * kFDSWordSize);
        VerifyOrExit(storedVal != NULL, err = CHIP_ERROR_NO_MEMORY);

        memcpy(storedVal, str, strLen);
        storedVal[strLen] = 0;

        FDSAsyncOp addOrUpdateOp(FDSAsyncOp::kAddOrUpdateRecord);
        addOrUpdateOp.FileId                = GetFileId(key);
        addOrUpdateOp.RecordKey             = GetRecordKey(key);
        addOrUpdateOp.RecordData            = storedVal;
        addOrUpdateOp.RecordDataLengthWords = storedValWords;

        err = DoAsyncFDSOp(addOrUpdateOp);
        SuccessOrExit(err);

        ChipLogProgress(DeviceLayer, "FDS set: 0x%04" PRIX16 "/0x%04" PRIX16 " = \"%s\"", GetFileId(key), GetRecordKey(key),
                        (const char *) storedVal);
    }

    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    if (storedVal != NULL)
    {
        vPortFree(storedVal);
    }
    return err;
}

CHIP_ERROR NRF5Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err;
    uint8_t * storedVal = NULL;

    if (data != NULL)
    {
        uint32_t storedValWords = FDSWords(dataLen + 2);

        storedVal = (uint8_t *) pvPortMalloc(storedValWords * kFDSWordSize);
        VerifyOrExit(storedVal != NULL, err = CHIP_ERROR_NO_MEMORY);

        // First two bytes encode the length.
        Encoding::LittleEndian::Put16(storedVal, (uint16_t) dataLen);

        memcpy(storedVal + 2, data, dataLen);

        FDSAsyncOp addOrUpdateOp(FDSAsyncOp::kAddOrUpdateRecord);
        addOrUpdateOp.FileId                = GetFileId(key);
        addOrUpdateOp.RecordKey             = GetRecordKey(key);
        addOrUpdateOp.RecordData            = storedVal;
        addOrUpdateOp.RecordDataLengthWords = storedValWords;

        err = DoAsyncFDSOp(addOrUpdateOp);
        SuccessOrExit(err);

        ChipLogProgress(DeviceLayer, "FDS set: 0x%04" PRIX16 "/0x%04" PRIX16 " = (blob length %" PRId32 ")", GetFileId(key),
                        GetRecordKey(key), dataLen);
    }

    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    if (storedVal != NULL)
    {
        vPortFree(storedVal);
    }
    return err;
}

CHIP_ERROR NRF5Config::ClearConfigValue(Key key)
{
    CHIP_ERROR err;

    FDSAsyncOp delOp(FDSAsyncOp::kDeleteRecordByKey);
    delOp.FileId    = GetFileId(key);
    delOp.RecordKey = GetRecordKey(key);

    err = DoAsyncFDSOp(delOp);
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "FDS delete: 0x%04" PRIX16 "/0x%04" PRIX16, GetFileId(key), GetRecordKey(key));

exit:
    return err;
}

bool NRF5Config::ConfigValueExists(Key key)
{
    ret_code_t fdsRes;
    fds_record_desc_t recDesc;
    fds_find_token_t findToken;

    // Search for the requested record.
    memset(&findToken, 0, sizeof(findToken));
    fdsRes = fds_record_find(GetFileId(key), GetRecordKey(key), &recDesc, &findToken);

    // Return true iff the record was found.
    return fdsRes == NRF_SUCCESS;
}

CHIP_ERROR NRF5Config::FactoryResetConfig(void)
{
    CHIP_ERROR err;

    // Delete the chipConfig file and all records its contains.
    {
        FDSAsyncOp delOp(FDSAsyncOp::kDeleteFile);
        delOp.FileId = kFileId_ChipConfig;
        err          = DoAsyncFDSOp(delOp);
        SuccessOrExit(err);
        ChipLogProgress(DeviceLayer, "FDS delete file: 0x%04" PRIX16, kFileId_ChipConfig);
    }

    // Force a GC
    {
        FDSAsyncOp gcOp(FDSAsyncOp::kGC);
        err = DoAsyncFDSOp(gcOp);
        SuccessOrExit(err);
    }

exit:
    return err;
}

#define CHIP_DEVICE_CONFIG_NRF5_FDS_ERROR_MIN 10000000
CHIP_ERROR NRF5Config::MapFDSError(ret_code_t fdsRes)
{
    return (fdsRes == NRF_SUCCESS) ? CHIP_NO_ERROR : CHIP_DEVICE_CONFIG_NRF5_FDS_ERROR_MIN + fdsRes;
}

CHIP_ERROR NRF5Config::OpenRecord(NRF5Config::Key key, fds_record_desc_t & recDesc, fds_flash_record_t & rec)
{
    CHIP_ERROR err;
    ret_code_t fdsRes;
    fds_find_token_t findToken;

    // Search for the requested record.  Return "CONFIG_NOT_FOUND" if it doesn't exist.
    memset(&findToken, 0, sizeof(findToken));
    fdsRes = fds_record_find(NRF5Config::GetFileId(key), NRF5Config::GetRecordKey(key), &recDesc, &findToken);
    err    = (fdsRes == FDS_ERR_NOT_FOUND) ? CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND : MapFDSError(fdsRes);
    SuccessOrExit(err);

    // Open the record for reading.
    fdsRes = fds_record_open(&recDesc, &rec);
    err    = MapFDSError(fdsRes);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR NRF5Config::ForEachRecord(uint16_t fileId, uint16_t recordKey, ForEachRecordFunct funct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ;
    ret_code_t fdsRes;
    fds_find_token_t findToken;
    fds_record_desc_t recDesc;
    fds_flash_record_t rec;
    bool needClose = false;

    memset(&findToken, 0, sizeof(findToken));

    while (true)
    {
        // Search for an occurrence of the requested record.  If there are no more records, break the loop.
        fdsRes = fds_record_find(fileId, recordKey, &recDesc, &findToken);
        if (fdsRes == FDS_ERR_NOT_FOUND)
        {
            break;
        }
        err = MapFDSError(fdsRes);
        SuccessOrExit(err);

        // Open the record for reading.
        fdsRes = fds_record_open(&recDesc, &rec);
        err    = MapFDSError(fdsRes);
        SuccessOrExit(err);
        needClose = true;

        bool deleteRec = false;

        // Invoke the caller's function.
        err = funct(rec, deleteRec);
        SuccessOrExit(err);

        // Close the record.
        needClose = false;
        fdsRes    = fds_record_close(&recDesc);
        err       = MapFDSError(fdsRes);
        SuccessOrExit(err);

        // Delete the record if requested.
        if (deleteRec)
        {
            FDSAsyncOp delOp(FDSAsyncOp::kDeleteRecord);
            delOp.FileId     = fileId;
            delOp.RecordKey  = recordKey;
            delOp.RecordDesc = recDesc;
            err              = DoAsyncFDSOp(delOp);
            SuccessOrExit(err);
        }
    }

exit:
    if (needClose)
    {
        fds_record_close(&recDesc);
    }
    return err;
}

CHIP_ERROR NRF5Config::DoAsyncFDSOp(FDSAsyncOp & asyncOp)
{
    CHIP_ERROR err;
    ret_code_t fdsRes;
    fds_record_t rec;
    bool gcPerformed = false;

    // Keep trying to perform the requested op until there's a definitive success or failure...
    while (true)
    {
        bool existingRecFound = false;

        // If performing an AddOrUpdateRecord or DeleteRecordByKey, search for an existing record with the given key.
        if (asyncOp.OpType == FDSAsyncOp::kAddOrUpdateRecord || asyncOp.OpType == FDSAsyncOp::kDeleteRecordByKey)
        {
            fds_find_token_t findToken;
            memset(&findToken, 0, sizeof(findToken));
            fdsRes = fds_record_find(asyncOp.FileId, asyncOp.RecordKey, &asyncOp.RecordDesc, &findToken);
            VerifyOrExit(fdsRes == NRF_SUCCESS || fdsRes == FDS_ERR_NOT_FOUND, err = MapFDSError(fdsRes));

            // Remember if we found an existing record.
            existingRecFound = (fdsRes == NRF_SUCCESS);
        }

        // If adding or updating a record, prepare the FDS record structure with the record data.
        if (asyncOp.OpType == FDSAsyncOp::kAddRecord || asyncOp.OpType == FDSAsyncOp::kUpdateRecord ||
            asyncOp.OpType == FDSAsyncOp::kAddOrUpdateRecord)
        {
            memset(&rec, 0, sizeof(rec));
            rec.file_id           = asyncOp.FileId;
            rec.key               = asyncOp.RecordKey;
            rec.data.p_data       = asyncOp.RecordData;
            rec.data.length_words = asyncOp.RecordDataLengthWords;
        }

        // Make the requested op the active op.
        sActiveAsyncOp = &asyncOp;

        // Initiate the requested FDS operation.
        switch (asyncOp.OpType)
        {
        case FDSAsyncOp::kInit:
            fdsRes = fds_init();
            break;
        case FDSAsyncOp::kAddOrUpdateRecord:
            // Depending on whether an existing record was found, call fds_record_write or fds_record_update.
            if (!existingRecFound)
            {
            case FDSAsyncOp::kAddRecord:
                fdsRes = fds_record_write(NULL, &rec);
                break;
            }
            else
            {
            case FDSAsyncOp::kUpdateRecord:
                fdsRes = fds_record_update(&asyncOp.RecordDesc, &rec);
                break;
            }
        case FDSAsyncOp::kDeleteRecordByKey:
            // If performing a kDeleteRecordByKey and no matching record was found, simply return success.
            if (!existingRecFound)
            {
                ExitNow(err = CHIP_NO_ERROR);
            }
            // fall through...
        case FDSAsyncOp::kDeleteRecord:
            fdsRes = fds_record_delete(&asyncOp.RecordDesc);
            break;
        case FDSAsyncOp::kDeleteFile:
            fdsRes = fds_file_delete(asyncOp.FileId);
            break;
        case FDSAsyncOp::kGC:
            fdsRes = fds_gc();
            break;
        case FDSAsyncOp::kWaitQueueSpaceAvailable:
            // In this case, arrange to wait for any operation to complete, which coincides with
            // space on the operation queue being available.
            fdsRes = NRF_SUCCESS;
            break;
        default:
            ChipDie();
        }

        // If the operation was queued successfully, wait for it to complete and retrieve the result.
        // If the FreeRTOS scheduler is not running, poll the completion semaphore; otherwise wait
        // indefinitely.
        //
        if (fdsRes == NRF_SUCCESS)
        {
            if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
            {
                while (xSemaphoreTake(sAsyncOpCompletionSem, 0) == pdFALSE)
                    ;
            }
            else
            {
                xSemaphoreTake(sAsyncOpCompletionSem, portMAX_DELAY);
            }

            fdsRes = asyncOp.Result;
        }

        // Clear the active operation in case it wasn't done by the event handler.
        sActiveAsyncOp = NULL;

        // Return immediately if the operation completed successfully.
        if (fdsRes == NRF_SUCCESS)
        {
            ExitNow(err = CHIP_NO_ERROR);
        }

        // If the operation failed for lack of flash space...
        if (fdsRes == FDS_ERR_NO_SPACE_IN_FLASH)
        {
            // If we've already performed a garbage collection, fail immediately.
            if (gcPerformed)
            {
                ExitNow(err = MapFDSError(fdsRes));
            }

            ChipLogProgress(DeviceLayer, "Initiating FDS GC to recover space");

            // Request a garbage collection cycle and wait for it to complete.
            FDSAsyncOp gcOp(FDSAsyncOp::kGC);
            err = DoAsyncFDSOp(gcOp);
            SuccessOrExit(err);

            // Repeat the requested operation.
            continue;
        }

        // If the write/update failed because the operation queue is full, wait for
        // space to become available and then repeat the requested operation.
        if (fdsRes == FDS_ERR_NO_SPACE_IN_QUEUES)
        {
            FDSAsyncOp waitOp(FDSAsyncOp::kWaitQueueSpaceAvailable);
            err = DoAsyncFDSOp(waitOp);
            SuccessOrExit(err);
            continue;
        }

        // If the operation timed out, simply try it again.
        if (fdsRes == FDS_ERR_OPERATION_TIMEOUT)
        {
            continue;
        }

        // Otherwise fail with an unrecoverable error.
        ExitNow(err = MapFDSError(fdsRes));
    }

exit:
    return err;
}

void NRF5Config::HandleFDSEvent(const fds_evt_t * fdsEvent)
{
    // Do nothing if there's no async operation active.
    if (sActiveAsyncOp == NULL)
    {
        return;
    }

    // Check if the event applies to the active async operation.
    switch (sActiveAsyncOp->OpType)
    {
    case FDSAsyncOp::kInit:
        if (fdsEvent->id != FDS_EVT_INIT)
        {
            return;
        }
        break;
    case FDSAsyncOp::kAddOrUpdateRecord:
    case FDSAsyncOp::kAddRecord:
    case FDSAsyncOp::kUpdateRecord:
        // Ignore the event if its not a WRITE or UPDATE, or if its for a different file/record.
        if ((fdsEvent->id != FDS_EVT_WRITE && fdsEvent->id != FDS_EVT_UPDATE) ||
            fdsEvent->write.file_id != sActiveAsyncOp->FileId || fdsEvent->write.record_key != sActiveAsyncOp->RecordKey)
        {
            return;
        }
        break;
    case FDSAsyncOp::kDeleteRecord:
    case FDSAsyncOp::kDeleteRecordByKey:
        // Ignore the event if its not a DEL_RECORD, or if its for a different file/record.
        if (fdsEvent->id != FDS_EVT_DEL_RECORD || fdsEvent->del.record_id != sActiveAsyncOp->RecordDesc.record_id)
        {
            return;
        }
        break;
    case FDSAsyncOp::kDeleteFile:
        // Ignore the event if its not a DEL_FILE or its for a different file.
        if (fdsEvent->id != FDS_EVT_DEL_FILE || fdsEvent->del.file_id != sActiveAsyncOp->FileId)
        {
            return;
        }
        break;
    case FDSAsyncOp::kGC:
        // Ignore the event if its not a GC.
        if (fdsEvent->id != FDS_EVT_GC)
        {
            return;
        }
        break;
    case FDSAsyncOp::kWaitQueueSpaceAvailable:
        break;
    }

    // Capture the result.
    sActiveAsyncOp->Result = fdsEvent->result;

    // Mark the operation as complete.
    sActiveAsyncOp = NULL;

    // Signal the CHIP thread that the operation has completed.
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    // When using the Nodic SoftDevice, HandleFDSEvent() is called in a SoftDevice interrupt
    // context.  Therefore, we must use xSemaphoreGiveFromISR() to signal completion.
    BaseType_t yieldRequired = xSemaphoreGiveFromISR(sAsyncOpCompletionSem, &yieldRequired);

    // Yield to the next runnable task, but only if the FreeRTOS scheduler has been started.
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED && yieldRequired == pdTRUE)
    {
        portYIELD_FROM_ISR(yieldRequired);
    }

#else // defined(SOFTDEVICE_PRESENT) || !SOFTDEVICE_PRESENT

    // When NOT using the Nodic SoftDevice, HandleFDSEvent() is called in a non-interrupt
    // context. Therefore, use xSemaphoreGive() to signal completion.
    xSemaphoreGive(sAsyncOpCompletionSem);

#endif // !(defined(SOFTDEVICE_PRESENT) || !SOFTDEVICE_PRESENT)
}

void NRF5Config::RunConfigUnitTest()
{
    CHIP_ERROR err;

    // Run common unit test
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<NRF5Config>();

    // NRF Config Test 1 -- Force GC
    {
        const static uint8_t kTestData[] = {
            0xD5, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x30, 0x01, 0x08, 0x79, 0x55, 0x9F, 0x15, 0x1F, 0x66, 0x3D, 0x8F, 0x24,
            0x02, 0x05, 0x37, 0x03, 0x27, 0x13, 0x02, 0x00, 0x00, 0xEE, 0xEE, 0x30, 0xB4, 0x18, 0x18, 0x26, 0x04, 0x80, 0x41,
            0x1B, 0x23, 0x26, 0x05, 0x7F, 0xFF, 0xFF, 0x52, 0x37, 0x06, 0x27, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x30, 0xB4,
            0x18, 0x18, 0x24, 0x07, 0x02, 0x26, 0x08, 0x25, 0x00, 0x5A, 0x23, 0x30, 0x0A, 0x39, 0x04, 0x9E, 0xC7, 0x77, 0xC5,
            0xA4, 0x13, 0x31, 0xF7, 0x72, 0x2E, 0x27, 0xC2, 0x86, 0x3D, 0xC5, 0x2E, 0xD5, 0xD2, 0x3C, 0xCF, 0x7E, 0x06, 0xE3,
            0x48, 0x53, 0x87, 0xE8, 0x4D, 0xB0, 0x27, 0x07, 0x58, 0x4A, 0x38, 0xB4, 0xF3, 0xB2, 0x47, 0x94, 0x45, 0x58, 0x65,
            0x80, 0x08, 0x17, 0x6B, 0x8E, 0x4F, 0x07, 0x41, 0xA3, 0x3D, 0x5D, 0xCE, 0x76, 0x86, 0x35, 0x83, 0x29, 0x01, 0x18,
            0x35, 0x82, 0x29, 0x01, 0x24, 0x02, 0x05, 0x18, 0x35, 0x84, 0x29, 0x01, 0x36, 0x02, 0x04, 0x02, 0x04, 0x01, 0x18,
            0x18, 0x35, 0x81, 0x30, 0x02, 0x08, 0x42, 0xBD, 0x2C, 0x6B, 0x5B, 0x3A, 0x18, 0x16, 0x18, 0x35, 0x80, 0x30, 0x02,
            0x08, 0x44, 0xE3, 0x40, 0x38, 0xA9, 0xD4, 0xB5, 0xA7, 0x18, 0x35, 0x0C, 0x30, 0x01, 0x19, 0x00, 0xA6, 0x5D, 0x54,
            0xF5, 0xAE, 0x5D, 0x63, 0xEB, 0x69, 0xD8, 0xDB, 0xCB, 0xE2, 0x20, 0x0C, 0xD5, 0x6F, 0x43, 0x5E, 0x96, 0xA8, 0x54,
            0xB2, 0x74, 0x30, 0x02, 0x19, 0x00, 0xE0, 0x37, 0x02, 0x8B, 0xB3, 0x04, 0x06, 0xDD, 0xBD, 0x28, 0xAA, 0xC4, 0xF1,
            0xFF, 0xFB, 0xB1, 0xD4, 0x1C, 0x78, 0x40, 0xDA, 0x2C, 0xD8, 0x40, 0x18, 0x18,
        };
        uint8_t buf[512];
        size_t dataLen;

        for (int i = 0; i < 100; i++)
        {
            err = WriteConfigValueBin(kConfigKey_MfrDeviceCert, kTestData, sizeof(kTestData));
            VerifyOrDie(err == CHIP_NO_ERROR);

            vTaskDelay(pdMS_TO_TICKS(50));
        }

        err = ReadConfigValueBin(kConfigKey_MfrDeviceCert, buf, sizeof(buf), dataLen);
        VerifyOrDie(err == CHIP_NO_ERROR);

        VerifyOrDie(dataLen == sizeof(kTestData));
        VerifyOrDie(memcmp(buf, kTestData, dataLen) == 0);
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
