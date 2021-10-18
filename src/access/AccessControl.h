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

#include "DataProvider.h"
#include "Privilege.h"
#include "RequestPath.h"
#include "SubjectDescriptor.h"

#include <lib/core/CHIPCore.h>

namespace chip {
namespace access {

class AccessControl
{
public:
    /**
     * Create an access control module. One is provided by default (see
     * GetInstance) but others can be created as needed (e.g. for testing). An
     * uninitialized DataProvider must be provided, and the module must then be
     * initialized before use, and deinitialized when finished.
     */
    AccessControl(DataProvider & dataProvider) : mDataProvider(dataProvider) {}

    AccessControl(const AccessControl &) = delete;
    AccessControl & operator=(const AccessControl &) = delete;

    /**
     * Initialize the access control module. Will also initialize its data
     * provider.
     *
     * @retval various errors, probably fatal.
     */
    CHIP_ERROR Init();

    /**
     * Deinitialize the access control module. Will also deinitialize its data
     * provider.
     */
    void Finish();

    /**
     * Check whether access (by a subject descriptor, to a request path,
     * requiring a privilege) should be allowed or denied.
     *
     * @retval #CHIP_ERROR_ACCESS_DENIED if denied.
     * @retval other errors should be treated as denied.
     * @retval #CHIP_NO_ERROR if allowed.
     */
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege privilege);

public:
    /**
     * Get the configured instance, for general use (i.e. non-testing). By
     * default an instance is preconfigured, but advanced use can configure
     * alternate instances, or even clear the configured instance.
     *
     * @retval nullptr if configured so.
     */
    static AccessControl * GetInstance() { return mInstance; }

    /**
     * Set the configured instance, for advanced use (e.g. testing). Does not
     * call Init or Finish (so ensure that happens appropriately). The
     * configured instance can be cleared (by setting to nullptr).
     */
    static void SetInstance(AccessControl * instance) { mInstance = instance; }

private:
    DataProvider & mDataProvider;

    static AccessControl * mInstance;
};

} // namespace access
} // namespace chip
