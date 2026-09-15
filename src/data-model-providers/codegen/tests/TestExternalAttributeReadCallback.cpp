/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <access/SubjectDescriptor.h>
#include <app/util/generic-callbacks.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using chip::Protocols::InteractionModel::Status;

namespace {

EndpointId gEndpoint;
ClusterId gCluster;
const EmberAfAttributeMetadata * gMetadata;
uint16_t gMaxReadLength;
Status gStatus;

} // namespace

// Override only the legacy callback: existing applications must still receive reads.
Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata * metadata,
                                            uint8_t * buffer, uint16_t maxReadLength)
{
    gEndpoint      = endpoint;
    gCluster       = clusterId;
    gMetadata      = metadata;
    gMaxReadLength = maxReadLength;
    buffer[0]      = 42;
    return gStatus;
}

TEST(TestExternalAttributeReadCallback, LegacyOverrideReceivesContextAndLocalReads)
{
    Access::SubjectDescriptor subject;
    subject.authMode                             = Access::AuthMode::kCase;
    subject.fabricIndex                          = 1;
    subject.subject                              = 0x1234;
    const Access::SubjectDescriptor * contexts[] = { &subject, nullptr };
    EmberAfAttributeMetadata metadata            = { EmberAfDefaultOrMinMaxAttributeValue(uint32_t{ 0 }) };

    for (const auto * context : contexts)
    {
        for (Status status : { Status::Success, Status::UnsupportedRead })
        {
            uint8_t buffer[2] = {};
            gStatus           = status;
            EXPECT_EQ(emberAfExternalAttributeReadWithContextCallback(123, 456, &metadata, buffer, sizeof(buffer), context),
                      status);
            EXPECT_EQ(gEndpoint, 123);
            EXPECT_EQ(gCluster, 456u);
            EXPECT_EQ(gMetadata, &metadata);
            EXPECT_EQ(gMaxReadLength, sizeof(buffer));
            EXPECT_EQ(buffer[0], 42);
        }
    }
}
