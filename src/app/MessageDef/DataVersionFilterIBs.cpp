/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "DataVersionFilterIBs.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR DataVersionFilterIBs::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("DataVersionFilterIBs =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        {
            DataVersionFilterIB::Parser DataVersionFilter;
            ReturnErrorOnFailure(DataVersionFilter.Init(reader));
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(DataVersionFilter.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT_BLANK_LINE();

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

DataVersionFilterIB::Builder & DataVersionFilterIBs::Builder::CreateDataVersionFilter()
{
    mError = mDataVersionFilter.Init(mpWriter);
    return mDataVersionFilter;
}

CHIP_ERROR DataVersionFilterIBs::Builder::EncodeDataVersionFilterIB(const DataVersionFilter & aFilter)
{
    DataVersionFilterIB::Builder & filterIB = CreateDataVersionFilter();
    ReturnErrorOnFailure(GetError());
    ClusterPathIB::Builder & path = filterIB.CreatePath();
    ReturnErrorOnFailure(filterIB.GetError());
    ReturnErrorOnFailure(path.Endpoint(aFilter.mEndpointId).Cluster(aFilter.mClusterId).EndOfClusterPathIB());
    ReturnErrorOnFailure(filterIB.DataVersion(aFilter.mDataVersion.Value()).EndOfDataVersionFilterIB());

    ChipLogProgress(DataManagement, "Encoded DataVersionFilter: Endpoint=%u Cluster=" ChipLogFormatMEI " Version=%" PRIu32,
                    aFilter.mEndpointId, ChipLogValueMEI(aFilter.mClusterId), aFilter.mDataVersion.Value());

    return CHIP_NO_ERROR;
}

CHIP_ERROR DataVersionFilterIBs::Builder::EndOfDataVersionFilterIBs()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip
