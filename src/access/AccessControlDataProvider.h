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

#include "AuthMode.h"
#include "BasicTypes.h"
#include "Privilege.h"
#include "SubjectDescriptor.h"

#include <lib/core/CHIPCore.h>

namespace chip {
namespace Access {

class Entry
{
public:
    virtual ~Entry() = default;

    /**
     * Whether the auth mode matches the entry. Must be called before calling
     * MatchesSubject.
     */
    virtual bool MatchesAuthMode(AuthMode authMode) const = 0;

    /**
     * Whether the fabric matches the entry. Entries with fabric index 0 will
     * match all fabrics.
     */
    virtual bool MatchesFabric(FabricIndex fabricIndex) const = 0;

    /**
     * Whether the privilege matches the entry, including subsumed privileges.
     */
    virtual bool MatchesPrivilege(Privilege privilege) const = 0;

    /**
     * Whether the subject matches the entry. Must only be called if auth mode
     * matches.
     */
    virtual bool MatchesSubject(SubjectId subject) const = 0;

    /**
     * Whether the target matches the entry. Some entries may match all
     * endpoints or all clusters.
     */
    virtual bool MatchesTarget(EndpointId endpoint, ClusterId cluster) const = 0;
};

class EntryIterator
{
public:
    /**
     * Create an entry iterator. Must call release when finished.
     */
    EntryIterator() = default;

    virtual ~EntryIterator() = default;

    /**
     * Returns the next entry, or nullptr if there is no next entry.
     */
    virtual Entry * Next() = 0;

    /**
     * Release the iterator. Must be called when finished.
     */
    virtual void Release() = 0;
};

class AccessControlDataProvider
{
public:
    /**
     * Create a data provider. Must be initialized before use, and deinitialized
     * when finished.
     */
    AccessControlDataProvider() = default;

    virtual ~AccessControlDataProvider() = default;

    AccessControlDataProvider(const AccessControlDataProvider &) = delete;
    AccessControlDataProvider & operator=(const AccessControlDataProvider &) = delete;

    /**
     * Initialize the data provider.
     *
     * @retval various errors, probably fatal.
     */
    virtual CHIP_ERROR Init() = 0;

    /**
     * Deinitialize the data provider.
     */
    virtual void Finish() = 0;

    /**
     * Get an iterator over all entries.
     *
     * @retval iterator, release when finished.
     * @retval nullptr if error, probably fatal, generally should not happen.
     */
    virtual EntryIterator * Entries() const = 0;

    /**
     * Get an iterator over all entries for a particular fabric.
     *
     * @retval iterator, release when finished.
     * @retval nullptr if error, probably fatal, generally should not happen.
     */
    virtual EntryIterator * Entries(FabricIndex fabricIndex) const = 0;
};

} // namespace Access
} // namespace chip
