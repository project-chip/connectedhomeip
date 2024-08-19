/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

// These limits are defined in the spec.
inline constexpr size_t kAreaNameMaxSize = 128u;
inline constexpr size_t kMapNameMaxSize  = 64u;

/**
 * This class is used to wrap the AreaStruct object and provide a more user-friendly interface for the data.
 * It provides a way to store the location name in a buffer, and provides a way to compare the location name with a given string.
 */
struct AreaStructureWrapper : public chip::app::Clusters::ServiceArea::Structs::AreaStruct::Type
{
    /**
     * @brief This is a default constructor that initializes the location object with the following
     * values: areaID = 0, mapID = null, locationInfo = null, landmarkInfo = null.
     */
    AreaStructureWrapper()
    {
        areaID = 0;
        mapID  = DataModel::NullNullable;
        SetLocationInfoNull();
        SetLandmarkInfoNull();
    }

    /**
     * @brief This is a copy constructor that initializes the location object with the values from another location object. All
     * values are deep copied.
     * @param[in] aOther The location object to copy.
     *
     * @note If the locationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    AreaStructureWrapper(const AreaStructureWrapper & aOther) { *this = aOther; }

    /**
     * @brief This is an assignment operator that initializes the location object with the values from another location object. All
     * values are deep copied.
     * @param[in] aOther The location object to copy.
     *
     * @note If the locationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    AreaStructureWrapper & operator=(const AreaStructureWrapper & aOther)
    {
        areaID = aOther.areaID;
        mapID  = aOther.mapID;
        SetLocationInfo(aOther.areaDesc.locationInfo);
        SetLandmarkInfo(aOther.areaDesc.landmarkInfo);

        return *this;
    }

    bool operator==(const AreaStructureWrapper & aOther) const
    {
        BitMask<IsEqualConfig> config = 0; // Do not ignore the AreaID or the MapID.

        return IsEqual(aOther, config);
    }

    AreaStructureWrapper & SetAreaId(uint32_t aAreaID)
    {
        areaID = aAreaID;
        return *this;
    }

    AreaStructureWrapper & SetMapId(const DataModel::Nullable<uint32_t> & aMapId)
    {
        mapID = aMapId;
        return *this;
    }

    AreaStructureWrapper & SetLocationInfoNull()
    {
        areaDesc.locationInfo.SetNull();
        return *this;
    }

    /**
     * @brief Set the location information.
     * @param[in] locationName The name of the location. If the name is larger than kAreaNameMaxSize, it will be truncated.
     * @param[in] floorNumber The floor number of the location.
     * @param[in] areaType The type of the area.
     */
    AreaStructureWrapper & SetLocationInfo(const CharSpan & locationName, const DataModel::Nullable<int16_t> & floorNumber,
                                           const DataModel::Nullable<Globals::AreaTypeTag> & areaType)
    {
        areaDesc.locationInfo.SetNonNull();
        // Copy the name
        auto sizeToCopy = std::min(sizeof(mAreaNameBuffer), locationName.size());
        memcpy(mAreaNameBuffer, locationName.data(), sizeToCopy);
        areaDesc.locationInfo.Value().locationName = CharSpan(mAreaNameBuffer, sizeToCopy);
        areaDesc.locationInfo.Value().floorNumber  = floorNumber;
        areaDesc.locationInfo.Value().areaType     = areaType;

        return *this;
    }

    /**
     * @brief Set the location information form a LocationDescriptorStruct object.
     *
     * @note If the locationName is larger than kAreaNameMaxSize, it will be truncated.
     */
    AreaStructureWrapper & SetLocationInfo(DataModel::Nullable<Globals::Structs::LocationDescriptorStruct::Type> locationInfo)
    {
        if (locationInfo.IsNull())
        {
            return SetLocationInfoNull();
        }

        return SetLocationInfo(locationInfo.Value().locationName, locationInfo.Value().floorNumber, locationInfo.Value().areaType);
    }

    AreaStructureWrapper & SetLandmarkInfoNull()
    {
        areaDesc.landmarkInfo.SetNull();
        return *this;
    }

    /**
     * @brief Set the landmark information.
     * @param[in] landmarkTag The landmark tag.
     * @param[in] relativePositionTag The relative position tag.
     */
    AreaStructureWrapper & SetLandmarkInfo(const Globals::LandmarkTag & landmarkTag,
                                           const DataModel::Nullable<Globals::RelativePositionTag> & relativePositionTag)
    {
        areaDesc.landmarkInfo.SetNonNull();
        areaDesc.landmarkInfo.Value().landmarkTag = landmarkTag;
        areaDesc.landmarkInfo.Value().positionTag = relativePositionTag;

        return *this;
    }

    /**
     * @brief Set the landmark information from a LandmarkInfoStruct object.
     */
    AreaStructureWrapper & SetLandmarkInfo(DataModel::Nullable<Structs::LandmarkInfoStruct::Type> landmarkInfo)
    {
        if (landmarkInfo.IsNull())
        {
            return SetLandmarkInfoNull();
        }

        return SetLandmarkInfo(landmarkInfo.Value().landmarkTag, landmarkInfo.Value().positionTag);
    }

    /**
     * @brief Compare the area's name with the given text.
     * @param[in] aAreaName The name to compare.
     * @return true if the area structure's name field matches aAreaName.
     * False otherwise, including if the location structure's LocationInfo structure is null.
     */
    bool IsNameEqual(const CharSpan & aAreaName) const
    {
        if (!areaDesc.locationInfo.IsNull())
        {
            return areaDesc.locationInfo.Value().locationName.data_equal(aAreaName);
        }

        return false;
    }

    /**
     * This is used for configuring the IsEqual method.
     * If kIgnoreAreaID is set, the area IDs are ignored when checking for equality.
     * If kIgnoreMapId is set, the map IDs are ignored when checking for equality.
     */
    enum class IsEqualConfig : uint8_t
    {
        kIgnoreAreaID = 0x1,
        kIgnoreMapId  = 0x2,
    };

    /**
     * @brief Checks if the given AreaStructureWrapper is equal to this one.
     * @param aOther The location to compare with.
     * @param aConfig Set if the area IDs and/or the map IDs should be ignored when checking for equality.
     * @return True if both locations are equal. False otherwise.
     */
    bool IsEqual(const AreaStructureWrapper & aOther, BitMask<IsEqualConfig> aConfig) const
    {
        if (!aConfig.Has(IsEqualConfig::kIgnoreAreaID) && (areaID != aOther.areaID))
        {
            return false;
        }

        if (!aConfig.Has(IsEqualConfig::kIgnoreMapId) && (mapID != aOther.mapID))
        {
            return false;
        }

        if (areaDesc.locationInfo.IsNull() != aOther.areaDesc.locationInfo.IsNull())
        {
            return false;
        }

        if (!areaDesc.locationInfo.IsNull())
        {

            if (!IsNameEqual(aOther.areaDesc.locationInfo.Value().locationName))
            {
                return false;
            }

            if (areaDesc.locationInfo.Value().floorNumber != aOther.areaDesc.locationInfo.Value().floorNumber)
            {
                return false;
            }

            if (areaDesc.locationInfo.Value().areaType != aOther.areaDesc.locationInfo.Value().areaType)
            {
                return false;
            }
        }

        if (areaDesc.landmarkInfo.IsNull() != aOther.areaDesc.landmarkInfo.IsNull())
        {
            return false;
        }

        if (!areaDesc.landmarkInfo.IsNull())
        {
            if (areaDesc.landmarkInfo.Value().landmarkTag != aOther.areaDesc.landmarkInfo.Value().landmarkTag)
            {
                return false;
            }

            if (areaDesc.landmarkInfo.Value().positionTag != aOther.areaDesc.landmarkInfo.Value().positionTag)
            {
                return false;
            }
        }

        return true;
    }

    /**
     * @return The location name.
     */
    CharSpan GetName()
    {
        if (areaDesc.locationInfo.IsNull())
        {
            return { mAreaNameBuffer, 0 };
        }

        return areaDesc.locationInfo.Value().locationName;
    }

private:
    char mAreaNameBuffer[kAreaNameMaxSize] = { 0 };
};

/**
 * This class wraps the MapStruct object and provides a more user-friendly interface for the data.
 */
struct MapStructureWrapper : public chip::app::Clusters::ServiceArea::Structs::MapStruct::Type
{
    MapStructureWrapper() { Set(0, CharSpan()); }

    /**
     * @brief This is a full constructor that initializes the map object with the given values. All values are deep copied.
     * @param[in] aMapId The identifier of this map.
     * @param[in] aMapName A human readable name (should not be empty string).
     *
     * @note Requirements regarding what combinations of fields and values are 'valid' are not checked by this class.
     * @note If aMapName is larger than kMapNameMaxSize, it will be truncated.
     */
    MapStructureWrapper(uint32_t aMapId, const CharSpan & aMapName) { Set(aMapId, aMapName); }

    /**
     * @brief This is a copy constructor that initializes the map object with the values from another map object. All values are
     * deep copied.
     * @param[in] aOther The map object to copy.
     */
    MapStructureWrapper(const MapStructureWrapper & aOther) { *this = aOther; }

    /**
     * @brief This is an assignment operator that initializes the map object with the values from another map object. All values are
     * deep copied.
     * @param[in] aOther The map object to copy.
     */
    MapStructureWrapper & operator=(const MapStructureWrapper & aOther)
    {
        Set(aOther.mapID, aOther.name);
        return *this;
    }

    /**
     * @brief Set all fields of the map object. All values are deep copied.
     * @param[in] aMapId The identifier of this map.
     * @param[in] aMapName A human readable name (should not be empty string).
     *
     * @note Requirements regarding what combinations of fields and values are 'valid' are not checked by this class.
     * @note if aMapName is larger than kMapNameMaxSize, it will be truncated.
     */
    void Set(uint32_t aMapId, const CharSpan & aMapName)
    {
        mapID = aMapId;

        if (aMapName.empty())
        {
            name = CharSpan(mMapNameBuffer, 0);
        }
        else if (aMapName.size() > sizeof(mMapNameBuffer))
        {
            // Save the truncated name that fits into available size.
            memcpy(mMapNameBuffer, aMapName.data(), sizeof(mMapNameBuffer));
            name = CharSpan(mMapNameBuffer, sizeof(mMapNameBuffer));
        }
        else
        {
            // Save full name.
            memcpy(mMapNameBuffer, aMapName.data(), aMapName.size());
            name = CharSpan(mMapNameBuffer, aMapName.size());
        }
    }

    /**
     * @brief Compare the map's name with given text.
     * @param[in] aMapName The name to compare.
     * @return true if the map structure's name field matches aMapName.
     */
    bool IsNameEqual(const CharSpan & aMapName) const { return name.data_equal(aMapName); }

    /**
     * @return The map name.
     */
    CharSpan GetName() const { return name; }

private:
    char mMapNameBuffer[kMapNameMaxSize] = { 0 };
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
