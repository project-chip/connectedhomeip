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

#include "AccessControlDataProvider.h"
#include "Privilege.h"
#include "RequestPath.h"
#include "SubjectDescriptor.h"

#include <lib/core/CHIPCore.h>

namespace chip {
namespace Access {

class AccessControl
{
public:
    /**
     * Create an access control module. This module must be initialized before
     * first use, and deinitialized when finished.
     */
    AccessControl(AccessControlDataProvider & dataProvider) : mDataProvider(dataProvider) {}

    AccessControl(const AccessControl &) = delete;
    AccessControl & operator=(const AccessControl &) = delete;

    /**
     * Initialize the access control module. Must be called before first use.
     *
     * @retval various errors, probably fatal.
     */
    CHIP_ERROR Init();

    /**
     * Deinitialize the access control module. Must be called when finished.
     */
    void Finish();

    /**
     * Check whether access (by a subject descriptor, to a request path,
     * requiring a privilege) should be allowed or denied.
     *
     * @retval #CHIP_ERROR_ACCESS_DENIED if denied.
     * @retval other errors should also be treated as denied.
     * @retval #CHIP_NO_ERROR if allowed.
     */
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege privilege);

private:
    AccessControlDataProvider & mDataProvider;
};

/**
 * Instance getter for the global AccessControl.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global AccessControl instance. Assume never null.
 */
AccessControl * GetAccessControl();

/**
 * Instance setter for the global AccessControl.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @param[in] accessControl the instance to start returning with the getter;
 *                          if nullptr, no change occurs.
 */
void SetAccessControl(AccessControl * accessControl);

} // namespace Access
} // namespace chip
