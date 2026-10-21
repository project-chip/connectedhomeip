/*
 * Copyright (c) 2026 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../PQCDeviceAttestationProfileReadOverride.h"

#include <app/data-model-provider/tests/ReadTesting.h>
#include <clusters/OperationalCredentials/AttributeIds.h>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

void ExpectRead(PQCDeviceAttestationProfileReadOverride & override, const ConcreteAttributePath & path, CHIP_ERROR expected)
{
    Testing::ReadOperation operation(path);
    auto encoder = operation.StartEncoding();
    ASSERT_NE(encoder, nullptr);
    EXPECT_EQ(override.Read(ConcreteReadAttributePath(path.mEndpointId, path.mClusterId, path.mAttributeId), *encoder), expected);
    EXPECT_FALSE(encoder->TriedEncode());
    EXPECT_EQ(operation.FinishEncoding(), CHIP_NO_ERROR);
}

const ConcreteAttributePath kProfilePath(kRootEndpointId, OperationalCredentials::Id,
                                         OperationalCredentials::Attributes::PQCDeviceAttestationProfile::Id);

TEST(TestPQCDeviceAttestationProfileReadOverride, FaultsAndRestoration)
{
    PQCDeviceAttestationProfileReadOverride override;
    ExpectRead(override, kProfilePath, CHIP_NO_ERROR);
    ASSERT_EQ(override.SetReadMode("UnsupportedAttribute"_span), CHIP_NO_ERROR);
    ExpectRead(override, kProfilePath, CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    ASSERT_EQ(override.SetReadMode("Failure"_span), CHIP_NO_ERROR);
    ExpectRead(override, kProfilePath, CHIP_IM_GLOBAL_STATUS(Failure));
    ASSERT_EQ(override.SetReadMode("Normal"_span), CHIP_NO_ERROR);
    ExpectRead(override, kProfilePath, CHIP_NO_ERROR);
}

TEST(TestPQCDeviceAttestationProfileReadOverride, InvalidModePreservesFault)
{
    PQCDeviceAttestationProfileReadOverride override;
    ASSERT_EQ(override.SetReadMode("Failure"_span), CHIP_NO_ERROR);
    EXPECT_EQ(override.SetReadMode(""_span), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(override.SetReadMode("Success"_span), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(override.SetReadMode("Normal\0Failure"_span), CHIP_ERROR_INVALID_ARGUMENT);
    ExpectRead(override, kProfilePath, CHIP_IM_GLOBAL_STATUS(Failure));
}

TEST(TestPQCDeviceAttestationProfileReadOverride, OtherPathsFallThrough)
{
    PQCDeviceAttestationProfileReadOverride override;
    ASSERT_EQ(override.SetReadMode("Failure"_span), CHIP_NO_ERROR);
    ExpectRead(
        override,
        ConcreteAttributePath(kRootEndpointId, OperationalCredentials::Id, OperationalCredentials::Attributes::FeatureMap::Id),
        CHIP_NO_ERROR);
    ExpectRead(override, ConcreteAttributePath(1, kProfilePath.mClusterId, kProfilePath.mAttributeId), CHIP_NO_ERROR);
    ExpectRead(override, ConcreteAttributePath(kRootEndpointId, 0xFFFF, kProfilePath.mAttributeId), CHIP_NO_ERROR);
}

} // namespace
