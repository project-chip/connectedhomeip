/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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
#pragma once

#include <app/clusters/basic-information/BasicInformationOptionalAttributes.h>
#include <cstddef>
#include <cstdint>
#include <lib/support/Span.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <app/persistence/AttributePersistence.h>
#include <app/AttributeValueEncoder.h>

namespace chip::app::Clusters {

constexpr size_t kMaxDeviceLocationTLVSize = 160;

using LocationDescriptorStructType = chip::app::Clusters::Globals::Structs::LocationDescriptorStruct::Type;

class DeviceLayerBasicInformationPolicy
{
public:
    // Base class for the cluster to inherit from, to satisfy PlatformManagerDelegate requirements
    using LifetimeDelegate = DeviceLayer::PlatformManagerDelegate;

    DeviceLayerBasicInformationPolicy(BasicInformationOptionalAttributesSet optionalAttributes,
                                      DeviceLayer::DeviceInstanceInfoProvider & deviceInstanceInfoProvider,
                                      DeviceLayer::ConfigurationManager & configurationManager,
                                      DeviceLayer::PlatformManager & platformManager, uint16_t subscriptionsPerFabric) :
        mOptionalAttributes(optionalAttributes),
        mDeviceInstanceInfoProvider(deviceInstanceInfoProvider), mConfigurationManager(configurationManager),
        mPlatformManager(platformManager), mSubscriptionsPerFabric(subscriptionsPerFabric)
    {
        // UniqueID is mandatory as of spec revision 4. We force it on here regardless
        // of what optionalAttributeSet says, to prevent accidental non-certifiable configs.
        mOptionalAttributes.template Set<BasicInformation::Attributes::UniqueID::Id>();

        // Use null for default, but note that if a device location was previously saved
        // to persistent storage that value will be loaded by the start up code. 
        if (mOptionalAttributes.IsSet(BasicInformation::Attributes::DeviceLocation::Id))
        {
            mDeviceLocation.emplace(DataModel::Nullable<OwnedDeviceLocation>(DataModel::NullNullable));
        }        
    }

    const BasicInformationOptionalAttributesSet & GetOptionalAttributes() const { return mOptionalAttributes; }
    BasicInformationOptionalAttributesSet & GetOptionalAttributes() { return mOptionalAttributes; }

    // Delegated methods
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) { return mDeviceInstanceInfoProvider.GetVendorName(buf, bufSize); }

    CHIP_ERROR GetVendorId(uint16_t & vendorId) const { return mDeviceInstanceInfoProvider.GetVendorId(vendorId); }

    CHIP_ERROR GetProductName(char * buf, size_t bufSize) { return mDeviceInstanceInfoProvider.GetProductName(buf, bufSize); }

    CHIP_ERROR GetProductId(uint16_t & productId) const { return mDeviceInstanceInfoProvider.GetProductId(productId); }

    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize)
            {
        return IgnoreUnimplemented(mDeviceInstanceInfoProvider.GetPartNumber(buf, bufSize), buf, bufSize);
            }

    CHIP_ERROR GetProductURL(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mDeviceInstanceInfoProvider.GetProductURL(buf, bufSize), buf, bufSize);
    }

    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mDeviceInstanceInfoProvider.GetProductLabel(buf, bufSize), buf, bufSize);
            }

    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mDeviceInstanceInfoProvider.GetSerialNumber(buf, bufSize), buf, bufSize);
    }

    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
    {
        return mDeviceInstanceInfoProvider.GetManufacturingDate(year, month, day);
    }

    CHIP_ERROR GetManufacturingDateSuffix(MutableCharSpan & suffixBuffer)
        {
        return mDeviceInstanceInfoProvider.GetManufacturingDateSuffix(suffixBuffer);
    }

    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion)
    {
        return mDeviceInstanceInfoProvider.GetHardwareVersion(hardwareVersion);
    }

    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize)
    {
        return mDeviceInstanceInfoProvider.GetHardwareVersionString(buf, bufSize);
    }

    CHIP_ERROR GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
        {
        return mDeviceInstanceInfoProvider.GetProductFinish(finish);
        }

    CHIP_ERROR GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
        {
        return mDeviceInstanceInfoProvider.GetProductPrimaryColor(primaryColor);
        }

    CHIP_ERROR GetLocalConfigDisabled(bool & localConfigDisabled)
    {
        return mDeviceInstanceInfoProvider.GetLocalConfigDisabled(localConfigDisabled);
    }

    CHIP_ERROR SetLocalConfigDisabled(bool localConfigDisabled)
    {
        return mDeviceInstanceInfoProvider.SetLocalConfigDisabled(localConfigDisabled);
    }

    DeviceLayer::DeviceInstanceInfoProvider::DeviceInfoCapabilityMinimas GetSupportedCapabilityMinimaValues()
    {
        return mDeviceInstanceInfoProvider.GetSupportedCapabilityMinimaValues();
    }

    // ConfigurationManager delegated methods
    CHIP_ERROR GetSoftwareVersion(uint32_t & softwareVersion) { return mConfigurationManager.GetSoftwareVersion(softwareVersion); }

    CHIP_ERROR GetSoftwareVersionString(char * buf, size_t bufSize)
    {
        return mConfigurationManager.GetSoftwareVersionString(buf, bufSize);
    }

    CHIP_ERROR GetUniqueId(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mConfigurationManager.GetUniqueId(buf, bufSize), buf, bufSize);
        }

    CHIP_ERROR GetConfigurationVersion(uint32_t & configurationVersion)
    {
        return mConfigurationManager.GetConfigurationVersion(configurationVersion);
    }

    CHIP_ERROR StoreConfigurationVersion(uint32_t configurationVersion)
    {
        return mConfigurationManager.StoreConfigurationVersion(configurationVersion);
    }

    CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)
    {
        return mConfigurationManager.GetCountryCode(buf, bufSize, codeLen);
    }

    CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen) { return mConfigurationManager.StoreCountryCode(code, codeLen); }

    // PlatformManagerDelegate logic
    void RegisterPlatformDelegate(DeviceLayer::PlatformManagerDelegate * delegate)
    {
        if (mPlatformManager.GetDelegate() == nullptr)
        {
            mPlatformManager.SetDelegate(delegate);
        }
    }

    void UnregisterPlatformDelegate(DeviceLayer::PlatformManagerDelegate * delegate)
    {
        if (mPlatformManager.GetDelegate() == delegate)
        {
            mPlatformManager.SetDelegate(nullptr);
        }
    }

    uint16_t GetSubscriptionsPerFabric() const { return mSubscriptionsPerFabric; }

    // The code for supporting the DeviceLocation attribute was borrowed + adapted 
    // from the BridgedDeviceBasicInformation cluster. Try to keep the two in sync, 
    // if any changes are required.
    // 
    struct OwnedDeviceLocation
    {
        std::string locationName;
        std::optional<int16_t> floorNumber;
        std::optional<Globals::AreaTypeTag> areaType;

        OwnedDeviceLocation() = default;
        OwnedDeviceLocation(const LocationDescriptorStructType & other)
        {
            *this = other;
        }

        LocationDescriptorStructType ToView() const
        {
            return {
                .locationName = { locationName.data(), locationName.size() },
                .floorNumber  = floorNumber.has_value() ? DataModel::MakeNullable(*floorNumber) : DataModel::Nullable<int16_t>(),
                .areaType     = areaType.has_value() ? DataModel::MakeNullable(*areaType) : DataModel::Nullable<Globals::AreaTypeTag>(),
            };       
        }

        // Safe for empty LocationName (CharSpan.data() may be nullptr)
        OwnedDeviceLocation &
        operator=(const LocationDescriptorStructType & value)
        {
            // Special handling since empty char-span will return nullptr for data() and
            // std::string does not like that
            if (value.locationName.empty())
            {
                locationName.clear();
            }
            else
            {
                locationName = std::string{ value.locationName.data(), value.locationName.size() };
            }

            if (value.floorNumber.IsNull())
            {
                floorNumber.reset();
            }
            else
            {
                floorNumber = value.floorNumber.Value();
            }

            if (value.areaType.IsNull())
            {
                areaType.reset();
            }
            else
            {
                areaType = value.areaType.Value();
            }

            return *this;
        }
    };

    /// Attempt to update the current DeviceLocation with the given value and store it to persistent storage.
    ///
    /// @param location The new device location to set.
    /// @param persistence The persistence handler.
    /// @return Status code indicating the result of the operation.
 
    CHIP_ERROR WriteDeviceLocation(
        const DataModel::Nullable<
            LocationDescriptorStructType> & value,
            AttributePersistence & persistence)
    {
        return SetDeviceLocationInternal(value, persistence, PersistenceMode::kPersist).GetUnderlyingError();
    }

    CHIP_ERROR LoadDeviceLocation(AttributePersistence & persistence)
    {
        const ConcreteAttributePath path{
            kRootEndpointId,
            BasicInformation::Id,
            BasicInformation::Attributes::DeviceLocation::Id
        };

        uint8_t buffer[kMaxDeviceLocationTLVSize];
        MutableByteSpan tlvBuffer(buffer);

        DataModel::Nullable<LocationDescriptorStructType> decoded;
        
        if (mDeviceLocation.has_value()){
            CHIP_ERROR err = persistence.LoadTLV(path, decoded, tlvBuffer);

            if (err == CHIP_NO_ERROR)
            {
                // Best effort: SetDeviceLocationInternal is called with kDoNotPersist, so it will not fail due to persistence errors.
                // Other failures (like constraint errors) are not expected here as the value comes from storage.
                LogErrorOnFailure(SetDeviceLocationInternal(decoded, persistence, PersistenceMode::kDoNotPersist).GetUnderlyingError());
            }
            else if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                // Nothing in storage, keep the initial value (from mDeviceLocation).
                // This is a best-effort attempt to keep persisted data in sync with startup value
                // and what is read through `ReadAttribute`.
                //
                // Failure to store here should not cause the cluster to stop initializing.
                LogErrorOnFailure(PersistDeviceLocation(persistence));
                
            }
        }

        return CHIP_NO_ERROR;
    }

    std::optional<DataModel::Nullable<LocationDescriptorStructType>> GetDeviceLocation() const
    {
        if (!mDeviceLocation.has_value())
        {
            return std::nullopt;
        } 
        else if (mDeviceLocation->IsNull()) 
        {
            return DataModel::Nullable<LocationDescriptorStructType>(DataModel::NullNullable);
        } 
        else 
        {
            return DataModel::MakeNullable(mDeviceLocation->Value().ToView());
        }
    }

private:

    enum class PersistenceMode
    {
        kPersist,
        kDoNotPersist,
    };

    bool IsLocationEqual(const LocationDescriptorStructType & a, const LocationDescriptorStructType & b)
    {
        return a.locationName.data_equal(b.locationName) && (a.floorNumber == b.floorNumber) && (a.areaType == b.areaType);
    }

    /// Updates the DeviceLocation attribute value with optional persistence.
    ///
    /// This internal helper validates the new value, persists it to NVM if requested,
    /// and then updates the in-memory state.
    ///
    /// @param location The new device location to set.
    /// @param persistence The persistence handler.
    /// @param mode Whether to persist the new value to NVM.
    /// @return Status code indicating the result of the operation.
    DataModel::ActionReturnStatus
    SetDeviceLocationInternal(const DataModel::Nullable<LocationDescriptorStructType> & location,
                            AttributePersistence & persistence,
                            PersistenceMode mode)
    {
        using chip::Protocols::InteractionModel::Status;

        // The cluster must support this attribute
        VerifyOrReturnError(mDeviceLocation.has_value(), Status::UnsupportedAttribute);

        if (!location.IsNull())
        {
            // Validation: At least one field must be non-null/empty
            VerifyOrReturnError(!location.Value().locationName.empty() || !location.Value().floorNumber.IsNull() ||
                                    !location.Value().areaType.IsNull(),
                                Status::ConstraintError);

            // constraint on location name
            VerifyOrReturnError(location.Value().locationName.size() <= 128, Status::ConstraintError);
        }

        // Check for equality
        if (mDeviceLocation->IsNull())
        {
            if (location.IsNull())
            {
                return Status::Success; // No change
            }
        }
        else
        {
            if (!location.IsNull())
            {
                if (IsLocationEqual(mDeviceLocation->Value().ToView(), location.Value()))
                {
                    return Status::Success; // No change
                }
            }
        }

        auto oldValue = mDeviceLocation;

        if (location.IsNull())
        {
            mDeviceLocation->SetNull();
        }
        else
        {
            mDeviceLocation->SetNonNull(location.Value());
        }

        if (mode == PersistenceMode::kPersist)
        {
            if (auto err = PersistDeviceLocation(persistence); err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Failed to persist DeviceLocation: %" CHIP_ERROR_FORMAT, err.Format());
                // Revert the change
                mDeviceLocation = oldValue;
                return Status::Failure;
            }
        }

        return Status::Success;
    }

    /// Store the current DeviceLocation to persistent storage
    CHIP_ERROR PersistDeviceLocation(AttributePersistence & persistence)
    {
        if (!mOptionalAttributes.IsSet(BasicInformation::Attributes::DeviceLocation::Id))
        {
            return CHIP_NO_ERROR;
        }

        VerifyOrReturnError(mDeviceLocation.has_value(), CHIP_ERROR_INCORRECT_STATE);

        std::optional<DataModel::Nullable<LocationDescriptorStructType>> loc;

        if (mDeviceLocation->IsNull()) 
        { 
            loc = DataModel::Nullable<LocationDescriptorStructType>(DataModel::NullNullable);
        } else {
            loc = DataModel::MakeNullable(mDeviceLocation->Value().ToView());
        }
    
        const ConcreteAttributePath path{
            kRootEndpointId,
            BasicInformation::Id,
            BasicInformation::Attributes::DeviceLocation::Id
        };

        return persistence.StoreTLV<kMaxDeviceLocationTLVSize>(path, *loc);
    }

    CHIP_ERROR IgnoreUnimplemented(CHIP_ERROR status, char * buf, size_t bufSize)
    {
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            if (bufSize > 0)
        {
                buf[0] = 0;
            }
            return CHIP_NO_ERROR;
        }
        return status;
    }

    BasicInformationOptionalAttributesSet mOptionalAttributes;
    DeviceLayer::DeviceInstanceInfoProvider & mDeviceInstanceInfoProvider;
    DeviceLayer::ConfigurationManager & mConfigurationManager;
    DeviceLayer::PlatformManager & mPlatformManager;
    uint16_t mSubscriptionsPerFabric;

    // Nullable backing store
    std::optional<DataModel::Nullable<OwnedDeviceLocation>> mDeviceLocation;
};

} // namespace chip::app::Clusters
