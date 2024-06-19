/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/scenes-server/SceneTable.h>
#include <app/util/attribute-storage.h>
#include <app/util/types_stub.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace scenes {

/// @brief Default implementation of handler, handle EFS from add scene and view scene commands for any cluster
///        The implementation of SerializeSave and ApplyScene were omitted and must be implemented in a way that
///        is compatible with the SerializeAdd output in order to function with the Default Scene Handler.
///        It is worth noting that this implementation is very memory consuming. In the current worst case,
///        (Color control cluster), the Extension Field Set's value pair list TLV occupies 99 bytes of memory
class DefaultSceneHandlerImpl : public scenes::SceneHandler
{
    template <typename T>
    using List = app::DataModel::List<T>;

    template <typename T>
    using DecodableList = app::DataModel::DecodableList<T>;

    using AttributeValuePairType          = app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type;
    using AttributeValuePairDecodableType = app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::DecodableType;
    using ExtensionFieldSetDecodableType  = app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::DecodableType;
    using ExtensionFieldSetType           = app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type;

public:
    /// @brief Struct meant to map the state of a cluster to a specific endpoint. Meant to be used to apply scenes using a timer for
    /// transitioning
    /// @tparam ValueType type of the value to map to the endpoint, must implement operator= and operator== for complex types
    template <typename ValueType>
    struct EndpointStatePair
    {
        EndpointStatePair(EndpointId endpoint = kInvalidEndpointId, ValueType value = ValueType{}) :
            mEndpoint(endpoint), mValue(value)
        {}
        EndpointId mEndpoint;
        ValueType mValue;
    };

    template <typename ValueType, size_t MaxEndpointCount>
    struct StatePairBuffer
    {
        static_assert(std::is_trivial<ValueType>::value, "ValueType must be trivial");
        static_assert(MaxEndpointCount < std::numeric_limits<uint16_t>::max(), "MaxEndpointCount must be less than 65535");

        bool IsEmpty() const { return (mPairCount == 0); }

        CHIP_ERROR FindPair(const EndpointId endpoint, uint16_t & found_index) const
        {
            VerifyOrReturnError(!IsEmpty(), CHIP_ERROR_NOT_FOUND);
            for (found_index = 0; found_index < mPairCount; found_index++)
            {
                if (endpoint == mStatePairBuffer[found_index].mEndpoint)
                {
                    return CHIP_NO_ERROR;
                }
            }

            return CHIP_ERROR_NOT_FOUND;
        }

        CHIP_ERROR InsertPair(const EndpointStatePair<ValueType> & status)
        {
            uint16_t idx;
            CHIP_ERROR err = FindPair(status.mEndpoint, idx);

            if (CHIP_NO_ERROR == err)
            {
                mStatePairBuffer[idx] = status;
            }
            else if (mPairCount < MaxEndpointCount)
            {
                // If not found, insert at the end
                mStatePairBuffer[mPairCount] = status;
                mPairCount++;
            }
            else
            {
                return CHIP_ERROR_NO_MEMORY;
            }

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR GetPair(const EndpointId endpoint, EndpointStatePair<ValueType> & status) const
        {
            uint16_t idx;
            ReturnErrorOnFailure(FindPair(endpoint, idx));

            status = mStatePairBuffer[idx];
            return CHIP_NO_ERROR;
        }

        /// @brief Removes Pair and decrements Pair count if the endpoint existed in the array
        /// @param endpoint : endpoint id of the pair
        CHIP_ERROR RemovePair(const EndpointId endpoint)
        {
            uint16_t position;
            VerifyOrReturnValue(CHIP_NO_ERROR == FindPair(endpoint, position), CHIP_NO_ERROR);

            uint16_t nextPos = static_cast<uint16_t>(position + 1);
            uint16_t moveNum = static_cast<uint16_t>(mPairCount - nextPos);

            // Compress array after removal, if the removed position is not the last
            if (moveNum)
            {
                memmove(&mStatePairBuffer[position], &mStatePairBuffer[nextPos], sizeof(EndpointStatePair<ValueType>) * moveNum);
            }

            mPairCount--;
            // Clear the last occupied position
            mStatePairBuffer[mPairCount].mEndpoint = kInvalidEndpointId;

            return CHIP_NO_ERROR;
        }

        uint16_t mPairCount = 0;
        EndpointStatePair<ValueType> mStatePairBuffer[MaxEndpointCount];
    };

    /// @brief Helper struct that allows clusters that do not have an existing mechanism for doing
    //         asynchronous work to perform scene transitions over some period of time.
    /// @tparam MaxEndpointCount
    template <size_t MaxEndpointCount, size_t FixedEndpointCount>
    struct TransitionTimeInterface
    {
        EmberEventControl sceneHandlerEventControls[MaxEndpointCount];

        TransitionTimeInterface(ClusterId clusterId, void (*callback)(EndpointId)) : mClusterId(clusterId), mCallback(callback) {}

        /**
         * @brief Configures EventControl callback
         *
         * @param[in] endpoint endpoint to start timer for
         * @return EmberEventControl* configured event control
         */
        EmberEventControl * sceneEventControl(EndpointId endpoint)
        {
            EmberEventControl * controller = getEventControl(endpoint, Span<EmberEventControl>(sceneHandlerEventControls));
            VerifyOrReturnValue(controller != nullptr, nullptr);

            controller->endpoint = endpoint;
            controller->callback = mCallback;

            return controller;
        }

        ClusterId mClusterId;
        void (*mCallback)(EndpointId);

    private:
        /**
         * @brief event control object for an endpoint
         *
         * @param[in] endpoint target endpoint
         * @param[in] eventControlArray Array where to find the event control
         * @return EmberEventControl* configured event control
         */
        EmberEventControl * getEventControl(EndpointId endpoint, const Span<EmberEventControl> & eventControlArray)
        {
            uint16_t index = emberAfGetClusterServerEndpointIndex(endpoint, mClusterId, FixedEndpointCount);
            if (index >= eventControlArray.size())
            {
                return nullptr;
            }

            return &eventControlArray[index];
        }
    };

    static constexpr uint8_t kMaxAvPair = CHIP_CONFIG_SCENES_MAX_AV_PAIRS_EFS;

    DefaultSceneHandlerImpl() = default;
    ~DefaultSceneHandlerImpl() override{};

    /// @brief Encodes an attribute value list into a TLV structure and resizes the buffer to the size of the encoded data
    /// @param aVlist[in] Attribute value list to encode
    /// @param serializedBytes[out] Buffer to fill from the Attribute value list in a TLV format
    /// @return CHIP_ERROR
    virtual CHIP_ERROR EncodeAttributeValueList(const List<AttributeValuePairType> & aVlist, MutableByteSpan & serializedBytes);

    /// @brief Decodes an attribute value list from a TLV structure and ensure it fits the member pair buffer
    /// @param serializedBytes [in] Buffer to read from
    /// @param aVlist [out] Attribute value list to fill from the TLV structure.  Only valid while the buffer backing
    /// serializedBytes exists and its contents are not modified.
    /// @return CHIP_ERROR
    virtual CHIP_ERROR DecodeAttributeValueList(const ByteSpan & serializedBytes,
                                                DecodableList<AttributeValuePairDecodableType> & aVlist);

    /// @brief From command AddScene, allows handler to filter through clusters in command to serialize only the supported ones.
    /// @param endpoint[in] Endpoint ID
    /// @param extensionFieldSet[in] ExtensionFieldSets provided by the AddScene Command, pre initialized
    /// @param serializedBytes[out] Buffer to fill from the ExtensionFieldSet in command
    /// @return CHIP_NO_ERROR if successful, CHIP_ERROR_INVALID_ARGUMENT if the cluster is not supported, CHIP_ERROR value
    /// otherwise
    virtual CHIP_ERROR SerializeAdd(EndpointId endpoint, const ExtensionFieldSetDecodableType & extensionFieldSet,
                                    MutableByteSpan & serializedBytes) override;

    /// @brief Simulates taking data from nvm and loading it in a command object if the cluster is supported by the endpoint
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serializedBytes data to deserialize into EFS
    /// @return CHIP_NO_ERROR if Extension Field Set was successfully populated, CHIP_ERROR_INVALID_ARGUMENT if the cluster is not
    /// supported, specific CHIP_ERROR otherwise
    virtual CHIP_ERROR Deserialize(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                   ExtensionFieldSetType & extensionFieldSet) override;

private:
    AttributeValuePairType mAVPairs[kMaxAvPair];
};

} // namespace scenes
} // namespace chip
