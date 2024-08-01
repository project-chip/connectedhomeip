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
inline constexpr size_t kLocationNameMaxSize = 128u;
inline constexpr size_t kMapNameMaxSize      = 64u;

/**
 * This class is used to wrap the AreaStruct object and provide a more user-friendly interface for the data.
 * It provides a way to store the location name in a buffer, and provides a way to compare the location name with a given string.
 */
struct AreaStructureWrapper : public chip::app::Clusters::ServiceArea::Structs::AreaStruct::Type
{
    AreaStructureWrapper()
    {
        Set(0, 0, CharSpan(), DataModel::Nullable<int16_t>(), DataModel::Nullable<Globals::AreaTypeTag>(),
            DataModel::Nullable<Globals::LandmarkTag>(), DataModel::Nullable<Globals::PositionTag>(),
            DataModel::Nullable<Globals::FloorSurfaceTag>());
    }

    /**
     * @brief This is a full constructor that initializes the location object with the given values. All values are deep copied.
     * @param[in] aAreaID The unique identifier of this location.
     * @param[in] aMapId The identifier of the supported map associated with this location.
     * @param[in] aLocationName A human readable name for this location (empty string if not used).
     * @param[in] aFloorNumber The floor level of this location - use negative values for below ground.
     * @param[in] aAreaTypeTag A common namespace Area tag - indicates an association of the location with an indoor or outdoor area
     * of a home.
     * @param[in] aLandmarkTag A common namespace Landmark tag - indicates an association of the location with a home landmark.
     * @param[in] aPositionTag A common namespace Position tag - indicates the position of the location with respect to the
     * landmark.
     * @param[in] aSurfaceTag A common namespace Floor Surface tag - indicates an association of the location with a surface type.
     *
     * @note Requirements regarding what combinations of fields and values are valid are not checked by this class.
     * @note If aLocationName is larger than kLocationNameMaxSize, it will be truncated.
     * @note If aLocationName is an empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    AreaStructureWrapper(uint32_t aAreaID, const DataModel::Nullable<uint8_t> & aMapId, const CharSpan & aLocationName,
                             const DataModel::Nullable<int16_t> & aFloorNumber,
                             const DataModel::Nullable<Globals::AreaTypeTag> & aAreaTypeTag,
                             const DataModel::Nullable<Globals::LandmarkTag> & aLandmarkTag,
                             const DataModel::Nullable<Globals::PositionTag> & aPositionTag,
                             const DataModel::Nullable<Globals::FloorSurfaceTag> & aSurfaceTag)
    {
        Set(aAreaID, aMapId, aLocationName, aFloorNumber, aAreaTypeTag, aLandmarkTag, aPositionTag, aSurfaceTag);
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
        if (aOther.areaDesc.locationInfo.IsNull())
        {
            Set(aOther.areaID, aOther.mapID, CharSpan(), NullOptional, NullOptional, aOther.areaDesc.landmarkTag,
                aOther.areaDesc.positionTag, aOther.areaDesc.surfaceTag);
        }
        else
        {
            Set(aOther.areaID, aOther.mapID, aOther.areaDesc.locationInfo.Value().locationName,
                aOther.areaDesc.locationInfo.Value().floorNumber, aOther.areaDesc.locationInfo.Value().areaType,
                aOther.areaDesc.landmarkTag, aOther.areaDesc.positionTag, aOther.areaDesc.surfaceTag);
        }

        return *this;
    }

    /**
     * @brief Set all fields of the location object. All values are deep copied.
     * @param[in] aAreaID The unique identifier of this location.
     * @param[in] aMapId The identifier of the supported map associated with this location.
     * @param[in] aLocationName A human readable name for this location (empty string if not used).
     * @param[in] aFloorNumber The floor level of this location - use negative values for below ground.
     * @param[in] aAreaTypeTag A common namespace Area tag - indicates an association of the location with an indoor or outdoor area
     * of a home.
     * @param[in] aLandmarkTag A common namespace Landmark tag - indicates an association of the location with a home landmark.
     * @param[in] aPositionTag A common namespace Position tag - indicates the position of the location with respect to the
     * landmark.
     * @param[in] aSurfaceTag A common namespace Floor Surface tag - indicates an association of the location with a surface type.
     *
     * @note Requirements regarding what combinations of fields and values are valid are not checked by this class.
     * @note If aLocationName is larger than kLocationNameMaxSize, it will be truncated.
     * @note If aLocationName is an empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    void Set(uint32_t aAreaID, const DataModel::Nullable<uint8_t> & aMapId, const CharSpan & aLocationName,
             const DataModel::Nullable<int16_t> & aFloorNumber, const DataModel::Nullable<Globals::AreaTypeTag> & aAreaType,
             const DataModel::Nullable<Globals::LandmarkTag> & aLandmarkTag,
             const DataModel::Nullable<Globals::PositionTag> & aPositionTag,
             const DataModel::Nullable<Globals::FloorSurfaceTag> & aSurfaceTag)
    {
        areaID = aAreaID;
        mapID  = aMapId;

        // If there is at least one non-null value for locationInfo, add it to the location structure.
        if ((!aLocationName.empty()) || (!aFloorNumber.IsNull()) || (!aAreaType.IsNull()))
        {
            // Create a home location desc structure and fill it in except for the location name. This is done below.
            areaDesc.locationInfo.SetNonNull(Globals::Structs::LocationDescriptorStruct::Type());

            areaDesc.locationInfo.Value().floorNumber = aFloorNumber;
            areaDesc.locationInfo.Value().areaType    = aAreaType;
        }
        else
        {
            areaDesc.locationInfo.SetNull();
        }

        areaDesc.landmarkTag = aLandmarkTag;
        areaDesc.positionTag = aPositionTag;
        areaDesc.surfaceTag  = aSurfaceTag;

        // this assumes areaDesc structure was created above, if appropriate
        if (!areaDesc.locationInfo.IsNull())
        {
            if (aLocationName.empty())
            {
                areaDesc.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, 0);
            }
            else if (aLocationName.size() > sizeof(mLocationNameBuffer))
            {
                // Save the truncated name that fits into available size.
                memcpy(mLocationNameBuffer, aLocationName.data(), sizeof(mLocationNameBuffer));
                areaDesc.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, sizeof(mLocationNameBuffer));
            }
            else
            {
                // Save full name.
                memcpy(mLocationNameBuffer, aLocationName.data(), aLocationName.size());
                areaDesc.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, aLocationName.size());
            }
        }
    }

    /**
     * @brief Compare the location's name with the given text.
     * @param[in] aLocationName The name to compare.
     * @return true if the location structure's name field matches aLocationName.
     * False otherwise, including if the location structure's HomeLocation structure is null.
     */
    bool IsNameEqual(const CharSpan & aLocationName) const
    {
        if (!areaDesc.locationInfo.IsNull())
        {
            return areaDesc.locationInfo.Value().locationName.data_equal(aLocationName);
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
        kIgnoreMapId      = 0x2,
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

        if (areaDesc.landmarkTag != aOther.areaDesc.landmarkTag)
        {
            return false;
        }

        if (areaDesc.positionTag != aOther.areaDesc.positionTag)
        {
            return false;
        }

        if (areaDesc.surfaceTag != aOther.areaDesc.surfaceTag)
        {
            return false;
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
            return { mLocationNameBuffer, 0 };
        }

        return areaDesc.locationInfo.Value().locationName;
    }

private:
    char mLocationNameBuffer[kLocationNameMaxSize] = { 0 };
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
    MapStructureWrapper(uint8_t aMapId, const CharSpan & aMapName) { Set(aMapId, aMapName); }

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
    void Set(uint8_t aMapId, const CharSpan & aMapName)
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
