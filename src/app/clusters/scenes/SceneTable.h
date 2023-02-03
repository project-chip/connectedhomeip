/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        https://urldefense.com/v3/__http://www.apache.org/licenses/LICENSE-2.0__;!!N30Cs7Jr!UgbMbEQ59BIK-1Xslc7QXYm0lQBh92qA3ElecRe1CF_9YhXxbwPOZa6j4plru7B7kCJ7bKQgHxgQrket3-Dnk268sIdA7Qb8$
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

class SceneTable
{
public:
    static constexpr size_t kIteratorsMax       = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;
    static constexpr size_t kSceneNameMax       = CHIP_CONFIG_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH;
    static constexpr uint8_t kMaxScenePerFabric = CHIP_CONFIG_SCENES_MAX_PER_FABRIC;
    /// @brief struct used to identify a scene in storage by 3 ids, endpoint, group and scene
    struct SceneStorageId
    {
        static constexpr TLV::Tag TagFirstSceneEndpointID() { return TLV::ContextTag(1); }
        static constexpr TLV::Tag TagFirstSceneGroupID() { return TLV::ContextTag(2); }
        static constexpr TLV::Tag TagFirstSceneID() { return TLV::ContextTag(3); }

        // Identifies endpoint to which this scene applies to
        EndpointId sceneEndpointId = kInvalidEndpointId;
        // Identifies group within the scope of the given fabric
        SceneGroupID sceneGroupId = kGlobalGroupSceneId;
        SceneId sceneId           = kUndefinedSceneId;

        SceneStorageId() = default;
        SceneStorageId(EndpointId endpoint, SceneId id, SceneGroupID groupId = kGlobalGroupSceneId) :
            sceneEndpointId(endpoint), sceneGroupId(groupId), sceneId(id)
        {}
        SceneStorageId(const SceneStorageId & storageId) :
            sceneEndpointId(storageId.sceneEndpointId), sceneGroupId(storageId.sceneGroupId), sceneId(storageId.sceneId)
        {}
        CHIP_ERROR Serialize(TLV::TLVWriter & writer) const
        {
            TLV::TLVType container;
            ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, container));

            ReturnErrorOnFailure(writer.Put(TagFirstSceneEndpointID(), static_cast<uint16_t>(this->sceneEndpointId)));
            ReturnErrorOnFailure(writer.Put(TagFirstSceneGroupID(), static_cast<uint16_t>(this->sceneGroupId)));
            ReturnErrorOnFailure(writer.Put(TagFirstSceneID(), static_cast<uint8_t>(this->sceneId)));

            return writer.EndContainer(container);
        }
        CHIP_ERROR Deserialize(TLV::TLVReader & reader)
        {
            TLV::TLVType container;
            ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(1)));
            ReturnErrorOnFailure(reader.EnterContainer(container));

            ReturnErrorOnFailure(reader.Next(TagFirstSceneEndpointID()));
            ReturnErrorOnFailure(reader.Get(this->sceneEndpointId));
            ReturnErrorOnFailure(reader.Next(TagFirstSceneGroupID()));
            ReturnErrorOnFailure(reader.Get(this->sceneGroupId));
            ReturnErrorOnFailure(reader.Next(TagFirstSceneID()));
            ReturnErrorOnFailure(reader.Get(this->sceneId));

            return reader.ExitContainer(container);
        }

        void Clear()
        {
            sceneEndpointId = kInvalidEndpointId;
            sceneGroupId    = kGlobalGroupSceneId;
            sceneId         = kUndefinedSceneId;
        }
        bool operator==(const SceneStorageId & other)
        {
            return (this->sceneEndpointId == other.sceneEndpointId && this->sceneGroupId == other.sceneGroupId &&
                    this->sceneId == other.sceneId);
        }
        void operator=(const SceneStorageId & other)
        {
            this->sceneEndpointId = other.sceneEndpointId;
            this->sceneGroupId    = other.sceneGroupId;
            this->sceneId         = other.sceneId;
        }
    };

    /// @brief struct used to store data held in a scene
    struct SceneData
    {
        static constexpr TLV::Tag TagSceneName() { return TLV::ContextTag(1); }
        static constexpr TLV::Tag TagSceneTransitionTime() { return TLV::ContextTag(2); }
        static constexpr TLV::Tag TagSceneTransitionTime100() { return TLV::ContextTag(3); }

        char name[kSceneNameMax]                = { 0 };
        size_t nameLength                       = 0;
        SceneTransitionTime sceneTransitionTime = 0;
        ExtensionFieldsSetsImpl extentsionFieldsSets;
        TransitionTime100ms transitionTime100 = 0;
        CharSpan nameSpan;

        SceneData(const CharSpan & sceneName = CharSpan(), SceneTransitionTime time = 0, TransitionTime100ms time100ms = 0) :
            sceneTransitionTime(time), transitionTime100(time100ms)
        {
            this->SetName(sceneName);
        }
        SceneData(ExtensionFieldsSetsImpl fields, const CharSpan & sceneName = CharSpan(), SceneTransitionTime time = 0,
                  TransitionTime100ms time100ms = 0) :
            sceneTransitionTime(time),
            transitionTime100(time100ms)
        {
            this->SetName(sceneName);
            extentsionFieldsSets = fields;
        }
        SceneData(const SceneData & other) :
            sceneTransitionTime(other.sceneTransitionTime), transitionTime100(other.transitionTime100)
        {
            this->SetName(other.nameSpan);
            extentsionFieldsSets = other.extentsionFieldsSets;
        }
        ~SceneData(){};

        CHIP_ERROR Serialize(TLV::TLVWriter & writer) const
        {
            TLV::TLVType container;
            ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, container));

            // A 0 size means the name wasn't used so it won't get stored
            if (!this->nameSpan.empty())
            {
                ReturnErrorOnFailure(writer.PutString(TagSceneName(), this->nameSpan));
            }

            ReturnErrorOnFailure(writer.Put(TagSceneTransitionTime(), static_cast<uint16_t>(this->sceneTransitionTime)));
            ReturnErrorOnFailure(writer.Put(TagSceneTransitionTime100(), static_cast<uint8_t>(this->transitionTime100)));
            ReturnErrorOnFailure(this->extentsionFieldsSets.Serialize(writer));

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
                ReturnErrorOnFailure(reader.Get(this->nameSpan));
                this->SetName(this->nameSpan);
                ReturnErrorOnFailure(reader.Next(TagSceneTransitionTime()));
            }

            ReturnErrorOnFailure(reader.Get(this->sceneTransitionTime));
            ReturnErrorOnFailure(reader.Next(TagSceneTransitionTime100()));
            ReturnErrorOnFailure(reader.Get(this->transitionTime100));
            ReturnErrorOnFailure(this->extentsionFieldsSets.Deserialize(reader));

            return reader.ExitContainer(container);
        }

        void SetName(const CharSpan & sceneName)
        {
            if (nullptr == sceneName.data())
            {
                name[0]    = 0;
                nameLength = 0;
            }
            else
            {
                Platform::CopyString(name, sceneName);
                nameLength = sceneName.size();
            }
            nameSpan = CharSpan(name, nameLength);
        }

        void Clear()
        {
            this->SetName(CharSpan());
            sceneTransitionTime = 0;
            transitionTime100   = 0;
            extentsionFieldsSets.Clear();
        }

        bool operator==(const SceneData & other)
        {
            return (this->nameSpan.data_equal(other.nameSpan) && (this->sceneTransitionTime == other.sceneTransitionTime) &&
                    (this->transitionTime100 == other.transitionTime100) &&
                    (this->extentsionFieldsSets == other.extentsionFieldsSets));
        }

        void operator=(const SceneData & other)
        {
            this->SetName(other.nameSpan);
            this->extentsionFieldsSets = other.extentsionFieldsSets;
            this->sceneTransitionTime  = other.sceneTransitionTime;
            this->transitionTime100    = other.transitionTime100;
        }
    };

    /// @brief Struct combining both ID and data of a table entry
    struct SceneTableEntry
    {

        // ID
        SceneStorageId storageId;

        // DATA
        SceneData storageData;

        SceneTableEntry() = default;
        SceneTableEntry(SceneStorageId id) : storageId(id) {}
        SceneTableEntry(const SceneStorageId id, const SceneData data) : storageId(id), storageData(data) {}

        bool operator==(const SceneTableEntry & other)
        {
            return (this->storageId == other.storageId && this->storageData == other.storageData);
        }

        void operator=(const SceneTableEntry & other)
        {
            this->storageId   = other.storageId;
            this->storageData = other.storageData;
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
