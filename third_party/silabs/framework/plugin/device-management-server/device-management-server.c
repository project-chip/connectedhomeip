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
 * @brief Server side implementation of the SE 1.2 cluster, Device Management.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/device-management-server/device-management-common.h"
#include "app/framework/plugin/device-management-server/device-management-server.h"

static EmberAfDeviceManagementInfo pmInfo;

void emberAfPluginDeviceManagementServerInitCallback(void)
{
  // invalidate passwords since 0 means forever valid
  pmInfo.servicePassword.durationInMinutes = 1;
  pmInfo.consumerPassword.durationInMinutes = 1;

  // since implementationDateTime of 0 is special. we'll be using 1 (12/31/1970)
  // to initialize as a non-valid pending time.
  pmInfo.cin.implementationDateTime = 1;
  pmInfo.pendingUpdates = 0;
}

bool emberAfPluginDeviceManagementSetProviderId(uint32_t providerId)
{
  pmInfo.providerId = providerId;
  return true;
}

bool emberAfPluginDeviceManagementSetIssuerEventId(uint32_t issuerEventId)
{
  pmInfo.issuerEventId = issuerEventId;
  return true;
}

bool emberAfPluginDeviceManagementSetTariffType(EmberAfTariffType tariffType)
{
  pmInfo.tariffType = (EmberAfTariffType )tariffType;
  return true;
}

bool emberAfPluginDeviceManagementSetInfoGlobalData(uint32_t providerId,
                                                    uint32_t issuerEventId,
                                                    uint8_t tariffType)
{
  pmInfo.providerId = providerId;
  pmInfo.issuerEventId = issuerEventId;
  pmInfo.tariffType = (EmberAfTariffType ) tariffType;

  return true;
}

bool emberAfPluginDeviceManagementSetTenancy(EmberAfDeviceManagementTenancy *tenancy,
                                             bool validateOptionalFields)
{
  // Don't allow tenancy to be set with an older event ID than we currently have.
  if (validateOptionalFields) {
    if ((tenancy->providerId != pmInfo.providerId)
        || (tenancy->issuerEventId < pmInfo.issuerEventId)
        || (tenancy->tariffType != pmInfo.tariffType)) {
      return false;
    }
  }

  if (tenancy == NULL) {
    MEMSET(&(pmInfo.tenancy), 0, sizeof(EmberAfDeviceManagementTenancy));
  } else {
    MEMMOVE(&(pmInfo.tenancy), tenancy, sizeof(EmberAfDeviceManagementTenancy));
  }

  return true;
}

bool emberAfPluginDeviceManagementGetTenancy(EmberAfDeviceManagementTenancy *tenancy)
{
  if (tenancy == NULL) {
    return false;
  }

  MEMMOVE(tenancy, &(pmInfo.tenancy), sizeof(EmberAfDeviceManagementTenancy));

  return true;
}

bool emberAfPluginDeviceManagementSetSupplier(uint8_t endpoint, EmberAfDeviceManagementSupplier *supplier)
{
  if (supplier == NULL) {
    MEMSET(&(pmInfo.supplier), 0, sizeof(EmberAfDeviceManagementSupplier));
  } else {
    MEMMOVE(&(pmInfo.supplier), supplier, sizeof(EmberAfDeviceManagementSupplier));
  }

  if (supplier->implementationDateTime <= emberAfGetCurrentTime()) {
    EmberStatus status  = emberAfWriteAttribute(endpoint,
                                                ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                                ZCL_PROVIDER_NAME_ATTRIBUTE_ID,
                                                CLUSTER_MASK_SERVER,
                                                (uint8_t *)&(supplier->proposedProviderName),
                                                ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      status  = emberAfWriteAttribute(endpoint,
                                      ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
                                      ZCL_PROVIDER_CONTACT_DETAILS_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      (uint8_t *)&(supplier->proposedProviderContactDetails),
                                      ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
    }
  } else {
    // TODO: need to setup timer to 'schedule' attribute update when implementation is reached.
  }

  return true;
}

bool emberAfPluginDeviceManagementGetSupplier(EmberAfDeviceManagementSupplier *supplier)
{
  if (supplier == NULL) {
    return false;
  }

  MEMMOVE(supplier, &(pmInfo.supplier), sizeof(EmberAfDeviceManagementSupplier));

  return true;
}

bool emberAfPluginDeviceManagementSetSupply(EmberAfDeviceManagementSupply *supply)
{
  if (supply == NULL) {
    MEMSET(&(pmInfo.supply), 0, sizeof(EmberAfDeviceManagementSupply));
  } else {
    MEMMOVE(&(pmInfo.supply), supply, sizeof(EmberAfDeviceManagementSupply));
  }

  return true;
}

bool emberAfPluginDeviceManagementGetSupply(EmberAfDeviceManagementSupply *supply)
{
  if (supply == NULL) {
    return false;
  }

  MEMMOVE(supply, &(pmInfo.supply), sizeof(EmberAfDeviceManagementSupply));

  return true;
}

bool emberAfPluginDeviceManagementSetSiteId(EmberAfDeviceManagementSiteId *siteId)
{
  if (siteId == NULL) {
    MEMSET(&(pmInfo.siteId), 0, sizeof(EmberAfDeviceManagementSiteId));
  } else {
    MEMMOVE(&(pmInfo.siteId), siteId, sizeof(EmberAfDeviceManagementSiteId));
  }

  return true;
}

bool emberAfPluginDeviceManagementGetSiteId(EmberAfDeviceManagementSiteId *siteId)
{
  if (siteId == NULL) {
    return false;
  }

  MEMMOVE(siteId, &(pmInfo.siteId), sizeof(EmberAfDeviceManagementSiteId));

  return true;
}

bool emberAfPluginDeviceManagementSetCIN(EmberAfDeviceManagementCIN *cin)
{
  MEMCOPY(&(pmInfo.cin), cin, sizeof(EmberAfDeviceManagementCIN));
  return true;
}

bool emberAfPluginDeviceManagementGetCIN(EmberAfDeviceManagementCIN *cin)
{
  if (cin == NULL) {
    return false;
  }

  MEMCOPY(cin, &(pmInfo.cin), sizeof(EmberAfDeviceManagementCIN));
  return true;
}

bool emberAfPluginDeviceManagementSetPassword(EmberAfDeviceManagementPassword *password)
{
  if (password == NULL) {
    MEMSET(&(pmInfo.servicePassword), 0, sizeof(EmberAfDeviceManagementPassword));
    MEMSET(&(pmInfo.consumerPassword), 0, sizeof(EmberAfDeviceManagementPassword));
  } else {
    if (password->passwordType == SERVICE_PASSWORD) {
      MEMMOVE(&(pmInfo.servicePassword), password, sizeof(EmberAfDeviceManagementPassword));
    } else if (password->passwordType == CONSUMER_PASSWORD) {
      MEMMOVE(&(pmInfo.consumerPassword), password, sizeof(EmberAfDeviceManagementPassword));
    } else {
      return false;
    }
  }

  return true;
}

bool emberAfPluginDeviceManagementGetPassword(EmberAfDeviceManagementPassword *password,
                                              uint8_t passwordType)
{
  if (password == NULL) {
    return false;
  }

  switch (passwordType) {
    case SERVICE_PASSWORD:
      MEMMOVE(password, &(pmInfo.servicePassword), sizeof(EmberAfDeviceManagementPassword));
      break;
    case CONSUMER_PASSWORD:
      MEMMOVE(password, &(pmInfo.consumerPassword), sizeof(EmberAfDeviceManagementPassword));
      break;
    default:
      return false;
  }

  return true;
}

void emberAfDeviceManagementServerPrint(void)
{
  emberAfDeviceManagementClusterPrintln("== Device Management Information ==\n");

  emberAfDeviceManagementClusterPrintln("  == Tenancy ==");
  emberAfDeviceManagementClusterPrintln("  Implementation Date / Time: %4x", pmInfo.tenancy.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("  Tenancy: %4x\n", pmInfo.tenancy.tenancy);

  emberAfDeviceManagementClusterPrintln("  == Supplier ==");
  emberAfDeviceManagementClusterPrint("  Provider name: ");
  emberAfDeviceManagementClusterPrintString(pmInfo.supplier.proposedProviderName);
  emberAfDeviceManagementClusterPrintln("\n  Proposed Provider Id: %4x", pmInfo.supplier.proposedProviderId);
  emberAfDeviceManagementClusterPrintln("  Implementation Date / Time: %4x", pmInfo.supplier.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("  Provider Change Control: %4x\n", pmInfo.supplier.providerChangeControl);

  emberAfDeviceManagementClusterPrintln("  == Supply ==");
  emberAfDeviceManagementClusterPrintln("  Request Date / Time: %4x", pmInfo.supply.requestDateTime);
  emberAfDeviceManagementClusterPrintln("  Implementation Date / Time: %4x", pmInfo.supply.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("  Supply Status: %x", pmInfo.supply.supplyStatus);
  emberAfDeviceManagementClusterPrintln("  Originator Id / Supply Control Bits: %x\n", pmInfo.supply.originatorIdSupplyControlBits);

  emberAfDeviceManagementClusterPrintln("  == Site ID ==");
  emberAfDeviceManagementClusterPrint("  Site ID: ");
  emberAfDeviceManagementClusterPrintString(pmInfo.siteId.siteId);
  emberAfDeviceManagementClusterPrintln("\n  Site Id Implementation Date / Time: %4x\n", pmInfo.siteId.implementationDateTime);

  emberAfDeviceManagementClusterPrintln("  == Customer ID Number ==");
  emberAfDeviceManagementClusterPrint("  Customer ID Number: ");
  emberAfDeviceManagementClusterPrintString(pmInfo.cin.cin);
  emberAfDeviceManagementClusterPrintln("\n  Customer ID Number Implementation Date / Time: %4x\n", pmInfo.cin.implementationDateTime);

  emberAfDeviceManagementClusterPrintln("  == Supply Status ==");
  emberAfDeviceManagementClusterPrintln("  Supply Status: %x\n", pmInfo.supplyStatus.supplyStatus);
  emberAfDeviceManagementClusterPrintln("  Implementation Date / Time: %4x", pmInfo.supplyStatus.implementationDateTime);

  emberAfDeviceManagementClusterPrintln("  == Passwords ==");

  emberAfDeviceManagementClusterPrintln("   = Service Password =");
  emberAfDeviceManagementClusterPrint("   Password: ");
  emberAfDeviceManagementClusterPrintString(pmInfo.servicePassword.password);
  emberAfDeviceManagementClusterPrintln("\n   Implementation Date / Time: %4x", pmInfo.servicePassword.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("   Duration In Minutes: %2x", pmInfo.servicePassword.durationInMinutes);
  emberAfDeviceManagementClusterPrintln("   Password Type: %x\n", pmInfo.servicePassword.passwordType);

  emberAfDeviceManagementClusterPrintln("   = Consumer Password =");
  emberAfDeviceManagementClusterPrint("   Password: ");
  emberAfDeviceManagementClusterPrintString(pmInfo.consumerPassword.password);
  emberAfDeviceManagementClusterPrintln("\n   Implementation Date / Time: %4x", pmInfo.consumerPassword.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("   Duration In Minutes: %2x", pmInfo.consumerPassword.durationInMinutes);
  emberAfDeviceManagementClusterPrintln("   Password Type: %x\n", pmInfo.consumerPassword.passwordType);

  emberAfDeviceManagementClusterPrintln("  == Uncontrolled Flow Threshold ==");
  emberAfDeviceManagementClusterPrintln("  Uncontrolled Flow Threshold: %2x", pmInfo.threshold.uncontrolledFlowThreshold);
  emberAfDeviceManagementClusterPrintln("  Multiplier: %2x", pmInfo.threshold.multiplier);
  emberAfDeviceManagementClusterPrintln("  Divisor: %2x", pmInfo.threshold.divisor);
  emberAfDeviceManagementClusterPrintln("  Measurement Period: %2x", pmInfo.threshold.measurementPeriod);
  emberAfDeviceManagementClusterPrintln("  Unit of Measure: %x", pmInfo.threshold.unitOfMeasure);
  emberAfDeviceManagementClusterPrintln("  Stabilisation Period: %x\n", pmInfo.threshold.stabilisationPeriod);

  emberAfDeviceManagementClusterPrintln("  == Issuer Event ID ==");
  emberAfDeviceManagementClusterPrintln("  Issuer Event Id: %4x", pmInfo.issuerEventId);

  emberAfDeviceManagementClusterPrintln("== End of Device Management Information ==");
}

bool emberAfDeviceManagementClusterGetChangeOfTenancyCallback(void)
{
  EmberAfDeviceManagementTenancy *tenancy = &(pmInfo.tenancy);
  if ((pmInfo.pendingUpdates & EMBER_AF_DEVICE_MANAGEMENT_CHANGE_OF_TENANCY_PENDING_MASK) == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    emberAfFillCommandDeviceManagementClusterPublishChangeOfTenancy(pmInfo.providerId,
                                                                    pmInfo.issuerEventId,
                                                                    pmInfo.tariffType,
                                                                    tenancy->implementationDateTime,
                                                                    tenancy->tenancy);
    emberAfSendResponse();
  }

  return true;
}

bool emberAfDeviceManagementClusterPublishChangeOfTenancy(EmberNodeId dstAddr,
                                                          uint8_t srcEndpoint,
                                                          uint8_t dstEndpoint)
{
  EmberStatus status;
  EmberAfDeviceManagementTenancy *tenancy = &(pmInfo.tenancy);
  emberAfFillCommandDeviceManagementClusterPublishChangeOfTenancy(pmInfo.providerId,
                                                                  pmInfo.issuerEventId,
                                                                  pmInfo.tariffType,
                                                                  tenancy->implementationDateTime,
                                                                  tenancy->tenancy);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);

  if (status != EMBER_SUCCESS) {
    emberAfDeviceManagementClusterPrintln("Unable to unicast PublishChangeOfTenancy command: 0x%x", status);
  }
  return true;
}

bool emberAfDeviceManagementClusterGetChangeOfSupplierCallback(void)
{
  EmberAfDeviceManagementSupplier *supplier = &(pmInfo.supplier);

  if ((pmInfo.pendingUpdates & EMBER_AF_DEVICE_MANAGEMENT_CHANGE_OF_SUPPLIER_PENDING_MASK) == 0 ) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    emberAfFillCommandDeviceManagementClusterPublishChangeOfSupplier(pmInfo.providerId,
                                                                     pmInfo.issuerEventId,
                                                                     pmInfo.tariffType,
                                                                     supplier->proposedProviderId,
                                                                     supplier->implementationDateTime,
                                                                     supplier->providerChangeControl,
                                                                     supplier->proposedProviderName,
                                                                     supplier->proposedProviderContactDetails);
    emberAfSendResponse();
  }

  return true;
}

bool emberAfDeviceManagementClusterPublishChangeOfSupplier(EmberNodeId dstAddr,
                                                           uint8_t srcEndpoint,
                                                           uint8_t dstEndpoint)
{
  EmberStatus status;
  EmberAfDeviceManagementSupplier *supplier = &(pmInfo.supplier);
  emberAfFillCommandDeviceManagementClusterPublishChangeOfSupplier(pmInfo.providerId,
                                                                   pmInfo.issuerEventId,
                                                                   pmInfo.tariffType,
                                                                   supplier->proposedProviderId,
                                                                   supplier->implementationDateTime,
                                                                   supplier->providerChangeControl,
                                                                   supplier->proposedProviderName,
                                                                   supplier->proposedProviderContactDetails);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);

  if (status != EMBER_SUCCESS) {
    emberAfDeviceManagementClusterPrintln("Unable to unicast PublishChangeOfSupplier command: 0x%x", status);
  }
  return true;
}

bool emberAfDeviceManagementClusterGetSiteIdCallback(void)
{
  EmberAfDeviceManagementSiteId *siteId = &(pmInfo.siteId);

  if (pmInfo.pendingUpdates & EMBER_AF_DEVICE_MANAGEMENT_UPDATE_SITE_ID_PENDING_MASK) {
    emberAfFillCommandDeviceManagementClusterUpdateSiteId(pmInfo.issuerEventId,
                                                          siteId->implementationDateTime,
                                                          pmInfo.providerId,
                                                          siteId->siteId);
    emberAfSendResponse();
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  }

  return true;
}

bool emberAfDeviceManagementClusterGetCINCallback(void)
{
  EmberAfDeviceManagementCIN *cin = &(pmInfo.cin);

  if (pmInfo.pendingUpdates & EMBER_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK) {
    emberAfFillCommandDeviceManagementClusterUpdateCIN(pmInfo.issuerEventId,
                                                       cin->implementationDateTime,
                                                       pmInfo.providerId,
                                                       cin->cin);
    emberAfSendResponse();
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  }

  return true;
}

bool emberAfDeviceManagementClusterRequestNewPasswordCallback(uint8_t passwordType)
{
  EmberAfDeviceManagementPassword *password;
  switch (passwordType) {
    case SERVICE_PASSWORD:
      password = &(pmInfo.servicePassword);
      break;
    case CONSUMER_PASSWORD:
      password = &(pmInfo.consumerPassword);
      break;
    default:
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
      return true;
  }

  // Is the password still valid?
  if ((password->durationInMinutes != 0)
      && (emberAfGetCurrentTime() > password->implementationDateTime + (password->durationInMinutes * 60))) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    emberAfFillCommandDeviceManagementClusterRequestNewPasswordResponse(pmInfo.issuerEventId,
                                                                        password->implementationDateTime,
                                                                        password->durationInMinutes,
                                                                        password->passwordType,
                                                                        password->password);

    emberAfSendResponse();
  }

  return true;
}

bool emberAfDeviceManagementClusterSendRequestNewPasswordResponse(uint8_t passwordType,
                                                                  EmberNodeId dstAddr,
                                                                  uint8_t srcEndpoint,
                                                                  uint8_t dstEndpoint)
{
  EmberStatus status;
  EmberAfDeviceManagementPassword *password;

  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

  switch (passwordType) {
    case SERVICE_PASSWORD:
      password = &(pmInfo.servicePassword);
      break;
    case CONSUMER_PASSWORD:
      password = &(pmInfo.consumerPassword);
      break;
    default:
      return false;
  }

  // Is the password still valid?
  if ((password->durationInMinutes != 0)
      && (emberAfGetCurrentTime() > password->implementationDateTime + (password->durationInMinutes * 60))) {
    return false;
  } else {
    emberAfFillCommandDeviceManagementClusterRequestNewPasswordResponse(pmInfo.issuerEventId,
                                                                        password->implementationDateTime,
                                                                        password->durationInMinutes,
                                                                        password->passwordType,
                                                                        password->password);

    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);
    if (status != EMBER_SUCCESS) {
      return false;
    } else {
      return true;
    }
  }
}

bool emberAfDeviceManagementClusterUpdateSiteId(EmberNodeId dstAddr,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint)
{
  EmberStatus status;
  EmberAfDeviceManagementSiteId *siteId = &(pmInfo.siteId);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

  emberAfFillCommandDeviceManagementClusterUpdateSiteId(pmInfo.issuerEventId,
                                                        siteId->implementationDateTime,
                                                        pmInfo.providerId,
                                                        siteId->siteId);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);

  if (status != EMBER_SUCCESS) {
    emberAfDeviceManagementClusterPrintln("Unable to unicast UpdateSiteId command: 0x%x", status);
  }
  return true;
}

bool emberAfDeviceManagementClusterUpdateCIN(EmberNodeId dstAddr,
                                             uint8_t srcEndpoint,
                                             uint8_t dstEndpoint)
{
  EmberStatus status;
  EmberAfDeviceManagementCIN *cin = &(pmInfo.cin);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

  pmInfo.pendingUpdates |= EMBER_AF_DEVICE_MANAGEMENT_UPDATE_CIN_PENDING_MASK;

  emberAfFillCommandDeviceManagementClusterUpdateCIN(pmInfo.issuerEventId,
                                                     cin->implementationDateTime,
                                                     pmInfo.providerId,
                                                     cin->cin);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dstAddr);

  if (status != EMBER_SUCCESS) {
    emberAfDeviceManagementClusterPrintln("Unable to unicast UpdateCIN command: 0x%x", status);
  }
  return true;
}

void emberAfDeviceManagementClusterSetPendingUpdates(EmberAfDeviceManagementChangePendingFlags pendingUpdatesMask)
{
  pmInfo.pendingUpdates = pendingUpdatesMask;
}

void emberAfDeviceManagementClusterGetPendingUpdates(EmberAfDeviceManagementChangePendingFlags *pendingUpdatesMask)
{
  *(pendingUpdatesMask) = pmInfo.pendingUpdates;
}

bool emberAfDeviceManagementClusterReportEventConfigurationCallback(uint8_t commandIndex,
                                                                    uint8_t totalCommands,
                                                                    uint8_t* eventConfigurationPayload)
{
  return true;
}
