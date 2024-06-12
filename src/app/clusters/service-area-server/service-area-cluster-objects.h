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

inline constexpr size_t kLocationNameMaxSize    = 128u;
inline constexpr size_t kMapNameMaxSize         = 64u;


/**
 * This class is used to wrap the LocationStruct object and provide a more user-friendly interface for the data.
 * It provides a way to store the location name in a buffer, and provides a way to compare the location name with a given string.
 */
struct LocationStructureWrapper : public chip::app::Clusters::ServiceArea::Structs::LocationStruct::Type
{
    /**
     * @brief This is a default constructor that initializes the location object with default values.
     */
    LocationStructureWrapper() : mLocationNameBuffer()
    {
        Set(0, 0, 
             CharSpan(), DataModel::Nullable<int16_t>(), DataModel::Nullable<AreaTypeTag>(),
             DataModel::Nullable<LandmarkTag>(), DataModel::Nullable<PositionTag>(), DataModel::Nullable<FloorSurfaceTag>());
    }

    /**
     * @brief This is a full constructor that initializes the location object with the given values. All values are deep copied.
     * @param[in] aLocationId The unique identifier of this location.
     * @param[in] aMapId The identifier of the supported map associated with this location.
     * @param[in] aLocationName A human readable name for this location (empty string if not used).
     * @param[in] aFloorNumber The floor level of this location - use negative values for below ground.
     * @param[in] aAreaTypeTag A common namespace Area tag - indicates an association of the location with an indoor or outdoor area of a home.
     * @param[in] aLandmarkTag A common namespace Landmark tag - indicates an association of the location with a home landmark.
     * @param[in] aPositionTag A common namespace Position tag - indicates the position of the location with respect to the landmark.
     * @param[in] aSurfaceTag A common namespace Floor Surface tag - indicates an association of the location with a surface type.
     * 
     * @note Requirements regarding what combinations of fields and values are valid are not checked by this class.
     * @note If aLocationName is larger than kLocationNameMaxSize, it will be truncated.
     * @note If aLocationName is an empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    LocationStructureWrapper( uint32_t                                     aLocationId, 
                              const DataModel::Nullable<uint8_t>         & aMapId, 
                              const CharSpan                             & aLocationName,
                              const DataModel::Nullable<int16_t>         & aFloorNumber,
                              const DataModel::Nullable<AreaTypeTag>     & aAreaTypeTag,
                              const DataModel::Nullable<LandmarkTag>     & aLandmarkTag,
                              const DataModel::Nullable<PositionTag>     & aPositionTag,
                              const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag  ) :
        mLocationNameBuffer()
    {
        Set( aLocationId, aMapId, 
             aLocationName, aFloorNumber, aAreaTypeTag, 
             aLandmarkTag, aPositionTag, aSurfaceTag);
    }

    /**
     * @brief This is a copy constructor that initializes the location object with the values from another location object. All values are deep copied.
     * @param[in] originalObject The location object to copy.
     * 
     * @note If the locationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    LocationStructureWrapper(const LocationStructureWrapper & originalObject) : mLocationNameBuffer() { *this = originalObject; }

    /**
     * @brief This is an assignment operator that initializes the location object with the values from another location object. All values are deep copied.
     * @param[in] aOriginalObject The location object to copy.
     * 
     * @note If the locationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    LocationStructureWrapper & operator=(const LocationStructureWrapper & aOriginalObject)
    {
        if (aOriginalObject.locationInfo.locationInfo.IsNull())
        {
            Set(aOriginalObject.locationID, aOriginalObject.mapID,
                CharSpan(), NullOptional, NullOptional,
                aOriginalObject.locationInfo.landmarkTag, aOriginalObject.locationInfo.positionTag,
                aOriginalObject.locationInfo.surfaceTag);
        }
        else
        {
            Set(aOriginalObject.locationID, aOriginalObject.mapID, aOriginalObject.locationInfo.locationInfo.Value().locationName,
                aOriginalObject.locationInfo.locationInfo.Value().floorNumber,
                aOriginalObject.locationInfo.locationInfo.Value().areaType, aOriginalObject.locationInfo.landmarkTag,
                aOriginalObject.locationInfo.positionTag, aOriginalObject.locationInfo.surfaceTag);
        }

        return *this;
    }

    /**
     * @brief Set all fields of the location object. All values are deep copied.
     * @param[in] aLocationId The unique identifier of this location.
     * @param[in] aMapId The identifier of the supported map associated with this location.
     * @param[in] aLocationName A human readable name for this location (empty string if not used).
     * @param[in] aFloorNumber The floor level of this location - use negative values for below ground.
     * @param[in] aAreaTypeTag A common namespace Area tag - indicates an association of the location with an indoor or outdoor area of a home.
     * @param[in] aLandmarkTag A common namespace Landmark tag - indicates an association of the location with a home landmark.
     * @param[in] aPositionTag A common namespace Position tag - indicates the position of the location with respect to the landmark.
     * @param[in] aSurfaceTag A common namespace Floor Surface tag - indicates an association of the location with a surface type.
     *
     * @note Requirements regarding what combinations of fields and values are valid are not checked by this class.
     * @note If aLocationName is larger than kLocationNameMaxSize, it will be truncated.
     * @note If aLocationName is an empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null.
     */
    void Set( uint32_t                                     aLocationId, 
              const DataModel::Nullable<uint8_t>         & aMapId, 
              const CharSpan                             & aLocationName,
              const DataModel::Nullable<int16_t>         & aFloorNumber,
              const DataModel::Nullable<AreaTypeTag>     & aAreaType,
              const DataModel::Nullable<LandmarkTag>     & aLandmarkTag,
              const DataModel::Nullable<PositionTag>     & aPositionTag,
              const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag  )
    {
        locationID = aLocationId;
        mapID      = aMapId;

        // If there is at least one non-null value for locationInfo, add it to the location structure.
        if ((!aLocationName.empty()) ||
            (!aFloorNumber.IsNull()) ||
            (!aAreaType.IsNull())  )
        {
            // Create a home location info structure and fill it in except for the location name. This is done below.
            locationInfo.locationInfo.SetNonNull(Structs::HomeLocationStruct::Type());

            locationInfo.locationInfo.Value().floorNumber    = aFloorNumber;
            locationInfo.locationInfo.Value().areaType       = aAreaType;
        }
        else
        {
            locationInfo.locationInfo.SetNull();
        }
                                                                         
        locationInfo.landmarkTag   = aLandmarkTag;
        locationInfo.positionTag   = aPositionTag;
        locationInfo.surfaceTag    = aSurfaceTag;

        // Save the location name to buffer, so access and lifetime is controlled.
        // If aLocationName is not null, create a CharSpan to reference the text.
        // Make sure there is always a terminating character, mainly so we can get c_str for logging.
        memset(mLocationNameBuffer, 0, sizeof(mLocationNameBuffer+1));

        // this assumes locationInfo structure was created above, if appropriate
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
     * @brief Compare the location's name with the given text.
     * @param[in] aLocationName The name to compare.
     * @return true if the location structure's name field matches aLocationName.
     * 
     * @note if locations structure's name field is null, returns false.
     */
    bool DoesNameMatch(const CharSpan & aLocationName) const
    {
        if (!locationInfo.locationInfo.IsNull())
        {
            return locationInfo.locationInfo->locationName.data_equal(aLocationName);
        }

        return false;  // name is Null or text sizes do not match
    }

    /**
     * @brief Get the location name (for logging).
     * @return The map name as a c style char string.
     * 
     * @note This is only available through the LocationStructure object (no access from base structure).
     */
    const char* name_c_str() const
    {
        // note: if name is null, this will return an empty terminated c string - ""
        return mLocationNameBuffer;
    }

private:
    // extra char to allow for terminating 0 ( or name_c_str())
    char mLocationNameBuffer[kLocationNameMaxSize +1];
};


/**
 * This class wraps the MapStruct object and provides a more user-friendly interface for the data.
 */
struct MapStructureWrapper : public chip::app::Clusters::ServiceArea::Structs::MapStruct::Type
{
    /**
     * @brief This is a default constructor that initializes the map object with default values.
     */
    MapStructureWrapper() : mMapNameBuffer()
    {
        Set( 0, CharSpan());
    }

    /**
     * @brief This is a full constructor that initializes the map object with the given values. All values are deep copied.
     * @param[in] aMapId The identifier of this map.
     * @param[in] aMapName A human readable name (should not be empty string).
     * 
     * @note Requirements regarding what combinations of fields and values are 'valid' are not checked by this class.
     * @note If aMapName is larger than kMapNameMaxSize, it will be truncated.
     */
    MapStructureWrapper (uint8_t aMapId, const CharSpan & aMapName) : mMapNameBuffer()
    {
        Set(aMapId, aMapName);
    }

    /**
     * @brief This is a copy constructor that initializes the map object with the values from another map object. All values are deep copied.
     * @param[in] originalMap The map object to copy.
     */
    MapStructureWrapper(const MapStructureWrapper & originalMap) { *this = originalMap; }

    /**
     * @brief This is an assignment operator that initializes the map object with the values from another map object. All values are deep copied.
     * @param[in] originalMap The map object to copy.
     */
    MapStructureWrapper & operator=(const MapStructureWrapper & originalMap)
    {
        Set(originalMap.mapID, originalMap.name);
        return *this;
    }

    /**
     * @brief Set all fields of the mop object. All values are deep copied.
     * @param[in] aMapId The identifier of this map.
     * @param[in] aMapName A human readable name (should not be empty string).
     *
     * @note Requirements regarding what combinations of fields and values are 'valid' are not checked by this class.
     * @note if aMapName is larger than kMapNameMaxSize, it will be truncated.
     */
    void Set( uint8_t aMapId, 
              const CharSpan & aMapName)
    {
        mapID = aMapId;

        // Save the map name to the buffer so that access and lifetime is controlled.
        // Create a CharSpan reference the text.
        // Make sure there is always a terminating character, mainly so we can get c_str for logging.
        memset(mMapNameBuffer, 0, sizeof(mMapNameBuffer+1));

        if (aMapName.size() == 0)
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
    bool DoesNameMatch(const CharSpan & aMapName) const
    {
        return name.data_equal(aMapName);
    }

    /**
     * @brief Get the map name (for logging).
     * @return The map name as a c style char string.
     * 
     * @note This is only available through the MapStructure object (no access from base structure).
     */
    const char* name_c_str() const
    {
        return mMapNameBuffer;
    }

private:
    // extra char to allow for terminating 0 (for name_c_str())
    char mMapNameBuffer[kMapNameMaxSize+1];
};


} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
