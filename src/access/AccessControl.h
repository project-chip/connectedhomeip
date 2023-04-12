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

#include "lib/support/CodeUtils.h"
#include <lib/core/CHIPCore.h>

// Dump function for use during development only (0 for disabled, non-zero for enabled).
#define CHIP_ACCESS_CONTROL_DUMP_ENABLED 0

namespace chip {
namespace Access {

class AccessControl
{
public:
    /**
     * Used by access control to determine if a device type resolves to an endpoint.
     */
    struct DeviceTypeResolver
    {
    public:
        virtual ~DeviceTypeResolver() = default;

        virtual bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) = 0;
    };

    /**
     * Handle to an entry in the access control list.
     *
     * Must be prepared (`AccessControl::PrepareEntry`) or read (`AccessControl::ReadEntry`) before first use.
     */
    class Entry
    {
    public:
        struct Target
        {
            using Flags                        = unsigned;
            static constexpr Flags kCluster    = 1 << 0;
            static constexpr Flags kEndpoint   = 1 << 1;
            static constexpr Flags kDeviceType = 1 << 2;
            Flags flags                        = 0;
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
            virtual CHIP_ERROR GetAuthMode(AuthMode & authMode) const { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR GetFabricIndex(FabricIndex & fabricIndex) const { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR GetPrivilege(Privilege & privilege) const { return CHIP_NO_ERROR; }

            // Simple setters
            virtual CHIP_ERROR SetAuthMode(AuthMode authMode) { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR SetFabricIndex(FabricIndex fabricIndex) { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR SetPrivilege(Privilege privilege) { return CHIP_NO_ERROR; }

            // Subjects
            virtual CHIP_ERROR GetSubjectCount(size_t & count) const
            {
                count = 0;
                return CHIP_NO_ERROR;
            }
            virtual CHIP_ERROR GetSubject(size_t index, NodeId & subject) const { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR SetSubject(size_t index, NodeId subject) { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR AddSubject(size_t * index, NodeId subject) { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR RemoveSubject(size_t index) { return CHIP_NO_ERROR; }

            // Targets
            virtual CHIP_ERROR GetTargetCount(size_t & count) const
            {
                count = 0;
                return CHIP_NO_ERROR;
            }
            virtual CHIP_ERROR GetTarget(size_t index, Target & target) const { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR SetTarget(size_t index, const Target & target) { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR AddTarget(size_t * index, const Target & target) { return CHIP_NO_ERROR; }
            virtual CHIP_ERROR RemoveTarget(size_t index) { return CHIP_NO_ERROR; }
        };

        Entry() = default;

        Entry(Entry && other) : mDelegate(other.mDelegate) { other.mDelegate = &mDefaultDelegate; }

        Entry & operator=(Entry && other)
        {
            if (this != &other)
            {
                mDelegate->Release();
                mDelegate       = other.mDelegate;
                other.mDelegate = &mDefaultDelegate;
            }
            return *this;
        }

        Entry(const Entry &) = delete;
        Entry & operator=(const Entry &) = delete;

        ~Entry() { mDelegate->Release(); }

        // Simple getters
        CHIP_ERROR GetAuthMode(AuthMode & authMode) const { return mDelegate->GetAuthMode(authMode); }
        CHIP_ERROR GetFabricIndex(FabricIndex & fabricIndex) const { return mDelegate->GetFabricIndex(fabricIndex); }
        CHIP_ERROR GetPrivilege(Privilege & privilege) const { return mDelegate->GetPrivilege(privilege); }

        // Simple setters
        CHIP_ERROR SetAuthMode(AuthMode authMode) { return mDelegate->SetAuthMode(authMode); }
        CHIP_ERROR SetFabricIndex(FabricIndex fabricIndex) { return mDelegate->SetFabricIndex(fabricIndex); }
        CHIP_ERROR SetPrivilege(Privilege privilege) { return mDelegate->SetPrivilege(privilege); }

        /**
         * Gets the number of subjects.
         *
         * @param [out] count   The number of subjects.
         */
        CHIP_ERROR GetSubjectCount(size_t & count) const { return mDelegate->GetSubjectCount(count); }

        /**
         * Gets the specified subject.
         *
         * @param [in]  index       The index of the subject to get.
         * @param [out] subject     The subject into which to get.
         */
        CHIP_ERROR GetSubject(size_t index, NodeId & subject) const { return mDelegate->GetSubject(index, subject); }

        /**
         * Sets the specified subject.
         *
         * @param [in] index        The index of the subject to set.
         * @param [in] subject      The subject from which to set.
         */
        CHIP_ERROR SetSubject(size_t index, NodeId subject) { return mDelegate->SetSubject(index, subject); }

        /**
         * Adds the specified subject.
         *
         * @param [out] index       The index of the added subject, if not null.
         * @param [in]  subject     The subject to add.
         */
        CHIP_ERROR AddSubject(size_t * index, NodeId subject) { return mDelegate->AddSubject(index, subject); }

        /**
         * Removes the specified subject.
         *
         * @param [in] index        The index of the subject to delete.
         */
        CHIP_ERROR RemoveSubject(size_t index) { return mDelegate->RemoveSubject(index); }

        /**
         * Gets the number of targets.
         *
         * @param [out] count   The number of targets.
         */
        CHIP_ERROR GetTargetCount(size_t & count) const { return mDelegate->GetTargetCount(count); }

        /**
         * Gets the specified target.
         *
         * @param [in]  index       The index of the target to get.
         * @param [out] target      The target into which to get.
         */
        CHIP_ERROR GetTarget(size_t index, Target & target) const { return mDelegate->GetTarget(index, target); }

        /**
         * Sets the specified target.
         *
         * @param [in] index        The index of the target to set.
         * @param [in] target       The target from which to set.
         */
        CHIP_ERROR SetTarget(size_t index, const Target & target) { return mDelegate->SetTarget(index, target); }

        /**
         * Adds the specified target.
         *
         * @param [out] index       The index of the added target, if not null.
         * @param [in]  target      The target to add.
         */
        CHIP_ERROR AddTarget(size_t * index, const Target & target) { return mDelegate->AddTarget(index, target); }

        /**
         * Removes the specified target.
         *
         * @param [in] index        The index of the target to delete.
         */
        CHIP_ERROR RemoveTarget(size_t index) { return mDelegate->RemoveTarget(index); }

        bool HasDefaultDelegate() const { return mDelegate == &mDefaultDelegate; }

        const Delegate & GetDelegate() const { return *mDelegate; }

        Delegate & GetDelegate() { return *mDelegate; }

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

    /**
     * Handle to an entry iterator in the access control list.
     *
     * Must be initialized (`AccessControl::Entries`) before first use.
     */
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

        ~EntryIterator() { mDelegate->Release(); }

        CHIP_ERROR Next(Entry & entry) { return mDelegate->Next(entry); }

        const Delegate & GetDelegate() const { return *mDelegate; }

        Delegate & GetDelegate() { return *mDelegate; }

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

    /**
     * Used by access control to notify of changes in access control list.
     */
    class EntryListener
    {
    public:
        enum class ChangeType
        {
            kAdded   = 1,
            kRemoved = 2,
            kUpdated = 3
        };

        virtual ~EntryListener() = default;

        /**
         * Notifies of a change in the access control list.
         *
         * The fabric is indicated by its own parameter. If available, a subject descriptor will
         * have more detail (and its fabric index will match). A best effort is made to provide
         * the latest value of the changed entry.
         *
         * @param [in] subjectDescriptor Optional (if available) subject descriptor for this operation.
         * @param [in] fabric            Index of fabric in which entry has changed.
         * @param [in] index             Index of entry to which has changed (relative to fabric).
         * @param [in] entry             Optional (best effort) latest value of entry which has changed.
         * @param [in] changeType        Type of change.
         */
        virtual void OnEntryChanged(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index,
                                    const Entry * entry, ChangeType changeType) = 0;

    private:
        EntryListener * mNext = nullptr;

        friend class AccessControl;
    };

    class Delegate
    {
    public:
        Delegate() = default;

        Delegate(const Delegate &) = delete;
        Delegate & operator=(const Delegate &) = delete;

        virtual ~Delegate() = default;

        virtual void Release() {}

        virtual CHIP_ERROR Init() { return CHIP_NO_ERROR; }
        virtual void Finish() {}

        // Capabilities
        virtual CHIP_ERROR GetMaxEntriesPerFabric(size_t & value) const
        {
            value = 0;
            return CHIP_NO_ERROR;
        }

        virtual CHIP_ERROR GetMaxSubjectsPerEntry(size_t & value) const
        {
            value = 0;
            return CHIP_NO_ERROR;
        }

        virtual CHIP_ERROR GetMaxTargetsPerEntry(size_t & value) const
        {
            value = 0;
            return CHIP_NO_ERROR;
        }

        virtual CHIP_ERROR GetMaxEntryCount(size_t & value) const
        {
            value = 0;
            return CHIP_NO_ERROR;
        }

        // Actualities
        virtual CHIP_ERROR GetEntryCount(FabricIndex fabric, size_t & value) const
        {
            value = 0;
            return CHIP_NO_ERROR;
        }

        virtual CHIP_ERROR GetEntryCount(size_t & value) const
        {
            value = 0;
            return CHIP_NO_ERROR;
        }

        // Preparation
        virtual CHIP_ERROR PrepareEntry(Entry & entry) { return CHIP_NO_ERROR; }

        // CRUD
        virtual CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, FabricIndex * fabricIndex) { return CHIP_NO_ERROR; }
        virtual CHIP_ERROR ReadEntry(size_t index, Entry & entry, const FabricIndex * fabricIndex) const { return CHIP_NO_ERROR; }
        virtual CHIP_ERROR UpdateEntry(size_t index, const Entry & entry, const FabricIndex * fabricIndex) { return CHIP_NO_ERROR; }
        virtual CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex) { return CHIP_NO_ERROR; }

        // Iteration
        virtual CHIP_ERROR Entries(EntryIterator & iterator, const FabricIndex * fabricIndex) const { return CHIP_NO_ERROR; }

        // Check
        // Return CHIP_NO_ERROR if allowed, CHIP_ERROR_ACCESS_DENIED if denied,
        // CHIP_ERROR_NOT_IMPLEMENTED to use the default check algorithm (against entries),
        // or any other CHIP_ERROR if another error occurred.
        virtual CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                                 Privilege requestPrivilege)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }
    };

    AccessControl() = default;

    AccessControl(const AccessControl &) = delete;
    AccessControl & operator=(const AccessControl &) = delete;

    ~AccessControl()
    {
        // Never-initialized AccessControl instances will not have the delegate set.
        if (IsInitialized())
        {
            mDelegate->Release();
        }
    }

    /**
     * Initialize the access control module. Must be called before first use.
     *
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INCORRECT_STATE if called more than once,
     *         CHIP_ERROR_INVALID_ARGUMENT if delegate is null, or other fatal error.
     */
    CHIP_ERROR Init(AccessControl::Delegate * delegate, DeviceTypeResolver & deviceTypeResolver);

    /**
     * Deinitialize the access control module. Must be called when finished.
     */
    void Finish();

    // Capabilities
    CHIP_ERROR GetMaxEntriesPerFabric(size_t & value) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->GetMaxEntriesPerFabric(value);
    }

    CHIP_ERROR GetMaxSubjectsPerEntry(size_t & value) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->GetMaxSubjectsPerEntry(value);
    }

    CHIP_ERROR GetMaxTargetsPerEntry(size_t & value) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->GetMaxTargetsPerEntry(value);
    }

    CHIP_ERROR GetMaxEntryCount(size_t & value) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->GetMaxEntryCount(value);
    }

    // Actualities
    CHIP_ERROR GetEntryCount(FabricIndex fabric, size_t & value) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->GetEntryCount(fabric, value);
    }

    CHIP_ERROR GetEntryCount(size_t & value) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->GetEntryCount(value);
    }

    /**
     * Prepares an entry.
     *
     * An entry must be prepared or read (`ReadEntry`) before first use.
     *
     * @param [in] entry        Entry to prepare.
     */
    CHIP_ERROR PrepareEntry(Entry & entry)
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->PrepareEntry(entry);
    }

    /**
     * Creates an entry in the access control list.
     *
     * @param [in]  subjectDescriptor Optional subject descriptor for this operation.
     * @param [in]  fabric            Index of fabric in which to create entry.
     * @param [out] index             (If not nullptr) index of created entry (relative to fabric).
     * @param [in]  entry             Entry from which created entry is copied.
     */
    CHIP_ERROR CreateEntry(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t * index, const Entry & entry);

    /**
     * Creates an entry in the access control list.
     *
     * @param [out] index       Entry index of created entry, if not null. May be relative to `fabricIndex`.
     * @param [in]  entry       Entry from which to copy.
     * @param [out] fabricIndex Fabric index of created entry, if not null, in which case entry `index` will be relative to fabric.
     */
    CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, FabricIndex * fabricIndex = nullptr)
    {
        ReturnErrorCodeIf(!IsValid(entry), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->CreateEntry(index, entry, fabricIndex);
    }

    /**
     * Reads an entry in the access control list.
     *
     * @param [in] fabric            Index of fabric in which to read entry.
     * @param [in] index             Index of entry to read (relative to fabric).
     * @param [in] entry             Entry into which read entry is copied.
     */
    CHIP_ERROR ReadEntry(FabricIndex fabric, size_t index, Entry & entry) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->ReadEntry(index, entry, &fabric);
    }

    /**
     * Reads an entry from the access control list.
     *
     * @param [in]  index       Entry index of entry to read. May be relative to `fabricIndex`.
     * @param [out] entry       Entry into which to copy.
     * @param [in]  fabricIndex Fabric to which entry `index` is relative, if not null.
     */
    CHIP_ERROR ReadEntry(size_t index, Entry & entry, const FabricIndex * fabricIndex = nullptr) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->ReadEntry(index, entry, fabricIndex);
    }

    /**
     * Updates an entry in the access control list.
     *
     * @param [in] subjectDescriptor Optional subject descriptor for this operation.
     * @param [in] fabric            Index of fabric in which to update entry.
     * @param [in] index             Index of entry to update (relative to fabric).
     * @param [in] entry             Entry from which updated entry is copied.
     */
    CHIP_ERROR UpdateEntry(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index, const Entry & entry);

    /**
     * Updates an entry in the access control list.
     *
     * @param [in] index        Entry index of entry to update, if not null. May be relative to `fabricIndex`.
     * @param [in] entry        Entry from which to copy.
     * @param [in] fabricIndex  Fabric to which entry `index` is relative, if not null.
     */
    CHIP_ERROR UpdateEntry(size_t index, const Entry & entry, const FabricIndex * fabricIndex = nullptr)
    {
        ReturnErrorCodeIf(!IsValid(entry), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->UpdateEntry(index, entry, fabricIndex);
    }

    /**
     * Deletes an entry in the access control list.
     *
     * @param [in] subjectDescriptor Optional subject descriptor for this operation.
     * @param [in] fabric            Index of fabric in which to delete entry.
     * @param [in] index             Index of entry to delete (relative to fabric).
     */
    CHIP_ERROR DeleteEntry(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index);

    /**
     * Deletes an entry from the access control list.
     *
     * @param [in] index        Entry index of entry to delete. May be relative to `fabricIndex`.
     * @param [in] fabricIndex  Fabric to which entry `index` is relative, if not null.
     */
    CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex = nullptr)
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->DeleteEntry(index, fabricIndex);
    }

    /**
     * @brief Remove all ACL entries for the given fabricIndex
     *
     * @param[in] fabricIndex fabric index for which to remove all entries
     */
    CHIP_ERROR DeleteAllEntriesForFabric(FabricIndex fabricIndex)
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

        CHIP_ERROR stickyError = CHIP_NO_ERROR;

        // Remove access control entries in reverse order (it could be any order, but reverse order
        // will cause less churn in persistent storage).
        size_t aclCount = 0;
        if (GetEntryCount(fabricIndex, aclCount) == CHIP_NO_ERROR)
        {
            while (aclCount)
            {
                CHIP_ERROR err = DeleteEntry(nullptr, fabricIndex, --aclCount);
                stickyError    = (stickyError == CHIP_NO_ERROR) ? err : stickyError;
            }
        }

        return stickyError;
    }

    /**
     * Iterates over entries in the access control list.
     *
     * @param [in]  fabric   Fabric over which to iterate entries.
     * @param [out] iterator Iterator controlling the iteration.
     */
    CHIP_ERROR Entries(FabricIndex fabric, EntryIterator & iterator) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->Entries(iterator, &fabric);
    }

    /**
     * Iterates over entries in the access control list.
     *
     * @param [out] iterator    Iterator controlling the iteration.
     * @param [in]  fabricIndex Iteration is confined to fabric, if not null.
     */
    CHIP_ERROR Entries(EntryIterator & iterator, const FabricIndex * fabricIndex = nullptr) const
    {
        VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return mDelegate->Entries(iterator, fabricIndex);
    }

    // Adds a listener to the end of the listener list, if not already in the list.
    void AddEntryListener(EntryListener & listener);

    // Removes a listener from the listener list, if in the list.
    void RemoveEntryListener(EntryListener & listener);

    /**
     * Check whether access (by a subject descriptor, to a request path,
     * requiring a privilege) should be allowed or denied.
     *
     * @retval #CHIP_ERROR_ACCESS_DENIED if denied.
     * @retval other errors should also be treated as denied.
     * @retval #CHIP_NO_ERROR if allowed.
     */
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege requestPrivilege);

#if CHIP_ACCESS_CONTROL_DUMP_ENABLED
    CHIP_ERROR Dump(const Entry & entry);
#endif

private:
    bool IsInitialized() const { return (mDelegate != nullptr); }

    bool IsValid(const Entry & entry);

    void NotifyEntryChanged(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index, const Entry * entry,
                            EntryListener::ChangeType changeType);

private:
    Delegate * mDelegate = nullptr;

    DeviceTypeResolver * mDeviceTypeResolver = nullptr;

    EntryListener * mEntryListener = nullptr;
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
void ResetAccessControlToDefault();

} // namespace Access
} // namespace chip
