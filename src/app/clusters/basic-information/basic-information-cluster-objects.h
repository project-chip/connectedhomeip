#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/cluster-enums.h>


namespace chip {
namespace app {
namespace Clusters {
namespace BasicInformation {

const uint32_t kMaxDeviceLocationNameLength = 128;
using DeviceLocatioType = Attributes::DeviceLocation::TypeInfo::Type;


struct MutableDeviceLocation : public DeviceLocatioType
{
    MutableDeviceLocation() {
        MutableDeviceLocation(CharSpan(mLocationNameBuffer, 0), DataModel::NullNullable, DataModel::NullNullable);
    }

    MutableDeviceLocation(CharSpan aLocationName, DataModel::Nullable<int16_t> floorNumber, DataModel::Nullable<AreaTypeTag> aAreaTypeTag) {
        Set(aLocationName, floorNumber, aAreaTypeTag);
    }

    MutableDeviceLocation & operator = (const MutableDeviceLocation & other) {
        Set(other->locationName, other->floorNumber, other->areaType);
        return *this;
    }

    MutableDeviceLocation(const MutableDeviceLocation & other)  : Nullable(other) {
        *this = other;
    }

    // todo is this needed?
    MutableDeviceLocation & operator = (const DeviceLocatioType & other) {
        if (other.IsNull())
        {
            this->SetNull();
            return *this;
        }

        Set(other->locationName, other->floorNumber, other->areaType);
        return *this;
    }

    /**
     * @brief Set the location name, floor number, and area type tag. The location name is truncated to kMaxDeviceLocationNameLength. The location name is deep copied.
    */
    void Set(CharSpan aLocationName, DataModel::Nullable<int16_t> aFloorNumber, DataModel::Nullable<AreaTypeTag> aAreaTypeTag) {

        auto nameLength = aLocationName.size();

        if (nameLength > kMaxDeviceLocationNameLength)
        {
            nameLength = kMaxDeviceLocationNameLength;
        }

        memcpy(mLocationNameBuffer, aLocationName.data(), nameLength);

        this->SetNonNull();
        this->Value().locationName = CharSpan(mLocationNameBuffer, nameLength);
        this->Value().floorNumber = aFloorNumber;
        this->Value().areaType = aAreaTypeTag;
    }

    // todo add setters and getters for all the attributes

private:
    char mLocationNameBuffer[kMaxDeviceLocationNameLength] = { 0 };
};


} // namespace BasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip