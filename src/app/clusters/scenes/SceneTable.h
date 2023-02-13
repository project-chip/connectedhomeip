/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <app/clusters/scenes/ExtensionFieldsSetsImpl.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/CommonPersistentData.h>
#include <lib/support/Span.h>

namespace chip {
namespace scenes {

typedef uint16_t SceneTransitionTime;
typedef uint8_t TransitionTime100ms;

static constexpr uint8_t kMaxScenePerFabric = CHIP_CONFIG_SCENES_MAX_PER_FABRIC;

class SceneTable
{
public:
    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;
    static constexpr size_t kSceneNameMax = CHIP_CONFIG_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH;

    /// @brief struct used to identify a scene in storage by 3 ids, endpoint, group and scene
    struct SceneStorageId
    {
        static constexpr TLV::Tag TagFirstSceneEndpointID() { return TLV::ContextTag(1); }
        static constexpr TLV::Tag TagFirstSceneGroupID() { return TLV::ContextTag(2); }
        static constexpr TLV::Tag TagFirstSceneID() { return TLV::ContextTag(3); }

        // Identifies endpoint to which this scene applies to
        EndpointId mSceneEndpointId = kInvalidEndpointId;
        // Identifies group within the scope of the given fabric
        SceneGroupID mSceneGroupId = kGlobalGroupSceneId;
        SceneId mSceneId           = kUndefinedSceneId;

        SceneStorageId() = default;
        SceneStorageId(EndpointId endpoint, SceneId id, SceneGroupID groupId = kGlobalGroupSceneId) :
            mSceneEndpointId(endpoint), mSceneGroupId(groupId), mSceneId(id)
        {}
        SceneStorageId(const SceneStorageId & storageId) :
            mSceneEndpointId(storageId.mSceneEndpointId), mSceneGroupId(storageId.mSceneGroupId), mSceneId(storageId.mSceneId)
        {}
        CHIP_ERROR Serialize(TLV::TLVWriter & writer) const
        {
            TLV::TLVType container;
            ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, container));

            ReturnErrorOnFailure(writer.Put(TagFirstSceneEndpointID(), static_cast<uint16_t>(this->mSceneEndpointId)));
            ReturnErrorOnFailure(writer.Put(TagFirstSceneGroupID(), static_cast<uint16_t>(this->mSceneGroupId)));
            ReturnErrorOnFailure(writer.Put(TagFirstSceneID(), static_cast<uint8_t>(this->mSceneId)));

            return writer.EndContainer(container);
        }
        CHIP_ERROR Deserialize(TLV::TLVReader & reader)
        {
            TLV::TLVType container;
            ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(1)));
            ReturnErrorOnFailure(reader.EnterContainer(container));

            ReturnErrorOnFailure(reader.Next(TagFirstSceneEndpointID()));
            ReturnErrorOnFailure(reader.Get(this->mSceneEndpointId));
            ReturnErrorOnFailure(reader.Next(TagFirstSceneGroupID()));
            ReturnErrorOnFailure(reader.Get(this->mSceneGroupId));
            ReturnErrorOnFailure(reader.Next(TagFirstSceneID()));
            ReturnErrorOnFailure(reader.Get(this->mSceneId));

            return reader.ExitContainer(container);
        }

        void Clear()
        {
            mSceneEndpointId = kInvalidEndpointId;
            mSceneGroupId    = kGlobalGroupSceneId;
            mSceneId         = kUndefinedSceneId;
        }
        bool operator==(const SceneStorageId & other)
        {
            return (this->mSceneEndpointId == other.mSceneEndpointId && this->mSceneGroupId == other.mSceneGroupId &&
                    this->mSceneId == other.mSceneId);
        }
        void operator=(const SceneStorageId & other)
        {
            this->mSceneEndpointId = other.mSceneEndpointId;
            this->mSceneGroupId    = other.mSceneGroupId;
            this->mSceneId         = other.mSceneId;
        }
    };

    /// @brief struct used to store data held in a scene
    struct SceneData
    {
        static constexpr TLV::Tag TagSceneName() { return TLV::ContextTag(1); }
        static constexpr TLV::Tag TagSceneTransitionTime() { return TLV::ContextTag(2); }
        static constexpr TLV::Tag TagSceneTransitionTime100() { return TLV::ContextTag(3); }

        char mName[kSceneNameMax]                = { 0 };
        size_t mNameLength                       = 0;
        SceneTransitionTime mSceneTransitionTime = 0;
        ExtensionFieldsSetsImpl mExtentsionFieldsSets;
        TransitionTime100ms mTransitionTime100 = 0;
        CharSpan mNameSpan;

        SceneData(const CharSpan & sceneName = CharSpan(), SceneTransitionTime time = 0, TransitionTime100ms time100ms = 0) :
            mSceneTransitionTime(time), mTransitionTime100(time100ms)
        {
            this->SetName(sceneName);
        }
        SceneData(ExtensionFieldsSetsImpl fields, const CharSpan & sceneName = CharSpan(), SceneTransitionTime time = 0,
                  TransitionTime100ms time100ms = 0) :
            mSceneTransitionTime(time),
            mTransitionTime100(time100ms)
        {
            this->SetName(sceneName);
            mExtentsionFieldsSets = fields;
        }
        SceneData(const SceneData & other) :
            mSceneTransitionTime(other.mSceneTransitionTime), mTransitionTime100(other.mTransitionTime100)
        {
            this->SetName(other.mNameSpan);
            mExtentsionFieldsSets = other.mExtentsionFieldsSets;
        }
        ~SceneData(){};

        CHIP_ERROR Serialize(TLV::TLVWriter & writer) const
        {
            TLV::TLVType container;
            ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, container));

            // A 0 size means the name wasn't used so it won't get stored
            if (!this->mNameSpan.empty())
            {
                ReturnErrorOnFailure(writer.PutString(TagSceneName(), this->mNameSpan));
            }

            ReturnErrorOnFailure(writer.Put(TagSceneTransitionTime(), static_cast<uint16_t>(this->mSceneTransitionTime)));
            ReturnErrorOnFailure(writer.Put(TagSceneTransitionTime100(), static_cast<uint8_t>(this->mTransitionTime100)));
            ReturnErrorOnFailure(this->mExtentsionFieldsSets.Serialize(writer));

            return writer.EndContainer(container);
        }
        CHIP_ERROR Deserialize(TLV::TLVReader & reader)
        {
            TLV::TLVType container;
            ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(1)));
            ReturnErrorOnFailure(reader.EnterContainer(container));

            ReturnErrorOnFailure(reader.Next());
            TLV::Tag currTag = reader.GetTag();
            VerifyOrReturnError(TagSceneName() == currTag || TagSceneTransitionTime() == currTag, CHIP_ERROR_WRONG_TLV_TYPE);

            // If there was no error, a name is expected from the storage, if there was an unexpectec TLV element,
            if (currTag == TagSceneName())
            {
                ReturnErrorOnFailure(reader.Get(this->mNameSpan));
                this->SetName(this->mNameSpan);
                ReturnErrorOnFailure(reader.Next(TagSceneTransitionTime()));
            }

            ReturnErrorOnFailure(reader.Get(this->mSceneTransitionTime));
            ReturnErrorOnFailure(reader.Next(TagSceneTransitionTime100()));
            ReturnErrorOnFailure(reader.Get(this->mTransitionTime100));
            ReturnErrorOnFailure(this->mExtentsionFieldsSets.Deserialize(reader));

            return reader.ExitContainer(container);
        }

        void SetName(const CharSpan & sceneName)
        {
            if (nullptr == sceneName.data())
            {
                mName[0]    = 0;
                mNameLength = 0;
            }
            else
            {
                Platform::CopyString(mName, sceneName);
                mNameLength = sceneName.size();
            }
            mNameSpan = CharSpan(mName, mNameLength);
        }

        void Clear()
        {
            this->SetName(CharSpan());
            mSceneTransitionTime = 0;
            mTransitionTime100   = 0;
            mExtentsionFieldsSets.Clear();
        }

        bool operator==(const SceneData & other)
        {
            return (this->mNameSpan.data_equal(other.mNameSpan) && (this->mSceneTransitionTime == other.mSceneTransitionTime) &&
                    (this->mTransitionTime100 == other.mTransitionTime100) &&
                    (this->mExtentsionFieldsSets == other.mExtentsionFieldsSets));
        }

        void operator=(const SceneData & other)
        {
            this->SetName(other.mNameSpan);
            this->mExtentsionFieldsSets = other.mExtentsionFieldsSets;
            this->mSceneTransitionTime  = other.mSceneTransitionTime;
            this->mTransitionTime100    = other.mTransitionTime100;
        }
    };

    /// @brief Struct combining both ID and data of a table entry
    struct SceneTableEntry
    {

        // ID
        SceneStorageId mStorageId;

        // DATA
        SceneData mStorageData;

        SceneTableEntry() = default;
        SceneTableEntry(SceneStorageId id) : mStorageId(id) {}
        SceneTableEntry(const SceneStorageId id, const SceneData data) : mStorageId(id), mStorageData(data) {}

        bool operator==(const SceneTableEntry & other)
        {
            return (this->mStorageId == other.mStorageId && this->mStorageData == other.mStorageData);
        }

        void operator=(const SceneTableEntry & other)
        {
            this->mStorageId   = other.mStorageId;
            this->mStorageData = other.mStorageData;
        }
    };

    SceneTable(){};

    virtual ~SceneTable() = default;

    // Not copyable
    SceneTable(const SceneTable &)             = delete;
    SceneTable & operator=(const SceneTable &) = delete;

    virtual CHIP_ERROR Init(PersistentStorageDelegate * storage) = 0;
    virtual void Finish()                                        = 0;

    // Data
    virtual CHIP_ERROR SetSceneTableEntry(FabricIndex fabric_index, const SceneTableEntry & entry)                    = 0;
    virtual CHIP_ERROR GetSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id, SceneTableEntry & entry) = 0;
    virtual CHIP_ERROR RemoveSceneTableEntry(FabricIndex fabric_index, SceneStorageId scene_id)                       = 0;
    virtual CHIP_ERROR RemoveSceneTableEntryAtPosition(FabricIndex fabric_index, SceneIndex scened_idx)               = 0;

    // Iterators
    using SceneEntryIterator = CommonIterator<SceneTableEntry>;

    virtual SceneEntryIterator * IterateSceneEntry(FabricIndex fabric_index) = 0;

    // Fabrics
    virtual CHIP_ERROR RemoveFabric(FabricIndex fabric_index) = 0;

protected:
    const uint8_t mMaxScenePerFabric = kMaxScenePerFabric;
};

} // namespace scenes
} // namespace chip
