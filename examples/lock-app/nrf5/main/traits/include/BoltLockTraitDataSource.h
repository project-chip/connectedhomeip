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

/**
 *    @file
 *      A trait data source implementation for the Weave Security BoltLockTrait.
 *
 */

#ifndef BOLT_LOCK_TRAIT_DATA_SOURCE_H
#define BOLT_LOCK_TRAIT_DATA_SOURCE_H

#include <Weave/Profiles/data-management/DataManagement.h>

class BoltLockTraitDataSource : public nl::Weave::Profiles::DataManagement::TraitDataSource
{
public:
    BoltLockTraitDataSource();

    bool IsLocked();
    void InitiateLock(int32_t aLockActor);
    void InitiateUnlock(int32_t aLockActor);

    void LockingSuccessful(void);
    void UnlockingSuccessful(void);

private:
    WEAVE_ERROR GetLeafData(::nl::Weave::Profiles::DataManagement_Current::PropertyPathHandle aLeafHandle, uint64_t aTagToWrite,
                            ::nl::Weave::TLV::TLVWriter & aWriter);

    void OnCustomCommand(nl::Weave::Profiles::DataManagement::Command * aCommand, const nl::Weave::WeaveMessageInfo * aMsgInfo,
                         nl::Weave::PacketBuffer * aPayload, const uint64_t & aCommandType, const bool aIsExpiryTimeValid,
                         const int64_t & aExpiryTimeMicroSecond, const bool aIsMustBeVersionValid, const uint64_t & aMustBeVersion,
                         nl::Weave::TLV::TLVReader & aArgumentReader);

    int32_t mLockedState;
    int32_t mLockActor;
    int32_t mActuatorState;
    int32_t mState;
};

#endif /* BOLT_LOCK_TRAIT_DATA_SOURCE_H */
