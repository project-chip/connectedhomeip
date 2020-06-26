/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief Module for interacting with Ethernet
 *******************************************************************************
   ******************************************************************************/

#ifdef ETHERNET_STANDALONE_DRIVER
  #include PLATFORM_HEADER //compiler/micro specifics, types
  #include "stack/include/ember.h"
  #include "stack/include/error.h"
  #include "hal/hal.h"
  #include "plugin/serial/serial.h"
  #include "app/slabgw/micrel-ethernet.h"
  #include "app/slabgw/slabgw-arp.h"
  #include "app/slabgw/slabgw-ip.h"
  #include "app/slabgw/slabgw-main.h"
  #define emberAfCorePrint(...) emberSerialPrintf(gAppSerial, __VA_ARGS__)
#else
  #include "app/framework/include/af.h"
  #include "lwip/opt.h"
  #include "lwip/def.h"
  #include "lwip/pbuf.h"
  #include "lwip/sys.h"
  #include "lwip/stats.h"
  #include "lwip/snmp.h"
  #include "lwip/netif.h"
  #include "lwip/sockets.h"
  #include "lwip/tcpip.h"
  #include "lwip/err.h"
  #include "lwip/def.h"
  #include "micrel-ethernet.h"
  #include "lwip/src/include/netif/etharp.h"
  #include "app/framework/plugin/lwip/lwip-main.h"
  #define SLABGW_MAJOR_VERSION 1
  #define SLABGW_MINOR_VERSION 7
  #define SLABGW_CHANGE_VERSION 750
#endif //ETHERNET_STANDALONE_DRIVER

// ethernet frame:
// --------------
// done by Micrel --> preamble = 6 bytes =  0x55 55 55 55 55 55
// done by Micrel --> SFD = 1 byte = 0xD5
// done by Driver --> Dest MAC address = 6 bytes = 0xXX XX XX XX XX XX
// done by Driver --> Src MAC address = 6 bytes = 0xXX XX XX XX XX XX
// done by Driver --> EtherType = 1 byte
// done by Driver --> payload = X bytes (46-1500)
// done by Micrel --> frame check sequence (FCS) = 4 bytes

// Common Ethertypes
// -----------------
// 0x0800 = IPv4 datagram
// 0x0806 = ARP
// 0x86DD = IPv6 frame

// ***************************************************************************
// defines for configuring SPI speed, byte order, trigger edge on Micrel Ethernet
// ****************************************************************************

// 2 Mhz
//#define SPI_LIN  (5)
//#define SPI_EXP  (0)

// 12 Mhz
#define SPI_LIN  (0)
#define SPI_EXP  (0)

// use MSB first
#define SPI_ORD_MSB_FIRST (0 << SC_SPIORD_BIT) // Send the MSB first
//#define SPI_ORD_LSB_FIRST (1<<SC_SPIORD_BIT) // Send the LSB first

//#define SPI_PHA_FIRST_EDGE (0<<SC_SPIPHA_BIT)  // Sample on first edge
#define SPI_PHA_SECOND_EDGE (1 << SC_SPIPHA_BIT) // Sample on second edge

#define SPI_POL_RISING_LEAD  (0 << SC_SPIPOL_BIT) // Leading edge is rising
//#define SPI_POL_FALLING_LEAD (1<<SC_SPIPOL_BIT) // Leading edge is falling

#ifdef ETHERNET_STANDALONE_DRIVER
// for serial printing
extern uint8_t gAppSerial;
#define ENET_LINK_DOWN 0
#define ENET_LINK_UP  1
#define ENET_STATUS_OVERRUN 2
#endif

// for enabling/disabling debug prints
static bool debugPrintOn = false;
static bool debugIcmpPrintOn = false;
static bool debugPrintTick = false;

#if LWIP_STATS
#if LINK_STATS
#define ETHERNET_STATS_INC(X) LINK_STATS_INC(link.X)
#define ETHERNET_STATS_CLR(X) lwip_stats.link.X = 0
#else
#define ETHERNET_STATS_INC(X)
#define ETHERNET_STATS_CLR(X)
#endif
#else
#define ETHERNET_STATS_INC(X)
#define ETHERNET_STATS_CLR(X)
#endif

#define ETHERNET_DEBUG_PRINT(X)  if (debugPrintOn) { emberAfCorePrint(X); }
#define ETHERNET_DEBUG_PRINT1(X, arg1)  if (debugPrintOn) { emberAfCorePrint(X, arg1); }
#define ETHERNET_DEBUG_PRINT2(X, arg1, arg2)  if (debugPrintOn) { emberAfCorePrint(X, arg1, arg2); }
#define ETHERNET_DEBUG_PRINT3(X, arg1, arg2, arg3)  if (debugPrintOn) { emberAfCorePrint(X, arg1, arg2, arg3); }
#define ETHERNET_DEBUG_PRINT4(X, arg1, arg2, arg3, arg4)  if (debugPrintOn) { emberAfCorePrint(X, arg1, arg2, arg3, arg4); }
#define ETHERNET_DEBUG_PRINT5(X, arg1, arg2, arg3, arg4, arg5)  if (debugPrintOn) { emberAfCorePrint(X, arg1, arg2, arg3, arg4, arg5); }
#define ETHERNET_DEBUG_PRINT6(X, arg1, arg2, arg3, arg4, arg5, arg6)  if (debugPrintOn) { emberAfCorePrint(X, arg1, arg2, arg3, arg4, arg5, arg6); }

#define ETHERNET_TICK_DEBUG_PRINT(X)  if (debugPrintOn && debugPrintTick) { emberAfCorePrint(X); }
#define ETHERNET_TICK_DEBUG_PRINT1(X, arg1)  if (debugPrintOn && debugPrintTick) { emberAfCorePrint(X, arg1); }
#define ETHERNET_TICK_DEBUG_PRINT4(X, arg1, arg2, arg3, arg4)  if (debugPrintOn && debugPrintTick ) { emberAfCorePrint(X, arg1, arg2, arg3, arg4); }

// enable debug prints
//#define ETHERNET_DEBUG_PRINT(X) emberSerialWaitSend(gAppSerial); emberSerialPrintf(gAppSerial, X)

#define ETH_STATE_BOOTING 0
#define ETH_STATE_UP      1
#define ETH_STATE_DOWN    2

#define READ 1
#define WRITE 2

#define DONT_CARE_VALUE 0xFF

#include "rtos/rtos.h"
#include "FreeRTOS.h"
#include "task.h"

uint8_t myEthernetState = ETH_STATE_BOOTING;
uint8_t gInitialArpState = 0;

// globals to this module
uint8_t gBroadcastMacAddr[NETIF_HWADDR_LEN];
uint8_t gZeroMacAddr[NETIF_HWADDR_LEN];

static uint8_t resultBuffer[12];  //Must fit MAC
static uint8_t inputBuffer[8];

// statistics for this interface
struct ethStats gMyStats;

// forward declarations for this module
void innerHardwareInit(struct netif* netif);
void innerMicrelSpiInit(void);
void innerMicrelEthernetInit(struct netif* netif);
uint8_t micrelLinkStatus(void);
void waitInMilliseconds(uint16_t msDelay);
void micrelGetCurrentState(void);
static void clearEthStats(void);
uint8_t* micrelSendCommand(uint8_t readOrWrite, uint8_t registerAddr, uint8_t numOfBytes,
                           uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);

#ifdef ETHERNET_STANDALONE_DRIVER
// ****************************************************************************
// read MAC address from tokens and set it in interface
// ****************************************************************************
void readAndSetMacAddress(struct netif* netif)
{
  uint16_t macFlags;

  // setup my MAC address from tokens
  halCommonGetToken(&macFlags, TOKEN_SLABGW_MACADDR_FLAGS);
  halCommonGetToken(netif->hwaddr, TOKEN_SLABGW_MACADDR_VALUE);

  // to be valid we need to have 0b10xxxxxxxxxxxxxx
  if (!((macFlags & 0xC000) == 0x8000) ) {
    ETHERNET_DEBUG_PRINT("WARN: using default MAC address, token invalid\r\n");
    // use Eagle Mountain Technology Mac address
    netif->hwaddr[0] = 0x70;
    netif->hwaddr[1] = 0xB3;
    netif->hwaddr[2] = 0xD5;
    netif->hwaddr[3] = 0x55;
    netif->hwaddr[4] = 0x90;
    netif->hwaddr[5] = 0x00;
  }
  ETHERNET_DEBUG_PRINT("INFO: using MAC addr of ");
  printMacAddress(netif->hwaddr, true);
}

// ****************************************************************************
// read IP address, netmask, gateway from tokens and set in interface
// ****************************************************************************
void readAndSetIpAddress(struct netif* netif)
{
  uint16_t ipFlags;
  uint32_t ipAddrValue;
  uint32_t ipAddrNetmask;
  uint32_t ipAddrGateway;

  // read all the tokens

  // setup my IP address from tokens
  halCommonGetToken(&ipFlags, TOKEN_SLABGW_IPADDR_FLAGS);
  halCommonGetToken(&ipAddrValue, TOKEN_SLABGW_IPADDR_VALUE);
  halCommonGetToken(&ipAddrNetmask, TOKEN_SLABGW_IPADDR_NETMASK);
  halCommonGetToken(&ipAddrGateway, TOKEN_SLABGW_IPADDR_GATEWAY);

  // to be valid we need to have 0b10xxxxxxxxxxxxxx
  if (!((ipFlags & 0xC000) == 0x8000) ) {
    ETHERNET_DEBUG_PRINT("WARN: using default IP address, token invalid\r\n");
    netif->ip_addr.ipAddr[0] = 10;
    netif->ip_addr.ipAddr[1] = 1;
    netif->ip_addr.ipAddr[2] = 1;
    netif->ip_addr.ipAddr[3] = 100;

    netif->netmask.ipAddr[0] = 255;
    netif->netmask.ipAddr[1] = 255;
    netif->netmask.ipAddr[2] = 255;
    netif->netmask.ipAddr[3] = 0;

    netif->gw.ipAddr[0] = 10;
    netif->gw.ipAddr[1] = 1;
    netif->gw.ipAddr[2] = 1;
    netif->gw.ipAddr[3] = 1;
  } else {
    netif->ip_addr.ipAddr[0] = BYTE_3(ipAddrValue);
    netif->ip_addr.ipAddr[1] = BYTE_2(ipAddrValue);
    netif->ip_addr.ipAddr[2] = BYTE_1(ipAddrValue);
    netif->ip_addr.ipAddr[3] = BYTE_0(ipAddrValue);

    netif->netmask.ipAddr[0] = BYTE_3(ipAddrNetmask);
    netif->netmask.ipAddr[1] = BYTE_2(ipAddrNetmask);
    netif->netmask.ipAddr[2] = BYTE_1(ipAddrNetmask);
    netif->netmask.ipAddr[3] = BYTE_0(ipAddrNetmask);

    netif->gw.ipAddr[0] = BYTE_3(ipAddrGateway);
    netif->gw.ipAddr[1] = BYTE_2(ipAddrGateway);
    netif->gw.ipAddr[2] = BYTE_1(ipAddrGateway);
    netif->gw.ipAddr[3] = BYTE_0(ipAddrGateway);
  }
  ETHERNET_DEBUG_PRINT("INFO: using IP addr of ");
  printIpAddressStruct(netif->ip_addr.ipAddr, true);
}

void printEthStats(void)
{
  halResetWatchdog();
  emberSerialPrintf(gAppSerial, " Eth pkts TX: %d\r\n", gMyStats.txPackets);
  emberSerialPrintf(gAppSerial, "Eth bytes TX: %d\r\n", gMyStats.txBytes);
  emberSerialWaitSend(gAppSerial);
  emberSerialPrintf(gAppSerial, " Eth pkts RX: %d\r\n", gMyStats.rxPackets);
  emberSerialPrintf(gAppSerial, "Eth bytes RX: %d\r\n", gMyStats.rxBytes);
  emberSerialWaitSend(gAppSerial);
  emberSerialPrintf(gAppSerial, "Eth receive errors:\r\n");
  emberSerialPrintf(gAppSerial, "-------------------\r\n");
  emberSerialWaitSend(gAppSerial);
  emberSerialPrintf(gAppSerial, "pkt count zero: %d\r\n", gMyStats.rxErrPacketCountZero);
  emberSerialPrintf(gAppSerial, "  err flag set: %d\r\n", gMyStats.rxErrErrorFlagSet);
  emberSerialWaitSend(gAppSerial);
  emberSerialPrintf(gAppSerial, "    bad length: %d\r\n", gMyStats.rxErrBadLength);
  emberSerialPrintf(gAppSerial, " no pbuf avail: %d\r\n", gMyStats.rxErrNoPbufAvail);
  emberSerialWaitSend(gAppSerial);
  halResetWatchdog();
}
#endif //ETHERNET_STANDALONE_DRIVER

// ****************************************************************************
// main module init. calls other init functions, sets fields in interface struct
// ****************************************************************************
void micrelEthernetInit(struct netif* netif)
{
  // enables FEM and inits Eth driver
  innerHardwareInit(netif);
  // sets the pins correctly for using the Micrel Ethernet chip
  innerMicrelSpiInit();

  clearEthStats();

  ETHERNET_STATS_CLR(memerr);
  ETHERNET_STATS_CLR(xmit);
  ETHERNET_STATS_CLR(recv);
  ETHERNET_STATS_CLR(err);
  ETHERNET_STATS_CLR(drop);
  ETHERNET_STATS_CLR(chkerr);
  ETHERNET_STATS_CLR(lenerr);

  // ********************************
  // set up the netif struct
  // ********************************
  netif->name[0] = 'e';
  netif->name[1] = '0';

  netif->state = NULL;
  netif->mtu = MAX_ETHERNET_FRAME_SIZE;
  netif->hwaddr_len = NETIF_HWADDR_LEN;
  netif->num = 0;

  // init steps required by Micrel
  innerMicrelEthernetInit(netif);
}

// ****************************************************************************
// Ethernet Tick
//  1. just after restart, sends gratuitous ARPs
//  2. Polls for received packets
// ****************************************************************************

uint8_t ethAutoRxEnabled = false;

void micrelEthernetTick(struct netif* netif)
{
  if (netif->hwaddr[0] == 0xFF) {
    return;
  }

  uint16_t now = halCommonGetInt16uMillisecondTick();
  static uint16_t lastPollTime = 0;

  //
  // When 250msec has passed since the last buffer check take a look at the micrel vitals
  /*
     if ( (uint16_t)(now - lastPollTime) > 250) {
        if (myEthernetState != ETH_STATE_UP) {
          uint8_t* result = micrelSendCommand(READ, 0xF8, 2, 0, 0, 0, 0);
          // ...and state is now UP?
            if ((result[0] & 0x40) == 0x40) {
              // call init so all state is correct
              innerMicrelEthernetInit(netif);
              // that's enough
              return;
            }
        }
      // are we UP...
      if (myEthernetState == ETH_STATE_UP) {
        uint8_t* result = micrelSendCommand(READ, 0xF8, 2, 0, 0, 0, 0);
        // ...and state is now DOWN?
          if ((result[0] & 0x40) == 0x00) {
            myEthernetState = ETH_STATE_DOWN;
            // call the callback
            netif->flags &= !NETIF_FLAG_LINK_UP;
            do {if (netif->link_callback) { (netif->link_callback)(netif); }}while(0);
            return;
          }
        }
     }
   */
  // if we are in auto receive mode, poll micrel
  if (myEthernetState == ETH_STATE_UP && ethAutoRxEnabled == true) {
    micrelEthReceive(netif);
  }
}

// ****************************************************************************
// use these functions to enable or disable auto-rx, debug print, and print on rx tick
// ****************************************************************************

void setEthernetAutoRx(uint8_t value)
{
  ethAutoRxEnabled = value;
}

void micrelSetEthernetDebugPrint(const uint8_t value)
{
  emberAfCorePrint("Ethernet Debug Print Level: ");
  if (value < 1) {
    debugPrintOn = false;
    debugPrintTick = false;
    emberAfCorePrintln("Off");
  } else if (value == 1) {
    debugPrintOn = true;
    debugPrintTick = false;
    emberAfCorePrintln("1 (some)");
  } else {
    debugPrintOn = true;
    debugPrintTick = true;
    emberAfCorePrintln("2 (verbose)");
  }
}

// ****************************************************************************
// turns on FEM
// calls functions to read and set MAC addr and IP Addr
// ****************************************************************************
static void innerHardwareInit(struct netif* netif)
{
  ETHERNET_DEBUG_PRINT("In innerHardwareInit()...\r\n");

  // the Ethernet gateway hardware requires PB5 to be high - enables FEM
  // without this there is no RF

  uint8_t i;
  // set PB5 as output - this is connected to Micrel reset line
  uint8_t pinNum = 5;
  uint32_t pinCfgMask = PB4_CFG_MASK << ((pinNum - 4) * 4);
  uint32_t pinCfgBit = PB4_CFG_BIT + ((pinNum - 4) * 4);
  GPIO_PBCFGH = (GPIO_PBCFGH | pinCfgMask) & ((0xFFFF - pinCfgMask) | (GPIOCFG_OUT << pinCfgBit));

  // pull PB5 high
  GPIO_PBSET = GPIO_PBSET | BIT(5);

  // setup default values for variables
  for ( i = 0; i < 6; i++) {
    gBroadcastMacAddr[i] = 0xFF;
    gZeroMacAddr[i] = 0;
  }

#ifdef ETHERNET_STANDALONE_DRIVER
  // read the MAC address token and set the local variable
  readAndSetMacAddress(netif);
  // read the IP address token and set the local variable
  readAndSetIpAddress(netif);
#endif
}

// ***************************************************************************
// 1. init the SPI
// 2. reset and keep on the Micrel part
// ***************************************************************************
static void innerMicrelSpiInit(void)
{
  ETHERNET_DEBUG_PRINT("In innerMicrelSpiInit()...\r\n");

  waitInMilliseconds(500);

#ifdef ETHERNET_STANDALONE_DRIVER
  GPIO_PBCFGL = (GPIO_PBCFGL | PB1_CFG_MASK) & ((0xFFFF - PB1_CFG_MASK) | (GPIOCFG_OUT_ALT << PB1_CFG_BIT));
  GPIO_PBCFGL = (GPIO_PBCFGL | PB2_CFG_MASK) & ((0xFFFF - PB2_CFG_MASK) | (GPIOCFG_IN << PB2_CFG_BIT));
  GPIO_PBCFGL = (GPIO_PBCFGL | PB3_CFG_MASK) & ((0xFFFF - PB3_CFG_MASK) | (GPIOCFG_OUT_ALT << PB3_CFG_BIT));
  GPIO_PBCFGH = (GPIO_PBCFGH | PB4_CFG_MASK) & ((0xFFFF - PB4_CFG_MASK) | (GPIOCFG_OUT << PB4_CFG_BIT));

#else //ETHERNET_STANDALONE_DRIVER
  // we use SC1, PB1 = MOSI, PB2 = MISO, PB3 = SCLK, PB4 = CSN (chip select)
  uint32_t cfgReg = GPIO_PBCFGL;
  //clear out the current configuration
  cfgReg &= ~(0xF << PB1_CFG_BIT | 0xF << PB2_CFG_BIT | 0xF << PB3_CFG_BIT);
  //Now set the new values.
  cfgReg |= (GPIOCFG_OUT_ALT << PB1_CFG_BIT
             | GPIOCFG_IN      << PB2_CFG_BIT
             | GPIOCFG_OUT_ALT  << PB3_CFG_BIT);
  GPIO_PBCFGL = cfgReg;

  uint32_t selReg = GPIO_PBCFGH;
  selReg &= ~(0xF << PB4_CFG_BIT);
  selReg |= (GPIOCFG_OUT << PB4_CFG_BIT);
  GPIO_PBCFGH = selReg;
#endif //ETHERNET_STANDALONE_DRIVER

  GPIO_PBSET = GPIO_PBSET | BIT(4);

  SC1_RATELIN = SPI_LIN;
  SC1_RATEEXP = SPI_EXP;
  SC1_SPICFG  =  0;
  SC1_SPICFG = 0x13;
  SC1_MODE   =  SC1_MODE_SPI;

  // set PC1 as output - this is connected to Micrel reset line
  uint8_t pinNum = 1;
  uint32_t pinCfgMask = PC0_CFG_MASK << (pinNum * 4);
  uint32_t pinCfgBit = PC0_CFG_BIT + (pinNum * 4);
  GPIO_PCCFGL = (GPIO_PCCFGL | pinCfgMask) & ((0xFFFF - pinCfgMask) | (GPIOCFG_OUT << pinCfgBit));

  // pull PC1 low, wait 10 ms
  GPIO_PCCLR = GPIO_PCCLR | BIT(1);
  waitInMilliseconds(11);
  // pull PC1 high
  GPIO_PCSET = GPIO_PCSET | BIT(1);
}

// ***************************************************************************
// generic SPI write byte
// ***************************************************************************
uint8_t micrelSpiWrite(uint8_t txData)
{
  uint8_t rxData;

  SC1_DATA = txData;
  while ( (SC1_SPISTAT & SC_SPITXIDLE) != SC_SPITXIDLE) {
  }                                                     //wait to finish

  rxData = SC1_DATA;

  //emberAfCorePrintln("TX 0x%x, RX 0x%x\r\n", txData, rxData);

  return rxData;
}

// ***************************************************************************
// send the 2 byte commands for writing or reading
// ***************************************************************************

// two byte value
//
// Read  which byte   address     don't care
// 0 1    x x x x    a a a a a a   z z z z
// Write which byte   address     don't care
// 0 0    x x x x    a a a a a a   z z z z

void micrelInitiateTrans(uint8_t readOrWrite,
                         uint8_t numOfBytes,
                         uint8_t registerAddr)
{
  uint8_t byte0;
  uint8_t byte1;

  if (readOrWrite == READ) {
    byte0 = 0;
  } else {
    byte0 = 0x40;
  }

  switch (numOfBytes) {
    case 1:
      byte0 = byte0 | 0x04;
      break;
    case 2:
      byte0 = byte0 | 0x0C;
      break;
    case 3:
      byte0 = byte0 | 0x1C;
      break;
    case 4:
      byte0 = byte0 | 0x3C;
      break;
  }

  // byte0 gets the 2 highest bits of registerAddr
  //
  // we want ->   X  X  X  X  X  X A7 A6
  // we get  ->  A7 A6 A5 A4 A3 A2 A1 A0
  // shift 6 ->   X  X  X  X  X  X A7 A6
  byte0 = byte0 | (registerAddr >> 6);

  // byte 1 gets bits 2-5 (not 0,1 and not 6,7) but they need to
  // be shifted by 4 bits. The register address dont use bits 0,1
  // so we shift the value by 2 bits. We make sure the bottom 4 bits
  // are 0.
  //
  // we want ->  A5 A4 A3 A2  X  X  X  X
  // we get  ->  A7 A6 A5 A4 A3 A2 A1 A0
  // shift 2 ->  A5 A4 A3 A2 A1 A0  X  X
  // & 0xF0  ->  A5 A4 A3 A2  X  X  X  X
  byte1 = (registerAddr << 2) & 0xF0;

  micrelSpiWrite(byte0);
  micrelSpiWrite(byte1);
}

// ***************************************************************************
// send a read or write bytes for up to 4 bytes
// ***************************************************************************

uint8_t* micrelSendCommand(uint8_t readOrWrite,
                           uint8_t registerAddr,
                           uint8_t numOfBytes,
                           uint8_t byte0,
                           uint8_t byte1,
                           uint8_t byte2,
                           uint8_t byte3)
{
  uint8_t i;

  // set PB4 to LOW -- this is chip select
  GPIO_PBCLR = GPIO_PBCLR | BIT(4);

  inputBuffer[0] = byte0;
  inputBuffer[1] = byte1;
  inputBuffer[2] = byte2;
  inputBuffer[3] = byte3;

  // emberAfCorePrintln("micrelSendCommand");

  // read/write bytes from register
  micrelInitiateTrans(readOrWrite, numOfBytes, registerAddr);
  //emberAfCorePrintln("micrelSendCommand after trans");

  for (i = 0; i < numOfBytes; i++) {
    resultBuffer[i] = micrelSpiWrite(inputBuffer[i]);
  }
  //emberAfCorePrintln("micrelSendCommand after write");

  // set PB4 to HIGH -- this is chip select
  GPIO_PBSET = GPIO_PBSET | BIT(4);

  return resultBuffer;
}

// ***************************************************************************
// used to verify results rx from SPI
// ***************************************************************************

static void checkResults(uint8_t* result, uint8_t* label, uint8_t numBytes, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
  switch (numBytes) {
    case 4:
      if ( (result[3] != byte3) || (result[2] != byte2) || (result[1] != byte1) || (result[0] != byte0)) {
        ETHERNET_DEBUG_PRINT1("   unexpected results from %p, expect ", label);
        ETHERNET_DEBUG_PRINT4("0x%x %x %x %x got ", byte0, byte1, byte2, byte3);
        ETHERNET_DEBUG_PRINT4("0x%x %x %x %x\r\n", result[0], result[1], result[2], result[3]);

        return;
      }
      break;

    case 3:
      if ( (result[2] != byte2) || (result[1] != byte1) || (result[0] != byte0)) {
        ETHERNET_DEBUG_PRINT1("   unexpected results from %p, expect ", label);
        ETHERNET_DEBUG_PRINT3("0x%x %x %x got ", byte0, byte1, byte2);
        ETHERNET_DEBUG_PRINT3("0x%x %x %x\r\n", result[0], result[1], result[2]);
        return;
      }
      break;

    case 2:
      if ( (result[1] != byte1) || (result[0] != byte0)) {
        ETHERNET_DEBUG_PRINT1("   unexpected results from %p, expect ", label);
        ETHERNET_DEBUG_PRINT2("0x%x %x got ", byte0, byte1);
        ETHERNET_DEBUG_PRINT2("0x%x %x\r\n", result[0], result[1]);
        return;
      }
      break;

    case 1:
      if (result[0] != byte0) {
        ETHERNET_DEBUG_PRINT1("   unexpected results from %p, expect ", label);
        ETHERNET_DEBUG_PRINT2("0x%x got 0x%x\r\n", byte0, result[0]);
        return;
      }
      break;
  }

  //emberAfCorePrintln( "   values check: PASS\r\n");
}

static void clearEthStats(void)
{
  gMyStats.txPackets = 0;
  gMyStats.txBytes = 0;
  gMyStats.rxPackets = 0;
  gMyStats.rxBytes = 0;
  gMyStats.rxErrPacketCountZero = 0;
  gMyStats.rxErrErrorFlagSet = 0;
  gMyStats.rxErrBadLength = 0;
  gMyStats.rxErrNoPbufAvail = 0;
}
// ***************************************************************************
// routine to get Micrel ready to TX and RX
// ***************************************************************************
static void innerMicrelEthernetInit(struct netif* netif)
{
  uint8_t* result;

#ifdef ETHERNET_STANDALONE_DRIVER
  netif->status_callback = lwipStatusCallback;
  netif->link_callback = lwipLinkCallback;
#endif //ETHERNET_STANDALONE_DRIVER

  ETHERNET_DEBUG_PRINT("In innerMicrelEthernetInit()...\r\n");

  waitInMilliseconds(500);

  // init step 1
  result = micrelSendCommand(READ, 0xC0, 2, 0, 0, 0, 0);

  // init step 2-3
  micrelSendCommand(WRITE, 0x10, 4, netif->hwaddr[5], netif->hwaddr[4], netif->hwaddr[3], netif->hwaddr[2]);

  // init step 4
  micrelSendCommand(WRITE, 0x14, 2, netif->hwaddr[1], netif->hwaddr[0], 0, 0);

  // check steps 2-3
  result = micrelSendCommand(READ, 0x10, 4, 0, 0, 0, 0);

  // check step 4
  result = micrelSendCommand(READ, 0x14, 2, 0, 0, 0, 0);

  // init step 5 & 7
  micrelSendCommand(WRITE, 0x84, 4, 0x00, 0x40, 0x00, 0x40);

  // check step 5 & 7
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x84: expect 0x00 40\r\n");
  result = micrelSendCommand(READ, 0x84, 4, 0, 0, 0, 0);
  checkResults(result, "step5&7", 2, 0x00, 0x40, 0x00, 0x40);

  // init step 6
  //ETHERNET_DEBUG_PRINT("init6: write 2 bytes to 0x70: enable QMU xmit flow ctrl\r\n");
  micrelSendCommand(WRITE, 0x70, 2, 0xEE, 0x01, 0x00, 0x00);

  // check step 6
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x70: expect 0x7E 01\r\n");
  result = micrelSendCommand(READ, 0x70, 2, 0, 0, 0, 0);
  checkResults(result, "step6", 2, 0xEE, 0x01, 0, 0);

  // init step 7
  // NOT on 4 byte boundary - done as part of step 5
  //ETHERNET_DEBUG_PRINT("init7: write 2 bytes to 0x86: enable QMU recv FDP auto inc\r\n");
  //micrelSendCommand(WRITE, 0x86, 2, 0x00, 0x40, 0x00, 0x00);

  // check step 7
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x86: expect 0x00 40\r\n");
  //result = micrelSendCommand(READ, 0x86, 2, 0, 0, 0, 0);
  //checkResults(result, "step7", 2, 0x00, 0x40, 0, 0);

  // init step 8
  //ETHERNET_DEBUG_PRINT("init8: write 2 bytes to 0x9C: recv frame thold for 1 frame\r\n");
  micrelSendCommand(WRITE, 0x9C, 2, 0x01, 0x00, 0, 0);

  // check step 8
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x9C: expect 0x00 01\r\n");
  result = micrelSendCommand(READ, 0x9C, 2, 0, 0, 0, 0);
  checkResults(result, "step8", 2, 0x01, 0x00, 0, 0);

  // init step 9 & 10
  //ETHERNET_DEBUG_PRINT("init9&10: write 4 bytes to 0x74: enable QMU rx options\r\n");
  //default
  micrelSendCommand(WRITE, 0x74, 4, NETIF_RXCR1_BROADCAST_SET, 0x7C, 0x96, 0x00);

  // check step 9 & 10
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x74: expect 0xA3 7C\r\n");
  result = micrelSendCommand(READ, 0x74, 2, 0, 0, 0, 0);
  checkResults(result, "step9&10", 2, NETIF_RXCR1_BROADCAST_SET, 0x7C, 0x96, 0x00);

  // init step 10
  //ETHERNET_DEBUG_PRINT("init10: write 2 bytes to 0x76: enable QMU rx options\r\n");
  //micrelSendCommand(WRITE, 0x76, 2, 0x96, 0x00, 0, 0);

  // check step 10
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x76: expect 0x96 00\r\n");
  //result = micrelSendCommand(READ, 0x76, 2, 0, 0, 0, 0);
  //checkResults(result, "step10", 2, 0x96, 0x00, 0, 0);

  // to write to register 0x82 we need to write to register 0x80 and 0x81
  // default value is supposed to be 0x0000 for reg 0x80 and 0x81. check that
  //ETHERNET_DEBUG_PRINT("check 0x80: read 2 bytes expect 0x00 00\r\n");
  result = micrelSendCommand(READ, 0x80, 2, 0x00, 0x00, 0, 0);
  checkResults(result, "preStep11", 2, 0x00, 0x00, 0, 0);

  // init step 11
  //ETHERNET_DEBUG_PRINT("init11: write 4 bytes to 0x80: enable QMU rx options\r\n");
  micrelSendCommand(WRITE, 0x80, 4, 0x00, 0x00, 0x30, 0x02);

  // check step 11
  //ETHERNET_DEBUG_PRINT("read 4 bytes from 0x80: expect 0x00 00 30 02\r\n");
  result = micrelSendCommand(READ, 0x80, 4, 0, 0, 0, 0);
  checkResults(result, "step10", 2, 0x00, 0x00, 0x30, 0x02);

  // TODO: don't know what value to set here. Leave as default of 0 for now
  // init step 12
  //ETHERNET_DEBUG_PRINT("init12: SPI data output delay\r\n");
  //micrelSendCommand(WRITE, 0x20, 2, 0x00, 0x00, 0, 0);

  // check step 12
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x20: expect 0x00 00\r\n");
  result = micrelSendCommand(READ, 0x20, 2, 0, 0, 0, 0);
  checkResults(result, "step12", 2, 0x00, 0x00, 0, 0);

  // init step 13
  // not on 4 byte value so we need to write 2 prev bytes
  // default value of 0xF4-0xF5 is 0x0000
  // default value of 0xF6-0xF7 is 0x00FF
  // we want to set bit 13 (in 0xF6) to a 1, so 0x20FF
  //ETHERNET_DEBUG_PRINT("init13: restart port 1 auto negotiation\r\n");
  result = micrelSendCommand(WRITE, 0xF4, 4, 0x00, 0x00, 0xFF, 0x20);

  // check step 13
  //ETHERNET_DEBUG_PRINT("read 4 bytes from 0xF4: expect 0x00 00 FF 20\r\n");
  result = micrelSendCommand(READ, 0xF4, 2, 0, 0, 0, 0);
  checkResults(result, "step13", 2, 0x00, 0x00, 0xFF, 0x20);

  // wait for auto neg to complete
  // check bit 6 in 0xF8
  uint8_t maxWaits = 10;
  uint8_t waitTime = 0;
  result[0] = 0;
  do {
    waitInMilliseconds(300);
    result = micrelSendCommand(READ, 0xF8, 2, 0, 0, 0, 0);
    waitTime++;
  } while ( ((result[0] & 0x40) == 0) && (waitTime < maxWaits) );

  // TODO: note error and return
  if ((result[0] & 0x40) == 0) {
    ETHERNET_DEBUG_PRINT("ERROR: auto negotiate failed\r\n");
    netif->flags &= !NETIF_FLAG_LINK_UP;
    do {
      if (netif->link_callback) {
        (netif->link_callback)(netif);
      }
    } while (0);
  }

  // step 13.1 - set for half duplex - bit 5 of 0xF6
  micrelSendCommand(WRITE, 0xF4, 4, 0x00, 0x00, 0xFF, 0x02);

  // init step 14 & 15
  //ETHERNET_DEBUG_PRINT("init14&15: clear int, enable interrupt\r\n");
  micrelSendCommand(WRITE, 0x90, 4, 0x00, 0xEB, 0xFF, 0xFF);

  // check step 14 & 15
  //ETHERNET_DEBUG_PRINT("read 4 bytes from 0x90: expect 0x00 EB FF FF\r\n");
  result = micrelSendCommand(READ, 0x90, 2, 0, 0, 0, 0);
  checkResults(result, "step14&15", 4, 0x00, 0xEB, 0x00, 0x00);

  // step 16
  //ETHERNET_DEBUG_PRINT("init16: write 2 bytes to 0x70: enable QMU transmit\r\n");
  micrelSendCommand(WRITE, 0x70, 2, 0xEF, 0x01, 0x00, 0x00);

  // check step 16
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x70: expect 0xEF 01\r\n");
  result = micrelSendCommand(READ, 0x70, 2, 0, 0, 0, 0);
  checkResults(result, "step16", 2, 0xEF, 0x01, 0, 0);

  // step 17
  //ETHERNET_DEBUG_PRINT("init17: write 2 bytes to 0x74: enable QMU rx\r\n");
  micrelSendCommand(WRITE, 0x74, 2, NETIF_RXCR1_BROADCAST_SET, 0x7C, 0, 0);

  // check step 17
  //ETHERNET_DEBUG_PRINT("read 2 bytes from 0x74: expect 0xE1 70\r\n");
  result = micrelSendCommand(READ, 0x74, 2, 0, 0, 0, 0);
  checkResults(result, "step17", 2, NETIF_RXCR1_BROADCAST_SET, 0x7C, 0, 0);

#ifdef ETHERNET_STANDALONE_DRIVER
  netif->linkoutput = micrelEthTransmitRaw;

#else //ETHERNET_STANDALONE_DRIVER

  clearEthStats();

  netif->name[0] = 'z';
  netif->name[1] = 'i';
  netif->linkoutput = micrelEthTransmitRaw;
  netif->mtu = MAX_ETHERNET_FRAME_SIZE;
  netif->hwaddr_len = NETIF_MAX_HWADDR_LEN;
  netif->flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST | micrelLinkStatus();
  netif->num = 0;
  netif->state = (void *)micrelGetCurrentState;
#endif //!ETHERNET_STANDALONE_DRIVER

  myEthernetState = ETH_STATE_UP;
  do {
    if (netif->link_callback) {
      (netif->link_callback)(netif);
    }
  } while (0);
  // enable polling for received packets
  ethAutoRxEnabled = true;
}

// ***************************************************************************
// send the raw packet out Ethernet using the Micrel
// ***************************************************************************
int8_t micrelEthTransmitRaw(struct netif* netif, struct pbuf *p)
{
  uint8_t* result;
  uint16_t i;
  struct pbuf *q;
  uint16_t txPacketLength = 0;

  // step 1 - read to make sure we have enough space
  result = micrelSendCommand(READ, 0x78, 2, 0, 0, 0, 0);
  // check space vs (packet being sent + 8)
  i = (result[1] & 0x1F) << 8;
  i += result[0];
  if (i < p->len + 8) {
    ETHERNET_STATS_INC(memerr);
    return ERR_MEM;
  }

  // step 2
  micrelSendCommand(WRITE, 0x90, 2, 0x00, 0x00, 0, 0);

  // step 3
  micrelSendCommand(WRITE, 0x80, 4, 0x00, 0x00, 0x38, 0x02);

  // check step 3
  result = micrelSendCommand(READ, 0x80, 4, 0, 0, 0, 0);

  taskENTER_CRITICAL();

  // step 4 -- set PB4 to LOW -- this is chip select
  GPIO_PBCLR = GPIO_PBCLR | BIT(4);

  // step 5 - TXQ Write Cmd
  micrelSpiWrite(0xC0);

  // step 6 - Frame header 'control word'
  micrelSpiWrite(0x00);
  micrelSpiWrite(0x80);

  // step 7 -- byte count
  for (q = p; q != NULL; q = q->next) {
    txPacketLength += q->len;
  }
  uint16_t padLength = txPacketLength % 4;
  if (padLength > 0) {
    padLength = 4 - padLength;
  }
  uint16_t totalTxLength = txPacketLength + padLength;

  micrelSpiWrite(LOW_BYTE(totalTxLength));
  micrelSpiWrite(HIGH_BYTE(totalTxLength));

  // MUST BE ALIGNED IN 4 BYTES
  // step 8-10 -- write the payload bytes
  for (q = p; q != NULL; q = q->next) {
    uint8_t *qTxData = q->payload;
    for (i = 0; i < q->len; i++) {
      micrelSpiWrite(qTxData[i]);
    }
  }

  // pad out to multiple of 4
  for (i = 0; i < padLength; i++) {
    micrelSpiWrite(0xFF);
  }

  // step 11 -- set PB4 to HIGH -- this is chip select
  GPIO_PBSET = GPIO_PBSET | BIT(4);

  // step 12
  micrelSendCommand(WRITE, 0x80, 4, 0x00, 0x00, 0x30, 0x02);

  // step 12.1
  micrelSendCommand(WRITE, 0x80, 2, 0x01, 0x00, 0, 0);

  // check step 12

  // step 13
  micrelSendCommand(WRITE, 0x90, 2, 0x00, 0xEB, 0, 0);

  taskEXIT_CRITICAL();

  // check 13
  result = micrelSendCommand(READ, 0x90, 2, 0, 0, 0, 0);

  gMyStats.txPackets++;
  ETHERNET_STATS_INC(xmit);
  gMyStats.txBytes += txPacketLength;

  //This is a temporary step. This doesn't guarantee success by any means.

  return ERR_OK;
}

uint8_t ethBuffer[MAX_ETHERNET_FRAME_SIZE];

// ***************************************************************************
// send an ethernet packet
// this function makes the Ethernetr header:
//   1. destMacAddr passed in
//   2. the local srcMacAddr
//   3. etherType passed in
// then sends the bytes in the buffer passed in as the ethernet payload
// ***************************************************************************

// adds 14 byte Ethernet header (mac src, mac dest, etherType)
void micrelEthTransmitEthPayload(uint8_t* destMacAddr, uint16_t etherType, uint16_t payloadLength, uint8_t* pTxData, struct netif* netif)
{
  uint16_t i;

  // dest mac address
  for (i = 0; i < 6; i++) {
    ethBuffer[i] = destMacAddr[i];
  }

  // my source mac address
  for (i = 0; i < 6; i++) {
    ethBuffer[i + 6] = netif->hwaddr[i];
  }

  // ethertype
  ethBuffer[12] = HIGH_BYTE(etherType);
  ethBuffer[13] = LOW_BYTE(etherType);

  // payload
  for (i = 0; i < payloadLength; i++) {
    ethBuffer[14 + i] = pTxData[i];
  }

  struct pbuf p;
  p.len = payloadLength + 14;
  p.payload = ethBuffer;
  micrelEthTransmitRaw(netif, &p);
}

// ***************************************************************************
// check if a packet is waiting to be RX from Micrel
// ***************************************************************************

struct pbuf ethRxBuff;
uint8_t rxBuffer[MAX_ETHERNET_FRAME_SIZE];

void micrelEthReceive(struct netif *netif)
{
  uint8_t* result;
  uint16_t i;
  uint16_t rxStart = halCommonGetInt16uMillisecondTick();

  // step 1
  //ETHERNET_DEBUG_PRINT("rx1: read 4 bytes from 0x90\r\n");
  result = micrelSendCommand(READ, 0x90, 4, 0, 0, 0, 0);

  // check for receive overrun
  if (result[3] & 0x08) {
    ETHERNET_STATS_INC(err);
  }

  // check for a packet being received (rx int bit)
  // check byte 0x93, bit 5 (bit 13 from byte 0x92)
  if (!(result[3] & 0x20)) {
    return;
  }

  // set 0x92 to 0x2800 to clear RX interrupt and Overrun flag
  ETHERNET_DEBUG_PRINT("rx2&3: write 4 bytes from 0x90\r\n");
  result = micrelSendCommand(WRITE, 0x90, 4, 0x00, 0x00, 0x00, 0x28);

  // step 4: get frame count
  ETHERNET_DEBUG_PRINT("rx4: read 2 bytes from 0x9C\r\n");
  result = micrelSendCommand(READ, 0x9C, 2, 0, 0, 0, 0);

  uint8_t rxFrameCount = result[1];

  ETHERNET_DEBUG_PRINT1("\r\nEth RX: found %d waiting frame\r\n", rxFrameCount);

  do {
    // step 5: if we have 0 packets we re-enable interrupts and get out
    if (rxFrameCount < 1) {
      ETHERNET_DEBUG_PRINT("abort read\r\n");
      // step 24
      ETHERNET_DEBUG_PRINT("rx24: enable interrupts write to 0x90\r\n");
      micrelSendCommand(WRITE, 0x90, 2, 0x00, 0xEB, 0, 0);
      gMyStats.rxErrPacketCountZero++;
      return;
    }

    // step 6&7: read frame status & byte count
    ETHERNET_DEBUG_PRINT("rx4: read 2 bytes from 0x7C\r\n");
    result = micrelSendCommand(READ, 0x7C, 4, 0, 0, 0, 0);
    uint16_t frameStatus = (result[1] << 8) + result[0];
    uint16_t rxPacketLength = (result[3] << 8) + result[2];

    ETHERNET_DEBUG_PRINT3("frame status: 0x%2x (%x %x)\r\n", frameStatus, result[0], result[1]);
    ETHERNET_DEBUG_PRINT5("got packet length %d (%x %x %x %x)\r\n", rxPacketLength, result[0], result[1], result[2], result[3]);

    // check frame status
    if (frameStatus & 0x0017) { // use 0x3C17 to include checksum errors
      ETHERNET_DEBUG_PRINT("frame error, abort\r\n");
      // step 8 - set bit 0 in 0x82/0x83 to 1 to clear error
      ETHERNET_DEBUG_PRINT("rx8: release error frame\r\n");
      micrelSendCommand(WRITE, 0x80, 4, 0x00, 0x00, 0x31, 0x02);
      ETHERNET_STATS_INC(chkerr);

      // wait for error frame to be released
      uint8_t maxWaits = 30;
      do {
        waitInMilliseconds(1);
        result = micrelSendCommand(READ, 0x80, 4, 0, 0, 0, 0);
      } while ( (result[2] & 0x01) && (maxWaits > 0) );

      // step 24
      ETHERNET_DEBUG_PRINT("rx24: enable interrupts write to 0x90\r\n");
      micrelSendCommand(WRITE, 0x90, 2, 0x00, 0xEB, 0, 0);
      gMyStats.rxErrErrorFlagSet++;
      return;
    }

    // step 7
    //ETHERNET_DEBUG_PRINT("rx7: read 2 bytes from 0x7E\r\n");
    //result = micrelSendCommand(READ, 0x7C, 4, 0, 0, 0, 0);
    //uint16_t rxPacketLength = (result[3] << 8) + result[2];
    //emberAfCorePrintln( "got packet length %d (%x %x %x %x)\r\n", rxPacketLength, result[0], result[1], result[2], result[3]);

    // check packet length
    if ((rxPacketLength < 1) || (rxPacketLength > MAX_ETHERNET_FRAME_SIZE)) {
      //statsBadLength++;
      // step 8 - set bit 0 in 0x82/0x83 to 1 to clear error
      micrelSendCommand(WRITE, 0x80, 4, 0x00, 0x00, 0x31, 0x02);

      ETHERNET_STATS_INC(lenerr);
      // wait for error frame to be released
      uint8_t maxWaits = 30;
      do {
        waitInMilliseconds(1);
        result = micrelSendCommand(READ, 0x80, 4, 0, 0, 0, 0);
      } while ( (result[2] & 0x01) && (maxWaits > 0) );

      // step 24
      micrelSendCommand(WRITE, 0x90, 2, 0x00, 0xEB, 0, 0);
      gMyStats.rxErrBadLength++;
      return;
    }

    taskENTER_CRITICAL();

    // step 9: reset RXQ frame ptr to 0
    //ETHERNET_DEBUG_PRINT("rx9: reset RXQ frame ptr to 0\r\n");
    micrelSendCommand(WRITE, 0x84, 4, 0x00, 0x40, 0x00, 0x40);

    // step 10: write 1 to bit 3 of 0x82
    //ETHERNET_DEBUG_PRINT("rx10: Start QMU data xfer\r\n");
    micrelSendCommand(WRITE, 0x80, 4, 0x00, 0x00, 0x38, 0x02);

    // step 11 -- set PB4 to LOW -- this is chip select
    GPIO_PBCLR = GPIO_PBCLR | BIT(4);

    // step 12 - RXQ Read Cmd
    micrelSpiWrite(0x80);

    // step 13: read out 4 byte dummy data
    //ETHERNET_DEBUG_PRINT("rx13: read out 4 byte dummy data\r\n");
    micrelSpiWrite(0x00);
    micrelSpiWrite(0x00);
    micrelSpiWrite(0x00);
    micrelSpiWrite(0x00);

    // step 14: status word
    //ETHERNET_DEBUG_PRINT("rx14: read out 2 byte status word\r\n");
    micrelSpiWrite(0x00);
    micrelSpiWrite(0x00);

    // step 15: byte count
    //ETHERNET_DEBUG_PRINT("rx15: read out 2 byte byte count\r\n");
    micrelSpiWrite(0x00);
    micrelSpiWrite(0x00);

    // step 16: dummy data (IP hdr 2-byte offset enable)
    //ETHERNET_DEBUG_PRINT("rx16: read out 2 byte dummy data\r\n");
    micrelSpiWrite(0x00);
    micrelSpiWrite(0x00);

    // step 17-19: read bytes of packet

    for (i = 0; i < rxPacketLength; i++) {
      rxBuffer[i] = micrelSpiWrite(0x00);
    }

    // step 20 -- set PB4 to HIGH -- this is chip select
    GPIO_PBSET = GPIO_PBSET | BIT(4);

    // step 21: stop transfer
    //ETHERNET_DEBUG_PRINT("rx21: Stop QMU data xfer\r\n");
    micrelSendCommand(WRITE, 0x80, 4, 0x00, 0x00, 0x30, 0x02);

    taskEXIT_CRITICAL();
    // step 22: pass to upper layers

    // step 23: process next frame

    // step 24: enable interrupts
    //ETHERNET_DEBUG_PRINT("rx24: enable interrupts write to 0x90\r\n");
    micrelSendCommand(WRITE, 0x90, 2, 0x00, 0xEB, 0, 0);

    // get a pbuf
    struct pbuf* p = pbuf_alloc(PBUF_RAW, rxPacketLength, PBUF_POOL);

    // if no buf we drop it and leave
    if (p == NULL) {
      ETHERNET_DEBUG_PRINT("ERROR: can't alloc pbuf, dropping RX packet\r\n");
      gMyStats.rxErrNoPbufAvail++;
      ETHERNET_STATS_INC(memerr);
      ETHERNET_STATS_INC(drop);
      return;
    }

    // length includes the 2 dummy bytes at the start that we ignore, account for that
    rxPacketLength = rxPacketLength - 2;

    // copy the received bytes to the pbuf
    pbuf_take(p, (uint8_t*) &rxBuffer, rxPacketLength);

    ETHERNET_STATS_INC(recv);
    gMyStats.rxPackets++;
    gMyStats.rxBytes += rxPacketLength;

    // call the input (receive function)
    (netif->input)(p, netif);

    rxFrameCount--;
  } while (rxFrameCount > 0);
}

void micrelGetCurrentState(void)
{
  ETHERNET_DEBUG_PRINT("Not implemented");
}

// ***************************************************************************
// some things require a delay - only used on startup
// ***************************************************************************

void waitInMilliseconds(uint16_t msDelay)
{
  uint16_t start = halCommonGetInt16uMillisecondTick();
  uint16_t now = start + 1;

  halResetWatchdog();

  while ( (uint16_t)(now - start) < msDelay) {
    now = halCommonGetInt16uMillisecondTick();
  }
  //emberAfCorePrintln( "waited for %d ms, from %d to %d\r\n", msDelay, start, now);
}

#ifdef ETHERNET_STANDALONE_DRIVER
void micrelDebugReadRegisters(void)
{
  uint8_t* result;
  uint8_t address;
  uint8_t i;

  for (i = 0; i < 64; i++) {
    address = i * 4;
    result = micrelSendCommand(READ, address, 4, 0, 0, 0, 0);
    emberSerialPrintf(gAppSerial, "0x%x: %x %x %x %x\r\n", address, result[0], result[1], result[2], result[3]);
  }
}
static void printIpAddressStruct(uint8_t* ipAddr, uint8_t printNewline)
{
  // the IP address
  ETHERNET_DEBUG_PRINT4("%d.%d.%d.%d",
                        ipAddr[0], ipAddr[1],
                        ipAddr[2], ipAddr[3]);

  // newline if asked for
  if (printNewline) {
    emberAfCorePrint("\r\n");
  }
}
#endif

uint8_t micrelLinkStatus(void)
{
  uint8_t* result;
  uint8_t ret = 0x00;

  result = micrelSendCommand(READ, 0xE4, 4, 0, 0, 0, 0); // Byte alignment for reaing E6.

  if (result[2] == 0x2C) {
    ret = NETIF_FLAG_LINK_UP;
  }
  return ret;
}

void micrelPrintEthernetRxStats(void)
{
  uint8_t* result;
  // packet filter setting
  emberAfCorePrint("Pkt Filtering...: ");
  result = micrelSendCommand(1, 0x74, 2, 0, 0, 0, 0);
  emberAfCorePrint("RXCR1(0x74)  = 0x%x %x\r\n", result[0], result[1]);

  emberAfCorePrint("Link Status.....: ");
  result = micrelSendCommand(1, 0xE4, 4, 0, 0, 0, 0);
  ETHERNET_DEBUG_PRINT2("P1MBSR(0xE6) = 0x%x %x\r\n", result[2], result[3]);
}

void micrelPrintMacAddress(void)
{
  uint8_t* result;
  result = micrelSendCommand(1, 0x14, 2, 0, 0, 0, 0);
  ETHERNET_DEBUG_PRINT2("%x:%x:", result[1], result[0]);
  result = micrelSendCommand(1, 0x10, 4, 0, 0, 0, 0);
  ETHERNET_DEBUG_PRINT4("%x:%x:%x:%x", result[3], result[2], result[1], result[0]);
}

void micrelPrintVersion(void)
{
  ETHERNET_DEBUG_PRINT3("driver version..: %d.%d changelist %d\r\n",
                        SLABGW_MAJOR_VERSION,
                        SLABGW_MINOR_VERSION,
                        SLABGW_CHANGE_VERSION);
  ETHERNET_DEBUG_PRINT4("stack version...: %d.%d.%d build %d\r\n",
                        emberVersion.major,
                        emberVersion.minor,
                        emberVersion.patch,
                        emberVersion.build);
}
