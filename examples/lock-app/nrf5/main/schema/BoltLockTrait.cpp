/*
 *
 *    Copyright (c) 2019 Google LLC.
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

#include <schema/include/BoltLockTrait.h>

namespace Schema {
namespace Weave {
namespace Trait {
namespace Security {
namespace BoltLockTrait {

using namespace ::nl::Weave::Profiles::DataManagement;

//
// Property Table
//

const TraitSchemaEngine::PropertyInfo PropertyMap[] = {
    { kPropertyHandle_Root, 1 }, // state
    { kPropertyHandle_Root, 2 }, // actuator_state
    { kPropertyHandle_Root, 3 }, // locked_state
    { kPropertyHandle_Root, 4 }, // bolt_lock_actor
    { kPropertyHandle_BoltLockActor, 1 }, // method
    { kPropertyHandle_BoltLockActor, 2 }, // originator
    { kPropertyHandle_BoltLockActor, 3 }, // agent
    { kPropertyHandle_Root, 5 }, // locked_state_last_changed_at
};

//
// IsNullable Table
//

uint8_t IsNullableHandleBitfield[] = {
        0xe8
};

//
// IsEphemeral Table
//

uint8_t IsEphemeralHandleBitfield[] = {
        0x88
};

//
// Supported version
//
const ConstSchemaVersionRange traitVersion = { .mMinVersion = 1, .mMaxVersion = 2 };

//
// Schema
//

const TraitSchemaEngine TraitSchema = {
    {
        kWeaveProfileId,
        PropertyMap,
        sizeof(PropertyMap) / sizeof(PropertyMap[0]),
        2,
#if (TDM_EXTENSION_SUPPORT) || (TDM_VERSIONING_SUPPORT)
        2,
#endif
        NULL,
        NULL,
        NULL,
        &IsNullableHandleBitfield[0],
        &IsEphemeralHandleBitfield[0],
#if (TDM_EXTENSION_SUPPORT)
        NULL,
#endif
#if (TDM_VERSIONING_SUPPORT)
        &traitVersion,
#endif
    }
};

    //
    // Events
    //

const nl::FieldDescriptor BoltActuatorStateChangeEventFieldDescriptors[] =
{
    {
        NULL, offsetof(BoltActuatorStateChangeEvent, state), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeInt32, 0), 1
    },

    {
        NULL, offsetof(BoltActuatorStateChangeEvent, actuatorState), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeInt32, 0), 2
    },

    {
        NULL, offsetof(BoltActuatorStateChangeEvent, lockedState), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeInt32, 0), 3
    },

    {
        &Schema::Weave::Trait::Security::BoltLockTrait::BoltLockActorStruct::FieldSchema, offsetof(BoltActuatorStateChangeEvent, boltLockActor), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeStructure, 0), 4
    },

};

const nl::SchemaFieldDescriptor BoltActuatorStateChangeEvent::FieldSchema =
{
    .mNumFieldDescriptorElements = sizeof(BoltActuatorStateChangeEventFieldDescriptors)/sizeof(BoltActuatorStateChangeEventFieldDescriptors[0]),
    .mFields = BoltActuatorStateChangeEventFieldDescriptors,
    .mSize = sizeof(BoltActuatorStateChangeEvent)
};
const nl::Weave::Profiles::DataManagement::EventSchema BoltActuatorStateChangeEvent::Schema =
{
    .mProfileId = kWeaveProfileId,
    .mStructureType = 0x1,
    .mImportance = nl::Weave::Profiles::DataManagement::ProductionCritical,
    .mDataSchemaVersion = 2,
    .mMinCompatibleDataSchemaVersion = 1,
};

//
// Event Structs
//

const nl::FieldDescriptor BoltLockActorStructFieldDescriptors[] =
{
    {
        NULL, offsetof(BoltLockActorStruct, method), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeInt32, 0), 1
    },

    {
        NULL, offsetof(BoltLockActorStruct, originator), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeByteString, 1), 2
    },

    {
        NULL, offsetof(BoltLockActorStruct, agent), SET_TYPE_AND_FLAGS(nl::SerializedFieldTypeByteString, 1), 3
    },

};

const nl::SchemaFieldDescriptor BoltLockActorStruct::FieldSchema =
{
    .mNumFieldDescriptorElements = sizeof(BoltLockActorStructFieldDescriptors)/sizeof(BoltLockActorStructFieldDescriptors[0]),
    .mFields = BoltLockActorStructFieldDescriptors,
    .mSize = sizeof(BoltLockActorStruct)
};

} // namespace BoltLockTrait
} // namespace Security
} // namespace Trait
} // namespace Weave
} // namespace Schema
