// Unify bridge components

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include "UnifyBridgeContext.h"
// Third party library
#include <nlunit-test.h>
#include "matter_bridge_qrcode_publisher.hpp"
#include "Options.h"
#include "TestHelpers.hpp"

using namespace unify::matter_bridge::Test;

static void QRCodePublisher(nlTestSuite * inSuite, void * aContext)
{
  auto & opt = LinuxDeviceOptions::GetInstance();
  opt.payload.commissioningFlow = chip::CommissioningFlow::kStandard;
  opt.payload.rendezvousInformation.Emplace().ClearAll();
  opt.payload.rendezvousInformation.Emplace().Set(chip::RendezvousInformationFlag::kOnNetwork);
  opt.payload.productID = 1234;
  opt.payload.vendorID  = 5678;
  opt.payload.setUpPINCode = 0xfedeab;
  opt.discriminator =  chip::Optional<uint16_t>(0xe);

  auto ctx =  static_cast<UnifyBridgeContext*>(aContext);
  unify::matter_bridge::QRCodePublisher publisher(ctx->mMqttHandler);

  publisher.publish();
  NL_TEST_ASSERT_EQUAL(inSuite, ctx->mMqttHandler.nNumerUicMqttPublishCall , 1);
  NL_TEST_ASSERT_EQUAL(inSuite, ctx->mMqttHandler.publish_topic, "ucl/SmartStart/CommissionableDevice/MT:CYUK4CRM00S05-5FD00");
  NL_TEST_ASSERT_EQUAL_JSON(inSuite, ctx->mMqttHandler.publish_payload, R"({ "QRCode" : "MT:CYUK4CRM00S05-5FD00" })");


  publisher.unretain();
  NL_TEST_ASSERT_EQUAL(inSuite, ctx->mMqttHandler.nNumerUicMqttPublishCall , 2);
  NL_TEST_ASSERT_EQUAL(inSuite, ctx->mMqttHandler.publish_topic, "ucl/SmartStart/CommissionableDevice/MT:CYUK4CRM00S05-5FD00");
  NL_TEST_ASSERT_EQUAL(inSuite, ctx->mMqttHandler.publish_payload,"");

}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("QRCodePublisher::QRCodePublisher",             QRCodePublisher),
    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "QRCodePublisher",
    &sTests[0],
    UnifyBridgeContext::nlTestSetUpTestSuite,
    UnifyBridgeContext::nlTestTearDownTestSuite
};

int QRCodePublisherSuite(void)
{
    return chip::ExecuteTestsWithContext<UnifyBridgeContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(QRCodePublisherSuite)