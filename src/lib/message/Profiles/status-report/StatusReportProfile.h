/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file defines an object for reading and writing CHIP
 *      Status Reports.
 *
 *      Status Reports are used in CHIP for over-the-wire status and
 *      error reporting that can act as a notification or confirmation
 *      in response to a request or command.
 *
 *      The format of a status report is:
 *      @code
 *        | <profile ID> | <status code> | <additional status info> |
 *      @endcode
 *
 */

#ifndef _STATUS_REPORT_PROFILE_H
#define _STATUS_REPORT_PROFILE_H

#include <support/DLLUtil.h>

/**
 *   @namespace chip::Profiles::StatusReporting
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP Status Reporting subprofile, which is part of and within
 *     the CHIP Common profile.
 */

namespace chip {
namespace Profiles {
namespace StatusReporting {

/*
 * in-memory, a status report, at its most basic, is a CHIP profile
 * ID and status code pair. then there's the option of chaining on
 * more status information as TLV.
 */

class DLL_EXPORT StatusReport
{
public:
    StatusReport(void);
    ~StatusReport(void);

    CHIP_ERROR init(uint32_t aProfileId, uint16_t aCode, ReferencedTLVData * aInfo = NULL);

    /*
     * this version of the intializer is provided as a convenience in
     * the case where we want to make a status report that reports an
     * internal error.
     */

    CHIP_ERROR init(CHIP_ERROR aError);

    CHIP_ERROR pack(PacketBuffer * aBuffer, uint32_t maxLen = 0xFFFFFFFFUL);
    uint16_t packedLength(void);
    static CHIP_ERROR parse(PacketBuffer * aBuffer, StatusReport & aDestination);

    bool operator==(const StatusReport & another) const;

    bool success(void);

    /*
     * here are some static convenience methods for adding metadata
     */

    static CHIP_ERROR StartMetaData(chip::TLV::TLVWriter & aWriter);
    static CHIP_ERROR EndMetaData(chip::TLV::TLVWriter & aWriter);

    static CHIP_ERROR AddErrorCode(chip::TLV::TLVWriter & aWriter, CHIP_ERROR aError);

    // data members

    uint32_t mProfileId;
    uint16_t mStatusCode;
    CHIP_ERROR mError;
    ReferencedTLVData mAdditionalInfo;
};

} // namespace StatusReporting
} // namespace Profiles
} // namespace chip

#endif // _STATUS_REPORT_PROFILE_H
