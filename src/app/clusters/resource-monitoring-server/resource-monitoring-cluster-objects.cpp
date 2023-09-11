/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "resource-monitoring-cluster-objects.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

namespace Commands {
namespace ResetCondition {

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVType outer;
    VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(reader.EnterContainer(outer));
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        // check that this is not malformed TLV
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace ResetCondition.
} // namespace Commands

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
