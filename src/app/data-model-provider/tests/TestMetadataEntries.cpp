/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataTypes.h>

#include <access/Privilege.h>

#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;

namespace {

constexpr AttributeQualityFlags kAllAttributeQualities[] = {
    AttributeQualityFlags::kListAttribute,   //
    AttributeQualityFlags::kFabricScoped,    //
    AttributeQualityFlags::kFabricSensitive, //
    AttributeQualityFlags::kChangesOmitted,  //
    AttributeQualityFlags::kTimed,
};

constexpr CommandQualityFlags kAllCommandQualities[] = {
    CommandQualityFlags::kFabricScoped, //
    CommandQualityFlags::kTimed,        //
    CommandQualityFlags::kLargeMessage, //
};

constexpr Access::Privilege kAllPrivileges[] = {
    Access::Privilege::kView,      //
    Access::Privilege::kProxyView, //
    Access::Privilege::kOperate,   //
    Access::Privilege::kManage,    //
    Access::Privilege::kAdminister,
};

} // namespace

TEST(TestMetadataEntries, TestAttributeQualityEncoding)
{
    // Test no options set
    {
        AttributeEntry a(123, {}, std::nullopt, std::nullopt);

        EXPECT_EQ(a.attributeId, 123u);

        EXPECT_FALSE(a.HasFlags(AttributeQualityFlags::kListAttribute));
        EXPECT_FALSE(a.HasFlags(AttributeQualityFlags::kFabricScoped));
        EXPECT_FALSE(a.HasFlags(AttributeQualityFlags::kFabricSensitive));
        EXPECT_FALSE(a.HasFlags(AttributeQualityFlags::kChangesOmitted));
        EXPECT_FALSE(a.HasFlags(AttributeQualityFlags::kTimed));
    }

    for (auto flag : kAllAttributeQualities)
    {
        AttributeEntry a(123, flag, std::nullopt, std::nullopt);

        for (auto test : kAllAttributeQualities)
        {
            if (test == flag)
            {
                EXPECT_TRUE(a.HasFlags(test));
            }
            else
            {
                EXPECT_FALSE(a.HasFlags(test));
            }
        }
    }

    // Test multi-option set
    {
        AttributeEntry a(123, { AttributeQualityFlags::kFabricSensitive, AttributeQualityFlags::kTimed }, std::nullopt,
                         std::nullopt);

        EXPECT_FALSE(a.HasFlags(AttributeQualityFlags::kListAttribute));
        EXPECT_FALSE(a.HasFlags(AttributeQualityFlags::kFabricScoped));
        EXPECT_TRUE(a.HasFlags(AttributeQualityFlags::kFabricSensitive));
        EXPECT_FALSE(a.HasFlags(AttributeQualityFlags::kChangesOmitted));
        EXPECT_TRUE(a.HasFlags(AttributeQualityFlags::kTimed));
    }
}

TEST(TestMetadataEntries, TestAttributePrivilegeEncoding)
{
    for (auto rp : kAllPrivileges)
    {
        for (auto wp : kAllPrivileges)
        {
            AttributeEntry a(123, {}, rp, wp);

            // assert that we can set the read and write privilege
            EXPECT_TRUE(a.GetReadPrivilege().has_value());
            EXPECT_EQ(a.GetReadPrivilege(), rp);

            EXPECT_TRUE(a.GetWritePrivilege().has_value());
            EXPECT_EQ(a.GetWritePrivilege(), wp);
        }

        // test with one privilege unset
        AttributeEntry a_ro(123, {}, rp, std::nullopt);
        EXPECT_FALSE(a_ro.GetWritePrivilege().has_value());
        EXPECT_EQ(a_ro.GetWritePrivilege(), std::nullopt);
        EXPECT_TRUE(a_ro.GetReadPrivilege().has_value());
        EXPECT_EQ(a_ro.GetReadPrivilege(), rp);

        AttributeEntry a_wo(123, {}, std::nullopt, rp);
        EXPECT_TRUE(a_wo.GetWritePrivilege().has_value());
        EXPECT_EQ(a_wo.GetWritePrivilege(), rp);
        EXPECT_FALSE(a_wo.GetReadPrivilege().has_value());
        EXPECT_EQ(a_wo.GetReadPrivilege(), std::nullopt);
    }
}

TEST(TestMetadataEntries, TestCommandEntry)
{
    {
        AcceptedCommandEntry c(123, {}, Access::Privilege::kView);

        EXPECT_EQ(c.commandId, 123u);
        EXPECT_FALSE(c.HasFlags(CommandQualityFlags::kFabricScoped));
        EXPECT_FALSE(c.HasFlags(CommandQualityFlags::kTimed));
        EXPECT_FALSE(c.HasFlags(CommandQualityFlags::kLargeMessage));
        EXPECT_EQ(c.GetInvokePrivilege(), Access::Privilege::kView);
    }

    for (auto p : kAllPrivileges)
    {
        for (auto q : kAllCommandQualities)
        {
            AcceptedCommandEntry c(1, q, p);

            for (auto test_quality : kAllCommandQualities)
            {
                if (test_quality == q)
                {
                    EXPECT_TRUE(c.HasFlags(test_quality));
                }
                else
                {
                    EXPECT_FALSE(c.HasFlags(test_quality));
                }
            }

            EXPECT_EQ(c.GetInvokePrivilege(), p);
        }
    }
}
