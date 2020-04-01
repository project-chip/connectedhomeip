/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_COAP
#include EMBER_AF_API_NETWORK_DIAGNOSTICS
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#include "stack/ip/commission.h"

#ifndef ALIAS
  #define ALIAS(x) x
#endif

void networkManagementDiagGetCommand(void)
{
  EmberIpv6Address destination;
  uint8_t *tlvs;
  uint8_t length;

  if (!emberGetIpArgument(0, destination.bytes)) {
    emberAfAppPrintln("%p: %p", "ERR", "Can't parse IP address");
    return;
  }

  tlvs = emberStringCommandArgument(1, &length);

  emberSendDiagnosticGet(&destination, tlvs, length);
}

void networkManagementDiagQueryCommand(void)
{
  EmberIpv6Address destination;
  uint8_t *tlvs;
  uint8_t length;

  if (!emberGetIpArgument(0, destination.bytes)) {
    emberAfAppPrintln("%p: %p", "ERR", "Can't parse IP address");
    return;
  }

  tlvs = emberStringCommandArgument(1, &length);

  emberSendDiagnosticQuery(&destination, tlvs, length);
}

void networkManagementDiagResetCommand(void)
{
  EmberIpv6Address destination;
  uint8_t *tlvs;
  uint8_t length;

  if (!emberGetIpArgument(0, destination.bytes)) {
    emberAfAppPrintln("%p: %p", "ERR", "Can't parse IP address");
    return;
  }

  tlvs = emberStringCommandArgument(1, &length);

  emberSendDiagnosticReset(&destination, tlvs, length);
}

void ALIAS(emberDiagnosticAnswerHandler)(EmberStatus status,
                                         const EmberIpv6Address * remoteAddress,
                                         const uint8_t * payload,
                                         uint16_t payloadLength)
{
  EmberDiagnosticData diagnosticData;

  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p: %p, %p: %u", "ERR", "can't send diagnostic request", "status", status);
    return;
  }

  emberAfAppPrint("Received diagnostic response: ");
  emberAfAppDebugExec(emberAfPrintIpv6Address(remoteAddress));
  emberAfAppPrintln("");

  if (!emberParseDiagnosticData(&diagnosticData, payload, payloadLength)) {
    emberAfAppPrintln("%p: %p", "ERR", "Can't parse diagnostic data");
    return;
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_MAC_EXTENDED_ADDRESS)) {
    emberAfAppPrint("Mac extended address: ");
    emberAfAppPrintBuffer(diagnosticData.macExtendedAddress, 8, true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_ADDRESS_16)) {
    emberAfAppPrintln("Address16: 0x%2X", diagnosticData.address16);
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_MODE)) {
    emberAfAppPrintln("Mode: %u", diagnosticData.mode);
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_TIMEOUT)) {
    emberAfAppPrintln("Timeout: %u", diagnosticData.timeout);
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_CONNECTIVITY)) {
    emberAfAppPrint("Connectivity: ");
    emberAfAppPrintBuffer(diagnosticData.connectivity + 1, diagnosticData.connectivity[0], true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_ROUTING_TABLE)) {
    emberAfAppPrint("Routing table: ");
    emberAfAppPrintBuffer(diagnosticData.routingTable + 1, diagnosticData.routingTable[0], true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_LEADER_DATA)) {
    emberAfAppPrint("Leader data: ");
    emberAfAppPrintBuffer(diagnosticData.leaderData + 1, diagnosticData.leaderData[0], true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_NETWORK_DATA)) {
    emberAfAppPrint("Network data: ");
    emberAfAppPrintBuffer(diagnosticData.networkData + 1, diagnosticData.networkData[0], true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_IPV6_ADDRESS_LIST)) {
    emberAfAppPrint("IPv6 address list: ");
    emberAfAppPrintBuffer(diagnosticData.ipv6AddressList + 1, diagnosticData.ipv6AddressList[0], true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_MAC_COUNTERS)) {
    emberAfAppPrint("Mac counters list: ");
    emberAfAppPrintBuffer(diagnosticData.macCounters + 1, diagnosticData.macCounters[0], true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_BATTERY_LEVEL)) {
    emberAfAppPrintln("Battery level: %u", diagnosticData.batteryLevel);
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_VOLTAGE)) {
    emberAfAppPrintln("Voltage: %u", diagnosticData.voltage);
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_CHILD_TABLE)) {
    emberAfAppPrint("Child table: ");
    emberAfAppPrintBuffer(diagnosticData.childTable + 1, diagnosticData.childTable[0], true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_CHANNEL_PAGES)) {
    emberAfAppPrint("Channel pages: ");
    emberAfAppPrintBuffer(diagnosticData.channelPages + 1, diagnosticData.channelPages[0], true);
    emberAfAppPrintln("");
  }

  if (emberDiagnosticDataHasTlv(&diagnosticData, DIAGNOSTIC_MAX_CHILD_TIMEOUT)) {
    emberAfAppPrintln("Max child timeout: %u", diagnosticData.maxChildTimeout);
  }
}

void networkManagementSetVendorInfoCommand(void)
{
  uint8_t *tlvs;
  uint8_t length;

  tlvs = emberStringCommandArgument(0, &length);

  emberSetVendorTlvs(tlvs, length);
}

void ALIAS(emberSetVendorTlvsReturn)(EmberStatus status, uint16_t length)
{
  emberAfAppPrintln("%p %u", "Set Vendor TLVs", status);
}

void networkManagementMgmtGetCommand(void)
{
  EmberIpv6Address destination;
  uint8_t *tlvs;
  uint8_t length;

  if (!emberGetIpArgument(0, destination.bytes)) {
    emberAfAppPrintln("%p: %p", "ERR", "Can't parse IP address");
    return;
  }

  tlvs = emberStringCommandArgument(1, &length);

  EmberStatus status = emberSendManagementGetRequest(&destination, tlvs, length);
  if (status != EMBER_SUCCESS) {
    emberAfAppPrintln("%p: %p, %p: %u", "ERR", "can't send mgmt_get request", "status", status);
  }
}

void ALIAS(emberManagementGetResponseHandler)(EmberCoapStatus status,
                                              EmberCoapCode code,
                                              EmberCoapReadOptions * options,
                                              uint8_t * payload,
                                              uint16_t payloadLength,
                                              EmberCoapResponseInfo * info)
{
  EmberCommissionData commissionData;

  if (status != EMBER_COAP_MESSAGE_RESPONSE) {
    return;
  }

  emberAfAppPrint("Received MGMT_GET response: ");
  emberAfAppDebugExec(emberAfPrintIpv6Address(&info->remoteAddress));
  emberAfAppPrintln("");

  if (!emberParseManagementGetData(&commissionData, payload, payloadLength)) {
    emberAfAppPrintln("%p: %p", "ERR", "Can't parse commission data");
    return;
  }

  if (emberCommissionDataHasTlv(&commissionData, COMMISSION_PROVISIONING_URL_TLV)) {
    emberAfAppPrint("Provisioning URL: ");
    emberAfAppPrintBuffer(commissionData.provisioningUrl + 1, commissionData.provisioningUrl[0], true);
    emberAfAppPrintln("");
  }

  if (emberCommissionDataHasTlv(&commissionData, COMMISSION_VENDOR_NAME_TLV)) {
    emberAfAppPrint("Vendor name: ");
    emberAfAppPrintBuffer(commissionData.vendorName + 1, commissionData.vendorName[0], true);
    emberAfAppPrintln("");
  }

  if (emberCommissionDataHasTlv(&commissionData, COMMISSION_VENDOR_MODEL_TLV)) {
    emberAfAppPrint("Vendor model: ");
    emberAfAppPrintBuffer(commissionData.vendorModel + 1, commissionData.vendorModel[0], true);
    emberAfAppPrintln("");
  }

  if (emberCommissionDataHasTlv(&commissionData, COMMISSION_VENDOR_SW_VERSION_TLV)) {
    emberAfAppPrint("SW version: ");
    emberAfAppPrintBuffer(commissionData.swVersion + 1, commissionData.swVersion[0], true);
    emberAfAppPrintln("");
  }

  if (emberCommissionDataHasTlv(&commissionData, COMMISSION_VENDOR_DATA_TLV)) {
    emberAfAppPrint("Vendor data: ");
    emberAfAppPrintBuffer(commissionData.vendorData + 1, commissionData.vendorData[0], true);
    emberAfAppPrintln("");
  }

  if (emberCommissionDataHasTlv(&commissionData, COMMISSION_VENDOR_STACK_VERSION_TLV)) {
    emberAfAppPrint("Vendor stack version: ");
    emberAfAppPrintBuffer(commissionData.vendorStackVersion + 1, commissionData.vendorStackVersion[0], true);
    emberAfAppPrintln("");
  }
}
