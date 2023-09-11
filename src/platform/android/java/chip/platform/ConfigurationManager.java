/*
 * SPDX-FileCopyrightText: 2021-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

/** Java interface for ConfigurationManager */
public interface ConfigurationManager {
  // NVS namespaces used to store device configuration information.
  String kConfigNamespace_ChipFactory = "chip-factory";
  String kConfigNamespace_ChipConfig = "chip-config";
  String kConfigNamespace_ChipCounters = "chip-counters";

  // Keys stored in the Chip-factory namespace
  String kConfigKey_SerialNum = "serial-num";
  String kConfigKey_MfrDeviceId = "device-id";
  String kConfigKey_MfrDeviceCert = "device-cert";
  String kConfigKey_MfrDeviceICACerts = "device-ca-certs";
  String kConfigKey_MfrDevicePrivateKey = "device-key";
  String kConfigKey_HardwareVersion = "hardware-ver";
  String kConfigKey_HardwareVersionString = "hardware-ver-str";
  String kConfigKey_ManufacturingDate = "mfg-date";
  String kConfigKey_SetupPinCode = "pin-code";
  String kConfigKey_SetupDiscriminator = "discriminator";
  String kConfigKey_ProductId = "product-id";
  String kConfigKey_ProductName = "product-name";
  String kConfigKey_SoftwareVersion = "software-version";
  String kConfigKey_SoftwareVersionString = "software-version-str";
  String kConfigKey_PartNumber = "part-number";
  String kConfigKey_ProductURL = "product-url";
  String kConfigKey_ProductLabel = "product-label";
  String kConfigKey_UniqueId = "uniqueId";
  String kConfigKey_Spake2pIterationCount = "iteration-count";
  String kConfigKey_Spake2pSalt = "salt";
  String kConfigKey_Spake2pVerifier = "verifier";
  String kConfigKey_DeviceTypeId = "device-type-id";
  String kConfigKey_DeviceName = "device-name";

  // Keys stored in the Chip-config namespace
  String kConfigKey_ServiceConfig = "service-config";
  String kConfigKey_PairedAccountId = "account-id";
  String kConfigKey_ServiceId = "service-id";
  String kConfigKey_LastUsedEpochKeyId = "last-ek-id";
  String kConfigKey_FailSafeArmed = "fail-safe-armed";
  String kConfigKey_WiFiStationSecType = "sta-sec-type";
  String kConfigKey_RegulatoryLocation = "regulatory-location";
  String kConfigKey_CountryCode = "country-code";

  long readConfigValueLong(String namespace, String name) throws AndroidChipPlatformException;

  String readConfigValueStr(String namespace, String name) throws AndroidChipPlatformException;

  byte[] readConfigValueBin(String namespace, String name) throws AndroidChipPlatformException;

  void writeConfigValueLong(String namespace, String name, long val)
      throws AndroidChipPlatformException;

  void writeConfigValueStr(String namespace, String name, String val)
      throws AndroidChipPlatformException;

  void writeConfigValueBin(String namespace, String name, byte[] val)
      throws AndroidChipPlatformException;

  void clearConfigValue(String namespace, String name) throws AndroidChipPlatformException;

  boolean configValueExists(String namespace, String name) throws AndroidChipPlatformException;
}
