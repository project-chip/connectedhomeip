/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "UnifyMPCContext.h"
#include "attribute.hpp"
#include "attribute_store_fixt.h"
#include "datastore_fixt.h"

#include "mpc_attribute_parser_fwk.h"
#include "mpc_attribute_store.h"
#include "mpc_attribute_store_defined_attribute_types.h"

// Chip components
#include <lib/core/TLVDebug.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <filesystem>

#include <string>

using namespace unify::mpc::Test;
using namespace chip;
using namespace chip::System;
using namespace attribute_store;
using namespace std;

string CLUSTERLIST_VALUE    = "29,30,31,40,43";
string PARTSLIST_VALUE      = "0,2";
string DEVICETYPELIST_VALUE = "22";

#define TLVDEBUG

class TestContext : public UnifyMPCContext
{
public:
    nlTestSuite * mTestSuite;
    uint32_t mNumTimersHandled;
    attribute epNode;

    static int initialize(void * inContext)
    {
        TestContext * ctxt = static_cast<TestContext *>(inContext);
        
        // Makes sure our required state folders exists
        std::filesystem::create_directories(LOCALSTATEDIR);

        if (SUCCESS != TestContext::Initialize(inContext))
            return FAILURE;

        attribute_store_init();
        mpc_attribute_store_init();
        auto devNode = attribute::root().add_node(ATTRIBUTE_NODE_ID);
        attribute_store_set_reported_string(devNode, "mt-01");
        ctxt->epNode = devNode.add_node(ATTRIBUTE_ENDPOINT_ID).set_reported<EndpointId>(0);

        return SUCCESS;
    }

    static int finalize(void * inContext)
    {

        attribute_store_teardown();
        return TestContext::Finalize(inContext);
    }
};

static void TestMPCParserInit(nlTestSuite * inSuite, void * aContext)
{
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
}

#ifdef TLVDEBUG
static void dumpWriter(const char * aFormat, ...)
{
    va_list args;
    va_start(args, aFormat);
    vprintf(aFormat, args);
    va_end(args);
}
#endif

template <typename T>
static sl_status_t encodeList(chip::TLV::TLVReader & data, System::PacketBufferHandle & buffer, chip::app::DataModel::List<T> list)
{
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer));

    if (CHIP_NO_ERROR != chip::app::DataModel::Encode(writer, chip::TLV::AnonymousTag(), list))
        return SL_STATUS_FAIL;
    if (CHIP_NO_ERROR != writer.Finalize(&buffer))
        return SL_STATUS_FAIL;

    System::PacketBufferTLVReader reader;
    reader.Init(std::move(buffer));
    data.Init(reader);
    data.Next(chip::TLV::AnonymousTag());

    return SL_STATUS_OK;
}

static void TestMPCParserInvokeDescClusterAttr0(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::Descriptor::Structs;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();

    TestContext * ctxt = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    char value[256];
    chip::app::ConcreteDataAttributePath path(0, 0x1D, 0);
    DeviceTypeStruct::Type dTArr[] = { { 22, 1 } };
    chip::app::DataModel::List<DeviceTypeStruct::Type> dtList;
    dtList = dTArr;

    auto descDTListNode = ctxt->epNode.add_node(ATTRIBUTE_DEVICETYPELIST_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeList<DeviceTypeStruct::Type>(reader, bufferRef, dtList)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, descDTListNode)));
    NL_TEST_ASSERT(inSuite, (descDTListNode.reported_exists()));
    attribute_store_get_reported_string(descDTListNode, value, 256);
    NL_TEST_ASSERT(inSuite, (string(value).compare(DEVICETYPELIST_VALUE) == 0));
}

static void TestMPCParserInvokeDescClusterAttr1(nlTestSuite * inSuite, void * aContext)
{
    TestContext * ctxt                = static_cast<TestContext *>(aContext);
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();
    chip::TLV::TLVReader reader;
    char value[256];
    chip::app::ConcreteDataAttributePath path(0, 0x1D, 1);
    ClusterId clustArr[] = { 29, 30, 31, 40, 43 };
    chip::app::DataModel::List<ClusterId> serList;
    serList = clustArr;

    auto descSerListNode = ctxt->epNode.add_node(ATTRIBUTE_SERVERLIST_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeList<ClusterId>(reader, bufferRef, serList)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, descSerListNode)));
    NL_TEST_ASSERT(inSuite, (descSerListNode.reported_exists()));
    attribute_store_get_reported_string(descSerListNode, value, 256);
    NL_TEST_ASSERT(inSuite, (string(value).compare(CLUSTERLIST_VALUE) == 0));
}

static void TestMPCParserInvokeDescClusterAttr2(nlTestSuite * inSuite, void * aContext)
{
    TestContext * ctxt                = static_cast<TestContext *>(aContext);
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();
    chip::TLV::TLVReader reader;
    char value[256];
    chip::app::ConcreteDataAttributePath path(0, 0x1D, 2);
    ClusterId clustArr[] = { 29, 30, 31, 40, 43 };
    chip::app::DataModel::List<ClusterId> cliList;
    cliList = clustArr;

    auto descCliListNode = ctxt->epNode.add_node(ATTRIBUTE_CLIENTLIST_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeList<ClusterId>(reader, bufferRef, cliList)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, descCliListNode)));
    NL_TEST_ASSERT(inSuite, (descCliListNode.reported_exists()));
    attribute_store_get_reported_string(descCliListNode, value, 256);
    NL_TEST_ASSERT(inSuite, (string(value).compare(CLUSTERLIST_VALUE) == 0));
}

static void TestMPCParserInvokeDescClusterAttr3(nlTestSuite * inSuite, void * aContext)
{
    TestContext * ctxt                = static_cast<TestContext *>(aContext);
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();
    chip::TLV::TLVReader reader;
    char value[256];
    chip::app::ConcreteDataAttributePath path(0, 0x1D, 3);
    EndpointId epArr[] = { 0, 2 };
    chip::app::DataModel::List<EndpointId> epList;
    epList = epArr;

    auto descPartListNode = ctxt->epNode.add_node(ATTRIBUTE_PARTSLIST_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeList<EndpointId>(reader, bufferRef, epList)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, descPartListNode)));
    NL_TEST_ASSERT(inSuite, (descPartListNode.reported_exists()));
    attribute_store_get_reported_string(descPartListNode, value, 256);
    NL_TEST_ASSERT(inSuite, (string(value).compare(PARTSLIST_VALUE) == 0));
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestMPCParserInit",             TestMPCParserInit),
    NL_TEST_DEF("TestMPCParserInvokeDescClusterAttr0", TestMPCParserInvokeDescClusterAttr0),
    NL_TEST_DEF("TestMPCParserInvokeDescClusterAttr1", TestMPCParserInvokeDescClusterAttr1),
    NL_TEST_DEF("TestMPCParserInvokeDescClusterAttr2", TestMPCParserInvokeDescClusterAttr2),
    NL_TEST_DEF("TestMPCParserInvokeDescClusterAttr3", TestMPCParserInvokeDescClusterAttr3),
    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "TestParserFwkInterface",
    &sTests[0],
    TestContext::initialize,
    TestContext::finalize

};

int TestParserFwkInterface(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestParserFwkInterface)
