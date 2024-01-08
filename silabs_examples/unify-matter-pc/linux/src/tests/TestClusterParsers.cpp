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
        
        if (CHIP_NO_ERROR != ctxt->Initialize())
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
        return TestContext::nlTestTearDownTestSuite(inContext);
    }
};

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

template <typename T>
static sl_status_t encodeValue(chip::TLV::TLVReader & data, System::PacketBufferHandle & buffer, T value)
{
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer));

    if (CHIP_NO_ERROR != chip::app::DataModel::Encode(writer, chip::TLV::AnonymousTag(), value))
        return SL_STATUS_FAIL;
    if (CHIP_NO_ERROR != writer.Finalize(&buffer))
        return SL_STATUS_FAIL;

    System::PacketBufferTLVReader reader;
    reader.Init(std::move(buffer));
    data.Init(reader);
    data.Next(chip::TLV::AnonymousTag());

    return SL_STATUS_OK;
}

static void TestMPCParserInvokeOnOffAttrList(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();
    TestContext * ctxt                = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    char value[256];
    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::AttributeList::Id);
    AttributeId attrIdArr[] = { 0, 0x4001, 0x4002 };
    chip::app::DataModel::List<AttributeId> attrList;
    attrList = attrIdArr;

    auto onoffAttrListNode = ctxt->epNode.add_node(ONOFF_ATTRIBUTE_LIST);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeList<AttributeId>(reader, bufferRef, attrList)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffAttrListNode)));
    NL_TEST_ASSERT(inSuite, (onoffAttrListNode.reported_exists()));
    attribute_store_get_reported_string(onoffAttrListNode, value, 256);
    cout << "attr list decoded : " << string(value) << endl;
    NL_TEST_ASSERT(inSuite, (string(value).compare("0,16385,16386") == 0));
}

static void TestMPCParserInvokeOnOffGenCmdList(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();
    TestContext * ctxt                = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    char value[256];
    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::GeneratedCommandList::Id);
    CommandId cmdArr[] = { 0, 1, 2 };
    chip::app::DataModel::List<CommandId> cmdList;
    cmdList = cmdArr;

    auto onoffGenCmdListNode = ctxt->epNode.add_node(ONOFF_GENERATED_COMMAND_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeList<AttributeId>(reader, bufferRef, cmdList)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffGenCmdListNode)));
    NL_TEST_ASSERT(inSuite, (onoffGenCmdListNode.reported_exists()));
    attribute_store_get_reported_string(onoffGenCmdListNode, value, 256);
    NL_TEST_ASSERT(inSuite, (string(value).compare("0,1,2") == 0));
}

static void TestMPCParserInvokeOnOffAccCmdList(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();
    TestContext * ctxt                = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    char value[256];
    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::AcceptedCommandList::Id);
    CommandId cmdArr[] = { 0, 1, 2 };
    chip::app::DataModel::List<CommandId> cmdList;
    cmdList = cmdArr;

    auto onoffAccCmdListNode = ctxt->epNode.add_node(ONOFF_ACCEPTED_COMMAND_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeList<AttributeId>(reader, bufferRef, cmdList)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffAccCmdListNode)));
    NL_TEST_ASSERT(inSuite, (onoffAccCmdListNode.reported_exists()));
    attribute_store_get_reported_string(onoffAccCmdListNode, value, 256);
    NL_TEST_ASSERT(inSuite, (string(value).compare("0,1,2") == 0));
}

static void TestMPCParserInvokeOnOffFeatureMap(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    TestContext * ctxt = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();

    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::FeatureMap::Id);

    auto onoffFeatMapNode = ctxt->epNode.add_node(ONOFF_FEATURE_MAP_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeValue<Attributes::FeatureMap::TypeInfo::Type>(reader, bufferRef, 1)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffFeatMapNode)));
    NL_TEST_ASSERT(inSuite, (onoffFeatMapNode.reported_exists()));
    // attribute_store_get_reported_string(onoffFeatMapNode, value, 256);
    NL_TEST_ASSERT(inSuite, (onoffFeatMapNode.reported<uint32_t>() == 1));
}

static void TestMPCParserInvokeOnOffClustRev(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    TestContext * ctxt = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();

    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::ClusterRevision::Id);

    auto onoffClustRevNode = ctxt->epNode.add_node(ONOFF_CLUSTER_REVISION_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeValue<Attributes::ClusterRevision::TypeInfo::Type>(reader, bufferRef, 4)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffClustRevNode)));
    NL_TEST_ASSERT(inSuite, (onoffClustRevNode.reported_exists()));
    // attribute_store_get_reported_string(onoffClustRevNode, value, 256);
    NL_TEST_ASSERT(inSuite, (onoffClustRevNode.reported<uint16_t>() == 4));
}

static void TestMPCParserInvokeOnOffOnOff(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    TestContext * ctxt = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();

    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::OnOff::Id);

    auto onoffOnOffNode = ctxt->epNode.add_node(ONOFF_CLUSTER_REVISION_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeValue<Attributes::OnOff::TypeInfo::Type>(reader, bufferRef, 1)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffOnOffNode)));
    NL_TEST_ASSERT(inSuite, (onoffOnOffNode.reported_exists()));
    // attribute_store_get_reported_string(onoffClustRevNode, value, 256);
    NL_TEST_ASSERT(inSuite, (onoffOnOffNode.reported<bool>() == true));
}

static void TestMPCParserInvokeOnOffGS(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    TestContext * ctxt = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();

    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::GlobalSceneControl::Id);

    auto onoffGSNode = ctxt->epNode.add_node(ONOFF_CLUSTER_REVISION_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeValue<Attributes::GlobalSceneControl::TypeInfo::Type>(reader, bufferRef, 0)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffGSNode)));
    NL_TEST_ASSERT(inSuite, (onoffGSNode.reported_exists()));
    // attribute_store_get_reported_string(onoffClustRevNode, value, 256);
    NL_TEST_ASSERT(inSuite, (onoffGSNode.reported<bool>() == 0));
}

static void TestMPCParserInvokeOnOffOT(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    TestContext * ctxt = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();

    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::OnTime::Id);

    auto onoffOTNode = ctxt->epNode.add_node(ONOFF_CLUSTER_REVISION_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeValue<Attributes::OnTime::TypeInfo::Type>(reader, bufferRef, 4)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffOTNode)));
    NL_TEST_ASSERT(inSuite, (onoffOTNode.reported_exists()));
    // attribute_store_get_reported_string(onoffClustRevNode, value, 256);
    NL_TEST_ASSERT(inSuite, (onoffOTNode.reported<uint16_t>() == 4));
}

static void TestMPCParserInvokeOnOffOWT(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    TestContext * ctxt = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();

    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::OffWaitTime::Id);

    auto onoffOWTNode = ctxt->epNode.add_node(ONOFF_CLUSTER_REVISION_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeValue<Attributes::OffWaitTime::TypeInfo::Type>(reader, bufferRef, 84)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffOWTNode)));
    NL_TEST_ASSERT(inSuite, (onoffOWTNode.reported_exists()));
    // attribute_store_get_reported_string(onoffClustRevNode, value, 256);
    NL_TEST_ASSERT(inSuite, (onoffOWTNode.reported<uint16_t>() == 84));
}

static void TestMPCParserInvokeOnOffSUOnOff(nlTestSuite * inSuite, void * aContext)
{
    using namespace chip::app::Clusters::OnOff;
    TestContext * ctxt = static_cast<TestContext *>(aContext);
    chip::TLV::TLVReader reader;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    auto bufferRef = buffer.Retain();
    chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::StartUpOnOffEnum> value;
    value.SetNull();

    chip::app::ConcreteDataAttributePath path(0, Id, Attributes::StartUpOnOff::Id);

    auto onoffClustRevNode = ctxt->epNode.add_node(ONOFF_CLUSTER_REVISION_ID);

    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_init()));
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == encodeValue<Attributes::StartUpOnOff::TypeInfo::Type>(reader, bufferRef, value)));

#ifdef TLVDEBUG
    chip::TLV::Debug::Dump(reader, dumpWriter);
#endif
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_attribute_parser_invoke(path, &reader, onoffClustRevNode)));
    NL_TEST_ASSERT(inSuite, (onoffClustRevNode.reported_exists()));
    // attribute_store_get_reported_string(onoffClustRevNode, value, 256);
    NL_TEST_ASSERT(inSuite, (onoffClustRevNode.reported<uint16_t>() == 3));
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestMPCParserInvokeOnOffAttrList", TestMPCParserInvokeOnOffAttrList),
    NL_TEST_DEF("TestMPCParserInvokeOnOffGenCmdList", TestMPCParserInvokeOnOffGenCmdList),
    NL_TEST_DEF("TestMPCParserInvokeOnOffAccCmdList", TestMPCParserInvokeOnOffAccCmdList),
    NL_TEST_DEF("TestMPCParserInvokeOnOffFeatureMap", TestMPCParserInvokeOnOffFeatureMap),
    NL_TEST_DEF("TestMPCParserInvokeOnOffClustRev", TestMPCParserInvokeOnOffClustRev),
    NL_TEST_DEF("TestMPCParserInvokeOnOffOnOff", TestMPCParserInvokeOnOffOnOff),
    NL_TEST_DEF("TestMPCParserInvokeOnOffGS", TestMPCParserInvokeOnOffGS),
    NL_TEST_DEF("TestMPCParserInvokeOnOffOT", TestMPCParserInvokeOnOffOT),
    NL_TEST_DEF("TestMPCParserInvokeOnOffOWT", TestMPCParserInvokeOnOffOWT),
    NL_TEST_DEF("TestMPCParserInvokeOnOffSUOnOff", TestMPCParserInvokeOnOffSUOnOff),
    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "TestClusterParsers",
    &sTests[0],
    TestContext::initialize,
    TestContext::finalize

};

int TestClusterParsers(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestClusterParsers)
