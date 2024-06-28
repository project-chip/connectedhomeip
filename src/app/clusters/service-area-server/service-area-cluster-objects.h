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
 * This class is used to wrap the LocationStruct object and provide a more user-friendly interface for the data.
 * It provides a way to store the location name in a buffer, and provides a way to compare the location name with a given string.
 */
struct LocationStructureWrapper : public chip::app::Clusters::ServiceArea::Structs::LocationStruct::Type
{
    LocationStructureWrapper()
    {
        Set(0, 0, CharSpan(), DataModel::Nullable<int16_t>(), DataModel::Nullable<AreaTypeTag>(),
            DataModel::Nullable<LandmarkTag>(), DataModel::Nullable<PositionTag>(), DataModel::Nullable<FloorSurfaceTag>());
    }

    /**
     * @brief This is a full constructor that initializes the location object with the given values. All values are deep copied.
     * @param[in] aLocationId The unique identifier of this location.
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
    LocationStructureWrapper(uint32_t aLocationId, const DataModel::Nullable<uint8_t> & aMapId, const CharSpan & aLocationName,
                             const DataModel::Nullable<int16_t> & aFloorNumber,
                             const DataModel::Nullable<AreaTypeTag> & aAreaTypeTag,
                             const DataModel::Nullable<LandmarkTag> & aLandmarkTag,
                             const DataModel::Nullable<PositionTag> & aPositionTag,
                             const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag)
    {
        Set(aLocationId, aMapId, aLocationName, aFloorNumber, aAreaTypeTag, aLandmarkTag, aPositionTag, aSurfaceTag);
    }

    /**
     * @brief This is a copy constructor that initializes the location object with the values from another location object. All
     * values are deep copied.
     * @param[in] aOther The location object to copy.
     *
     * @note If the locationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    LocationStructureWrapper(const LocationStructureWrapper & aOther) { *this = aOther; }

    /**
     * @brief This is an assignment operator that initializes the location object with the values from another location object. All
     * values are deep copied.
     * @param[in] aOther The location object to copy.
     *
     * @note If the locationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    LocationStructureWrapper & operator=(const LocationStructureWrapper & aOther)
    {
        if (aOther.locationInfo.locationInfo.IsNull())
        {
            Set(aOther.locationID, aOther.mapID, CharSpan(), NullOptional, NullOptional, aOther.locationInfo.landmarkTag,
                aOther.locationInfo.positionTag, aOther.locationInfo.surfaceTag);
        }
        else
        {
            Set(aOther.locationID, aOther.mapID, aOther.locationInfo.locationInfo.Value().locationName,
                aOther.locationInfo.locationInfo.Value().floorNumber, aOther.locationInfo.locationInfo.Value().areaType,
                aOther.locationInfo.landmarkTag, aOther.locationInfo.positionTag, aOther.locationInfo.surfaceTag);
        }

        return *this;
    }

    /**
     * @brief Set all fields of the location object. All values are deep copied.
     * @param[in] aLocationId The unique identifier of this location.
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
    void Set(uint32_t aLocationId, const DataModel::Nullable<uint8_t> & aMapId, const CharSpan & aLocationName,
             const DataModel::Nullable<int16_t> & aFloorNumber, const DataModel::Nullable<AreaTypeTag> & aAreaType,
             const DataModel::Nullable<LandmarkTag> & aLandmarkTag, const DataModel::Nullable<PositionTag> & aPositionTag,
             const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag)
    {
        locationID = aLocationId;
        mapID      = aMapId;

        // If there is at least one non-null value for locationInfo, add it to the location structure.
        if ((!aLocationName.empty()) || (!aFloorNumber.IsNull()) || (!aAreaType.IsNull()))
        {
            // Create a home location info structure and fill it in except for the location name. This is done below.
            locationInfo.locationInfo.SetNonNull(Structs::HomeLocationStruct::Type());

            locationInfo.locationInfo.Value().floorNumber = aFloorNumber;
            locationInfo.locationInfo.Value().areaType    = aAreaType;
        }
        else
        {
            locationInfo.locationInfo.SetNull();
        }

        locationInfo.landmarkTag = aLandmarkTag;
        locationInfo.positionTag = aPositionTag;
        locationInfo.surfaceTag  = aSurfaceTag;

        // Save the location name to buffer, so access and lifetime is controlled.
        // If aLocationName is not null, create a CharSpan to reference the text.
        // Make sure there is always a terminating character, mainly so we can get c_str for logging.
        memset(mLocationNameBuffer, 0, sizeof(mLocationNameBuffer));

        // this assumes locationInfo structure was created above, if appropriate
        if (!locationInfo.locationInfo.IsNull())
        {
            if (aLocationName.empty())
            {
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, 0);
                mLocationNameBuffer[0]                         = '\0';
            }
            else if (aLocationName.size() > sizeof(mLocationNameBuffer))
            {
                // Save the truncated name that fits into available size.
                memcpy(mLocationNameBuffer, aLocationName.data(), sizeof(mLocationNameBuffer));
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, sizeof(mLocationNameBuffer));
                mLocationNameBuffer[kLocationNameMaxSize]      = '\0';
            }
            else
            {
                // Save full name.
                memcpy(mLocationNameBuffer, aLocationName.data(), aLocationName.size());
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, aLocationName.size());
                mLocationNameBuffer[aLocationName.size()]      = '\0';
            }
        }
    }

    /**
     * @brief Compare the location's name with the given text.
     * @param[in] aLocationName The name to compare.
     * @return true if the location structure's name field matches aLocationName.
     *
     * @note if locations structure's name field is null, returns false.
     */
    bool IsNameEqual(const CharSpan & aLocationName) const
    {
        if (!locationInfo.locationInfo.IsNull())
        {
            return locationInfo.locationInfo.Value().locationName.data_equal(aLocationName);
        }

        return false; // name is Null or text sizes do not match
    }

    /**
     * This is use for configuring the IsEqual method.
     * If kIgnoreLocationId is set, the location IDs are ignored when checking for equality.
     * If kIgnoreMapId is set, the map IDs are ignored when checking for equality.
     */
    enum class IsEqualConfig : uint8_t
    {
        kIgnoreLocationId = 0x1,
        kIgnoreMapId      = 0x2,
    };

    /**
     * @brief Checks if the given LocationStructureWrapper is equal to this one.
     * @param aOther The location to compare with.
     * @param aConfig Set if the location IDs and/or the map IDs should be ignored when checking for equality.
     * @return True if both locations are equal. False otherwise.
     */
    bool IsEqual(const LocationStructureWrapper & aOther, BitMask<IsEqualConfig> aConfig) const
    {
        if (!aConfig.Has(IsEqualConfig::kIgnoreLocationId) && (locationID != aOther.locationID))
        {
            return false;
        }

        if (!aConfig.Has(IsEqualConfig::kIgnoreMapId) && (mapID != aOther.mapID))
        {
            return false;
        }

        if (locationInfo.locationInfo.IsNull() != aOther.locationInfo.locationInfo.IsNull())
        {
            return false;
        }

        if (!locationInfo.locationInfo.IsNull())
        {

            if (!IsNameEqual(aOther.locationInfo.locationInfo.Value().locationName))
            {
                return false;
            }

            if (locationInfo.locationInfo.Value().floorNumber != aOther.locationInfo.locationInfo.Value().floorNumber)
            {
                return false;
            }

            if (locationInfo.locationInfo.Value().areaType != aOther.locationInfo.locationInfo.Value().areaType)
            {
                return false;
            }
        }

        if (locationInfo.landmarkTag != aOther.locationInfo.landmarkTag)
        {
            return false;
        }

        if (locationInfo.positionTag != aOther.locationInfo.positionTag)
        {
            return false;
        }

        if (locationInfo.surfaceTag != aOther.locationInfo.surfaceTag)
        {
            return false;
        }

        return true;
    }

    /**
     * @return The location name as a c style char string.
     *
     * @note This is only available through the LocationStructure object (no access from base structure).
     */
    const char * GetNameAsCString() const
    {
        // note: if name is null, this will return an empty terminated c string - ""
        return mLocationNameBuffer;
    }

private:
    // The extra char is to allow for the null terminator \0 (for GetNameAsCString()).
    char mLocationNameBuffer[kLocationNameMaxSize + 1];
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

        // Save the map name to the buffer so that access and lifetime is controlled.
        // Create a CharSpan reference the text.
        // Make sure there is always a terminating character, mainly so we can get c_str for logging.
        memset(mMapNameBuffer, 0, sizeof(mMapNameBuffer));

        if (aMapName.empty())
        {
            name              = CharSpan(mMapNameBuffer, 0);
            mMapNameBuffer[0] = '\0';
        }
        else if (aMapName.size() > sizeof(mMapNameBuffer))
        {
            // Save the truncated name that fits into available size.
            memcpy(mMapNameBuffer, aMapName.data(), sizeof(mMapNameBuffer));
            name                            = CharSpan(mMapNameBuffer, sizeof(mMapNameBuffer));
            mMapNameBuffer[kMapNameMaxSize] = '\0';
        }
        else
        {
            // Save full name.
            memcpy(mMapNameBuffer, aMapName.data(), aMapName.size());
            name                            = CharSpan(mMapNameBuffer, aMapName.size());
            mMapNameBuffer[aMapName.size()] = '\0';
        }
    }

    /**
     * @brief Compare the map's name with given text.
     * @param[in] aMapName The name to compare.
     * @return true if the map structure's name field matches aMapName.
     */
    bool IsNameEqual(const CharSpan & aMapName) const { return name.data_equal(aMapName); }

    /**
     * @return The map name as a c style char string.
     *
     * @note This is only available through the MapStructure object (no access from base structure).
     */
    const char * GetNameAsCString() const { return mMapNameBuffer; }

private:
    // The extra char is to allow for the null terminator \0 (for GetNameAsCString()).
    char mMapNameBuffer[kMapNameMaxSize + 1];
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
