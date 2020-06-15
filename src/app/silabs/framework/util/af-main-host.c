/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

// ZCL - ZigBee Cluster Library
#include "app/framework/util/attribute-storage.h"
#include "app/framework/util/util.h"
#include "app/framework/util/af-event.h"

// ZDO - ZigBee Device Object
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "app/util/zigbee-framework/zigbee-device-host.h"

// Service discovery library
#include "service-discovery.h"

// Fragmentation
#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
#include "app/framework/plugin/fragmentation/fragmentation.h"
#endif
#ifdef EMBER_AF_PLUGIN_ADDRESS_TABLE
#include "app/framework/plugin/address-table/address-table.h"
#endif

// determines the number of in-clusters and out-clusters based on defines
// in config.h
#include "app/framework/util/af-main.h"

// Needed for zaTrustCenterSecurityPolicyInit()
#include "app/framework/security/af-security.h"

#ifdef GATEWAY_APP
  #define COMMAND_INTERPRETER_SUPPORT
  #include "app/util/gateway/gateway.h"
#endif

#include "app/util/security/security.h"  // Trust Center Address Cache
#include "app/util/common/form-and-join.h"

#include "app/framework/plugin/partner-link-key-exchange/partner-link-key-exchange.h"
#include "app/util/common/library.h"
#include "app/framework/security/crypto-state.h"

#include "znet-bookkeeping.h"

// This is used to store the local EUI of the NCP when using
// fake certificates.
// Fake certificates are constructed by setting the data to all F's
// but using the device's real IEEE in the cert.  The Key establishment
// code requires access to the local IEEE to do this.
EmberEUI64 emLocalEui64;

// APP_SERIAL is set in the project files
uint8_t serialPort = APP_SERIAL;

typedef struct {
  EmberNodeId nodeId;
  EmberPanId  panId;
  EmberNetworkStatus networkState;
  uint8_t radioChannel;
} NetworkCache;
static NetworkCache networkCache[EMBER_SUPPORTED_NETWORKS];

#define UNKNOWN_NETWORK_STATE 0xFF

// the stack version that the NCP is running
static uint16_t ncpStackVer;

#if defined(EMBER_TEST)
  #define EMBER_TEST_ASSERT(x) assert(x)
#else
  #define EMBER_TEST_ASSERT(x)
#endif

// ******************************************************************
// Globals

// when this is set to true it means the NCP has reported a serious error
// and the host needs to reset and re-init the NCP
static bool ncpNeedsResetAndInit = false;

// Declarations related to idling
#ifndef EMBER_TASK_COUNT
  #define EMBER_TASK_COUNT (3)
#endif
EmberTaskControl emTasks[EMBER_TASK_COUNT];
const uint8_t emTaskCount = EMBER_TASK_COUNT;

#if defined (GATEWAY_APP)
// Baud rate on the gateway application is meaningless, but we must
// define it satisfy compilation.
  #define BAUD_RATE BAUD_115200

// Port 1 on the gateway application is used for CLI, while port 0
// is used for "raw" binary.  We ignore what was set via App. Builder
// since only port 1 is used by the application.
  #undef APP_SERIAL
  #define APP_SERIAL 1

#elif (EMBER_AF_BAUD_RATE == 300)
  #define BAUD_RATE BAUD_300
#elif (EMBER_AF_BAUD_RATE == 600)
  #define BAUD_RATE BAUD_600
#elif (EMBER_AF_BAUD_RATE == 900)
  #define BAUD_RATE BAUD_900
#elif (EMBER_AF_BAUD_RATE == 1200)
  #define BAUD_RATE BAUD_1200
#elif (EMBER_AF_BAUD_RATE == 2400)
  #define BAUD_RATE BAUD_2400
#elif (EMBER_AF_BAUD_RATE == 4800)
  #define BAUD_RATE BAUD_4800
#elif (EMBER_AF_BAUD_RATE == 9600)
  #define BAUD_RATE BAUD_9600
#elif (EMBER_AF_BAUD_RATE == 14400)
  #define BAUD_RATE BAUD_14400
#elif (EMBER_AF_BAUD_RATE == 19200)
  #define BAUD_RATE BAUD_19200
#elif (EMBER_AF_BAUD_RATE == 28800)
  #define BAUD_RATE BAUD_28800
#elif (EMBER_AF_BAUD_RATE == 38400)
  #define BAUD_RATE BAUD_38400
#elif (EMBER_AF_BAUD_RATE == 50000)
  #define BAUD_RATE BAUD_50000
#elif (EMBER_AF_BAUD_RATE == 57600)
  #define BAUD_RATE BAUD_57600
#elif (EMBER_AF_BAUD_RATE == 76800)
  #define BAUD_RATE BAUD_76800
#elif (EMBER_AF_BAUD_RATE == 100000)
  #define BAUD_RATE BAUD_100000
#elif (EMBER_AF_BAUD_RATE == 115200)
  #define BAUD_RATE BAUD_115200
#elif (EMBER_AF_BAUD_RATE == 230400)
  #define BAUD_RATE BAUD_230400
#elif (EMBER_AF_BAUD_RATE == 460800)
  #define BAUD_RATE BAUD_460800
#else
  #error EMBER_AF_BAUD_RATE set to an invalid baud rate
#endif

#define MAX_CLUSTER (SECURE_EZSP_MAX_FRAME_LENGTH - 12) / 2 //currently == 94

// We only get the sender EUI callback when the sender EUI is in the incoming
// message. This keeps track of if the value in the variable is valid or not.
// This is set to VALID (true) when the callback happens and set to INVALID
// (false) at the end of IncomingMessageHandler.
static bool currentSenderEui64IsValid;
static EmberEUI64 currentSenderEui64;
static EmberNodeId currentSender = EMBER_NULL_NODE_ID;
static uint8_t currentBindingIndex = EMBER_NULL_BINDING;
static uint8_t currentAddressIndex = EMBER_NULL_ADDRESS_TABLE_INDEX;

#if defined(EMBER_TEST) && defined(EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM)
void emAfSetupFakeEepromForSimulation(void);
  #define SETUP_FAKE_EEPROM_FOR_SIMULATION() emAfSetupFakeEepromForSimulation()
#else
  #define SETUP_FAKE_EEPROM_FOR_SIMULATION()
#endif

#if defined(ZA_CLI_FULL)
  #define COMMAND_READER_INIT() emberCommandReaderInit()
#else
  #define COMMAND_READER_INIT()
#endif

#if defined(MAIN_FUNCTION_HAS_STANDARD_ARGUMENTS)
  #define APP_FRAMEWORK_MAIN_ARGUMENTS argc, argv
#else
  #define APP_FRAMEWORK_MAIN_ARGUMENTS 0, NULL
#endif

static uint16_t cachedConfigIdValues[EZSP_CONFIG_ID_MAX + 1];
static bool cacheConfigIdValuesAllowed = false;

static uint8_t ezspSequenceNumber = 0;

//------------------------------------------------------------------------------
// Forward declarations

#ifdef EMBER_AF_GENERATED_PLUGIN_NCP_INIT_FUNCTION_DECLARATIONS
EMBER_AF_GENERATED_PLUGIN_NCP_INIT_FUNCTION_DECLARATIONS
#endif

static void createEndpoint(uint8_t endpointIndex);
static uint8_t ezspNextSequence(void);

//------------------------------------------------------------------------------
// Functions

bool emberAfMemoryByteCompare(const uint8_t* pointer, uint8_t count, uint8_t byteValue)
{
  uint8_t i;
  for (i = 0; i < count; i++, pointer++) {
    if (*pointer != byteValue) {
      return false;
    }
  }
  return true;
}

static uint8_t ezspNextSequence(void)
{
  return ((++ezspSequenceNumber) & EMBER_AF_MESSAGE_TAG_MASK);
}

bool emberAfNcpNeedsReset(void)
{
  return ncpNeedsResetAndInit;
}

// Because an EZSP call can be expensive in terms of bandwidth,
// we cache the node ID so it can be quickly retrieved by the host.
EmberNodeId emberAfGetNodeId(void)
{
  uint8_t networkIndex = emberGetCurrentNetwork();
  if (networkCache[networkIndex].nodeId == EMBER_NULL_NODE_ID) {
    networkCache[networkIndex].nodeId = emberGetNodeId();
  }
  return networkCache[networkIndex].nodeId;
}

EmberPanId emberAfGetPanId(void)
{
  uint8_t networkIndex = emberGetCurrentNetwork();
  if (networkCache[networkIndex].panId == 0xFFFF) {
    EmberNodeType nodeType;
    EmberNetworkParameters parameters;
    emberAfGetNetworkParameters(&nodeType, &parameters);
    networkCache[networkIndex].panId = parameters.panId;
  }
  return networkCache[networkIndex].panId;
}

EmberNetworkStatus emberAfNetworkState(void)
{
  uint8_t networkIndex = emberGetCurrentNetwork();
  if (networkCache[networkIndex].networkState == UNKNOWN_NETWORK_STATE) {
    networkCache[networkIndex].networkState = emberNetworkState();
  }
  return networkCache[networkIndex].networkState;
}

uint8_t emberAfGetRadioChannel(void)
{
  uint8_t networkIndex = emberGetCurrentNetwork();
  if (networkCache[networkIndex].radioChannel == 0xFF) {
    EmberNodeType nodeType;
    EmberNetworkParameters parameters;
    emberAfGetNetworkParameters(&nodeType, &parameters);
    networkCache[networkIndex].radioChannel = parameters.radioChannel;
  }
  return networkCache[networkIndex].radioChannel;
}

void emberAfGetMfgString(uint8_t* returnData)
{
  static uint8_t mfgString[MFG_STRING_MAX_LENGTH];
  static bool mfgStringRetrieved = false;

  if (mfgStringRetrieved == false) {
    ezspGetMfgToken(EZSP_MFG_STRING, mfgString);
    mfgStringRetrieved = true;
  }
  // NOTE:  The MFG string is not NULL terminated.
  MEMMOVE(returnData, mfgString, MFG_STRING_MAX_LENGTH);
}

void emAfClearNetworkCache(uint8_t networkIndex)
{
  networkCache[networkIndex].nodeId = EMBER_NULL_NODE_ID;
  networkCache[networkIndex].panId = 0xFFFF;
  networkCache[networkIndex].networkState = UNKNOWN_NETWORK_STATE;
  networkCache[networkIndex].radioChannel = 0xFF;
}

// Some NCP's support a 'maximize packet buffer' call.  If that doesn't
// work, slowly ratchet up the packet buffer count.
static void setPacketBufferCount(void)
{
  uint16_t value;
  EzspStatus ezspStatus;
  EzspStatus maxOutBufferStatus;

  maxOutBufferStatus
    = ezspSetConfigurationValue(EZSP_CONFIG_PACKET_BUFFER_COUNT,
                                EZSP_MAXIMIZE_PACKET_BUFFER_COUNT);

  // We start from the default used by the NCP and increase up from there
  // rather than use a hard coded default in the code.
  // This is more portable to different NCP hardware (i.e. 357 vs. 260).
  ezspStatus = ezspGetConfigurationValue(EZSP_CONFIG_PACKET_BUFFER_COUNT,
                                         &value);

  if (maxOutBufferStatus == EZSP_SUCCESS) {
    emberAfAppPrintln("NCP supports maxing out packet buffers");
    goto setPacketBufferCountDone;
  }

  while (ezspStatus == EZSP_SUCCESS) {
    value++;
    ezspStatus = ezspSetConfigurationValue(EZSP_CONFIG_PACKET_BUFFER_COUNT,
                                           value);
  }

  setPacketBufferCountDone:
  emberAfAppPrintln("Ezsp Config: set packet buffers to %d",
                    (maxOutBufferStatus == EZSP_SUCCESS
                     ? value
                     : value - 1));
  emberAfAppFlush();
}

// initialize the network co-processor (NCP)
void emAfResetAndInitNCP(void)
{
  uint8_t ep;
  EzspStatus ezspStatus;
  bool memoryAllocation;
  uint16_t seed0, seed1;

  emberAfPreNcpResetCallback();

  // ezspInit resets the NCP by calling halNcpHardReset on a SPI host or
  // ashResetNcp on a UART host
  ezspStatus = ezspInit();

  if (ezspStatus != EZSP_SUCCESS) {
    emberAfCorePrintln("ERROR: ezspForceReset 0x%x", ezspStatus);
    emberAfCoreFlush();
    assert(false);
  }

  // send the version command before any other commands
  emAfCliVersionCommand();

  emSecureEzspInit();

  // The random number generator on the host needs to be seeded with some
  // random data, which we can get from the NCP.
  ezspGetRandomNumber(&seed0);
  ezspGetRandomNumber(&seed1);
  halStackSeedRandom(((uint32_t)seed1 << 16) | (uint32_t)seed0);

  emberAfSetEzspConfigValue(EZSP_CONFIG_SECURITY_LEVEL,
                            EMBER_SECURITY_LEVEL,
                            "security level");

  // set the address table size
  emberAfSetEzspConfigValue(EZSP_CONFIG_ADDRESS_TABLE_SIZE,
                            EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE,
                            "address table size");

  // set the trust center address cache size
  emberAfSetEzspConfigValue(EZSP_CONFIG_TRUST_CENTER_ADDRESS_CACHE_SIZE,
                            EMBER_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE,
                            "TC addr cache");

  // the stack profile is defined in the config file
  emberAfSetEzspConfigValue(EZSP_CONFIG_STACK_PROFILE,
                            EMBER_STACK_PROFILE,
                            "stack profile");

  // BUG 14222: If stack profile is 2 (ZigBee Pro), we need to enforce
  // the standard stack configuration values for that feature set.
  if ( EMBER_STACK_PROFILE == 2 ) {
    // MAC indirect timeout should be 7.68 secs
    emberAfSetEzspConfigValue(EZSP_CONFIG_INDIRECT_TRANSMISSION_TIMEOUT,
                              7680,
                              "MAC indirect TX timeout");

    // Max hops should be 2 * nwkMaxDepth, where nwkMaxDepth is 15
    emberAfSetEzspConfigValue(EZSP_CONFIG_MAX_HOPS,
                              30,
                              "max hops");
  }

  emberAfSetEzspConfigValue(EZSP_CONFIG_TX_POWER_MODE,
                            EMBER_AF_TX_POWER_MODE,
                            "tx power mode");

  emberAfSetEzspConfigValue(EZSP_CONFIG_SUPPORTED_NETWORKS,
                            EMBER_SUPPORTED_NETWORKS,
                            "supported networks");

  uint8_t mode = EMBER_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE;

  emberAfSetEzspValue(EZSP_VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE,
                      1,       // value length
                      &mode,
                      "end device keep alive support mode");

  // allow other devices to modify the binding table
  emberAfSetEzspPolicy(EZSP_BINDING_MODIFICATION_POLICY,
                       EZSP_CHECK_BINDING_MODIFICATIONS_ARE_VALID_ENDPOINT_CLUSTERS,
                       "binding modify",
                       "allow for valid endpoints & clusters only");

  // return message tag and message contents in ezspMessageSentHandler()
  emberAfSetEzspPolicy(EZSP_MESSAGE_CONTENTS_IN_CALLBACK_POLICY,
                       EZSP_MESSAGE_TAG_AND_CONTENTS_IN_CALLBACK,
                       "message content in msgSent",
                       "return");

  {
    uint8_t value[2];
    value[0] = LOW_BYTE(EMBER_AF_INCOMING_BUFFER_LENGTH);
    value[1] = HIGH_BYTE(EMBER_AF_INCOMING_BUFFER_LENGTH);
    emberAfSetEzspValue(EZSP_VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE,
                        2,     // value length
                        value,
                        "maximum incoming transfer size");
    value[0] = LOW_BYTE(EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH);
    value[1] = HIGH_BYTE(EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH);
    emberAfSetEzspValue(EZSP_VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE,
                        2,     // value length
                        value,
                        "maximum outgoing transfer size");
  }

  // Set the manufacturing code. This is defined by ZigBee document 053874r10
  // Ember's ID is 0x1002 and is the default, but this can be overridden in App Builder.
  emberSetManufacturerCode(EMBER_AF_MANUFACTURER_CODE);

  // Call the plugin and user-specific NCP inits.  This is when configuration
  // that affects table sizes should occur, which means it must happen before
  // setPacketBufferCount.
  memoryAllocation = true;
#ifdef EMBER_AF_GENERATED_PLUGIN_NCP_INIT_FUNCTION_CALLS
  EMBER_AF_GENERATED_PLUGIN_NCP_INIT_FUNCTION_CALLS
#endif
  emberAfNcpInitCallback(memoryAllocation);

  setPacketBufferCount();

  // Call the plugin and user-specific NCP inits again.  This is where non-
  // sizing configuration should occur.
  memoryAllocation = false;
#ifdef EMBER_AF_GENERATED_PLUGIN_NCP_INIT_FUNCTION_CALLS
  EMBER_AF_GENERATED_PLUGIN_NCP_INIT_FUNCTION_CALLS
#endif
  emberAfNcpInitCallback(memoryAllocation);

#ifdef EMBER_AF_MAX_TOTAL_CLUSTER_COUNT
#if EMBER_AF_MAX_TOTAL_CLUSTER_COUNT > MAX_CLUSTER
#error "ERROR: too many clusters are enabled on some of the endpoints"
#endif
#endif  //#ifdef EMBER_AF_MAX_TOTAL_CLUSTER_COUNT

  // create endpoints
  for ( ep = 0; ep < emberAfEndpointCount(); ep++ ) {
    createEndpoint(ep);
  }

  EM_AF_NETWORK_INIT();

  MEMSET(cachedConfigIdValues, 0xFF, ((EZSP_CONFIG_ID_MAX + 1) * sizeof(uint16_t)));
  cacheConfigIdValuesAllowed = true;

  // Set the localEui64 global
  ezspGetEui64(emLocalEui64);

  // Initialize messageSentCallbacks table
  emAfInitializeMessageSentCallbackArray();
}

// *******************************************************************
// *******************************************************************
// The main() loop and the application's contribution.

void emberAfMainInit(void)
{
}

int emberAfMain(MAIN_FUNCTION_PARAMETERS)
{
  SETUP_FAKE_EEPROM_FOR_SIMULATION();

  //Initialize the hal
  halInit();
  INTERRUPTS_ON();  // Safe to enable interrupts at this point

  {
    int returnCode;
    if (emberAfMainStartCallback(&returnCode,
                                 APP_FRAMEWORK_MAIN_ARGUMENTS)) {
      return returnCode;
    }
  }

  emberSerialInit(APP_SERIAL, BAUD_RATE, PARITY_NONE, 1);

  emberAfAppPrintln("Reset info: %d (%p)",
                    halGetResetInfo(),
                    halGetResetString());
  emberAfCoreFlush();

  // This will initialize the stack of networks maintained by the framework,
  // including setting the default network.
  emAfInitializeNetworkIndexStack();

  // We must initialize the endpoint information first so
  // that they are correctly added by emAfResetAndInitNCP()
  emberAfEndpointConfigure();

  // initialize the network co-processor (NCP)
  emAfResetAndInitNCP();

  // initialize the ZCL framework
  emAfInit();

  COMMAND_READER_INIT();

  // main loop
  while (true) {
    halResetWatchdog();   // Periodically reset the watchdog.

    // see if the NCP has anything waiting to send us
    ezspTick();

    while (ezspCallbackPending()) {
      ezspCallback();
    }

    // check if we have hit an EZSP Error and need to reset and init the NCP
    if (ncpNeedsResetAndInit) {
      ncpNeedsResetAndInit = false;
      // re-initialize the NCP
      emAfResetAndInitNCP();
    }

    // Wait until ECC operations are done.  Don't allow any of the clusters
    // to send messages as the NCP is busy doing ECC
    if (emAfIsCryptoOperationInProgress()) {
      continue;
    }

    // let the ZCL Utils run - this should go after ezspTick
    emAfTick();

    emberSerialBufferTick();

    emberAfRunEvents();

#if defined(ZA_CLI_FULL)
    if (emberProcessCommandInput(APP_SERIAL)) {
#if !defined GATEWAY_APP
      // Gateway app. has its own way of handling the command-line prompt.
      emberAfGuaranteedPrint("%p>", ZA_PROMPT);
#endif
    }
#endif

#if defined(EMBER_TEST)
    if (1) {
      // Simulation only
      uint32_t timeToNextEventMax = emberMsToNextStackEvent();
      timeToNextEventMax = emberAfMsToNextEvent(timeToNextEventMax);
      simulatedTimePassesMs(timeToNextEventMax);
    }
#endif

    // After each interation through the main loop, our network index stack
    // should be empty and we should be on the default network index again.
    emAfAssertNetworkIndexStackIsEmpty();
  }
  return 0;
}

// ******************************************************************
// binding
// ******************************************************************
EmberStatus emberAfSendEndDeviceBind(uint8_t endpoint)
{
  EmberStatus status;
  EmberEUI64 eui;
  uint8_t inClusterCount, outClusterCount;
  EmberAfClusterId clusterList[MAX_CLUSTER];
  EmberAfClusterId *inClusterList;
  EmberAfClusterId *outClusterList;
  EmberAfProfileId profileId;
  EmberApsOption options = ((EMBER_AF_DEFAULT_APS_OPTIONS
                             | EMBER_APS_OPTION_SOURCE_EUI64)
                            & ~EMBER_APS_OPTION_RETRY);
  uint8_t index = emberAfIndexFromEndpoint(endpoint);
  if (index == 0xFF) {
    return EMBER_INVALID_ENDPOINT;
  }
  status = emberAfPushEndpointNetworkIndex(endpoint);
  if (status != EMBER_SUCCESS) {
    return status;
  }

  emberAfGetEui64(eui);

  emberAfZdoPrintln("send %x %2x ", endpoint, options);
  inClusterList = clusterList;
  inClusterCount = emberAfGetClustersFromEndpoint(endpoint,
                                                  inClusterList,
                                                  MAX_CLUSTER,
                                                  true); // server?
  outClusterList = clusterList + inClusterCount;
  outClusterCount = emberAfGetClustersFromEndpoint(endpoint,
                                                   outClusterList,
                                                   (MAX_CLUSTER
                                                    - inClusterCount),
                                                   false); // server?
  profileId = emberAfProfileIdFromIndex(index);

  status = ezspEndDeviceBindRequest(emberAfGetNodeId(),
                                    eui,
                                    endpoint,
                                    profileId,
                                    inClusterCount,  // cluster in count
                                    outClusterCount, // cluster out count
                                    inClusterList,   // list of input clusters
                                    outClusterList,  // list of output clusters
                                    options);
  emberAfZdoPrintln("done: %x.", status);

  emberAfPopNetworkIndex();
  return status;
}

// **********************************************************************
// this function sets an EZSP config value and prints out the results to
// the serial output
// **********************************************************************
EzspStatus emberAfSetEzspConfigValue(EzspConfigId configId,
                                     uint16_t value,
                                     const char * configIdName)
{
  EzspStatus ezspStatus = ezspSetConfigurationValue(configId, value);
  emberAfAppFlush();
  emberAfAppPrint("Ezsp Config: set %p to 0x%2x:", configIdName, value);

  emberAfAppDebugExec(emAfPrintStatus("set", ezspStatus));
  emberAfAppFlush();

  emberAfAppPrintln("");
  emberAfAppFlush();

  // If this fails, odds are the simulated NCP doesn't have enough
  // memory allocated to it.
  EMBER_TEST_ASSERT(ezspStatus == EZSP_SUCCESS);

  return ezspStatus;
}

// **********************************************************************
// this function sets an EZSP policy and prints out the results to
// the serial output
// **********************************************************************
EzspStatus emberAfSetEzspPolicy(EzspPolicyId policyId,
                                EzspDecisionId decisionId,
                                const char * policyName,
                                const char * decisionName)
{
  EzspStatus ezspStatus = ezspSetPolicy(policyId,
                                        decisionId);
  emberAfAppPrint("Ezsp Policy: set %p to \"%p\":",
                  policyName,
                  decisionName);
  emberAfAppDebugExec(emAfPrintStatus("set",
                                      ezspStatus));
  emberAfAppPrintln("");
  emberAfAppFlush();
  return ezspStatus;
}

// **********************************************************************
// this function sets an EZSP value and prints out the results to
// the serial output
// **********************************************************************
EzspStatus emberAfSetEzspValue(EzspValueId valueId,
                               uint8_t valueLength,
                               uint8_t *value,
                               const char * valueName)

{
  EzspStatus ezspStatus = ezspSetValue(valueId, valueLength, value);

  emberAfAppPrint("Ezsp Value : set %p to ", valueName);

  // print the value based on the length of the value
  // for length 1/2/4 bytes, fetch int of that length and promote to 32 bits for printing
  switch (valueLength) {
    case 1:
      emberAfAppPrint("0x%4x:", (uint32_t)(*value));
      break;
    case 2:
      emberAfAppPrint("0x%4x:", (uint32_t)(*((uint16_t *)value)));
      break;
    case 4:
      emberAfAppPrint("0x%4x:", (uint32_t)(*((uint32_t *)value)));
      break;
    default:
      emberAfAppPrint("{val of len %x}:", valueLength);
      break;
  }

  emberAfAppDebugExec(emAfPrintStatus("set", ezspStatus));
  emberAfAppPrintln("");
  emberAfAppFlush();
  return ezspStatus;
}

// ******************************************************************
// setup endpoints and clusters for responding to ZDO requests
// ******************************************************************

//
// Creates the endpoint for 260 by calling ezspAddEndpoint()
//
static void createEndpoint(uint8_t endpointIndex)
{
  uint16_t clusterList[MAX_CLUSTER];
  uint16_t *inClusterList;
  uint16_t *outClusterList;

  uint8_t endpoint = emberAfEndpointFromIndex(endpointIndex);

  //to cover the code path that gets here after unexpected NCP reset
  bool initiallyDisabled = !emberAfEndpointIsEnabled(endpoint);
  if (initiallyDisabled) {
    emberAfEndpointEnableDisable(endpoint, true);
  }

  uint8_t inClusterCount;
  uint8_t outClusterCount;

  {
    EmberStatus status = emberAfPushEndpointNetworkIndex(endpoint);
    if (status != EMBER_SUCCESS) {
      emberAfAppPrintln("Error in creating endpoint %d: 0x%x", endpoint, status);
      return;
    }
  }

  // Lay out clusters in the arrays.
  inClusterList = clusterList;
  inClusterCount = emberAfGetClustersFromEndpoint(endpoint, inClusterList, MAX_CLUSTER, true);

  outClusterList = clusterList + inClusterCount;
  outClusterCount = emberAfGetClustersFromEndpoint(endpoint, outClusterList, (MAX_CLUSTER - inClusterCount), false);

  // Call EZSP function with data.
  {
    EzspStatus status = ezspAddEndpoint(endpoint,
                                        emberAfProfileIdFromIndex(endpointIndex),
                                        emberAfDeviceIdFromIndex(endpointIndex),
                                        emberAfDeviceVersionFromIndex(endpointIndex),
                                        inClusterCount,
                                        outClusterCount,
                                        (uint16_t *)inClusterList,
                                        (uint16_t *)outClusterList);
    if (status == EZSP_SUCCESS) {
      //this is to sync the host and NCP, after an internal EZSP ERROR and NCP reset
      if (initiallyDisabled) {
        emberAfEndpointEnableDisable(endpoint, false);
      }
      emberAfAppPrintln("Ezsp Endpoint %d added, profile 0x%2x, in clusters: %d, out clusters %d",
                        endpoint,
                        emberAfProfileIdFromIndex(endpointIndex),
                        inClusterCount,
                        outClusterCount);
    } else {
      emberAfAppPrintln("Error in creating endpoint %d: 0x%x", endpoint, status);
    }
  }

  emberAfPopNetworkIndex();
}

// *******************************************************************
// Handlers required to use the Ember Stack.

// Called when the stack status changes, usually as a result of an
// attempt to form, join, or leave a network.
void ezspStackStatusHandler(EmberStatus status)
{
  emberAfPushCallbackNetworkIndex();
  emAfStackStatusHandler(status);
  emberAfPopNetworkIndex();
}

EmberNodeId emberGetSender(void)
{
  return currentSender;
}

uint8_t emberAfGetBindingIndex(void)
{
  return currentBindingIndex;
}

uint8_t emberAfGetAddressIndex(void)
{
  return currentAddressIndex;
}

// This is not called if the incoming message did not contain the EUI64 of
// the sender.
void ezspIncomingSenderEui64Handler(EmberEUI64 senderEui64)
{
  // current sender is now valid
  MEMMOVE(currentSenderEui64, senderEui64, EUI64_SIZE);
  currentSenderEui64IsValid = true;
}

EmberStatus emberGetSenderEui64(EmberEUI64 senderEui64)
{
  // if the current sender EUI is valid then copy it in and send it back
  if (currentSenderEui64IsValid) {
    MEMMOVE(senderEui64, currentSenderEui64, EUI64_SIZE);
    return EMBER_SUCCESS;
  }
  // in the not valid case just return error
  return EMBER_ERR_FATAL;
}

//
// ******************************************************************

void ezspIncomingMessageHandler(EmberIncomingMessageType type,
                                EmberApsFrame *apsFrame,
                                uint8_t lastHopLqi,
                                int8_t lastHopRssi,
                                EmberNodeId sender,
                                uint8_t bindingIndex,
                                uint8_t addressIndex,
                                uint8_t messageLength,
                                uint8_t *messageContents)
{
  uint8_t sourceRouteOverhead;
  emberAfPushCallbackNetworkIndex();

  // The following code caches valid Source Route overheads sent pro actively
  // by the NCP and uses it once to calculate the overhead for a target, after
  // which it gets cleared.
  sourceRouteOverhead = getSourceRouteOverhead(messageLength);
  emberAfSetSourceRouteOverheadCallback(sender, sourceRouteOverhead);

  currentSender = sender;
  currentBindingIndex = bindingIndex;
  currentAddressIndex = addressIndex;
  emAfIncomingMessageHandler(type,
                             apsFrame,
                             lastHopLqi,
                             lastHopRssi,
                             messageLength,
                             messageContents);
  currentSenderEui64IsValid = false;
  currentSender = EMBER_NULL_NODE_ID;
  currentBindingIndex = EMBER_NULL_BINDING;
  currentAddressIndex = EMBER_NULL_ADDRESS_TABLE_INDEX;

  // Invalidate the sourceRouteOverhead cached at the end of the current incomingMessageHandler
  emberAfSetSourceRouteOverheadCallback(sender, EZSP_SOURCE_ROUTE_OVERHEAD_UNKNOWN);
  emberAfPopNetworkIndex();
}

// Called when a message we sent is acked by the destination or when an
// ack fails to arrive after several retransmissions.
void ezspMessageSentHandler(EmberOutgoingMessageType type,
                            uint16_t indexOrDestination,
                            EmberApsFrame *apsFrame,
                            uint8_t messageTag,
                            EmberStatus status,
                            uint8_t messageLength,
                            uint8_t *messageContents)
{
  emberAfPushCallbackNetworkIndex();
#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
  if (emAfFragmentationMessageSent(apsFrame, status)) {
    goto kickout;
  }
#endif //EMBER_AF_PLUGIN_FRAGMENTATION
  emAfMessageSentHandler(type,
                         indexOrDestination,
                         apsFrame,
                         status,
                         messageLength,
                         messageContents,
                         messageTag);
  goto kickout; // silence a warning when not using fragmentation
  kickout:
  emberAfPopNetworkIndex();
}

void emberChildJoinHandler(uint8_t index, bool joining)
{
}

// This is called when an EZSP error is reported
void ezspErrorHandler(EzspStatus status)
{
  emberAfCorePrintln("ERROR: ezspErrorHandler 0x%x", status);
  emberAfCoreFlush();

  // Rather than detect whether or not we can recover from the error,
  // we just flag the NCP for reboot.
  // Do not reset if this is a decryption failure, as we ignored the packet.
  if (status != EZSP_ERROR_SECURITY_PARAMETERS_INVALID) {
    ncpNeedsResetAndInit = true;
  }
}

EmberStatus emAfSend(EmberOutgoingMessageType type,
                     uint16_t indexOrDestination,
                     EmberApsFrame *apsFrame,
                     uint8_t messageLength,
                     uint8_t *message,
                     uint8_t *messageTag,
                     EmberNodeId alias,
                     uint8_t sequence)
{
  EmberStatus status = EMBER_SUCCESS;
  *messageTag = ezspNextSequence();

  switch (type) {
    case EMBER_OUTGOING_VIA_BINDING:
    case EMBER_OUTGOING_VIA_ADDRESS_TABLE:
    case EMBER_OUTGOING_DIRECT:
    {
      status = ezspSendUnicast(type,
                               indexOrDestination,
                               apsFrame,
                               *messageTag,
                               (uint8_t)messageLength,
                               message,
                               &apsFrame->sequence);

      break;
    }
    case EMBER_OUTGOING_MULTICAST:
    {
      status = ezspSendMulticast(apsFrame,
                                 ZA_MAX_HOPS, // hops
                                 ZA_MAX_HOPS, // nonmember radius
                                 *messageTag,
                                 messageLength,
                                 message,
                                 &apsFrame->sequence);
      break;
    }
    case EMBER_OUTGOING_MULTICAST_WITH_ALIAS:
    {
      status = ezspSendMulticastWithAlias(apsFrame,
                                          apsFrame->radius,  //radius
                                          apsFrame->radius,  //nonmember radius
                                          alias,
                                          sequence,
                                          *messageTag,
                                          messageLength,
                                          message,
                                          &apsFrame->sequence);
      break;
    }
    case EMBER_OUTGOING_BROADCAST:
    {
      status = ezspSendBroadcast(indexOrDestination,
                                 apsFrame,
                                 ZA_MAX_HOPS, // radius
                                 *messageTag,
                                 messageLength,
                                 message,
                                 &apsFrame->sequence);
      break;
    }
    case EMBER_OUTGOING_BROADCAST_WITH_ALIAS:
      status = ezspProxyBroadcast(alias,
                                  indexOrDestination,
                                  sequence,
                                  apsFrame,
                                  apsFrame->radius,   // radius
                                  *messageTag,
                                  messageLength,
                                  message,
                                  &apsFrame->sequence);
      break;
    default:
      status = EMBER_BAD_ARGUMENT;
  }

  return status;
}

// Platform dependent interface to get various stack parameters.
void emberAfGetEui64(EmberEUI64 returnEui64)
{
  MEMCOPY(returnEui64, emLocalEui64, EUI64_SIZE);
}

EmberStatus emberAfGetNetworkParameters(EmberNodeType* nodeType,
                                        EmberNetworkParameters* parameters)
{
  return ezspGetNetworkParameters(nodeType, parameters);
}

EmberStatus emberAfGetNodeType(EmberNodeType *nodeType)
{
  EmberNetworkParameters parameters;
  return ezspGetNetworkParameters(nodeType, &parameters);
}

// This will cache all config items to make sure repeated calls do not
// go all the way to the NCP.
uint8_t emberAfGetNcpConfigItem(EzspConfigId id)
{
  // In case we can't cache config items yet, we need a temp
  // variable to store the retrieved EZSP config ID.
  uint16_t temp = 0xFFFF;
  uint16_t *configItemPtr = &temp;
  bool cacheValid;

  EMBER_TEST_ASSERT(id <= EZSP_CONFIG_ID_MAX);

  cacheValid = (cacheConfigIdValuesAllowed
                && id <= EZSP_CONFIG_ID_MAX);

  if (cacheValid) {
    configItemPtr = &(cachedConfigIdValues[id]);
  }

  if (*configItemPtr == 0xFFFF
      && EZSP_SUCCESS != ezspGetConfigurationValue(id,
                                                   configItemPtr)) {
    // We return a 0 size (for tables) on error to prevent code from using the
    // invalid value of 0xFFFF.  This is particularly necessary for loops that
    // iterate over all indexes.
    return 0;
  }

  return (uint8_t)(*configItemPtr);
}

EmberStatus emberAfGetSourceRouteTableEntry(
  uint8_t index,
  EmberNodeId *destination,
  uint8_t *closerIndex)
{
  return ezspGetSourceRouteTableEntry(index,
                                      destination,
                                      closerIndex);
}

uint8_t emberAfGetSourceRouteTableFilledSize(void)
{
  return ezspGetSourceRouteTableFilledSize();
}

uint8_t emberAfGetSourceRouteTableTotalSize(void)
{
  return ezspGetSourceRouteTableTotalSize();
}

EmberStatus emberAfGetChildData(uint8_t index,
                                EmberChildData *childData)
{
  return ezspGetChildData(index,
                          childData);
}

uint8_t emberAfGetChildTableSize(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_MAX_END_DEVICE_CHILDREN);
}

uint8_t emberAfGetKeyTableSize(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_KEY_TABLE_SIZE);
}

uint8_t emberAfGetAddressTableSize(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_ADDRESS_TABLE_SIZE);
}

uint8_t emberAfGetBindingTableSize(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_BINDING_TABLE_SIZE);
}

uint8_t emberAfGetNeighborTableSize(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_NEIGHBOR_TABLE_SIZE);
}

uint8_t emberAfGetRouteTableSize(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_ROUTE_TABLE_SIZE);
}

uint8_t emberAfGetSecurityLevel(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_SECURITY_LEVEL);
}

uint8_t emberAfGetStackProfile(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_STACK_PROFILE);
}

uint8_t emberAfGetSleepyMulticastConfig(void)
{
  return emberAfGetNcpConfigItem(EZSP_CONFIG_SEND_MULTICASTS_TO_SLEEPY_ADDRESS);
}

// On the System-on-a-chip this function is provided by the stack.
// Here is a copy for the host based applications.
void emberReverseMemCopy(uint8_t* dest, const uint8_t* src, uint16_t length)
{
  uint16_t i;
  uint16_t j = (length - 1);

  for ( i = 0; i < length; i++) {
    dest[i] = src[j];
    j--;
  }
}

// ******************************************************************
// Functions called by the Serial Command Line Interface (CLI)
// ******************************************************************

// *****************************
// emAfCliVersionCommand
//
// version <no arguments>
// *****************************
void emAfCliVersionCommand(void)
{
  // Note that NCP == Network Co-Processor

  EmberVersion versionStruct;

  // the EZSP protocol version that the NCP is using
  uint8_t ncpEzspProtocolVer;

  // the stackType that the NCP is running
  uint8_t ncpStackType;

  // the EZSP protocol version that the Host is running
  // we are the host so we set this value
  uint8_t hostEzspProtocolVer = EZSP_PROTOCOL_VERSION;

  // send the Host version number to the NCP. The NCP returns the EZSP
  // version that the NCP is running along with the stackType and stackVersion
  ncpEzspProtocolVer = ezspVersion(hostEzspProtocolVer,
                                   &ncpStackType,
                                   &ncpStackVer);

  // verify that the stack type is what is expected
  if (ncpStackType != EZSP_STACK_TYPE_MESH) {
    emberAfAppPrint("ERROR: stack type 0x%x is not expected!",
                    ncpStackType);
    assert(false);
  }

  // verify that the NCP EZSP Protocol version is what is expected
  if (ncpEzspProtocolVer != EZSP_PROTOCOL_VERSION) {
    emberAfAppPrint("ERROR: NCP EZSP protocol version of 0x%x does not match Host version 0x%x\r\n",
                    ncpEzspProtocolVer,
                    hostEzspProtocolVer);
    assert(false);
  }

  emberAfAppPrint("ezsp ver 0x%x stack type 0x%x ",
                  ncpEzspProtocolVer, ncpStackType, ncpStackVer);

  if (EZSP_SUCCESS != ezspGetVersionStruct(&versionStruct)) {
    // NCP has Old style version number
    emberAfAppPrintln("stack ver [0x%2x]", ncpStackVer);
  } else {
    // NCP has new style version number
    emAfParseAndPrintVersion(versionStruct);
  }
  emberAfAppFlush();
}

uint8_t emAfGetPacketBufferFreeCount(void)
{
  uint8_t freeCount;
  uint8_t valueLength = 1;
  ezspGetValue(EZSP_VALUE_FREE_BUFFERS,
               &valueLength,
               &freeCount);
  return freeCount;
}

uint8_t emAfGetPacketBufferTotalCount(void)
{
  uint16_t value;
  ezspGetConfigurationValue(EZSP_CONFIG_PACKET_BUFFER_COUNT,
                            &value);
  return (uint8_t)value;
}

// WARNING:  This function executes in ISR context
void halNcpIsAwakeIsr(bool isAwake)
{
  if (isAwake) {
    emberAfNcpIsAwakeIsrCallback();
  } else {
    // If we got indication that the NCP failed to wake up
    // there is not much that can be done.  We will reset the
    // host (which in turn will reset the NCP) and that will
    // hopefully bring things back in sync.
    assert(0);
  }
}

void ezspNetworkFoundHandler(EmberZigbeeNetwork *networkFound,
                             uint8_t lqi,
                             int8_t rssi)
{
  emberAfPushCallbackNetworkIndex();

#ifdef EMBER_AF_PLUGIN_FORM_AND_JOIN
  if (!emberFormAndJoinNetworkFoundHandler(networkFound, lqi, rssi))
#endif
  emberAfNetworkFoundCallback(networkFound, lqi, rssi);

  emberAfPopNetworkIndex();
}

void ezspScanCompleteHandler(uint8_t channel, EmberStatus status)
{
  emberAfPushCallbackNetworkIndex();

#ifdef EMBER_AF_PLUGIN_FORM_AND_JOIN
  if (!emberFormAndJoinScanCompleteHandler(channel, status))
#endif
  emberAfScanCompleteCallback(channel, status);

  emberAfPopNetworkIndex();
}

void ezspEnergyScanResultHandler(uint8_t channel, int8_t rssi)
{
  emberAfPushCallbackNetworkIndex();

#ifdef EMBER_AF_PLUGIN_FORM_AND_JOIN
  if (!emberFormAndJoinEnergyScanResultHandler(channel, rssi))
#endif
  emberAfEnergyScanResultCallback(channel, rssi);

  emberAfPopNetworkIndex();
}

void ezspUnusedPanIdFoundHandler(EmberPanId panId, uint8_t channel)
{
  emberAfPushCallbackNetworkIndex();

#ifdef EMBER_AF_PLUGIN_FORM_AND_JOIN
  if (!emberFormAndJoinUnusedPanIdFoundHandler(panId, channel))
#endif
  emberAfUnusedPanIdFoundCallback(panId, channel);

  emberAfPopNetworkIndex();
}

void emAfPrintEzspEndpointFlags(uint8_t endpoint)
{
  EzspEndpointFlags flags;
  EzspStatus status = ezspGetEndpointFlags(endpoint,
                                           &flags);
  if (status != EZSP_SUCCESS) {
    emberAfCorePrint("Error retrieving EZSP endpoint flags.");
  } else {
    emberAfCorePrint("- EZSP Endpoint flags: 0x%2X", flags);
  }
}

void emSetAddDelay(uint8_t delay)
{
  //XXXEZSP
}

void ezspZigbeeKeyEstablishmentHandler(EmberEUI64 partner,
                                       EmberKeyStatus status)
{
  // This function is generated.
  emAfZigbeeKeyEstablishment(partner, status);
}
