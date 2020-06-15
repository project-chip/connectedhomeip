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
 * @file StandardizedRfTesting_callbacks.c
 * @brief Example callbacks file for Zigbee Alliance TIS/TRP Application
 * @author Silicon Labs
 * @version 4.8
 *******************************************************************************
   ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"
#include "stack/include/mfglib.h"

//////////////////////////////////////////////
// forward declarations
//////////////////////////////////////////////

void mfglibRxHandler(int8u *packet, int8u linkQuality, int8s rssi);

// Event control struct declarations
EmberEventControl sendPingEventControl;
EmberEventControl sendAckEventControl;
EmberEventControl sendSetChannelEventControl;
EmberEventControl sendGetChannelResponseEventControl;
EmberEventControl sendSetPowerEventControl;
EmberEventControl sendGetPowerEventControl;
EmberEventControl sendGetPowerResponseEventControl;
EmberEventControl sendStreamEventControl;
EmberEventControl sendStartRxTestEventControl;
EmberEventControl sendEndRxTestEventControl;
EmberEventControl sendReportEventControl;
EmberEventControl sendRebootEventControl;
EmberEventControl sendGetRebootByteResponseEventControl;
EmberEventControl sendGetHardwareVersionEventControl;
EmberEventControl sendGetHardwareVersionResponseEventControl;
EmberEventControl sendGetSoftwareVersionEventControl;
EmberEventControl sendGetSoftwareVersionResponseEventControl;
EmberEventControl pingTimerEventControl;
EmberEventControl setChannelEventControl;
EmberEventControl changeChannelWithPingEventControl;
EmberEventControl rebootWithPingEventControl;
EmberEventControl findWithPingEventControl;
EmberEventControl streamTimerEventControl;
EmberEventControl custom1EventControl; // rebootControl
EmberEventControl custom2EventControl;
EmberEventControl custom3EventControl;
EmberEventControl custom4EventControl;
EmberEventControl custom5EventControl;
EmberEventControl custom6EventControl;
EmberEventControl initEventControl; //for initialization

// Event function forward declarations
void sendPingEventFunction(void);
void sendAckEventFunction(void);
void sendSetChannelEventFunction(void);
void sendGetChannelResponseEventFunction(void);
void sendSetPowerEventFunction(void);
void sendGetPowerEventFunction(void);
void sendGetPowerResponseEventFunction(void);
void sendStreamEventFunction(void);
void sendStartRxTestEventFunction(void);
void sendEndRxTestEventFunction(void);
void sendReportEventFunction(void);
void sendRebootEventFunction(void);
void sendGetRebootByteResponseEventFunction(void);
void sendGetHardwareVersionEventFunction(void);
void sendGetHardwareVersionResponseEventFunction(void);
void sendGetSoftwareVersionEventFunction(void);
void sendGetSoftwareVersionResponseEventFunction(void);
void pingTimerEventFunction(void);
void setChannelEventFunction(void);
void changeChannelWithPingEventFunction(void);
void rebootWithPingEventFunction(void);
void findWithPingEventFunction(void);
void streamTimerEventFunction(void);
void custom1EventFunction(void);
void custom2EventFunction(void);
void custom3EventFunction(void);
void custom4EventFunction(void);
void custom5EventFunction(void);
void custom6EventFunction(void);
void initEventFunction(void);

// commands
void customRping(void);
void customLpingtimeout(void);
void customSetchannel(void);
void customLsetchannel(void);
void customRsetchannel(void);
void customLgetchannel(void);
void customLsetpower(void);
void customRsetpower(void);
void customLgetpower(void);
void customRgetpower(void);
void customRstream(void);
void customRstart(void);
void customRend(void);
void customReboot(void);
void customLreboot(void);
void customRreboot(void);
void customFind(void);
void customRhardwareversion(void);
void customRsoftwareversion(void);

// silabs commands
void customSilabsTest(void);
void customSilabsGetLocalVersion(void);
void customSilabsTest16(void);
void customSilabsTest32(void);
void customSilabsSetChannel(void);
void customSilabsGetChannel(void);
void customSilabsGetPower(void);
void customSilabsTx (void);
void customSilabsListChannelPower(void);
void customSilabsLocalReport(void);

// helpers
void helperSendAck(void);
void helperLocalChannelWithPower(int8u channel);
int8u helperFromChannelMaskToChannel(int32u channelMask);
void helperSetChannelFromMask(int32u channelMask);
void helperFromChannelMaskToGlobalChannelMaskBytes(int32u channelMask);
void sendSetChannelHelper(void);
int32u helperFromChannelToChannelMask(int8u channel);
void helperStream(int16u timeout);
void helperInit(void);
void helperSetPower(void);
void helperLocalReboot(void);
void helperSendReboot(void);
void helperAppInit(void); //init application

//////////////////////////////////////////////
// defines and variables
//////////////////////////////////////////////

// command frame headers
#define BYTE_SIGNATURE_FIRST  0xDE
#define BYTE_SIGNATURE_SECOND 0xDE
#define BYTE_TYPE_COMMAND 0x00
#define BYTE_TYPE_DATA    0x01

// custom commands
#define BYTE_COMMAND_PING                     0x00
#define BYTE_COMMAND_ACK                      0x01
#define BYTE_COMMAND_SET_CHANNEL              0x02
#define BYTE_COMMAND_SET_POWER                0x05
#define BYTE_COMMAND_GET_POWER                0x06
#define BYTE_COMMAND_GET_POWER_RESPONSE       0x07
#define BYTE_COMMAND_STREAM                   0x09
#define BYTE_COMMAND_START_RX_TEST            0x0A
#define BYTE_COMMAND_END_RX_TEST              0x0B
#define BYTE_COMMAND_REPORT                   0x0C
#define BYTE_GET_HARDWARE_VERSION             0x11
#define BYTE_GET_HARDWARE_VERSION_RESPONSE    0x12
#define BYTE_GET_SOFTWARE_VERSION             0x13
#define BYTE_GET_SOFTWARE_VERSION_RESPONSE    0x14

// custom silabs commands
#define BYTE_COMMAND_GET_CHANNEL_RESPONSE     0x04
#define BYTE_COMMAND_REBOOT                   0x0D
#define BYTE_COMMAND_GET_REBOOT_BYTE_RESPONSE 0x10

// buffers
#define TX_CMD_BUFF_LENGTH 30
#define RX_CMD_BUFF_LENGTH 30

int8u txCmdBuff[TX_CMD_BUFF_LENGTH];

int8u rChannel;

int32u numPackets = 0;
int32u numProtocolPackets = 0;
int32u totalLqi = 0;
int32u totalRssi = 0;

enum {
  STATE_LOCAL_INIT,
  STATE_LOCAL_PINGING,
  STATE_LOCAL_REBOOTING_DELAYING,
  STATE_LOCAL_REBOOTING_PINGING,
  STATE_LOCAL_CHANGING_CHANNEL_DELAYING,
  STATE_LOCAL_CHANGING_CHANNEL_PINGING,
  STATE_LOCAL_FINDING_DUT_DELAYING,
  STATE_LOCAL_FINDING_DUT_PINGING,
  STATE_LOCAL_STREAMING,
  STATE_LOCAL_GETTING_VERSION,
  STATE_LOCAL_GETTING_HW_VERSION,
  STATE_LOCAL_SENDING_HW_VERSION,
  STATE_LOCAL_GETTING_SW_VERSION,
  STATE_LOCAL_SENDING_SW_VERSION,
};

int8u deviceState;

// for passing to events
int8u  globalChannel;
int16u globalTimeout;
int32u globalChannelMask;
int8u  globalChannelMaskByte0;
int8u  globalChannelMaskByte1;
int8u  globalChannelMaskByte2;
int8u  globalChannelMaskByte3;
int8u  globalPowerMode0;
int8u  globalPowerMode1;
int8s  globalPower;
int8u  globalDuration0;
int8u  globalDuration1;
int8u  globalRebootByte;

// for transmitting
int16u globalNumPackets;
int16u globalTotalNumPackets;

// for rping timeout (in ms)
int16u globalRpingTimeout = DEFAULT_RPING_TIMEOUT_IN_MS;

//////////////////////////////////////////////
// commands
//////////////////////////////////////////////

/**
 * Custom CLI.  This command tree is executed by typing "custom <command>"
 * See app/util/serial/command-interpreter2.h for more detail on writing commands.
 **/

static EmberCommandEntry customSilabsMenu[] = {
  emberCommandEntryAction("test", customSilabsTest, "", ""),
  emberCommandEntryAction("lver", customSilabsGetLocalVersion, "", ""),
  emberCommandEntryAction("test16", customSilabsTest16, "uu", ""),
  emberCommandEntryAction("test32", customSilabsTest32, "uuuu", ""),
  emberCommandEntryAction("setchannel", customSilabsSetChannel, "u", ""),
  emberCommandEntryAction("getchannel", customSilabsGetChannel, "", ""),
  emberCommandEntryAction("getpower", customSilabsGetPower, "", ""),
  emberCommandEntryAction("tx", customSilabsTx, "u", ""),
  emberCommandEntryAction("listchpower", customSilabsListChannelPower, "", ""),
  emberCommandEntryAction("lreport", customSilabsLocalReport, "", ""),
  emberCommandEntryTerminator()
};

EmberCommandEntry emberAfCustomCommands[] = {
  /* Sample Custom CLI commands */
  // emberCommandEntrySubMenu("sub-menu", customSubMenu, "Sub menu of custom commands"),
  // emberCommandEntryAction("action", actionFunction, "", "Description of action command."),
  emberCommandEntryAction("rping", customRping, "", ""),
  emberCommandEntryAction("lpingtimeout", customLpingtimeout, "uu", ""),
  emberCommandEntryAction("setchannel", customSetchannel, "uuuu", ""),
  emberCommandEntryAction("lsetchannel", customLsetchannel, "uuuu", ""),
  emberCommandEntryAction("rsetchannel", customRsetchannel, "uuuu", ""),
  emberCommandEntryAction("lgetchannel", customLgetchannel, "", ""),
  emberCommandEntryAction("lsetpower", customLsetpower, "v", ""),
  emberCommandEntryAction("rsetpower", customRsetpower, "v", ""),
  emberCommandEntryAction("lgetpower", customLgetpower, "", ""),
  emberCommandEntryAction("rgetpower", customRgetpower, "", ""),
  emberCommandEntryAction("rstream", customRstream, "uu", ""),
  emberCommandEntryAction("rstart", customRstart, "", ""),
  emberCommandEntryAction("rend", customRend, "", ""),
  emberCommandEntryAction("reboot", customReboot, "", ""),
  emberCommandEntryAction("lreboot", customLreboot, "", ""),
  emberCommandEntryAction("rreboot", customRreboot, "", ""),
  emberCommandEntryAction("find", customFind, "", ""),
  emberCommandEntryAction("rhardwareversion", customRhardwareversion, "", ""),
  emberCommandEntryAction("rsoftwareversion", customRsoftwareversion, "", ""),
  emberCommandEntrySubMenu("silabs", customSilabsMenu, ""),
  emberCommandEntryTerminator()
};

//////////////////////////////////////////////
// command functions
//////////////////////////////////////////////

void customRping(void)
{
  deviceState = STATE_LOCAL_PINGING;
  emberEventControlSetActive(sendPingEventControl);
  emberEventControlSetDelayMS(pingTimerEventControl, globalRpingTimeout);
}

void customLpingtimeout(void)
{
  int8u byte0 = emberUnsignedCommandArgument(0);
  int8u byte1 = emberUnsignedCommandArgument(1);

  globalRpingTimeout = (byte0 << 8) + byte1;

  emberAfCorePrintln("globalRpingTimeout 0x%2x %d\r\n", globalRpingTimeout);
}

void customSetchannel(void)
{
  globalChannelMaskByte0 = emberUnsignedCommandArgument(0);
  globalChannelMaskByte1 = emberUnsignedCommandArgument(1);
  globalChannelMaskByte2 = emberUnsignedCommandArgument(2);
  globalChannelMaskByte3 = emberUnsignedCommandArgument(3);
  emberEventControlSetActive(sendSetChannelEventControl);
  deviceState = STATE_LOCAL_CHANGING_CHANNEL_DELAYING;
  emberEventControlSetDelayMS(changeChannelWithPingEventControl, 500);
}

void customLsetchannel(void)
{
  globalChannelMaskByte0 = emberUnsignedCommandArgument(0);
  globalChannelMaskByte1 = emberUnsignedCommandArgument(1);
  globalChannelMaskByte2 = emberUnsignedCommandArgument(2);
  globalChannelMaskByte3 = emberUnsignedCommandArgument(3);

  emberEventControlSetActive(setChannelEventControl);
}

void customRsetchannel(void)
{
  globalChannelMaskByte0 = emberUnsignedCommandArgument(0);
  globalChannelMaskByte1 = emberUnsignedCommandArgument(1);
  globalChannelMaskByte2 = emberUnsignedCommandArgument(2);
  globalChannelMaskByte3 = emberUnsignedCommandArgument(3);

  emberEventControlSetActive(sendSetChannelEventControl);
}

void customLgetchannel(void)
{
  int8u localChannel = mfglibGetChannel();
  int32u channelMask = helperFromChannelToChannelMask(localChannel);
  helperFromChannelMaskToGlobalChannelMaskBytes(channelMask);

  emberAfCorePrintln("channel 0x%x%x%x%x\r\n",
                     globalChannelMaskByte0, globalChannelMaskByte1, globalChannelMaskByte2, globalChannelMaskByte3);
}

void customLsetpower(void)
{
  EmberStatus status;

  // should use helper here
  // on dut, ignoring the first 2 params
  int8s pow = emberSignedCommandArgument(0);
  status = mfglibSetPower(RF_APP_TX_POWER_MODE, pow);
  emberAfCorePrintln("pow (st 0x%x)\r\n", status);
}

void customRsetpower(void)
{
  globalPower = emberSignedCommandArgument(0);
  emberEventControlSetActive(sendSetPowerEventControl);
}

void customLgetpower(void)
{
  int8s pow = mfglibGetPower();
  emberAfCorePrintln("power 0x%x\r\n", pow);
}

void customRgetpower(void)
{
  emberEventControlSetActive(sendGetPowerEventControl);
}

void customRstream(void)
{
  globalDuration0 = emberUnsignedCommandArgument(0);
  globalDuration1 = emberUnsignedCommandArgument(1);

  emberEventControlSetActive(sendStreamEventControl);
}

void customRstart(void)
{
  emberEventControlSetActive(sendStartRxTestEventControl);
}

void customRend(void)
{
  emberEventControlSetActive(sendEndRxTestEventControl);
}

void customReboot(void)
{
  helperSendReboot();
  deviceState = STATE_LOCAL_REBOOTING_DELAYING;
  emberEventControlSetDelayMS(rebootWithPingEventControl, 500);
}

void customLreboot(void)
{
  helperLocalReboot();
}

void customRreboot(void)
{
  helperSendReboot();
}

void customFind(void)
{
  int8u localChannel;

  localChannel = mfglibGetChannel();
  int32u channelMask = helperFromChannelToChannelMask(localChannel);
  helperFromChannelMaskToGlobalChannelMaskBytes(channelMask);

  helperLocalChannelWithPower(11);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(12);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(13);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(14);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(15);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(16);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(17);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(18);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(19);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(20);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(21);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(22);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(23);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(24);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(25);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(26);
  sendSetChannelHelper();
  emberAfRunEvents();
  helperLocalChannelWithPower(localChannel);
  deviceState = STATE_LOCAL_FINDING_DUT_DELAYING;
  emberEventControlSetActive(findWithPingEventControl);
}

void customRhardwareversion(void)
{
  deviceState = STATE_LOCAL_GETTING_HW_VERSION;
  emberEventControlSetActive(sendGetHardwareVersionEventControl);
}

void customRsoftwareversion(void)
{
  deviceState = STATE_LOCAL_GETTING_SW_VERSION;
  emberEventControlSetActive(sendGetSoftwareVersionEventControl);
}

void customSilabsTest(void)
{
  emberAfCorePrintln("test1\r\n");
}

void customSilabsGetLocalVersion(void)
{
  emberAfCorePrintln("Local RF Application Version 0x%x%x\r\n", APP_VERSION_MAJOR, APP_VERSION_MINOR);
}

void customSilabsTest16(void)
{
  int8u byte0 = emberUnsignedCommandArgument(0);
  int8u byte1 = emberUnsignedCommandArgument(1);
  int16u result = (byte0 << 8) + byte1;

  emberAfCorePrintln("customSilabsTest16 0x%x 0x%x 0x%2x %d\r\n",
                     byte0, byte1, result, result);
}

void customSilabsTest32(void)
{
  int8u byte0 = emberUnsignedCommandArgument(0);
  int8u byte1 = emberUnsignedCommandArgument(1);
  int8u byte2 = emberUnsignedCommandArgument(2);
  int8u byte3 = emberUnsignedCommandArgument(3);
  int32u result = (byte0 << 24) + (byte1 << 16) + (byte2 << 8) + byte3;

  emberAfCorePrintln("customSilabsTest32 0x%x 0x%x 0x%x 0x%x 0x%4x %d\r\n",
                     byte0, byte1, byte2, byte3, result, result);
}

void customSilabsSetChannel(void)
{
  int8u tempChannel = emberUnsignedCommandArgument(0);
  helperLocalChannelWithPower(tempChannel);
}

void customSilabsGetChannel(void)
{
  int8u channel = mfglibGetChannel();
  emberAfCorePrintln("ch 0x%x\r\n", channel);
}

void customSilabsGetPower(void)
{
  int8s txPower = mfglibGetPower();
  emberAfCorePrintln("pow 0x%x\r\n", txPower);
}

void customSilabsTx(void)
{
  globalNumPackets = 0;
  globalTotalNumPackets = emberUnsignedCommandArgument(0);

  emberEventControlSetActive(custom2EventControl);
}

/* Local RX report - allows self-contained verification of RX sensitivity */
void customSilabsLocalReport(void)
{
  /* Print report */
  emberAfCorePrintln("Local Report: [total]0x%4x [protocol]0x%4x [totalLqi]0x%4x [totalRssiMgnitude]0x%4x",
                     numPackets, numProtocolPackets, totalLqi, totalRssi);

  //clear values after printing report
  helperInit();
}

//////////////////////////////////////////////
// event functions
//////////////////////////////////////////////

// Event function stubs
void sendPingEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendPingEventControl);

  txCmdBuff[0] = 6; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_PING;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("ping (st 0x%x)\r\n", status);
}

void sendAckEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendAckEventControl);

  txCmdBuff[0] = 6; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_ACK;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("ack (st 0x%x)\r\n", status);
}

void sendSetChannelEventFunction(void)
{
  emberEventControlSetInactive(sendSetChannelEventControl);
  sendSetChannelHelper();
}

void sendGetChannelResponseEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendGetChannelResponseEventControl);

  txCmdBuff[0] = 10; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_GET_CHANNEL_RESPONSE;
  txCmdBuff[5] = globalChannelMaskByte0;
  txCmdBuff[6] = globalChannelMaskByte1;
  txCmdBuff[7] = globalChannelMaskByte2;
  txCmdBuff[8] = globalChannelMaskByte3;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("get ch resp (st 0x%x)\r\n", status);
}

void sendSetPowerEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendSetPowerEventControl);

  txCmdBuff[0] = 7; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_SET_POWER;
  txCmdBuff[5] = globalPower;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("set pow (st 0x%x)\r\n", status);
}

void sendGetPowerEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendGetPowerEventControl);

  txCmdBuff[0] = 6; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_GET_POWER;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("get pow (st 0x%x)\r\n", status);
}

void sendGetPowerResponseEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendGetPowerResponseEventControl);

  txCmdBuff[0] = 7; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_GET_POWER_RESPONSE;
  txCmdBuff[5] = globalPower;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("get pow resp (st 0x%x)\r\n", status);
}

void sendStreamEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendStreamEventControl);

  txCmdBuff[0] = 8; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_STREAM;
  txCmdBuff[5] = globalDuration0;
  txCmdBuff[6] = globalDuration1;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("stream (st 0x%x)\r\n", status);
}

void sendStartRxTestEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendStartRxTestEventControl);

  txCmdBuff[0] = 6; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_START_RX_TEST;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("start rx test (st 0x%x)\r\n", status);
}

void sendEndRxTestEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendEndRxTestEventControl);

  txCmdBuff[0] = 6; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_END_RX_TEST;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("end rx test (st 0x%x)\r\n", status);
}

void sendReportEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendReportEventControl);

  txCmdBuff[0] = 22; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_REPORT;
  txCmdBuff[5] = BYTE_3(numPackets);
  txCmdBuff[6] = BYTE_2(numPackets);
  txCmdBuff[7] = BYTE_1(numPackets);
  txCmdBuff[8] = BYTE_0(numPackets);
  txCmdBuff[9] = BYTE_3(numProtocolPackets);
  txCmdBuff[10] = BYTE_2(numProtocolPackets);
  txCmdBuff[11] = BYTE_1(numProtocolPackets);
  txCmdBuff[12] = BYTE_0(numProtocolPackets);
  txCmdBuff[13] = BYTE_3(totalLqi);
  txCmdBuff[14] = BYTE_2(totalLqi);
  txCmdBuff[15] = BYTE_1(totalLqi);
  txCmdBuff[16] = BYTE_0(totalLqi);
  txCmdBuff[17] = BYTE_3(totalRssi);
  txCmdBuff[18] = BYTE_2(totalRssi);
  txCmdBuff[19] = BYTE_1(totalRssi);
  txCmdBuff[20] = BYTE_0(totalRssi);

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("report (st 0x%x)\r\n", status);

  //clear after sending report
  helperInit();
}

void sendRebootEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendRebootEventControl);

  txCmdBuff[0] = 6; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_REBOOT;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("reboot (st 0x%x)\r\n", status);
}

void sendGetRebootByteResponseEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendGetRebootByteResponseEventControl);

  txCmdBuff[0] = 7; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_GET_REBOOT_BYTE_RESPONSE;
  txCmdBuff[5] = globalRebootByte;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("get reboot byte (st 0x%x)\r\n", status);
}

void sendGetHardwareVersionEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendGetHardwareVersionEventControl);

  txCmdBuff[0] = 6; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_GET_HARDWARE_VERSION;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("get hw ver (st 0x%x)\r\n", status);
}

void sendGetHardwareVersionResponseEventFunction(void)
{
  EmberStatus status;

  deviceState = STATE_LOCAL_INIT;

  emberEventControlSetInactive(sendGetHardwareVersionResponseEventControl);

  txCmdBuff[0] = 7; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_GET_HARDWARE_VERSION_RESPONSE;
  txCmdBuff[5] = DEVICE_HW_VERSION;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("get hw ver resp (st 0x%x)\r\n", status);
}

void sendGetSoftwareVersionEventFunction(void)
{
  EmberStatus status;

  deviceState = STATE_LOCAL_INIT;

  emberEventControlSetInactive(sendGetSoftwareVersionEventControl);

  txCmdBuff[0] = 6; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_GET_SOFTWARE_VERSION;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("get sw ver (st 0x%x)\r\n", status);
}

void sendGetSoftwareVersionResponseEventFunction(void)
{
  EmberStatus status;

  emberEventControlSetInactive(sendGetSoftwareVersionResponseEventControl);

  txCmdBuff[0] = 7; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_GET_SOFTWARE_VERSION_RESPONSE;
  txCmdBuff[5] = DEVICE_SW_VERSION;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("get sw ver resp (st 0x%x)\r\n", status);
}

void pingTimerEventFunction(void)
{
  emberEventControlSetInactive(pingTimerEventControl);
  deviceState = STATE_LOCAL_INIT;
  emberAfCorePrintln("NO PING ACK\r\n");
}

void setChannelEventFunction(void)
{
  emberEventControlSetInactive(setChannelEventControl);

  int8u byte0 = globalChannelMaskByte0;
  int8u byte1 = globalChannelMaskByte1;
  int8u byte2 = globalChannelMaskByte2;
  int8u byte3 = globalChannelMaskByte3;
  int32u channelMask = (byte0 << 24) + (byte1 << 16) + (byte2 << 8) + byte3;

  helperSetChannelFromMask(channelMask);
}

void changeChannelWithPingEventFunction(void)
{
  emberEventControlSetInactive(changeChannelWithPingEventControl);

  int8u byte0 = globalChannelMaskByte0;
  int8u byte1 = globalChannelMaskByte1;
  int8u byte2 = globalChannelMaskByte2;
  int8u byte3 = globalChannelMaskByte3;
  int32u channelMask = (byte0 << 24) + (byte1 << 16) + (byte2 << 8) + byte3;
  helperSetChannelFromMask(channelMask);
  deviceState = STATE_LOCAL_CHANGING_CHANNEL_PINGING;
  emberEventControlSetActive(sendPingEventControl);
  emberEventControlSetDelayMS(pingTimerEventControl, globalRpingTimeout);
}

void rebootWithPingEventFunction(void)
{
  emberEventControlSetInactive(rebootWithPingEventControl);
  helperLocalReboot();
  deviceState = STATE_LOCAL_REBOOTING_PINGING;
  emberEventControlSetActive(sendPingEventControl);
  emberEventControlSetDelayMS(pingTimerEventControl, globalRpingTimeout);
}

void findWithPingEventFunction(void)
{
  emberEventControlSetInactive(findWithPingEventControl);
  deviceState = STATE_LOCAL_FINDING_DUT_PINGING;
  emberEventControlSetActive(sendPingEventControl);
  emberEventControlSetDelayMS(pingTimerEventControl, globalRpingTimeout);
}

void streamTimerEventFunction(void)
{
  EmberStatus status;
  emberEventControlSetInactive(streamTimerEventControl);
  deviceState = STATE_LOCAL_INIT;
  status = mfglibStopStream();
  emberAfCorePrintln("stop stream (st 0x%x)\r\n", status);
}

void custom1EventFunction(void)
{
  emberEventControlSetInactive(custom1EventControl);

  halReboot();
}

void custom2EventFunction(void)
{
  EmberStatus status;

  txCmdBuff[0] = 19; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = 0x09;
  txCmdBuff[2] = 0x08;
  txCmdBuff[3] = 0x01;
  txCmdBuff[4] = 0xEF;
  txCmdBuff[5] = 0xBE;
  txCmdBuff[6] = 0xA8;
  txCmdBuff[7] = 0xA6;
  txCmdBuff[8] = 0x00;
  txCmdBuff[9] = 0x01;
  txCmdBuff[10] = 0x02;
  txCmdBuff[11] = 0x03;
  txCmdBuff[12] = 0x04;
  txCmdBuff[13] = 0x05;
  txCmdBuff[14] = 0x06;
  txCmdBuff[15] = 0x07;
  txCmdBuff[16] = 0x08;
  txCmdBuff[17] = 0x09;

  status = mfglibSendPacket(txCmdBuff, 0);

  /* Check for error */
  if (status != EMBER_SUCCESS) {
    /* If error, print status and don't arm event */
    emberAfCorePrintln("mfglib TX failure, status=0x%x,status");
    return;
  }

  globalNumPackets++;

  if (globalNumPackets >= globalTotalNumPackets) {
    emberEventControlSetInactive(custom2EventControl);
    emberAfCorePrintln("TX complete, 0x%2x packets sent", globalNumPackets);
  } else {
    emberEventControlSetDelayMS(custom2EventControl, 100);
  }
}

void custom3EventFunction(void)
{
}

void custom4EventFunction(void)
{
}

void custom5EventFunction(void)
{
}

void custom6EventFunction(void)
{
}

/* This function allows us to delay init after startup to give the radio a chance to initialize */
void initEventFunction(void)
{
  emberEventControlSetInactive(initEventControl);
  helperAppInit();
}
//////////////////////////////////////////////
// callbacks
//////////////////////////////////////////////

/** @brief Main Start
 *
 * This function is called at the start of main after the HAL has been
 * initialized.  The standard main function arguments of argc and argv are
 * passed in.  However not all platforms have support for main() function
 * arguments.  Those that do not are passed NULL for argv, therefore argv should
 * be checked for NULL before using it.  If the callback determines that the
 * program must exit, it should return true.  The value returned by main() will
 * be the value written to the returnCode pointer.  Otherwise the callback
 * should return false to let normal execution continue.
 *
 * @param returnCode   Ver.: always
 * @param argc   Ver.: always
 * @param argv   Ver.: always
 */
boolean emberAfMainStartCallback(int* returnCode, int argc, char** argv)
{
  emberEventControlSetDelayMS(initEventControl, 1000); //initialize after a 1 sec delay
  return false;
}

void helperAppInit(void)
{
  EmberStatus status;
  int8u channel;
  int8s txPower;

  status = mfglibStart(mfglibRxHandler);
  emberAfCorePrintln("INIT: mfglib start (st 0x%x)\r\n", status);
  helperLocalChannelWithPower(RF_APP_DEFAULT_CHANNEL);
  channel = mfglibGetChannel();
  emberAfCorePrintln("INIT: ch 0x%x\r\n", channel);
  txPower = mfglibGetPower();
  emberAfCorePrintln("INIT: pow 0x%x\r\n", txPower);

  deviceState = STATE_LOCAL_INIT;
}

//////////////////////////////////////////////
// mfglibRxHandler
//////////////////////////////////////////////

void mfglibRxHandler(int8u *packet, int8u linkQuality, int8s rssi)
{
  //EmberStatus status;

  //emberAfCorePrintln("DEBUG RX1\r\n");
  //emberAfCorePrintln("0x%x%x%x%x%x\r\n", packet[0], packet[1], packet[2], packet[3], packet[4]);
  //emberSerialWaitSend(APP_SERIAL);
  //emberSerialPrintf(APP_SERIAL,"RX packet RSSI=%d\r\n",rssi);

  // if incoming is a command
  if ((packet[1] == BYTE_SIGNATURE_FIRST) && (packet[2] == BYTE_SIGNATURE_SECOND) && (packet[3] == BYTE_TYPE_COMMAND)) {
    if (packet[4] == BYTE_COMMAND_PING) {
      helperSendAck();
    } else if (packet[4] == BYTE_COMMAND_ACK) {
      if (deviceState == STATE_LOCAL_PINGING) {
        deviceState = STATE_LOCAL_INIT;
        emberEventControlSetInactive(pingTimerEventControl);
        emberAfCorePrintln("PING ACK\r\n");
      }
      if (deviceState == STATE_LOCAL_CHANGING_CHANNEL_PINGING) {
        deviceState = STATE_LOCAL_INIT;
        emberEventControlSetInactive(pingTimerEventControl);
        emberAfCorePrintln("CHANNEL ACK\r\n");
      }
      if (deviceState == STATE_LOCAL_REBOOTING_PINGING) {
        deviceState = STATE_LOCAL_INIT;
        emberEventControlSetInactive(pingTimerEventControl);
        emberAfCorePrintln("REBOOT ACK\r\n");
      }
      if (deviceState == STATE_LOCAL_FINDING_DUT_PINGING) {
        deviceState = STATE_LOCAL_INIT;
        emberEventControlSetInactive(pingTimerEventControl);
        emberAfCorePrintln("FIND ACK\r\n");
      }
    } else if (packet[4] == BYTE_COMMAND_SET_CHANNEL) {
      globalChannelMaskByte0 = packet[5];
      globalChannelMaskByte1 = packet[6];
      globalChannelMaskByte2 = packet[7];
      globalChannelMaskByte3 = packet[8];

      emberEventControlSetActive(setChannelEventControl);
    } else if (packet[4] == BYTE_COMMAND_GET_CHANNEL_RESPONSE) {
    } else if (packet[4] == BYTE_COMMAND_SET_POWER) {
      // ignore the mode bytes and use the token!!
      globalPower = packet[5];
      helperSetPower();
    } else if (packet[4] == BYTE_COMMAND_GET_POWER) {
      globalPower = mfglibGetPower();
      emberEventControlSetActive(sendGetPowerResponseEventControl);
    } else if (packet[4] == BYTE_COMMAND_GET_POWER_RESPONSE) {
      emberAfCorePrintln("POW 0x%x\r\n", packet[5]);
    } else if (packet[4] == BYTE_COMMAND_STREAM) {
      int16u timeout = HIGH_LOW_TO_INT((int8u)packet[5], (int8u)packet[6]);
      helperStream(timeout);
    } else if (packet[4] == BYTE_COMMAND_START_RX_TEST) {
      helperInit();
    } else if (packet[4] == BYTE_COMMAND_END_RX_TEST) {
      emberEventControlSetActive(sendReportEventControl);
    } else if (packet[4] == BYTE_COMMAND_REPORT) {
      emberAfCorePrintln("[total]0x%x%x%x%x [protocol]0x%x%x%x%x [totalLqi]0x%x%x%x%x [totalRssiMgnitude]0x%x%x%x%x\r\n",
                         packet[5], packet[6], packet[7], packet[8], packet[9], packet[10], packet[11], packet[12],
                         packet[13], packet[14], packet[15], packet[16], packet[17], packet[18], packet[19], packet[20]);
    } else if (packet[4] == BYTE_COMMAND_REBOOT) {
      helperLocalReboot();
    } else if (packet[4] == BYTE_COMMAND_GET_REBOOT_BYTE_RESPONSE) {
    } else if (packet[4] == BYTE_GET_HARDWARE_VERSION) {
      deviceState = STATE_LOCAL_SENDING_HW_VERSION;
      emberEventControlSetActive(sendGetHardwareVersionResponseEventControl);
    } else if (packet[4] == BYTE_GET_HARDWARE_VERSION_RESPONSE) {
      deviceState = STATE_LOCAL_INIT;
      emberAfCorePrintln("HW VER 0x%x\r\n", packet[5]);
    } else if (packet[4] == BYTE_GET_SOFTWARE_VERSION) {
      deviceState = STATE_LOCAL_SENDING_SW_VERSION;
      emberEventControlSetActive(sendGetSoftwareVersionResponseEventControl);
    } else if (packet[4] == BYTE_GET_SOFTWARE_VERSION_RESPONSE) {
      deviceState = STATE_LOCAL_INIT;
      emberAfCorePrintln("SW VER 0x%x\r\n", packet[5]);
    }
  } else {
    numPackets++;

    //if ((packet[0]==0x13)&&(packet[1]==0x09)&&(packet[2]==0x08)&&(packet[4]==0xEF)&&(packet[5]==0xBE))
    //{
    int8u rssiMagnitude;

    numProtocolPackets++;

    totalLqi = totalLqi + (int32u)linkQuality;

    //assume that rssi will be negative
    assert(rssi < 0);
    rssiMagnitude = 0 - rssi;
    totalRssi = totalRssi + (int32u)rssiMagnitude;
    //}
  }
}

//////////////////////////////////////////////
// helper functions
//////////////////////////////////////////////

void helperSendAck(void)
{
  emberEventControlSetActive(sendAckEventControl);
}

int8s getTxPower(int8u channel)
{
  int8s txPower = 0;
  switch (channel) {
    case 11:
      txPower = RF_APP_TX_POWER_CHANNEL_11;
      break;
    case 12:
      txPower = RF_APP_TX_POWER_CHANNEL_12;
      break;
    case 13:
      txPower = RF_APP_TX_POWER_CHANNEL_13;
      break;
    case 14:
      txPower = RF_APP_TX_POWER_CHANNEL_14;
      break;
    case 15:
      txPower = RF_APP_TX_POWER_CHANNEL_15;
      break;
    case 16:
      txPower = RF_APP_TX_POWER_CHANNEL_16;
      break;
    case 17:
      txPower = RF_APP_TX_POWER_CHANNEL_17;
      break;
    case 18:
      txPower = RF_APP_TX_POWER_CHANNEL_18;
      break;
    case 19:
      txPower = RF_APP_TX_POWER_CHANNEL_19;
      break;
    case 20:
      txPower = RF_APP_TX_POWER_CHANNEL_20;
      break;
    case 21:
      txPower = RF_APP_TX_POWER_CHANNEL_21;
      break;
    case 22:
      txPower = RF_APP_TX_POWER_CHANNEL_22;
      break;
    case 23:
      txPower = RF_APP_TX_POWER_CHANNEL_23;
      break;
    case 24:
      txPower = RF_APP_TX_POWER_CHANNEL_24;
      break;
    case 25:
      txPower = RF_APP_TX_POWER_CHANNEL_25;
      break;
    case 26:
      txPower = RF_APP_TX_POWER_CHANNEL_26;
      break;
  }

  return txPower;
}

void helperLocalChannelWithPower(int8u channel)
{
  EmberStatus status;
  int8s txPower = getTxPower(channel);

  status = mfglibSetChannel(channel);
  emberAfCorePrintln("ch %x (st 0x%x) ", channel, status);

  status = mfglibSetPower(RF_APP_TX_POWER_MODE, txPower);
  emberAfCorePrintln("pow (st 0x%x)\r\n", txPower, status);
}

void customSilabsListChannelPower(void)
{
  int8u channel;
  int8s txPower;

  emberAfCorePrint("\r\n");
  /* List default power value for all channels */
  for (channel = LOW_CHANNEL; channel <= HIGH_CHANNEL; channel++) {
    txPower = getTxPower(channel);
    emberAfCorePrintln("Default power level for channel %d is %d\r\n", channel, txPower);
  }
}

// the index of a set bit within the channelMask correspond
// to the actual channel.
// e.g. 0x800 is channel 11.
int8u helperFromChannelMaskToChannel(int32u channelMask)
{
  int8u i = 0;
  for (i = LOW_CHANNEL; i <= HIGH_CHANNEL; i++) {
    if (channelMask == (1 << i)) {
      return i;
    }
  }

  emberAfCorePrintln("invalid channel mask\r\n");
  return 0; /* No channels in mask */
}

int32u helperFromChannelToChannelMask(int8u channel)
{
  if (LOW_CHANNEL <= channel && channel <= HIGH_CHANNEL) {
    return 1 << channel;
  } else {
    emberAfCorePrintln("invalid channel\r\n");
    return 0; /* No channels in mask */
  }
}

void helperSetChannelFromMask(int32u channelMask)
{
  int8u channel = helperFromChannelMaskToChannel(channelMask);
  helperLocalChannelWithPower(channel);
}

void helperFromChannelMaskToGlobalChannelMaskBytes(int32u channelMask)
{
  globalChannelMaskByte0 = (channelMask & 0xFF000000) >> 24;
  globalChannelMaskByte1 = (channelMask & 0x00FF0000) >> 16;
  globalChannelMaskByte2 = (channelMask & 0x0000FF00) >> 8;
  globalChannelMaskByte3 = (channelMask & 0x000000FF);
}

void sendSetChannelHelper(void)
{
  EmberStatus status;

  txCmdBuff[0] = 10; // length does not include length byte / leave 2 for crc
  txCmdBuff[1] = BYTE_SIGNATURE_FIRST;
  txCmdBuff[2] = BYTE_SIGNATURE_SECOND;
  txCmdBuff[3] = BYTE_TYPE_COMMAND;
  txCmdBuff[4] = BYTE_COMMAND_SET_CHANNEL;
  txCmdBuff[5] = globalChannelMaskByte0;
  txCmdBuff[6] = globalChannelMaskByte1;
  txCmdBuff[7] = globalChannelMaskByte2;
  txCmdBuff[8] = globalChannelMaskByte3;

  status = mfglibSendPacket(txCmdBuff, 0);
  emberAfCorePrintln("set channel (st 0x%x)\r\n", status);
}

void helperStream(int16u timeout)
{
  EmberStatus status;

  deviceState = STATE_LOCAL_STREAMING;

  status = mfglibStartStream();
  emberAfCorePrintln("stream (st 0x%x)\r\n", status);

  // infinity if timeout is equal to 0
  if (timeout != 0) {
    emberEventControlSetDelayMS(streamTimerEventControl, timeout);
  }
}

void helperInit(void)
{
  numPackets = 0;
  numProtocolPackets = 0;
  totalLqi = 0;
  totalRssi = 0;
}

void helperSetPower(void)
{
  EmberStatus status;

  status = mfglibSetPower(RF_APP_TX_POWER_MODE, globalPower);
  emberAfCorePrintln("pow (st 0x%x)\r\n", status);
}

void helperLocalReboot(void)
{
  emberEventControlSetActive(custom1EventControl);
}

void helperSendReboot(void)
{
  emberEventControlSetActive(sendRebootEventControl);
}
