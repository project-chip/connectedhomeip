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

#include <mdns/TxtFields.h>

#include <limits>
#include <string.h>

#include <mdns/Resolver.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Mdns;
using namespace chip::Mdns::Internal;

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
    char key[3];
    sprintf(key, "D");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kLongDiscriminator);

    sprintf(key, "VP");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kVendorProduct);

    sprintf(key, "AP");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kAdditionalPairing);

    sprintf(key, "CM");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kCommissioningMode);

    sprintf(key, "DT");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kDeviceType);

    sprintf(key, "DN");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kDeviceName);

    sprintf(key, "RI");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kRotatingDeviceId);

    sprintf(key, "PI");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kPairingInstruction);

    sprintf(key, "PH");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kPairingHint);

    sprintf(key, "XX");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kUnknown);
}

void TestGetTxtFieldKeyCaseInsensitive(nlTestSuite * inSuite, void * inContext)
{
    char key[3];
    sprintf(key, "d");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kLongDiscriminator);

    sprintf(key, "vp");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kVendorProduct);
    sprintf(key, "Vp");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kVendorProduct);
    sprintf(key, "vP");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kVendorProduct);

    sprintf(key, "Xx");
    NL_TEST_ASSERT(inSuite, GetTxtFieldKey(GetSpan(key)) == TxtFieldKey::kUnknown);
}

void TestGetProduct(nlTestSuite * inSuite, void * inContext)
{
    // Product and vendor are given as part of the same key, on either side of a + sign. Product is after the +
    char vp[64];
    sprintf(vp, "123+456");
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 456);

    sprintf(vp, "123+");
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 0);

    sprintf(vp, "+456");
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 456);

    sprintf(vp, "123");
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 0);

    // overflow a uint16
    sprintf(vp, "123+%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetProduct(GetSpan(vp)) == 0);
}
void TestGetVendor(nlTestSuite * inSuite, void * inContext)
{
    // Product and vendor are given as part of the same key, on either side of a + sign. Vendor is first
    char vp[64];
    sprintf(vp, "123+456");
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 123);

    sprintf(vp, "123+");
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 123);

    sprintf(vp, "+456");
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 0);

    sprintf(vp, "123");
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 123);

    // overflow a uint16
    sprintf(vp, "%" PRIu32 "+456", static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetVendor(GetSpan(vp)) == 0);
}

void TestGetLongDiscriminator(nlTestSuite * inSuite, void * inContext)
{
    char ld[64];
    sprintf(ld, "1234");
    NL_TEST_ASSERT(inSuite, GetLongDisriminator(GetSpan(ld)) == 1234);

    // overflow a uint16
    sprintf(ld, "%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    printf("ld = %s\n", ld);
    NL_TEST_ASSERT(inSuite, GetLongDisriminator(GetSpan(ld)) == 0);
}

void TestGetAdditionalPairing(nlTestSuite * inSuite, void * inContext)
{
    char ap[64];
    sprintf(ap, "1");
    NL_TEST_ASSERT(inSuite, GetAdditionalPairing(GetSpan(ap)) == 1);

    // overflow a uint8
    sprintf(ap, "%u", static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetAdditionalPairing(GetSpan(ap)) == 0);
}

void TestGetCommissioningMode(nlTestSuite * inSuite, void * inContext)
{
    char cm[64];
    sprintf(cm, "0");
    NL_TEST_ASSERT(inSuite, GetCommissioningMode(GetSpan(cm)) == 0);

    sprintf(cm, "1");
    NL_TEST_ASSERT(inSuite, GetCommissioningMode(GetSpan(cm)) == 1);

    // overflow a uint8
    sprintf(cm, "%u", static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetCommissioningMode(GetSpan(cm)) == 0);
}

void TestGetDeviceType(nlTestSuite * inSuite, void * inContext)
{
    char dt[64];
    sprintf(dt, "1234");
    NL_TEST_ASSERT(inSuite, GetDeviceType(GetSpan(dt)) == 1234);

    // overflow a uint16
    sprintf(dt, "%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetDeviceType(GetSpan(dt)) == 0);
}

void TestGetDeviceName(nlTestSuite * inSuite, void * inContext)
{
    char name[kMaxDeviceNameLen + 1] = "";
    char val[kMaxDeviceNameLen + 2];

    sprintf(val, "testname");
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

    sprintf(ri, "0A1B");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    printf("id[0] = %x\n", id[0]);
    NL_TEST_ASSERT(inSuite, id[0] == 0x0A);
    NL_TEST_ASSERT(inSuite, id[1] == 0x1B);
    NL_TEST_ASSERT(inSuite, len == 2);

    // odd number of characters can't be parsed.
    sprintf(ri, "0A1BC");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    NL_TEST_ASSERT(inSuite, len == 0);

    // non-hex characters can't be parsed
    sprintf(ri, "0ATT");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    NL_TEST_ASSERT(inSuite, len == 0);

    // Lower case should work on SDK even though devices shouldn't be sending that.
    sprintf(ri, "0a1b");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    NL_TEST_ASSERT(inSuite, id[0] == 0x0A);
    NL_TEST_ASSERT(inSuite, id[1] == 0x1B);
    NL_TEST_ASSERT(inSuite, len == 2);

    sprintf(ri, "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F3031");
    GetRotatingDeviceId(GetSpan(ri), id, &len);
    NL_TEST_ASSERT(inSuite, len == kMaxRotatingIdLen);
    for (uint8_t i = 0; i < kMaxRotatingIdLen; ++i)
    {
        NL_TEST_ASSERT(inSuite, id[i] == i);
    }
}

void TestGetPairingHint(nlTestSuite * inSuite, void * inContext)
{
    char ph[64];
    sprintf(ph, "0");
    NL_TEST_ASSERT(inSuite, GetPairingHint(GetSpan(ph)) == 0);

    sprintf(ph, "9");
    NL_TEST_ASSERT(inSuite, GetPairingHint(GetSpan(ph)) == 9);

    // overflow a uint16
    sprintf(ph, "%" PRIu32, static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1);
    NL_TEST_ASSERT(inSuite, GetPairingHint(GetSpan(ph)) == 0);
}

void TestGetPairingInstruction(nlTestSuite * inSuite, void * inContext)
{
    char data[kMaxPairingInstructionLen + 2];
    char ret[kMaxPairingInstructionLen + 1] = "";

    sprintf(data, "something");
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

bool NodeDataIsEmpty(const DiscoveredNodeData & node)
{

    if (node.longDiscriminator != 0 || node.vendorId != 0 || node.productId != 0 || node.additionalPairing != 0 ||
        node.commissioningMode != 0 || node.deviceType != 0 || node.rotatingIdLen != 0 || node.pairingHint != 0)
    {
        return false;
    }
    if (strcmp(node.deviceName, "") != 0 || strcmp(node.pairingInstruction, "") != 0)
    {
        return false;
    }
    for (size_t i = 0; i < sizeof(DiscoveredNodeData::rotatingId); ++i)
    {
        if (node.rotatingId[i] != 0)
        {
            return false;
        }
    }
    return true;
}

// The individual fill tests test the error cases for each key type, this test is used to ensure the proper record is filled.
void TestFillNodeDataFromTxt(nlTestSuite * inSuite, void * inContext)
{
    char key[3];
    char val[16];
    DiscoveredNodeData filled;

    // Long discriminator
    sprintf(key, "D");
    sprintf(val, "840");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, filled.longDiscriminator == 840);
    filled.longDiscriminator = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // vendor and product
    sprintf(key, "VP");
    sprintf(val, "123+456");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, filled.vendorId == 123);
    NL_TEST_ASSERT(inSuite, filled.productId == 456);
    filled.vendorId  = 0;
    filled.productId = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Additional Pairing
    sprintf(key, "AP");
    sprintf(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, filled.additionalPairing == 1);
    filled.additionalPairing = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Commissioning mode
    sprintf(key, "CM");
    sprintf(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, filled.commissioningMode == 1);
    filled.commissioningMode = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Device type
    sprintf(key, "DT");
    sprintf(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, filled.deviceType == 1);
    filled.deviceType = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Device name
    sprintf(key, "DN");
    sprintf(val, "abc");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, strcmp(filled.deviceName, "abc") == 0);
    memset(filled.deviceName, 0, sizeof(filled.deviceName));
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Rotating device id
    sprintf(key, "RI");
    sprintf(val, "1A2B");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, filled.rotatingId[0] == 0x1A);
    NL_TEST_ASSERT(inSuite, filled.rotatingId[1] == 0x2B);
    NL_TEST_ASSERT(inSuite, filled.rotatingIdLen == 2);
    filled.rotatingIdLen = 0;
    memset(filled.rotatingId, 0, sizeof(filled.rotatingId));
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Pairing instruction
    sprintf(key, "PI");
    sprintf(val, "hint");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, strcmp(filled.pairingInstruction, "hint") == 0);
    memset(filled.pairingInstruction, 0, sizeof(filled.pairingInstruction));
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));

    // Pairing hint
    sprintf(key, "PH");
    sprintf(val, "1");
    FillNodeDataFromTxt(GetSpan(key), GetSpan(val), &filled);
    NL_TEST_ASSERT(inSuite, filled.pairingHint == 1);
    filled.pairingHint = 0;
    NL_TEST_ASSERT(inSuite, NodeDataIsEmpty(filled));
}

const nlTest sTests[] = {
    NL_TEST_DEF("TxtFieldKey", TestGetTxtFieldKey),                               //
    NL_TEST_DEF("TxtFieldKeyCaseInsensitive", TestGetTxtFieldKeyCaseInsensitive), //
    NL_TEST_DEF("TxtFieldProduct", TestGetProduct),                               //
    NL_TEST_DEF("TxtFieldVendor", TestGetVendor),                                 //
    NL_TEST_DEF("TxtFieldLongDiscriminator", TestGetLongDiscriminator),           //
    NL_TEST_DEF("TxtFieldAdditionalPairing", TestGetAdditionalPairing),           //
    NL_TEST_DEF("TxtFieldCommissioningMode", TestGetCommissioningMode),           //
    NL_TEST_DEF("TxtFieldDeviceType", TestGetDeviceType),                         //
    NL_TEST_DEF("TxtFieldDeviceName", TestGetDeviceName),                         //
    NL_TEST_DEF("TxtFieldRotatingDeviceId", TestGetRotatingDeviceId),             //
    NL_TEST_DEF("TxtFieldPairingHint", TestGetPairingHint),                       //
    NL_TEST_DEF("TxtFieldPairingInstruction", TestGetPairingInstruction),         //
    NL_TEST_DEF("TxtFieldFillNodeDataFromTxt", TestFillNodeDataFromTxt),          //
    NL_TEST_SENTINEL()                                                            //
};

} // namespace

int TestCHIPTxtFields(void)
{
    nlTestSuite theSuite = { "TxtFields", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCHIPTxtFields);
