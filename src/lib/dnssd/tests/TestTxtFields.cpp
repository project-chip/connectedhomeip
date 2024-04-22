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
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

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

void TestGetTxtFieldKey(nlTestSuite * inSuite, void * inContext)
{
    char key[4];
    strcpy(key, "D");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kLongDiscriminator);

    strcpy(key, "VP");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kVendorProduct);

    strcpy(key, "CM");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kCommissioningMode);

    strcpy(key, "DT");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kDeviceType);

    strcpy(key, "DN");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kDeviceName);

    strcpy(key, "RI");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kRotatingDeviceId);

    strcpy(key, "PI");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kPairingInstruction);

    strcpy(key, "PH");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kPairingHint);

    strcpy(key, "SII");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kSessionIdleInterval);

    strcpy(key, "SAI");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kSessionActiveInterval);

    strcpy(key, "SAT");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kSessionActiveThreshold);

    strcpy(key, "T");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kTcpSupported);

    strcpy(key, "ICD");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kLongIdleTimeICD);

    strcpy(key, "XX");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kUnknown);

    strcpy(key, "CP");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kCommissionerPasscode);
}

void TestGetTxtFieldKeyCaseInsensitive(nlTestSuite * inSuite, void * inContext)
{
    char key[3];
    strcpy(key, "d");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kLongDiscriminator);

    strcpy(key, "vp");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kVendorProduct);
    strcpy(key, "Vp");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kVendorProduct);
    strcpy(key, "vP");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kVendorProduct);

    strcpy(key, "Xx");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kUnknown);
}

void TestGetProduct(nlTestSuite * inSuite, void * inContext)
{
    // Product and vendor are given as part of the same key, on either side of a + sign. Product is after the +
    char vp[64];
    strcpy(vp, "123+456");
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 456);

    strcpy(vp, "123+");
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 0);

    strcpy(vp, "+456");
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 456);

    strcpy(vp, "123");
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 0);

    // overflow a uint16
    sprintf(vp, "123+%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 0);
}
void TestGetVendor(nlTestSuite * inSuite, void * inContext)
{
    // Product and vendor are given as part of the same key, on either side of a + sign. Vendor is first
    char vp[64];
    strcpy(vp, "123+456");
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 123);

    strcpy(vp, "123+");
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 123);

    strcpy(vp, "+456");
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 0);

    strcpy(vp, "123");
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 123);

    // overflow a uint16
    sprintf(vp, "%" PRIu32 "+456", static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 0);
}

void TestGetLongDiscriminator(nlTestSuite * inSuite, void * inContext)
{
    char ld[64];
    strcpy(ld, "1234");
    NL_TEST_ASSERT(inSuite, GetLongDiscriminator(GetSpan(ld)) == 1234);

    // overflow a uint16
    sprintf(ld, "%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    printf("ld = %s\n", ld);
    NL_TEST_ASSERT(inSuite, GetLongDiscriminator(GetSpan(ld)) == 0);
}

void TestGetCommissioningMode(nlTestSuite * inSuite, void * inContext)
{
    char cm[64];
    strcpy(cm, "0");
    NL_TEST_ASSERT(inSuite, GetCommissioningMode(GetSpan(cm)) == 0);

    strcpy(cm, "1");
    NL_TEST_ASSERT(inSuite, GetCommissioningMode(GetSpan(cm)) == 1);

    strcpy(cm, "2");
    NL_TEST_ASSERT(inSuite, GetCommissioningMode(GetSpan(cm)) == 2);

    // overflow a uint8
    sprintf(cm, "%u", static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetCommissioningMode(GetSpan(cm)) == 0);
}

void TestGetDeviceType(nlTestSuite * inSuite, void * inContext)
{
    char dt[64];
    strcpy(dt, "1234");
    NL_TEST_ASSERT(inSuite, GetDeviceType(GetSpan(dt)) == 1234);

    // overflow a uint32
    sprintf(dt, "%" PRIu64, static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetDeviceType(GetSpan(dt)) == 0);
}

void TestGetDeviceName(nlTestSuite * inSuite, void * inContext)
{
    char name[kMaxDeviceNameLen + 1] = "";
    char val[kMaxDeviceNameLen + 2];

    strcpy(val, "testname");
    GetDeviceName(GetSpan(val), name);
    NL_TEST_ASSERT(inSuite, strcmp(name, "testname") == 0);

    // If the data passed in is too long, it should truncate the end.
    memset(val, 'a', kMaxDeviceNameLen);
    val[kMaxDeviceNameLen]     = 'b';
    val[kMaxDeviceNameLen + 1] = '\0';
    GetDeviceName(GetSpan(val), name);

    val[kMaxDeviceNameLen] = '\0';
    NL_TEST_ASSERT(inSuite, strcmp(name, val) == 0);
}

void TestGetRotatingDeviceId(nlTestSuite * inSuite, void * inContext)
{
    // Rotating device ID is given as up to 50 hex bytes
    char ri[kMaxRotatingIdLen * 2 + 1];
    uint8_t id[kMaxRotatingIdLen];
    size_t len;

    strcpy(ri, "0A1B");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    printf("id[0] = %x\n", id[0]);
    NL_TEST_ASSERT(inSuite, id[0] == 0x0A);
    NL_TEST_ASSERT(inSuite, id[1] == 0x1B);
    NL_TEST_ASSERT(inSuite, len == 2);

    // odd number of characters can't be parsed.
    strcpy(ri, "0A1BC");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    NL_TEST_ASSERT(inSuite, len == 0);

    // non-hex characters can't be parsed
    strcpy(ri, "0ATT");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    NL_TEST_ASSERT(inSuite, len == 0);

    // Lower case should work on SDK even though devices shouldn't be sending that.
    strcpy(ri, "0a1b");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    NL_TEST_ASSERT(inSuite, id[0] == 0x0A);
    NL_TEST_ASSERT(inSuite, id[1] == 0x1B);
    NL_TEST_ASSERT(inSuite, len == 2);

    strcpy(ri, "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F3031");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    NL_TEST_ASSERT(inSuite, len == sizeof(id));
    for (size_t i = 0; i < sizeof(id); ++i)
    {
        NL_TEST_ASSERT(inSuite, id[i] == i);
    }
}

void TestGetPairingHint(nlTestSuite * inSuite, void * inContext)
{
    char ph[64];
    strcpy(ph, "0");
    NL_TEST_ASSERT(inSuite, GetPairingHint(GetSpan(ph)) == 0);

    strcpy(ph, "9");
    NL_TEST_ASSERT(inSuite, GetPairingHint(GetSpan(ph)) == 9);

    // overflow a uint16
    sprintf(ph, "%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetPairingHint(GetSpan(ph)) == 0);
}

void TestGetPairingInstruction(nlTestSuite * inSuite, void * inContext)
{
    char data[kMaxPairingInstructionLen + 2];
    char ret[kMaxPairingInstructionLen + 1] = "";

    strcpy(data, "something");
    GetPairingInstruction(GetSpan(data), ret);
    NL_TEST_ASSERT(inSuite, strcmp(ret, "something") == 0);

    // Exactly the max len.
    memset(data, 'a', kMaxPairingInstructionLen);
    data[kMaxPairingInstructionLen] = '\0';
    GetPairingInstruction(GetSpan(data), ret);
    // Add back the null terminator removed by GetSpan.
    data[kMaxPairingInstructionLen] = '\0';
    NL_TEST_ASSERT(inSuite, strcmp(data, ret) == 0);

    // Too long - should truncate end.
    memset(data, 'a', kMaxPairingInstructionLen);
    data[kMaxPairingInstructionLen]     = 'b';
    data[kMaxPairingInstructionLen + 1] = '\0';
    GetPairingInstruction(GetSpan(data), ret);
    data[kMaxPairingInstructionLen] = '\0';
    NL_TEST_ASSERT(inSuite, strcmp(ret, data) == 0);
}

void TestGetCommissionerPasscode(nlTestSuite * inSuite, void * inContext)
{
    char cm[64];
    strcpy(cm, "0");
    NL_TEST_ASSERT(inSuite, GetCommissionerPasscode(GetSpan(cm)) == 0);

    strcpy(cm, "1");
    NL_TEST_ASSERT(inSuite, GetCommissionerPasscode(GetSpan(cm)) == 1);

    // overflow a uint8
    sprintf(cm, "%u", static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetCommissionerPasscode(GetSpan(cm)) == 0);
}

bool NodeDataIsEmpty(const DiscoveredNodeData & node)
{

    if (node.nodeData.longDiscriminator != 0 || node.nodeData.vendorId != 0 || node.nodeData.productId != 0 ||
        node.nodeData.commissioningMode != 0 || node.nodeData.deviceType != 0 || node.nodeData.rotatingIdLen != 0 ||
        node.nodeData.pairingHint != 0 || node.resolutionData.mrpRetryIntervalIdle.HasValue() ||
        node.resolutionData.mrpRetryIntervalActive.HasValue() || node.resolutionData.mrpRetryActiveThreshold.HasValue() ||
        node.resolutionData.isICDOperatingAsLIT.HasValue() || node.resolutionData.supportsTcp ||
        node.nodeData.commissionerPasscode != 0)
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
void TestFillDiscoveredNodeDataFromTxt(nlTestSuite * inSuite, void * inContext)
{
    char key[3];
    char val[16];
    DiscoveredNodeData filled;

    // Long discriminator
    strcpy(key, "D");
    strcpy(val, "840");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, filled.nodeData.longDiscriminator == 840);
    filled.nodeData.longDiscriminator = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // vendor and product
    strcpy(key, "VP");
    strcpy(val, "123+456");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, filled.nodeData.vendorId == 123);
    NL_TEST_ASSERT(inSuite, filled.nodeData.productId == 456);
    filled.nodeData.vendorId  = 0;
    filled.nodeData.productId = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Commissioning mode
    strcpy(key, "CM");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, filled.nodeData.commissioningMode == 1);
    filled.nodeData.commissioningMode = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Commissioning mode
    strcpy(key, "CP");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, filled.nodeData.commissionerPasscode == 1);
    filled.nodeData.commissionerPasscode = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Device type
    strcpy(key, "DT");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, filled.nodeData.deviceType == 1);
    filled.nodeData.deviceType = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Device name
    strcpy(key, "DN");
    strcpy(val, "abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, strcmp(filled.nodeData.deviceName, "abc") == 0);
    memset(filled.nodeData.deviceName, 0, sizeof(filled.nodeData.deviceName));
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Rotating device id
    strcpy(key, "RI");
    strcpy(val, "1A2B");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, filled.nodeData.rotatingId[0] == 0x1A);
    NL_TEST_ASSERT(inSuite, filled.nodeData.rotatingId[1] == 0x2B);
    NL_TEST_ASSERT(inSuite, filled.nodeData.rotatingIdLen == 2);
    filled.nodeData.rotatingIdLen = 0;
    memset(filled.nodeData.rotatingId, 0, sizeof(filled.nodeData.rotatingId));
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Pairing instruction
    strcpy(key, "PI");
    strcpy(val, "hint");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, strcmp(filled.nodeData.pairingInstruction, "hint") == 0);
    memset(filled.nodeData.pairingInstruction, 0, sizeof(filled.nodeData.pairingInstruction));
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Pairing hint
    strcpy(key, "PH");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), filled.nodeData);
    NL_TEST_ASSERT(inSuite, filled.nodeData.pairingHint == 1);
    filled.nodeData.pairingHint = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));
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
void TxtFieldSessionIdleInterval(nlTestSuite * inSuite, void * inContext)
{
    char key[4];
    char val[16];
    NodeData nodeData;

    // Minimum
    strcpy(key, "SII");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalIdle().Value() == 1_ms32);

    // Maximum
    strcpy(key, "SII");
    strcpy(val, "3600000");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalIdle().Value() == 3600000_ms32);

    // Test no other fields were populated
    ResetRetryIntervalIdle(nodeData);
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(nodeData));

    // Invalid SII - negative value
    strcpy(key, "SII");
    strcpy(val, "-1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - greater than maximum
    strcpy(key, "SII");
    strcpy(val, "3600001");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - much greater than maximum
    strcpy(key, "SII");
    strcpy(val, "1095216660481"); // 0xFF00000001 == 1 (mod 2^32)
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - hexadecimal value
    strcpy(key, "SII");
    strcpy(val, "0x20");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - leading zeros
    strcpy(key, "SII");
    strcpy(val, "0700");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());

    // Invalid SII - text at the end
    strcpy(key, "SII");
    strcpy(val, "123abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalIdle().HasValue());
}

// Test SAI (formerly CRA)
template <class NodeData>
void TxtFieldSessionActiveInterval(nlTestSuite * inSuite, void * inContext)
{
    char key[4];
    char val[16];
    NodeData nodeData;

    // Minimum
    strcpy(key, "SAI");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalActive().Value() == 1_ms32);

    // Maximum
    strcpy(key, "SAI");
    strcpy(val, "3600000");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryIntervalActive().Value() == 3600000_ms32);

    // Test no other fields were populated
    ResetRetryIntervalActive(nodeData);
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(nodeData));

    // Invalid SAI - negative value
    strcpy(key, "SAI");
    strcpy(val, "-1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - greater than maximum
    strcpy(key, "SAI");
    strcpy(val, "3600001");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - much greater than maximum
    strcpy(key, "SAI");
    strcpy(val, "1095216660481"); // 0xFF00000001 == 1 (mod 2^32)
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - hexadecimal value
    strcpy(key, "SAI");
    strcpy(val, "0x20");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - leading zeros
    strcpy(key, "SAI");
    strcpy(val, "0700");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());

    // Invalid SAI - text at the end
    strcpy(key, "SAI");
    strcpy(val, "123abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryIntervalActive().HasValue());
}

// Test SAT (Session Active Threshold)
template <class NodeData>
void TxtFieldSessionActiveThreshold(nlTestSuite * inSuite, void * inContext)
{
    char key[4];
    char val[16];
    NodeData nodeData;

    // Minimum
    strcpy(key, "SAT");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryActiveThreshold().Value() == 1_ms16);

    // Maximum
    strcpy(key, "SAT");
    strcpy(val, "65535");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.GetMrpRetryActiveThreshold().Value() == 65535_ms16);

    // Test no other fields were populated
    ResetRetryActiveThreshold(nodeData);
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(nodeData));

    // Invalid SAI - negative value
    strcpy(key, "SAT");
    strcpy(val, "-1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAI - greater than maximum
    strcpy(key, "SAT");
    strcpy(val, "65536");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAT - much greater than maximum
    strcpy(key, "SAT");
    strcpy(val, "1095216660481"); // 0xFF00000001 == 1 (mod 2^32)
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAT - hexadecimal value
    strcpy(key, "SAT");
    strcpy(val, "0x20");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAT - leading zeros
    strcpy(key, "SAT");
    strcpy(val, "0700");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());

    // Invalid SAT - text at the end
    strcpy(key, "SAT");
    strcpy(val, "123abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue());
}

// Test T (TCP support)
template <class NodeData>
void TxtFieldTcpSupport(nlTestSuite * inSuite, void * inContext)
{
    char key[4];
    char val[8];
    NodeData nodeData;

    // True
    strcpy(key, "T");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.supportsTcp);

    // Test no other fields were populated
    nodeData.resolutionData.supportsTcp = false;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(nodeData));

    // False
    strcpy(key, "T");
    strcpy(val, "0");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.supportsTcp == false);

    // Invalid value, stil false
    strcpy(key, "T");
    strcpy(val, "asdf");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.supportsTcp == false);
}

// Test ICD (ICD operation Mode)
template <class NodeData>
void TxtFieldICDoperatesAsLIT(nlTestSuite * inSuite, void * inContext)
{
    char key[4];
    char val[16];
    NodeData nodeData;

    // ICD is operating as a LIT device
    strcpy(key, "ICD");
    strcpy(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.isICDOperatingAsLIT.HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.isICDOperatingAsLIT.Value());

    // Test no other fields were populated
    nodeData.resolutionData.isICDOperatingAsLIT.ClearValue();
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(nodeData));

    // ICD is operating as a SIT device
    strcpy(key, "ICD");
    strcpy(val, "0");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.isICDOperatingAsLIT.HasValue());
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.isICDOperatingAsLIT.Value() == false);

    nodeData.resolutionData.isICDOperatingAsLIT.ClearValue();
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(nodeData));
    // Invalid value, No key set
    strcpy(key, "ICD");
    strcpy(val, "asdf");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.isICDOperatingAsLIT.HasValue() == false);
}

// Test IsDeviceTreatedAsSleepy() with CRI
template <class NodeData>
void TestIsDeviceSessionIdle(nlTestSuite * inSuite, void * inContext)
{
    char key[4];
    char val[32];
    NodeData nodeData;
    const ReliableMessageProtocolConfig defaultMRPConfig(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL,
                                                         CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL);

    // No key/val set, so the device can't be sleepy
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));

    // If the interval is the default value, the device is not sleepy
    strcpy(key, "SII");
    sprintf(val, "%d", static_cast<int>(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL.count()));
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));

    // If the interval is greater than the default value, the device is sleepy
    sprintf(key, "SII");
    sprintf(val, "%d", static_cast<int>(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL.count() + 1));
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));
}

// Test IsDeviceTreatedAsSleepy() with CRA
template <class NodeData>
void TestIsDeviceSessionActive(nlTestSuite * inSuite, void * inContext)
{
    char key[4];
    char val[32];
    NodeData nodeData;
    const ReliableMessageProtocolConfig defaultMRPConfig(CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL,
                                                         CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL);

    // No key/val set, so the device can't be sleepy
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));

    // If the interval is the default value, the device is not sleepy
    sprintf(key, "SAI");
    sprintf(val, "%d", static_cast<int>(CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL.count()));
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, !nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));

    // If the interval is greater than the default value, the device is sleepy
    strcpy(key, "SAI");
    sprintf(val, "%d", static_cast<int>(CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL.count() + 1));
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), nodeData.resolutionData);
    NL_TEST_ASSERT(inSuite, nodeData.resolutionData.IsDeviceTreatedAsSleepy(&defaultMRPConfig));
}

const nlTest sTests[] = {
    NL_TEST_DEF("TxtFieldKey", TestGetTxtFieldKey),                                          //
    NL_TEST_DEF("TxtFieldKeyCaseInsensitive", TestGetTxtFieldKeyCaseInsensitive),            //
    NL_TEST_DEF("TxtFieldProduct", TestGetProduct),                                          //
    NL_TEST_DEF("TxtFieldVendor", TestGetVendor),                                            //
    NL_TEST_DEF("TxtFieldLongDiscriminator", TestGetLongDiscriminator),                      //
    NL_TEST_DEF("TxtFieldCommissioningMode", TestGetCommissioningMode),                      //
    NL_TEST_DEF("TxtFieldDeviceType", TestGetDeviceType),                                    //
    NL_TEST_DEF("TxtFieldDeviceName", TestGetDeviceName),                                    //
    NL_TEST_DEF("TxtFieldRotatingDeviceId", TestGetRotatingDeviceId),                        //
    NL_TEST_DEF("TxtFieldPairingHint", TestGetPairingHint),                                  //
    NL_TEST_DEF("TxtFieldPairingInstruction", TestGetPairingInstruction),                    //
    NL_TEST_DEF("TxtFieldCommissionerPasscode", TestGetCommissionerPasscode),                //
    NL_TEST_DEF("TxtFieldFillDiscoveredNodeDataFromTxt", TestFillDiscoveredNodeDataFromTxt), //
    NL_TEST_DEF("TxtDiscoveredFieldMrpRetryIntervalIdle", TxtFieldSessionIdleInterval<DiscoveredNodeData>),
    NL_TEST_DEF("TxtDiscoveredFieldMrpRetryIntervalActive", TxtFieldSessionActiveInterval<DiscoveredNodeData>),
    NL_TEST_DEF("TxtDiscoveredFieldMrpRetryActiveThreshold", TxtFieldSessionActiveThreshold<DiscoveredNodeData>),
    NL_TEST_DEF("TxtDiscoveredFieldTcpSupport", (TxtFieldTcpSupport<DiscoveredNodeData>) ),
    NL_TEST_DEF("TxtDiscoveredIsICDoperatingAsLIT", (TxtFieldICDoperatesAsLIT<DiscoveredNodeData>) ),
    NL_TEST_DEF("TxtDiscoveredIsDeviceSessionIdle", TestIsDeviceSessionIdle<DiscoveredNodeData>),
    NL_TEST_DEF("TxtDiscoveredIsDeviceSessionActive", TestIsDeviceSessionActive<DiscoveredNodeData>),
    NL_TEST_DEF("TxtResolvedFieldMrpRetryIntervalIdle", TxtFieldSessionIdleInterval<ResolvedNodeData>),
    NL_TEST_DEF("TxtResolvedFieldMrpRetryIntervalActive", TxtFieldSessionActiveInterval<ResolvedNodeData>),
    NL_TEST_DEF("TxtResolvedFieldMrpRetryActiveThreshold", TxtFieldSessionActiveThreshold<ResolvedNodeData>),
    NL_TEST_DEF("TxtResolvedFieldTcpSupport", (TxtFieldTcpSupport<ResolvedNodeData>) ),
    NL_TEST_DEF("TxtResolvedFieldICDoperatingAsLIT", (TxtFieldICDoperatesAsLIT<ResolvedNodeData>) ),
    NL_TEST_DEF("TxtResolvedIsDeviceSessionIdle", TestIsDeviceSessionIdle<ResolvedNodeData>),
    NL_TEST_DEF("TxtResolvedIsDeviceSessionActive", TestIsDeviceSessionActive<ResolvedNodeData>),
    NL_TEST_SENTINEL()
};

} // namespace

int TestCHIPTxtFields()
{
    nlTestSuite theSuite = { "TxtFields", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCHIPTxtFields);
