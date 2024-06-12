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

inline constexpr size_t kLocationtNameMaxSize   = 128u;
inline constexpr size_t kMapNameMaxSize         = 64u;


/**
 * A class which wraps a Service Area Cluster LocStruct instance, adding string storage and string utility functions.
 */
struct LocationStructureWrapper : public chip::app::Clusters::ServiceArea::Structs::LocationStruct::Type
{
    /**
     * @brief default constructor
     */
    LocationStructureWrapper()
    {
        Set( 0, 0, 
             CharSpan(), DataModel::Nullable<int16_t>(), DataModel::Nullable<AreaTypeTag>(),
             DataModel::Nullable<LandmarkTag>(), DataModel::Nullable<PositionTag>(), DataModel::Nullable<FloorSurfaceTag>());
    }

    /**
     * @brief full constructor (deep copy)
     * @param[in] aLocationId unique identifier of this location
     * @param[in] aMapId identifier of supported map
     * @param[in]aLocationName human readable name for this location (empty string if not used)
     * @param[in] aFloorNumber represents floor level - negative values for below ground
     * @param[in] aAreaTypeTag common namespace Area tag - indicates an association of the location with an indoor or outdoor area of a home
     * @param[in] aLandmarkTag common namespace Landmak tag - indicates an association of the location with a home landmark
     * @param[in] aPositionTag common namespace Position tag - indicates the position of the location with respect to the landmark
     * @param[in] aSurfaceTag common namespace Floor Surface tag - indicates an association of the location with a surface type
     * 
     * @note Requirements regarding what combinations of fields and values are valid are not checked by this class.
     * @note if aLocationName is larger than kLocationtNameMaxSize, it will be truncated
     * @note if aLocationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null
     */
    LocationStructureWrapper( uint32_t                                     aLocationId, 
                              const DataModel::Nullable<uint8_t>         & aMapId, 
                              const CharSpan                             & aLocationName,
                              const DataModel::Nullable<int16_t>         & aFloorNumber,
                              const DataModel::Nullable<AreaTypeTag>     & aAreaTypeTag,
                              const DataModel::Nullable<LandmarkTag>     & aLandmarkTag,
                              const DataModel::Nullable<PositionTag>     & aPositionTag,
                              const DataModel::Nullable<FloorSurfaceTag> & aSurfaceTag  )
    {
        Set( aLocationId, aMapId, 
             aLocationName, aFloorNumber, aAreaTypeTag, 
             aLandmarkTag, aPositionTag, aSurfaceTag);
    }

    /**
     * @brief Copy constructor (deep copy)
     * @param[in] op initialization object
     * 
     * @note if locationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null
     */
    LocationStructureWrapper(const LocationStructureWrapper & op) { *this = op; }

    /**
     * @brief Assignment operator (deep copy)
     * @param[in] op 
     * 
     * @note if locationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null
     */
    LocationStructureWrapper & operator=(const LocationStructureWrapper & op)
    {
        if (op.locationInfo.locationInfo.IsNull())
        {
            Set(op.locationID, op.mapID, 
                CharSpan(), NullOptional, NullOptional,
                op.locationInfo.landmarkTag, op.locationInfo.positionTag, op.locationInfo.surfaceTag);
        }
        else
        {
            Set(op.locationID, op.mapID, 
                op.locationInfo.locationInfo.Value().locationName, op.locationInfo.locationInfo.Value().floorNumber,
                op.locationInfo.locationInfo.Value().areaType,
                op.locationInfo.landmarkTag, op.locationInfo.positionTag, op.locationInfo.surfaceTag);
        }

        return *this;
    }

    /**
     * @brief Set all fields of the location object (deep copy)
     * @param[in] aLocationId unique identifier of this location
     * @param[in] aMapId identifier of supported map
     * @param[in] aLocationName human readable name for this location (empty string if not used)
     * @param[in] aFloorNumber represents floor level - negative values for below ground
     * @param[in] aAreaType common namespace Area tag - indicates an association of the location with an indoor or outdoor area of a home
     * @param[in] aLandmarkTag common namespace Landmak tag - indicates an association of the location with a home landmark
     * @param[in] aPositionTag common namespace Position tag - indicates the position of the location with respect to the landmark
     * @param[in] aSurfaceTag common namespace Floor Surface tag - indicates an association of the location with a surface type
     * 
     * @note Requirements regarding what combinations of fields and values are valid are not checked by this class.
     * @note if aLocationName is larger than kLocationtNameMaxSize, it will be truncated
     * @note if aLocationName is empty string and aFloorNumber and aAreaTypeTag are null, locationInfo will be set to null
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
        locationID   = aLocationId;
        mapID        = aMapId;

        // if there is at least one non-null value for locationInfo, add it to the location structure
        if ((!aLocationName.size() == 0)  ||    
            (!aFloorNumber.IsNull())      ||
            (!aAreaType.IsNull())  )
        {
            // create home location info and fill in (except for locationInfo - see below)
            locationInfo.locationInfo.SetNonNull(Structs::HomeLocationStruct::Type());

            locationInfo.locationInfo.Value().floorNumber    = aFloorNumber;
            locationInfo.locationInfo.Value().areaType       = aAreaType;
        }
        else
        {
            // home location info is null
            locationInfo.locationInfo.SetNull();
        }
                                                                         
        locationInfo.landmarkTag   = aLandmarkTag;
        locationInfo.positionTag   = aPositionTag;
        locationInfo.surfaceTag    = aSurfaceTag;
        

        // Save location name to buffer, so access and lifetime is controlled.
        // If aLocationName is not null, create a CharSpan to reference the text.
        // Make sure there is always a terminating character, mainly so we can  get c_str for logging.
        memset(mLocationNameBuffer, 0, sizeof(mLocationNameBuffer+1));

        // this assumes locationInfo structure was created above, if appropriate
        if (!locationInfo.locationInfo.IsNull())
        {
            if (aLocationName.size() == 0)
            {
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, 0);
            }
            else if (aLocationName.size() > sizeof(mLocationNameBuffer))
            {
                // save truncated name that fits into available size
                memcpy(mLocationNameBuffer, aLocationName.data(), sizeof(mLocationNameBuffer));
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, sizeof(mLocationNameBuffer));
            }
            else
            {
                // save full name
                memcpy(mLocationNameBuffer, aLocationName.data(), aLocationName.size());
                locationInfo.locationInfo.Value().locationName = CharSpan(mLocationNameBuffer, aLocationName.size());
            }
        }
    }

    /**
     * @brief Compare the location's name with given text
     * @param[in] aLocationName name to compare
     * @return true if location structure's name field matches aLocationName
     * 
     * @note  if locations structure's name field is null, returns false
     */
    bool DoesNameMatch(const CharSpan & aLocationName) const
    {
        if ((!locationInfo.locationInfo.IsNull())   &&
            (locationInfo.locationInfo.Value().locationName.size() == aLocationName.size()))
        {
            if (locationInfo.locationInfo.Value().locationName.size() == 0)
            {
                return true;  // both "empty" string
            }
            else
            {
                return (0 == memcmp(locationInfo.locationInfo.Value().locationName.data(), aLocationName.data(), 
                                        locationInfo.locationInfo.Value().locationName.size()));
            }
        }
        else
        {
            return false;  // name is Null or text sizes do not match
        }
    }

    /**
     * @brief get the location name (for logging)
     * @return the map name as a c style char string
     * 
     * @note only available through the LocationStructure object (no access from base structure)
     */
    const char* name_c_str() const
    {
        // note: if name is null, this will return an empty terminated c string - ""
        return mLocationNameBuffer;
    }

private:
    // extra char to allow for terminating 0 ( or name_c_str())
    char mLocationNameBuffer[kLocationtNameMaxSize+1];
};


/**
 * A class which wraps a Service Area Cluster MapStruct instance, adding string storage and string utility functions.
 */
struct MapStructureWrapper : public chip::app::Clusters::ServiceArea::Structs::MapStruct::Type
{
    /**
     * @brief default constructor
     */
    MapStructureWrapper()
    {
        Set( 0, CharSpan());
    }

    /**
     * @brief full constructor (deep copy)
     * @param[in] aMapId identifier of this map
     * @param[in] aMapName human readable name (should not be empty string)
     * 
     * @note Requirements regarding what combinations of fields and values are 'valid' are not checked by this class.
     * @note if aMapName is larger than kMapNameMaxSize, it will be truncated
     */
    MapStructureWrapper( uint8_t aMapId, 
                         const CharSpan & aMapName)
    {
        Set(aMapId, aMapName);
    }

    /**
     * @brief Copy constructor (deep copy)
     * @param[in] op initialization object
     */
    MapStructureWrapper(const MapStructureWrapper & op) { *this = op; }

    /**
     * @brief Assignment operator (deep copy)
     * @param[in] op 
     */
    MapStructureWrapper & operator=(const MapStructureWrapper & op)
    {
        Set(op.mapID, op.name);
        return *this;
    }

    /**
     * @brief Set all fields of the map object (deep copy)
     * @param[in] aMapId identifier of this map
     * @param[in] aMapName human readable name (should not be empty string)
     * @note Requirements regarding what combinations of fields and values are 'valid' are not checked by this class.
     * 
     * @note if aMapName is larger than kMapNameMaxSize, it will be truncated
     */
    void Set( uint8_t aMapId, 
              const CharSpan & aMapName)
    {
        mapID = aMapId;

        // Save map name to buffer, so access and lifetime is controlled.
        // Create a CharSpan reference the text.
        // Make sure there is always a terminating character, mainly so we can  get c_str for logging.
        memset(mMapNameBuffer, 0, sizeof(mMapNameBuffer+1));

        if (aMapName.size() == 0)
        {
            name = CharSpan(mMapNameBuffer, 0);
        }
        else if (aMapName.size() > sizeof(mMapNameBuffer))
        {
            // save truncated name that fits into available size
            memcpy(mMapNameBuffer, aMapName.data(), sizeof(mMapNameBuffer));
            name = CharSpan(mMapNameBuffer, sizeof(mMapNameBuffer));
        }
        else
        {
            // save full name
            memcpy(mMapNameBuffer, aMapName.data(), aMapName.size());
            name = CharSpan(mMapNameBuffer, aMapName.size());
        }
    }

    /**
     * @brief Compare the map's name with given text
     * @param[in] aMapName name to compare
     * @return true if map structure's name field matches aMapName
     */
    bool DoesNameMatch(const CharSpan & aMapName) const
    {
        if (name.size() == 0)
        {
            return (aMapName.size() == 0);
        }
        else
        {
            return ((name.size() == aMapName.size()) && memcmp(name.data(), aMapName.data(), name.size()));
        }
    }

    /**
     * @brief get the map name (for logging)
     * @return the map name as a c style char string
     * 
     * @note only available through the MapStructure object (no access from base structure)
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
