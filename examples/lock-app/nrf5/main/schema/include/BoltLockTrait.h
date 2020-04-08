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

#ifndef _WEAVE_TRAIT_SECURITY__BOLT_LOCK_TRAIT_H_
#define _WEAVE_TRAIT_SECURITY__BOLT_LOCK_TRAIT_H_

#include <Weave/Profiles/data-management/DataManagement.h>
#include <Weave/Support/SerializationUtils.h>



namespace Schema {
namespace Weave {
namespace Trait {
namespace Security {
namespace BoltLockTrait {

extern const nl::Weave::Profiles::DataManagement::TraitSchemaEngine TraitSchema;

enum {
      kWeaveProfileId = (0x0U << 16) | 0xe02U
};

//
// Properties
//

enum {
    kPropertyHandle_Root = 1,

    //---------------------------------------------------------------------------------------------------------------------------//
    //  Name                                IDL Type                            TLV Type           Optional?       Nullable?     //
    //---------------------------------------------------------------------------------------------------------------------------//

    //
    //  state                               BoltState                            int               NO              NO
    //
    kPropertyHandle_State = 2,

    //
    //  actuator_state                      BoltActuatorState                    int               NO              NO
    //
    kPropertyHandle_ActuatorState = 3,

    //
    //  locked_state                        BoltLockedState                      int               NO              NO
    //
    kPropertyHandle_LockedState = 4,

    //
    //  bolt_lock_actor                     BoltLockActorStruct                  structure         NO              YES
    //
    kPropertyHandle_BoltLockActor = 5,

    //
    //  method                              BoltLockActorMethod                  int               NO              NO
    //
    kPropertyHandle_BoltLockActor_Method = 6,

    //
    //  originator                          weave.common.ResourceId              bytes             NO              YES
    //
    kPropertyHandle_BoltLockActor_Originator = 7,

    //
    //  agent                               weave.common.ResourceId              bytes             NO              YES
    //
    kPropertyHandle_BoltLockActor_Agent = 8,

    //
    //  locked_state_last_changed_at        google.protobuf.Timestamp            int64 millisecondsNO              YES
    //
    kPropertyHandle_LockedStateLastChangedAt = 9,

    //
    // Enum for last handle
    //
    kLastSchemaHandle = 9,
};

//
// Event Structs
//

struct BoltLockActorStruct
{
    int32_t method;
    nl::SerializedByteString originator;
    void SetOriginatorNull(void);
    void SetOriginatorPresent(void);
#if WEAVE_CONFIG_SERIALIZATION_ENABLE_DESERIALIZATION
    bool IsOriginatorPresent(void);
#endif
    nl::SerializedByteString agent;
    void SetAgentNull(void);
    void SetAgentPresent(void);
#if WEAVE_CONFIG_SERIALIZATION_ENABLE_DESERIALIZATION
    bool IsAgentPresent(void);
#endif
    uint8_t __nullified_fields__[2/8 + 1];

    static const nl::SchemaFieldDescriptor FieldSchema;

};

struct BoltLockActorStruct_array {
    uint32_t num;
    BoltLockActorStruct *buf;
};

inline void BoltLockActorStruct::SetOriginatorNull(void)
{
    SET_FIELD_NULLIFIED_BIT(__nullified_fields__, 0);
}

inline void BoltLockActorStruct::SetOriginatorPresent(void)
{
    CLEAR_FIELD_NULLIFIED_BIT(__nullified_fields__, 0);
}

#if WEAVE_CONFIG_SERIALIZATION_ENABLE_DESERIALIZATION
inline bool BoltLockActorStruct::IsOriginatorPresent(void)
{
    return (!GET_FIELD_NULLIFIED_BIT(__nullified_fields__, 0));
}
#endif
inline void BoltLockActorStruct::SetAgentNull(void)
{
    SET_FIELD_NULLIFIED_BIT(__nullified_fields__, 1);
}

inline void BoltLockActorStruct::SetAgentPresent(void)
{
    CLEAR_FIELD_NULLIFIED_BIT(__nullified_fields__, 1);
}

#if WEAVE_CONFIG_SERIALIZATION_ENABLE_DESERIALIZATION
inline bool BoltLockActorStruct::IsAgentPresent(void)
{
    return (!GET_FIELD_NULLIFIED_BIT(__nullified_fields__, 1));
}
#endif
//
// Events
//
struct BoltActuatorStateChangeEvent
{
    int32_t state;
    int32_t actuatorState;
    int32_t lockedState;
    Schema::Weave::Trait::Security::BoltLockTrait::BoltLockActorStruct boltLockActor;

    static const nl::SchemaFieldDescriptor FieldSchema;

    // Statically-known Event Struct Attributes:
    enum {
            kWeaveProfileId = (0x0U << 16) | 0xe02U,
        kEventTypeId = 0x1U
    };

    static const nl::Weave::Profiles::DataManagement::EventSchema Schema;
};

struct BoltActuatorStateChangeEvent_array {
    uint32_t num;
    BoltActuatorStateChangeEvent *buf;
};


//
// Commands
//

enum {
    kBoltLockChangeRequestId = 0x1,
};

enum BoltLockChangeRequestParameters {
    kBoltLockChangeRequestParameter_State = 1,
    kBoltLockChangeRequestParameter_BoltLockActor = 4,
};

//
// Enums
//

enum BoltState {
    BOLT_STATE_RETRACTED = 1,
    BOLT_STATE_EXTENDED = 2,
};

enum BoltLockActorMethod {
    BOLT_LOCK_ACTOR_METHOD_OTHER = 1,
    BOLT_LOCK_ACTOR_METHOD_PHYSICAL = 2,
    BOLT_LOCK_ACTOR_METHOD_KEYPAD_PIN = 3,
    BOLT_LOCK_ACTOR_METHOD_LOCAL_IMPLICIT = 4,
    BOLT_LOCK_ACTOR_METHOD_REMOTE_USER_EXPLICIT = 5,
    BOLT_LOCK_ACTOR_METHOD_REMOTE_USER_IMPLICIT = 6,
    BOLT_LOCK_ACTOR_METHOD_REMOTE_USER_OTHER = 7,
    BOLT_LOCK_ACTOR_METHOD_REMOTE_DELEGATE = 8,
    BOLT_LOCK_ACTOR_METHOD_LOW_POWER_SHUTDOWN = 9,
    BOLT_LOCK_ACTOR_METHOD_VOICE_ASSISTANT = 10,
};

enum BoltActuatorState {
    BOLT_ACTUATOR_STATE_OK = 1,
    BOLT_ACTUATOR_STATE_LOCKING = 2,
    BOLT_ACTUATOR_STATE_UNLOCKING = 3,
    BOLT_ACTUATOR_STATE_MOVING = 4,
    BOLT_ACTUATOR_STATE_JAMMED_LOCKING = 5,
    BOLT_ACTUATOR_STATE_JAMMED_UNLOCKING = 6,
    BOLT_ACTUATOR_STATE_JAMMED_OTHER = 7,
};

enum BoltLockedState {
    BOLT_LOCKED_STATE_UNLOCKED = 1,
    BOLT_LOCKED_STATE_LOCKED = 2,
    BOLT_LOCKED_STATE_UNKNOWN = 3,
};

} // namespace BoltLockTrait
} // namespace Security
} // namespace Trait
} // namespace Weave
} // namespace Schema
#endif // _WEAVE_TRAIT_SECURITY__BOLT_LOCK_TRAIT_H_
