/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/dnssd/TxtFields.h>

#include <limits>
#include <stdio.h>
#include <string.h>

#include <lib/dnssd/Resolver.h>

#include <gtest/gtest.h>

using namespace chip;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Internal;

namespace {

ByteSpan GetSpan(char * key)
{
    size_t len = strlen(key);
    // Stop the string from being null terminated to ensure the code makes no assumptions.
    key[len] = '1';
    return ByteSpan(reinterpret_cast<uint8_t *>(key), len);
}

TEST(TestTxtFields, TestGetTxtFieldKey)
{
    char key[4];
    strcpy(key, "D");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kLongDiscriminator);

    strcpy(key, "VP");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kVendorProduct);

    strcpy(key, "CM");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kCommissioningMode);

    strcpy(key, "DT");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kDeviceType);

    strcpy(key, "DN");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kDeviceName);

    strcpy(key, "RI");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kRotatingDeviceId);

    strcpy(key, "PI");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kPairingInstruction);

    strcpy(key, "PH");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kPairingHint);

    strcpy(key, "SII");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kSessionIdleInterval);

    strcpy(key, "SAI");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kSessionActiveInterval);

    strcpy(key, "SAT");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kSessionActiveThreshold);

    strcpy(key, "T");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kTcpSupported);

    strcpy(key, "ICD");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kLongIdleTimeICD);

    strcpy(key, "XX");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kUnknown);

    strcpy(key, "CP");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kCommissionerPasscode);
}

TEST(TestTxtFields, TestGetTxtFieldKeyCaseInsensitive)
{
    char key[3];
    strcpy(key, "d");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kLongDiscriminator);

    strcpy(key, "vp");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kVendorProduct);
    strcpy(key, "Vp");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kVendorProduct);
    strcpy(key, "vP");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kVendorProduct);

    strcpy(key, "Xx");
    EXPECT_EQ(GetTxtFieldKey(GetSpan(key)), TxtFieldKey::kUnknown);
}

TEST(TestTxtFields, TestGetProduct)
{
    // Product and vendor are given as part of the same key, on either side of a + sign. Product is after the +
    char vp[64];
    strcpy(vp, "123+456");
    EXPECT_EQ(GetProduct(GetSpan(vp)), 456);

    strcpy(vp, "123+");
    EXPECT_EQ(GetProduct(GetSpan(vp)), 0);

    strcpy(vp, "+456");
    EXPECT_EQ(GetProduct(GetSpan(vp)), 456);

    strcpy(vp, "123");
    EXPECT_EQ(GetProduct(GetSpan(vp)), 0);

    // overflow a uint16
    sprintf(vp, "123+%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    EXPECT_EQ(GetProduct(GetSpan(vp)), 0);
}
TEST(TestTxtFields, TestGetVendor)
{
    // Product and vendor are given as part of the same key, on either side of a + sign. Vendor is first
    char vp[64];
    strcpy(vp, "123+456");
    EXPECT_EQ(GetVendor(GetSpan(vp)), 123);

    strcpy(vp, "123+");
    EXPECT_EQ(GetVendor(GetSpan(vp)), 123);

    strcpy(vp, "+456");
    EXPECT_EQ(GetVendor(GetSpan(vp)), 0);

    strcpy(vp, "123");
    EXPECT_EQ(GetVendor(GetSpan(vp)), 123);

    // overflow a uint16
    sprintf(vp, "%" PRIu32 "+456", static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    EXPECT_EQ(GetVendor(GetSpan(vp)), 0);
}

TEST(TestTxtFields, TestGetLongDiscriminator)
{
    char ld[64];
    strcpy(ld, "1234");
    EXPECT_EQ(GetLongDiscriminator(GetSpan(ld)), 1234);

    // overflow a uint16
    sprintf(ld, "%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    printf("ld = %s\n", ld);
    EXPECT_EQ(GetLongDiscriminator(GetSpan(ld)), 0);
}

TEST(TestTxtFields, TestGetCommissioningMode)
{
    char cm[64];
    strcpy(cm, "0");
    EXPECT_EQ(GetCommissioningMode(GetSpan(cm)), 0);

    strcpy(cm, "1");
    EXPECT_EQ(GetCommissioningMode(GetSpan(cm)), 1);

    strcpy(cm, "2");
    EXPECT_EQ(GetCommissioningMode(GetSpan(cm)), 2);

    // overflow a uint8
    sprintf(cm, "%u", static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) + 1);
    EXPECT_EQ(GetCommissioningMode(GetSpan(cm)), 0);
}

TEST(TestTxtFields, TestGetDeviceType)
{
    char dt[64];
    strcpy(dt, "1234");
    EXPECT_EQ(GetDeviceType(GetSpan(dt)), 1234u);

    // overflow a uint32
    sprintf(dt, "%" PRIu64, static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1);
    EXPECT_EQ(GetDeviceType(GetSpan(dt)), 0u);
}

TEST(TestTxtFields, TestGetDeviceName)
{
    char name[kMaxDeviceNameLen + 1] = "";
    char val[kMaxDeviceNameLen + 2];

    strcpy(val, "testname");
    GetDeviceName(GetSpan(val), name);
    EXPECT_STREQ(name, "testname");

    // If the data passed in is too long, it should truncate the end.
    memset(val, 'a', kMaxDeviceNameLen);
    val[kMaxDeviceNameLen]     = 'b';
    val[kMaxDeviceNameLen + 1] = '\0';
    GetDeviceName(GetSpan(val), name);

    val[kMaxDeviceNameLen] = '\0';
    EXPECT_STREQ(name, val);
}

TEST(TestTxtFields, TestGetRotatingDeviceId)
{
    // Rotating device ID is given as up to 50 hex bytes
    char ri[kMaxRotatingIdLen * 2 + 1];
    uint8_t id[kMaxRotatingIdLen];
    size_t len;

    strcpy(ri, "0A1B");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    printf("id[0] = %x\n", id[0]);
    EXPECT_EQ(id[0], 0x0A);
    EXPECT_EQ(id[1], 0x1B);
    EXPECT_EQ(len, 2u);

    // odd number of characters can't be parsed.
    strcpy(ri, "0A1BC");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    EXPECT_EQ(len, 0u);

    // non-hex characters can't be parsed
    strcpy(ri, "0ATT");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    EXPECT_EQ(len, 0u);

    // Lower case should work on SDK even though devices shouldn't be sending that.
    strcpy(ri, "0a1b");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    EXPECT_EQ(id[0], 0x0A);
    EXPECT_EQ(id[1], 0x1B);
    EXPECT_EQ(len, 2u);

    strcpy(ri, "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F3031");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    EXPECT_EQ(len, sizeof(id));
    for (size_t i = 0; i < sizeof(id); ++i)
    {
        EXPECT_EQ(id[i], i);
    }
}

TEST(TestTxtFields, TestGetPairingHint)
{
    char ph[64];
    strcpy(ph, "0");
    EXPECT_EQ(GetPairingHint(GetSpan(ph)), 0);

    strcpy(ph, "9");
    EXPECT_EQ(GetPairingHint(GetSpan(ph)), 9);

    // overflow a uint16
    sprintf(ph, "%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    EXPECT_EQ(GetPairingHint(GetSpan(ph)), 0);
}

TEST(TestTxtFields, TestGetPairingInstruction)
{
    char data[kMaxPairingInstructionLen + 2];
    char ret[kMaxPairingInstructionLen + 1] = "";

    strcpy(data, "something");
    GetPairingInstruction(GetSpan(data), ret);
    EXPECT_STREQ(ret, "something");

    // Exactly the max len.
    memset(data, 'a', kMaxPairingInstructionLen);
    data[kMaxPairingInstructionLen] = '\0';
    GetPairingInstruction(GetSpan(data), ret);
    // Add back the null terminator removed by GetSpan.
    data[kMaxPairingInstructionLen] = '\0';
    EXPECT_STREQ(data, ret);

    // Too long - should truncate end.
    memset(data, 'a', kMaxPairingInstructionLen);
    data[kMaxPairingInstructionLen]     = 'b';
    data[kMaxPairingInstructionLen + 1] = '\0';
    GetPairingInstruction(GetSpan(data), ret);
    data[kMaxPairingInstructionLen] = '\0';
    EXPECT_STREQ(ret, data);
}

TEST(TestTxtFields, TestGetCommissionerPasscode)
{
    char cm[64];
    strcpy(cm, "0");
    EXPECT_EQ(GetCommissionerPasscode(GetSpan(cm)), 0);

    strcpy(cm, "1");
    EXPECT_EQ(GetCommissionerPasscode(GetSpan(cm)), 1);

    // overflow a uint8
    sprintf(cm, "%u", static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) + 1);
    EXPECT_EQ(GetCommissionerPasscode(GetSpan(cm)), 0);
}

bool NodeDataIsEmpty(const DiscoveredNodeData & node)
{

    if (node.nodeData.longDiscriminator != 0 || node.nodeData.vendorId != 0 || node.nodeData.productId != 0 ||
        node.nodeData.commissioningMode != 0 || node.nodeData.deviceType != 0 || node.nodeData.rotatingIdLen != 0 ||
        node.nodeData.pairingHint != 0 || node.resolutionData.mrpRetryIntervalIdle.HasValue() ||
        node.resolutionData.mrpRetryIntervalActive.HasValue() || node.resolutionData.mrpRetryActiveThreshold.HasValue() ||
        node.resolutionData.isICDOperatingAsLIT.HasValue() || node.resolutionData.supportsTcp ||
        node.nodeData.supportsCommissionerGeneratedPasscode != 0)
    {
        return false;
    }
    if (strcmp(node.nodeData.deviceName, "") != 0 || strcmp(node.nodeData.pairingInstruction, "") != 0)
    {
        return false;
    }
    for (uint8_t id : node.nodeData.rotatingId)
    {
        if (id != 0)
        {
            return false;
        }
    }
    return true;
}

// The individual fill tests test the error cases for each key type, this test is used to ensure the proper record is filled.
TEST(TestTxtFields, TestFillDiscoveredNodeDataFromTxt)
{
    char key[3];
    char val[16];
    DiscoveredNodeData filled;

    // Long discriminator
    strcpy(key, "D");
    strcpy(val, "840");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_EQ(filled.nodeData.longDiscriminator, 840);
    filled.nodeData.longDiscriminator = 0;
    EXPECT_TRUE(NodeDataIsEmpty(filled));

    // vendor and product
    strcpy(key, "VP");
    strcpy(val, "123+456");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_EQ(filled.nodeData.vendorId, 123);
    EXPECT_EQ(filled.nodeData.productId, 456);
    filled.nodeData.vendorId  = 0;
    filled.nodeData.productId = 0;
    EXPECT_TRUE(NodeDataIsEmpty(filled));

    // Commissioning mode
    strcpy(key, "CM");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_EQ(filled.nodeData.commissioningMode, 1);
    filled.nodeData.commissioningMode = 0;
    EXPECT_TRUE(NodeDataIsEmpty(filled));

    // Supports Commissioner Generated Passcode
    strcpy(key, "CP");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_TRUE(filled.nodeData.supportsCommissionerGeneratedPasscode);
    filled.nodeData.supportsCommissionerGeneratedPasscode = false;
    EXPECT_TRUE(NodeDataIsEmpty(filled));

    // Device type
    strcpy(key, "DT");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_EQ(filled.nodeData.deviceType, 1u);
    filled.nodeData.deviceType = 0;
    EXPECT_TRUE(NodeDataIsEmpty(filled));

    // Device name
    strcpy(key, "DN");
    strcpy(val, "abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_STREQ(filled.nodeData.deviceName, "abc");
    memset(filled.nodeData.deviceName, 0, sizeof(filled.nodeData.deviceName));
    EXPECT_TRUE(NodeDataIsEmpty(filled));

    // Rotating device id
    strcpy(key, "RI");
    strcpy(val, "1A2B");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_EQ(filled.nodeData.rotatingId[0], 0x1A);
    EXPECT_EQ(filled.nodeData.rotatingId[1], 0x2B);
    EXPECT_EQ(filled.nodeData.rotatingIdLen, 2u);
    filled.nodeData.rotatingIdLen = 0;
    memset(filled.nodeData.rotatingId, 0, sizeof(filled.nodeData.rotatingId));
    EXPECT_TRUE(NodeDataIsEmpty(filled));

    // Pairing instruction
    strcpy(key, "PI");
    strcpy(val, "hint");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_STREQ(filled.nodeData.pairingInstruction, "hint");
    memset(filled.nodeData.pairingInstruction, 0, sizeof(filled.nodeData.pairingInstruction));
    EXPECT_TRUE(NodeDataIsEmpty(filled));

    // Pairing hint
    strcpy(key, "PH");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    EXPECT_EQ(filled.nodeData.pairingHint, 1);
    filled.nodeData.pairingHint = 0;
    EXPECT_TRUE(NodeDataIsEmpty(filled));
}

bool NodeDataIsEmpty(const ResolvedNodeData & nodeData)
{
    return nodeData.operationalData.peerId == PeerId{} && nodeData.resolutionData.numIPs == 0 &&
        nodeData.resolutionData.port == 0 && !nodeData.resolutionData.mrpRetryIntervalIdle.HasValue() &&
        !nodeData.resolutionData.mrpRetryIntervalActive.HasValue() && !nodeData.resolutionData.supportsTcp &&
        !nodeData.resolutionData.isICDOperatingAsLIT.HasValue();
}

void ResetRetryIntervalIdle(DiscoveredNodeData & nodeData)
{
    nodeData.resolutionData.mrpRetryIntervalIdle.ClearValue();
}

void ResetRetryIntervalIdle(ResolvedNodeData & nodeData)
{
    nodeData.resolutionData.mrpRetryIntervalIdle.ClearValue();
}

void ResetRetryIntervalActive(DiscoveredNodeData & nodeData)
{
    nodeData.resolutionData.mrpRetryIntervalActive.ClearValue();
}

void ResetRetryIntervalActive(ResolvedNodeData & nodeData)
{
    nodeData.resolutionData.mrpRetryIntervalActive.ClearValue();
}

void ResetRetryActiveThreshold(DiscoveredNodeData & nodeData)
{
    nodeData.resolutionData.mrpRetryActiveThreshold.ClearValue();
}

void ResetRetryActiveThreshold(ResolvedNodeData & nodeData)
{
    nodeData.resolutionData.mrpRetryActiveThreshold.ClearValue();
}

// Test SAI (formally CRI)
template <class NodeData>
void TxtFieldSessionIdleInterval()
{
    char key[4];
    char val[16];
    NodeData nodeData;

    // Minimum
    strcpy(key, "SII");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());
    EXPECT_EQ(nodeData.resolutionData.GetMrpRetryIntervalIdle().Value(), 1_ms32);

    // Maximum
    strcpy(key, "SII");
    strcpy(val, "3600000");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());
    EXPECT_EQ(nodeData.resolutionData.GetMrpRetryIntervalIdle().Value(), 3600000_ms32);

    // Test no other fields were populated
    ResetRetryIntervalIdle(nodeData);
    EXPECT_TRUE(NodeDataIsEmpty(nodeData));

    // Invalid SII - negative value
    strcpy(key, "SII");
    strcpy(val, "-1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - greater than maximum
    strcpy(key, "SII");
    strcpy(val, "3600001");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - much greater than maximum
    strcpy(key, "SII");
    strcpy(val, "1095216660481"); // 0xFF00000001 == 1 (mod 2^32)
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - hexadecimal value
    strcpy(key, "SII");
    strcpy(val, "0x20");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - leading zeros
    strcpy(key, "SII");
    strcpy(val, "0700");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - text at the end
    strcpy(key, "SII");
    strcpy(val, "123abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());
}

TEST(TestTxtFields, TxtDiscoveredFieldMrpRetryIntervalIdle)
{
    TxtFieldSessionIdleInterval<DiscoveredNodeData>();
}

TEST(TestTxtFields, TxtResolvedFieldMrpRetryIntervalIdle)
{
    TxtFieldSessionIdleInterval<ResolvedNodeData>();
}

// Test SAI (formerly CRA)
template <class NodeData>
void TxtFieldSessionActiveInterval()
{
    char key[4];
    char val[16];
    NodeData nodeData;

    // Minimum
    strcpy(key, "SAI");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());
    EXPECT_EQ(nodeData.resolutionData.GetMrpRetryIntervalActive().Value(), 1_ms32);

    // Maximum
    strcpy(key, "SAI");
    strcpy(val, "3600000");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());
    EXPECT_EQ(nodeData.resolutionData.GetMrpRetryIntervalActive().Value(), 3600000_ms32);

    // Test no other fields were populated
    ResetRetryIntervalActive(nodeData);
    EXPECT_TRUE(NodeDataIsEmpty(nodeData));

    // Invalid SAI - negative value
    strcpy(key, "SAI");
    strcpy(val, "-1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - greater than maximum
    strcpy(key, "SAI");
    strcpy(val, "3600001");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - much greater than maximum
    strcpy(key, "SAI");
    strcpy(val, "1095216660481"); // 0xFF00000001 == 1 (mod 2^32)
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - hexadecimal value
    strcpy(key, "SAI");
    strcpy(val, "0x20");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - leading zeros
    strcpy(key, "SAI");
    strcpy(val, "0700");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - text at the end
    strcpy(key, "SAI");
    strcpy(val, "123abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());
}

TEST(TestTxtFields, TxtDiscoveredFieldMrpRetryIntervalActive)
{
    TxtFieldSessionActiveInterval<DiscoveredNodeData>();
}

TEST(TestTxtFields, TxtResolvedFieldMrpRetryIntervalActive)
{
    TxtFieldSessionActiveInterval<ResolvedNodeData>();
}

// Test SAT (Session Active Threshold)
template <class NodeData>
void TxtFieldSessionActiveThreshold()
{
    char key[4];
    char val[16];
    NodeData nodeData;

    // Minimum
    strcpy(key, "SAT");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());
    EXPECT_EQ(nodeData.resolutionData.GetMrpRetryActiveThreshold().Value(), 1_ms16);

    // Maximum
    strcpy(key, "SAT");
    strcpy(val, "65535");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());
    EXPECT_EQ(nodeData.resolutionData.GetMrpRetryActiveThreshold().Value(), 65535_ms16);

    // Test no other fields were populated
    ResetRetryActiveThreshold(nodeData);
    EXPECT_TRUE(NodeDataIsEmpty(nodeData));

    // Invalid SAI - negative value
    strcpy(key, "SAT");
    strcpy(val, "-1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAI - greater than maximum
    strcpy(key, "SAT");
    strcpy(val, "65536");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAT - much greater than maximum
    strcpy(key, "SAT");
    strcpy(val, "1095216660481"); // 0xFF00000001 == 1 (mod 2^32)
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAT - hexadecimal value
    strcpy(key, "SAT");
    strcpy(val, "0x20");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAT - leading zeros
    strcpy(key, "SAT");
    strcpy(val, "0700");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAT - text at the end
    strcpy(key, "SAT");
    strcpy(val, "123abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());
}

TEST(TestTxtFields, TxtDiscoveredFieldMrpRetryActiveThreshold)
{
    TxtFieldSessionActiveThreshold<DiscoveredNodeData>();
}

TEST(TestTxtFields, TxtResolvedFieldMrpRetryActiveThreshold)
{
    TxtFieldSessionActiveThreshold<ResolvedNodeData>();
}

// Test T (TCP support)
template <class NodeData>
void TxtFieldTcpSupport()
{
    char key[4];
    char val[8];
    NodeData nodeData;

    // True
    strcpy(key, "T");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.supportsTcp);

    // Test no other fields were populated
    nodeData.resolutionData.supportsTcp = false;
    EXPECT_TRUE(NodeDataIsEmpty(nodeData));

    // False
    strcpy(key, "T");
    strcpy(val, "0");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_EQ(nodeData.resolutionData.supportsTcp, false);

    // Invalid value, stil false
    strcpy(key, "T");
    strcpy(val, "asdf");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_EQ(nodeData.resolutionData.supportsTcp, false);
}

TEST(TestTxtFields, TxtDiscoveredFieldTcpSupport)
{
    TxtFieldTcpSupport<DiscoveredNodeData>();
}

TEST(TestTxtFields, TxtResolvedFieldTcpSupport)
{
    TxtFieldTcpSupport<ResolvedNodeData>();
}

// Test ICD (ICD operation Mode)
template <class NodeData>
void TxtFieldICDoperatesAsLIT()
{
    char key[4];
    char val[16];
    NodeData nodeData;

    // ICD is operating as a LIT device
    strcpy(key, "ICD");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.isICDOperatingAsLIT.HasValue());
    EXPECT_TRUE(nodeData.resolutionData.isICDOperatingAsLIT.Value());

    // Test no other fields were populated
    nodeData.resolutionData.isICDOperatingAsLIT.ClearValue();
    EXPECT_TRUE(NodeDataIsEmpty(nodeData));

    // ICD is operating as a SIT device
    strcpy(key, "ICD");
    strcpy(val, "0");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.isICDOperatingAsLIT.HasValue());
    EXPECT_EQ(nodeData.resolutionData.isICDOperatingAsLIT.Value(), false);

    nodeData.resolutionData.isICDOperatingAsLIT.ClearValue();
    EXPECT_TRUE(NodeDataIsEmpty(nodeData));
    // Invalid value, No key set
    strcpy(key, "ICD");
    strcpy(val, "asdf");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_EQ(nodeData.resolutionData.isICDOperatingAsLIT.HasValue(), false);
}

TEST(TestTxtFields, TxtDiscoveredIsICDoperatingAsLIT)
{
    TxtFieldICDoperatesAsLIT<DiscoveredNodeData>();
}

TEST(TestTxtFields, TxtResolvedFieldICDoperatingAsLIT)
{
    TxtFieldICDoperatesAsLIT<ResolvedNodeData>();
}

// Test IsDeviceTreatedAsSleepy() with CRI
template <class NodeData>
void TestIsDeviceSessionIdle()
{
    char key[4];
    char val[32];
    NodeData nodeData;
    const ReliableMessageProtocolConfig defaultMRPConfig(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL,
                                                         CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL);

    // No key/val set, so the device can't be sleepy
    EXPECT_FALSE(nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));

    // If the interval is the default value, the device is not sleepy
    strcpy(key, "SII");
    sprintf(val, "%d", static_cast<int>(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL.count()));
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));

    // If the interval is greater than the default value, the device is sleepy
    sprintf(key, "SII");
    sprintf(val, "%d", static_cast<int>(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL.count() + 1));
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));
}

TEST(TestTxtFields, TxtDiscoveredIsDeviceSessionIdle)
{
    TestIsDeviceSessionIdle<DiscoveredNodeData>();
}

TEST(TestTxtFields, TxtResolvedIsDeviceSessionIdle)
{
    TestIsDeviceSessionIdle<ResolvedNodeData>();
}

// Test IsDeviceTreatedAsSleepy() with CRA
template <class NodeData>
void TestIsDeviceSessionActive()
{
    char key[4];
    char val[32];
    NodeData nodeData;
    const ReliableMessageProtocolConfig defaultMRPConfig(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL,
                                                         CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL);

    // No key/val set, so the device can't be sleepy
    EXPECT_FALSE(nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));

    // If the interval is the default value, the device is not sleepy
    sprintf(key, "SAI");
    sprintf(val, "%d", static_cast<int>(CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL.count()));
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_FALSE(nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));

    // If the interval is greater than the default value, the device is sleepy
    strcpy(key, "SAI");
    sprintf(val, "%d", static_cast<int>(CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL.count() + 1));
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    EXPECT_TRUE(nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));
}

TEST(TestTxtFields, TxtDiscoveredIsDeviceSessionActive)
{
    TestIsDeviceSessionActive<DiscoveredNodeData>();
}

TEST(TestTxtFields, TxtResolvedIsDeviceSessionActive)
{
    TestIsDeviceSessionActive<ResolvedNodeData>();
}

} // namespace
