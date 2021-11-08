/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "Privilege.h"
#include "RequestPath.h"
#include "SubjectDescriptor.h"

#include <lib/core/CHIPCore.h>

namespace chip {
namespace Access {

class AccessControl
{
public:
    class Entry
    {
    public:
        struct Target
        {
            using Flags = int;
            static constexpr Flags kCluster = 1 << 0;
            static constexpr Flags kEndpoint = 1 << 1;
            static constexpr Flags kDeviceType = 1 << 2;
            Flags flags;
            ClusterId cluster;
            EndpointId endpoint;
            DeviceTypeId deviceType;
        };

        class Delegate
        {
        public:
            Delegate() = default;

            Delegate(const Delegate &) = delete;
            Delegate & operator=(const Delegate &) = delete;

            virtual ~Delegate() = default;

            virtual void Release() {}

            // Simple getters
            virtual CHIP_ERROR GetAuthMode(AuthMode & authMode) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR GetFabricIndex(FabricIndex & fabricIndex) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR GetPrivilege(Privilege & privilege) const { return CHIP_ERROR_NOT_IMPLEMENTED; }

            // Simple setters
            virtual CHIP_ERROR SetAuthMode(AuthMode authMode) { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR SetFabricIndex(FabricIndex fabricIndex) { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR SetPrivilege(Privilege privilege) { return CHIP_ERROR_NOT_IMPLEMENTED; }

            // Subjects
            virtual CHIP_ERROR GetSubjectCount(size_t & count) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR GetSubject(size_t index, NodeId & subject) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR SetSubject(size_t index, NodeId subject) { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR AddSubject(size_t * index, NodeId subject) { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR RemoveSubject(size_t index) { return CHIP_ERROR_NOT_IMPLEMENTED; }

            // Targets
            virtual CHIP_ERROR GetTargetCount(size_t & count) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR GetTarget(size_t index, Target & target) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR SetTarget(size_t index, const Target & target) { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR AddTarget(size_t * index, const Target & target) { return CHIP_ERROR_NOT_IMPLEMENTED; }
            virtual CHIP_ERROR RemoveTarget(size_t index) { return CHIP_ERROR_NOT_IMPLEMENTED; }
        };

        Entry() = default;

        Entry(const Entry &) = delete;
        Entry & operator=(const Entry &) = delete;

        ~Entry()
        {
            mDelegate->Release();
        }

        // Simple getters
        CHIP_ERROR GetAuthMode(AuthMode & authMode) const { return mDelegate->GetAuthMode(authMode); }
        CHIP_ERROR GetFabricIndex(FabricIndex & fabricIndex) const { return mDelegate->GetFabricIndex(fabricIndex); }
        CHIP_ERROR GetPrivilege(Privilege & privilege) const { return mDelegate->GetPrivilege(privilege); }

        // Simple setters
        CHIP_ERROR SetAuthMode(AuthMode authMode) { return mDelegate->SetAuthMode(authMode); }
        CHIP_ERROR SetFabricIndex(FabricIndex fabricIndex) { return mDelegate->SetFabricIndex(fabricIndex); }
        CHIP_ERROR SetPrivilege(Privilege privilege) { return mDelegate->SetPrivilege(privilege); }

        // Subjects
        CHIP_ERROR GetSubjectCount(size_t & count) const { return mDelegate->GetSubjectCount(count); }
        CHIP_ERROR GetSubject(size_t index, NodeId & subject) const { return mDelegate->GetSubject(index, subject); }
        CHIP_ERROR SetSubject(size_t index, NodeId subject) { return mDelegate->SetSubject(index, subject); }
        CHIP_ERROR AddSubject(size_t * index, NodeId subject) { return mDelegate->AddSubject(index, subject); }
        CHIP_ERROR RemoveSubject(size_t index) { return mDelegate->RemoveSubject(index); }

        // Targets
        CHIP_ERROR GetTargetCount(size_t & count) const { return mDelegate->GetTargetCount(count); }
        CHIP_ERROR GetTarget(size_t index, Target & target) const { return mDelegate->GetTarget(index, target); }
        CHIP_ERROR SetTarget(size_t index, const Target & target) { return mDelegate->SetTarget(index, target); }
        CHIP_ERROR AddTarget(size_t * index, const Target & target) { return mDelegate->AddTarget(index, target); }
        CHIP_ERROR RemoveTarget(size_t index) { return mDelegate->RemoveTarget(index); }

    public:
        const Delegate & GetDelegate() const
        {
            return *mDelegate;
        }

        Delegate & GetDelegate()
        {
            return *mDelegate;
        }

        void SetDelegate(Delegate & delegate)
        {
            mDelegate->Release();
            mDelegate = &delegate;
        }

        void ResetDelegate()
        {
            mDelegate->Release();
            mDelegate = &mDefaultDelegate;
        }

    protected:
        static Delegate mDefaultDelegate;
        Delegate * mDelegate = &mDefaultDelegate;
    };

    class EntryIterator
    {
    public:
        class Delegate
        {
        public:
            Delegate() = default;

            Delegate(const Delegate &) = delete;
            Delegate & operator=(const Delegate &) = delete;

            virtual ~Delegate() = default;

            virtual void Release() {}

            virtual CHIP_ERROR Next(Entry & entry) { return CHIP_ERROR_SENTINEL; }
        };

        EntryIterator() = default;

        EntryIterator(const EntryIterator &) = delete;
        EntryIterator & operator=(const EntryIterator &) = delete;

        ~EntryIterator()
        {
            mDelegate->Release();
        }

        CHIP_ERROR Next(Entry & entry) { return mDelegate->Next(entry); }

    public:
        const Delegate & GetDelegate() const
        {
            return *mDelegate;
        }

        Delegate & GetDelegate()
        {
            return *mDelegate;
        }

        void SetDelegate(Delegate & delegate)
        {
            mDelegate->Release();
            mDelegate = &delegate;
        }

        void ResetDelegate()
        {
            mDelegate->Release();
            mDelegate = &mDefaultDelegate;
        }

    private:
        static Delegate mDefaultDelegate;
        Delegate * mDelegate = &mDefaultDelegate;
    };

    class Extension
    {
        // TODO: implement extension
    };

    class ExtensionIterator
    {
        // TODO: implement extension iterator
    };

    class Listener
    {
    public:
        virtual ~Listener() = default;

        // TODO: add entry/extension to listener interface
        virtual void OnEntryChanged() = 0;
        virtual void OnExtensionChanged() = 0;
    };

    class Delegate
    {
    public:
        Delegate() = default;

        Delegate(const Delegate &) = delete;
        Delegate & operator=(const Delegate &) = delete;

        virtual ~Delegate() = default;

        virtual void Release() {}

        virtual CHIP_ERROR Init() { return CHIP_ERROR_NOT_IMPLEMENTED; }
        virtual CHIP_ERROR Finish() { return CHIP_ERROR_NOT_IMPLEMENTED; }

        // Capabilities
        virtual CHIP_ERROR GetMaxEntries(int & value) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
        // TODO: more capabilities

        // Preparation
        virtual CHIP_ERROR PrepareEntry(Entry & entry) { return CHIP_ERROR_NOT_IMPLEMENTED; }

        // CRUD
        virtual CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, const FabricIndex * fabricIndex) { return CHIP_ERROR_NOT_IMPLEMENTED; }
        virtual CHIP_ERROR ReadEntry(size_t index, Entry & entry, const FabricIndex * fabricIndex) const { return CHIP_ERROR_NOT_IMPLEMENTED; }
        virtual CHIP_ERROR UpdateEntry(size_t index, const Entry & entry, const FabricIndex * fabricIndex) { return CHIP_ERROR_NOT_IMPLEMENTED; }
        virtual CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex) { return CHIP_ERROR_NOT_IMPLEMENTED; }

        // Iteration
        virtual CHIP_ERROR Entries(EntryIterator & iterator, const FabricIndex * fabricIndex) const { return CHIP_ERROR_NOT_IMPLEMENTED; }

        // Listening
        virtual void SetListener(Listener & listener) { mListener = &listener; }
        virtual void ClearListener() { mListener = nullptr; }

    private:
        Listener * mListener = nullptr;
    };

    AccessControl() = default;

    AccessControl(Delegate & delegate)
        : mDelegate(delegate)
    {
    }

    AccessControl(const AccessControl &) = delete;
    AccessControl & operator=(const AccessControl &) = delete;

    ~AccessControl()
    {
        mDelegate.Release();
    }

    /**
     * Initialize the access control module. Must be called before first use.
     *
     * @retval various errors, probably fatal.
     */
    CHIP_ERROR Init();

    /**
     * Deinitialize the access control module. Must be called when finished.
     */
    CHIP_ERROR Finish();

    // Capabilities
    CHIP_ERROR GetMaxEntries(int & value) const { return mDelegate.GetMaxEntries(value); }

    // Preparation
    CHIP_ERROR PrepareEntry(Entry & entry) { return mDelegate.PrepareEntry(entry); }

    // CRUD
    CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, const FabricIndex * fabricIndex = nullptr) { return mDelegate.CreateEntry(index, entry, fabricIndex); }
    CHIP_ERROR ReadEntry(size_t index, Entry & entry, const FabricIndex * fabricIndex = nullptr) const { return mDelegate.ReadEntry(index, entry, fabricIndex); }
    CHIP_ERROR UpdateEntry(size_t index, const Entry & entry, const FabricIndex * fabricIndex = nullptr) { return mDelegate.UpdateEntry(index, entry, fabricIndex); }
    CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex = nullptr) { return mDelegate.DeleteEntry(index, fabricIndex); }

    // Iteration
    CHIP_ERROR Entries(EntryIterator & iterator, const FabricIndex * fabricIndex = nullptr) const { return mDelegate.Entries(iterator, fabricIndex); }

    /**
     * Check whether access (by a subject descriptor, to a request path,
     * requiring a privilege) should be allowed or denied.
     *
     * @retval #CHIP_ERROR_ACCESS_DENIED if denied.
     * @retval other errors should also be treated as denied.
     * @retval #CHIP_NO_ERROR if allowed.
     */
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege requestPrivilege);

private:
    static Delegate mDefaultDelegate;
    Delegate & mDelegate = mDefaultDelegate;
};

/**
 * Get the global instance set by SetAccessControl, or the default.
 *
 * Calls to this function must be synchronized externally.
 */
AccessControl & GetAccessControl();

/**
 * Set the global instance returned by GetAccessControl.
 *
 * Calls to this function must be synchronized externally.
 */
void SetAccessControl(AccessControl & accessControl);

/**
 * Reset the global instance to the default.
 *
 * Calls to this function must be synchronized externally.
 */
void ResetAccessControl();

} // namespace Access
} // namespace chip
