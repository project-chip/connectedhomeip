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

#include "application-basic/ApplicationBasicManager.h"

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

/** @brief Application Basic Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfApplicationBasicClusterInitCallback(chip::EndpointId endpoint)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    ApplicationBasicManager & aManager = ApplicationBasicManager::GetInstance();
    err                                = aManager.Init();
    if (CHIP_NO_ERROR == err)
    {
        Application application = aManager.getApplicationForEndpoint(endpoint);
        aManager.store(endpoint, &application);
    }
}
