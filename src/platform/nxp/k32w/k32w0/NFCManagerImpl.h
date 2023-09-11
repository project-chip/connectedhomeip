/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Platform-specific NFCManager implementation for NXP K32W.
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "ntag_driver.h"

namespace chip {
namespace DeviceLayer {

namespace {
#define NDEF_RECORD_TYPE 0xD1 /* MB=1, ME=1, TNF=1 */
#define NDEF_RECORD_TYPE_LEN 0x01
#define NFC_NDEF_RECORD_NAME 'U'
#define NDEF_URI_ID_CODE 0x00 /* No URI abbreviation */
#define NDEF_URI_ID_MAX_LENGTH 40
#define TERMINATOR_TLV_LEN 1
} // namespace

class NFCManagerImpl final : public NFCManager
{
    friend class NFCManager;

private:
    // ===== Members that implement the NFCManager internal interface.

    CHIP_ERROR _Init();
    CHIP_ERROR _StartTagEmulation(const char * payload, size_t payloadLength);
    CHIP_ERROR _StopTagEmulation();
    bool _IsTagEmulationStarted() const { return mIsStarted; };

    // ===== Members for internal use by this class.

    constexpr static uint8_t kNdefBufferSize = 128;

    uint8_t mNdefBuffer[kNdefBufferSize];
    bool mIsStarted;

    // ===== Members for internal use by the following friends.

    friend NFCManager & NFCMgr();
    friend NFCManagerImpl & NFCMgrImpl();

    typedef enum
    {
        E_APP_NTAG_NO_ERROR,
        E_APP_NTAG_I2C_ERROR,
        E_APP_NTAG_READ_ERROR,
        E_APP_NTAG_WRITE_ERROR
    } eAppNtagError;

    typedef PACKED_STRUCT
    {
        uint8_t recordType;
        uint8_t recordTypeLen;
        uint8_t payloadLen;
        uint8_t recordName;
        uint8_t uriIdCode;
        uint8_t uriIdData[NDEF_URI_ID_MAX_LENGTH + TERMINATOR_TLV_LEN];
    }
    NdefUriRecord_t;

    static bool AppNtagEepromWrite(uint8_t originalSize);
    static bool IsNtagConfigured(eAppNtagError * pNtagError, const char * payload);
    static eAppNtagError AppNtagWrite(const char * payload);
    static eAppNtagError AppNtagEepromUnlockThenWrite(uint8_t originalSize);
    static eAppNtagError AppNtagLockWriteAccess(void);
    static eAppNtagError AppNtagUnlockWriteAccess(void);
    static uint8_t AppNdefUriRecordGetSize(NdefUriRecord_t ndefUriRecord);

    NdefUriRecord_t ndefUriRecord;
    static NFCManagerImpl sInstance;
    NTAG_HANDLE_T ntagDriverHandleInstance;
};

inline NFCManager & NFCMgr()
{
    return NFCManagerImpl::sInstance;
}

inline NFCManagerImpl & NFCMgrImpl()
{
    return NFCManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
