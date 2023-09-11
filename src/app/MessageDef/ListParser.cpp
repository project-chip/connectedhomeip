/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ListParser.h"

namespace chip {
namespace app {
CHIP_ERROR ListParser::Init(const TLV::TLVReader & aReader)
{
    mReader.Init(aReader);
    VerifyOrReturnError(TLV::kTLVType_List == mReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    return mReader.EnterContainer(mOuterContainerType);
}
} // namespace app
} // namespace chip
