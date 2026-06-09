/*
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
 *    @file
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for stm32wb platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <platform/stm32/stm32wba/STM32Config.h>

#if CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING
#include <platform/internal/FactoryProvisioning.ipp>
#endif // CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ConfigurationManagerImpl& ConfigurationManagerImpl::GetDefaultInstance() {
	static ConfigurationManagerImpl sInstance;
	return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init() {
	CHIP_ERROR err;
	uint32_t rebootCount;
    BootReasonType matterBootCause;

	// Initialize the generic implementation base class.
	err = Internal::GenericConfigurationManagerImpl<STM32Config>::Init();

	if (STM32Config::ConfigValueExists(STM32Config::kCounterKey_RebootCount)) {
		err = GetRebootCount(rebootCount);
		SuccessOrExit(err);

		// Do not increment reboot count if the value is going to overflow UINT32.
		err = StoreRebootCount(
				rebootCount < UINT16_MAX ? rebootCount + 1 : rebootCount);
		SuccessOrExit(err);
	} else {
		// The first boot after factory reset of the Node.
		err = StoreRebootCount(1);
		SuccessOrExit(err);
	}

	matterBootCause = (BootReasonType)APPE_GetBootReason();

	ChipLogDetail(DataManagement, "----- BOOT cause : %d -----",matterBootCause);
	ChipLogDetail(DataManagement, "----- BOOT cause : %d -----",matterBootCause);
	/* Clear RCC reset flags after determining the boot reason type */

	/* Store Boot Reason Type */
	err = StoreBootReason(to_underlying(matterBootCause));
	SuccessOrExit(err);

	if (!STM32Config::ConfigValueExists(
			STM32Config::kCounterKey_TotalOperationalHours)) {
		err = StoreTotalOperationalHours(0);
		SuccessOrExit(err);
	}

	err = CHIP_NO_ERROR;

	exit: return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t &rebootCount) {
	return ReadConfigValue(STM32Config::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount) {
	return WriteConfigValue(STM32Config::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t &bootReason) {
	return ReadConfigValue(STM32Config::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::StoreBootReason(uint32_t bootReason) {
	return WriteConfigValue(STM32Config::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(
		uint32_t &totalOperationalHours) {
	return ReadConfigValue(STM32Config::kCounterKey_TotalOperationalHours,
			totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(
		uint32_t totalOperationalHours) {
	return WriteConfigValue(STM32Config::kCounterKey_TotalOperationalHours,
			totalOperationalHours);
}

bool ConfigurationManagerImpl::CanFactoryReset() {
// TODO: query the application to determine if factory reset is allowed.
	return true;
}

void ConfigurationManagerImpl::InitiateFactoryReset() {
	PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(
		::chip::Platform::PersistedStorage::Key persistedStorageKey,
		uint32_t &value) {
	CHIP_ERROR err = CHIP_NO_ERROR;
	ChipLogDetail(DataManagement, "ST => ReadPersistedStorageValue");

	err = ReadConfigValue(persistedStorageKey, value);
	if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND) {
		err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
	}
	SuccessOrExit(err);

	exit: return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(
		::chip::Platform::PersistedStorage::Key persistedStorageKey,
		uint32_t value) {
	CHIP_ERROR err = CHIP_NO_ERROR;
	ChipLogDetail(DataManagement, "ST => WritePersistedStorageValue");
	err = WriteConfigValue(persistedStorageKey, value);
	SuccessOrExit(err);
	exit: return err;
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool &val) {
	return STM32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t &val) {
	return STM32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t &val) {
	return STM32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char *buf,
		size_t bufSize, size_t &outLen) {
	return STM32Config::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t *buf,
		size_t bufSize, size_t &outLen) {
	return STM32Config::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val) {
	return STM32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val) {
	return STM32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val) {
	return STM32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key,
		const char *str) {
	return STM32Config::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key,
		const char *str, size_t strLen) {
	return STM32Config::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key,
		const uint8_t *data, size_t dataLen) {
	return STM32Config::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void) {
	STM32Config::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg) {
    osDelay(100);
    ChipLogProgress(DeviceLayer, "Performing factory reset");
    NVM_ResetFactory(); // Include system restarting
}

ConfigurationManager& ConfigurationMgrImpl() {
	return ConfigurationManagerImpl::GetDefaultInstance();
}

CHIP_ERROR ConfigurationManagerImpl::GetCountryCode(char *buf, size_t bufSize,
		size_t &codeLen) {
	return STM32Config::ReadConfigValueStr(STM32Config::kConfigKey_CountryCode,
			buf, bufSize, codeLen);
}

} // namespace DeviceLayer
} // namespace chip
