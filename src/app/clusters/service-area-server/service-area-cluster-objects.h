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
        SetLocationID(0);
        SetMapID(DataModel::Nullable<uint8_t>());
        SetLandmarkTag(DataModel::Nullable<LandmarkTag>());
        SetPositionTag(DataModel::Nullable<PositionTag>());
        SetSurfaceTag(DataModel::Nullable<FloorSurfaceTag>());

        // 'true' forces the 'canonical' form specified by the cluster requirements
        // where the LocationDescriptorStruct is null if all it's elements are/would be empty or null.
        SetLocationName(CharSpan(mLocationNameBuffer, 0), true);
        SetFloorNumber(DataModel::Nullable<int16_t>(), true);
        SetAreaType(DataModel::Nullable<AreaTypeTag>(), true);
    }

    /**
     * @brief This is a full constructor that initializes the location object with the given values. All values are deep copied.
     * @param[in] aLocationId The unique identifier of this location.
     * @param[in] aMapId The identifier of the supported map associated with this location.
     * @param[in] aLocationName A human readable name for this location (empty string if not used).
     * @param[in] aFloorNumber The floor level of this location - use negative values for below ground.
     * @param[in] aAreaType A common namespace Area tag - indicates an association of the location with an indoor or outdoor area
     * of a home.
     * @param[in] aLandmarkTag A common namespace Landmark tag - indicates an association of the location with a home landmark.
     * @param[in] aPositionTag A common namespace Position tag - indicates the position of the location with respect to the
     * landmark.
     * @param[in] aSurfaceTag A common namespace Floor Surface tag - indicates an association of the location with a surface type.
     *
     * @note Requirements regarding what combinations of fields and values are valid are not checked by this class.
     * @note If aLocationName is larger than kLocationNameMaxSize, it will be truncated.
     * @note If aLocationName is an empty string and aFloorNumber and aAreaType are null, locationInfo.locationinfo will be set to null.
     */
    LocationStructureWrapper(uint32_t aLocationId, 
                             const DataModel::Nullable<uint8_t> & aMapId, 
                             const CharSpan & aLocationName,
                             const DataModel::Nullable<int16_t> & aFloorNumber,
                             const DataModel::Nullable<AreaTypeTag> & aAreaType,
                             const DataModel::Nullable<LandmarkTag> & aLandmarkTag,
                             const DataModel::Nullable<PositionTag> & aPositionTag,
                             const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag)
    {
        SetLocationID(aLocationId);
        SetMapID(aMapId);
        SetLandmarkTag(aLandmarkTag);
        SetPositionTag(aPositionTag);
        SetSurfaceTag(aSurfaceTag);

        // 'true' forces the 'canonical' form specified by the cluster requirements
        // where the LocationDescriptorStruct is null if all it's elements are/would be empty or null.
        SetLocationName(aLocationName, true);
        SetFloorNumber(aFloorNumber, true);
        SetAreaType(aAreaType, true );
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
     * @note This will do an EXACT copy of the other object. 
     */
    LocationStructureWrapper & operator=(const LocationStructureWrapper & aOther)
    {
        SetLocationID(aOther.locationID);
        SetMapID(aOther.mapID);
        SetLandmarkTag(aOther.locationInfo.landmarkTag);
        SetPositionTag(aOther.locationInfo.positionTag);
        SetSurfaceTag(aOther.locationInfo.surfaceTag);

        if (!aOther.locationInfo.locationInfo.IsNull())
        {
            // 'true' forces the 'canonical' form specified by the cluster requirements
            // where the LocationDescriptorStruct is null if all it's elements are/would be empty or null.
            // We want an exact copy for "=", so that option is disabled here.
            SetLocationName(aOther.locationInfo.locationInfo.Value().locationName, false);
            SetFloorNumber(aOther.locationInfo.locationInfo.Value().floorNumber, false);
            SetAreaType(aOther.locationInfo.locationInfo.Value().areaType, false);
        }

        return *this;
    }

    /**
     * @brief setter for locationID
     * @param[in] aLocationId The unique identifier of this location.
     */
    void SetLocationID(uint32_t aLocationId)
    {
        locationID = aLocationId;
    }

    /**
     * @brief setter for mapID
     * @param[in] aMapId The identifier of the supported map associated with this location.
     */
    void SetMapID(const DataModel::Nullable<uint8_t> & aMapId)
    {
        mapID = aMapId;
    }

    /**
     * @brief setter for location name
     * @param[in] aLocationName A human readable name for this location (empty string if not used).
     * @param[in] doInfoStructCleanup If true, locationInfo.locationInfo will be set to null if
     *            aLocationName is an empty string and floorNumber and areaType are null.
     *            This generates the 'canonical' form specified by the cluster requirements.
     * @note If aLocationName is larger than kLocationNameMaxSize, it will be truncated.
     */
    void SetLocationName(const CharSpan & aLocationName, bool doInfoStructCleanup)
    {
        // does the locationInfo structure need to be added?
        if ((!aLocationName.empty()) && 
            (locationInfo.locationInfo.IsNull()))
        {
            // Create a location info structure
            locationInfo.locationInfo.SetNonNull(Structs::LocationDescriptorStruct::Type());
        }

        // does the locationInfo structure need to be removed and set to NULL?
        if ((doInfoStructCleanup) &&
            (aLocationName.empty()) && 
            (!locationInfo.locationInfo.IsNull()) && 
            (locationInfo.locationInfo.Value().floorNumber.IsNull()) &&
            (locationInfo.locationInfo.Value().areaType.IsNull()))
        {
            locationInfo.locationInfo.SetNull();
        }

        // if locationInfo.locationInfo structure exists update the location name (non-existance implies location name is empty)
        if (!locationInfo.locationInfo.IsNull())
        {
            if (aLocationName.empty())
            {
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, 0);
            }
            else if (aLocationName.size() > sizeof(mLocationNameBuffer))
            {
                // Save the truncated name that fits into available size.
                memcpy(mLocationNameBuffer, aLocationName.data(), sizeof(mLocationNameBuffer));
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, sizeof(mLocationNameBuffer));
            }
            else
            {
                // Save full name.
                memcpy(mLocationNameBuffer, aLocationName.data(), aLocationName.size());
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, aLocationName.size());
            }
        }
    }

    /**
     * @brief setter for floor number
     * @param[in] aFloorNumber The floor level of this location - use negative values for below ground.
     * @param[in] doInfoStructCleanup If true, locationInfo.locationInfo will be set to null if
     *            aFloorNumber is null, locationName is an empty string and areaType is null.
     *            This generates the 'canonical' form specified by the cluster requirements.
     */
    void SetFloorNumber(const DataModel::Nullable<int16_t> & aFloorNumber, bool doInfoStructCleanup)
    {
        // does the locationInfo structure need to be added?
        if ((!aFloorNumber.IsNull()) && 
            (locationInfo.locationInfo.IsNull()))
        {
            // Create a location info structure
            locationInfo.locationInfo.SetNonNull(Structs::LocationDescriptorStruct::Type());
        }

        // does the locationInfo structure need to be removed and set to NULL?
        if ((doInfoStructCleanup) &&
            (aFloorNumber.IsNull()) &&
            (!locationInfo.locationInfo.IsNull()) && 
            (locationInfo.locationInfo.Value().locationName.size() == 0) && 
            (locationInfo.locationInfo.Value().areaType.IsNull()))
        {
            locationInfo.locationInfo.SetNull();
        }

        // if locationInfo structure exists update the floor number (non-existance implies floor number is null)
        if (!locationInfo.locationInfo.IsNull())
        {
            locationInfo.locationInfo.Value().floorNumber = aFloorNumber;
        }
    }

    /**
     * @brief setter for area type
     * @param[in] aAreaType common namespace Area tag - indicates an association of the location with an indoor or outdoor area
     * of a home.
     * @param[in] doInfoStructCleanup If true, locationInfo.locationInfo will be set to null if
     *            aAreaType and floorNumber are null, and locationName is an empty string
     *            This generates the 'canonical' form specified by the cluster requirements.
     */
    void SetAreaType(const DataModel::Nullable<AreaTypeTag> & aAreaType, bool doInfoStructCleanup)
    {
        // does the locationInfo structure need to be added?
        if ((!aAreaType.IsNull()) && 
            (locationInfo.locationInfo.IsNull()))
        {
            // Create a location info structure
            locationInfo.locationInfo.SetNonNull(Structs::LocationDescriptorStruct::Type());
        }

        // does the locationInfo structure need to be removed and set to NULL?
        if ((doInfoStructCleanup) &&
            (aAreaType.IsNull()) &&
            (!locationInfo.locationInfo.IsNull()) && 
            (locationInfo.locationInfo.Value().locationName.size() == 0) && 
            (locationInfo.locationInfo.Value().areaType.IsNull()))
        {
            locationInfo.locationInfo.SetNull();
        }

        // if locationInfo structure exists update the area type (non-existance implies area type is null)
        if (!locationInfo.locationInfo.IsNull())
        {
            locationInfo.locationInfo.Value().areaType = aAreaType;
        }
    }

    /**
     * @brief setter for landmark tag
     * @param[in] aLandmarkTag A common namespace Landmark tag - indicates an association of the location with a home landmark.
     */
    void SetLandmarkTag(const DataModel::Nullable<LandmarkTag> & aLandmarkTag)
    {
        locationInfo.landmarkTag = aLandmarkTag;
    }

    /**
     * @brief setter for position tag
     * @param[in] aPositionTag A common namespace Position tag - indicates the position of the location with respect to the
     * landmark.
     */
    void SetPositionTag(const DataModel::Nullable<PositionTag> & aPositionTag)
    {
        locationInfo.positionTag = aPositionTag;
    }


    /**
     * @brief setter for surface tag
     * @param[in] aSurfaceTag A common namespace Floor Surface tag - indicates an association of the location with a surface type.
     */
    void SetSurfaceTag(const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag)
    {
        locationInfo.surfaceTag  = aSurfaceTag;
    }

    /**
     * @brief Compare the location's name with the given text.
     * @param[in] aLocationName The name to compare.
     * @return true if the location structure's name field matches aLocationName.
     * False otherwise, including if the location structure's HomeLocation structure is null.
     */
    bool IsNameEqual(const CharSpan & aLocationName) const
    {
        if (!locationInfo.locationInfo.IsNull())
        {
            return locationInfo.locationInfo.Value().locationName.data_equal(aLocationName);
        }

        return false;
    }

    /**
     * This is used for configuring the IsEqual method.
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
     * @return The location name.
     */
    CharSpan GetName()
    {
        if (locationInfo.locationInfo.IsNull())
        {
            return { mLocationNameBuffer, 0 };
        }

        return locationInfo.locationInfo.Value().locationName;
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
