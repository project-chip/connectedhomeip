/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <ElectricalGridConditionsDelegate.h>

#include <app/clusters/electrical-grid-conditions-server/electrical-grid-conditions-server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalGridConditions;
using namespace chip::app::Clusters::ElectricalGridConditions::Attributes;
using namespace chip::app::Clusters::ElectricalGridConditions::Structs;

CHIP_ERROR ElectricalGridConditionsInstance::Init()
{
    return Instance::Init();
}

void ElectricalGridConditionsInstance::Shutdown()
{
    Instance::Shutdown();
}

CHIP_ERROR ElectricalGridConditionsInstance::AppInit()
{
    DataModel::Nullable<bool> localGenerationAvailable;

    localGenerationAvailable.SetNonNull(true);
    ReturnErrorOnFailure(Instance::SetLocalGenerationAvailable(localGenerationAvailable));

    /* Manufacturers can optionally populate the CurrentConditions and
       ForecastConditions attributes here, or set up some logic to do this periodically */

    return CHIP_NO_ERROR;
}

ElectricalGridConditionsDelegate::ElectricalGridConditionsDelegate() {}
