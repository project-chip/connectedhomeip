/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#include <AppMain.h>
#include <EnergyEvseMain.h>
#include <WaterHeaterMain.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/BitMask.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;
using namespace chip::app::Clusters::WaterHeaterManagement;

// Parse a hex (prefixed by 0x) or decimal (no-prefix) string
static uint32_t ParseNumber(const char * pString);

// Parses the --featureMap option
static bool EnergyAppOptionHandler(const char * aProgram, chip::ArgParser::OptionSet * aOptions, int aIdentifier,
                                   const char * aName, const char * aValue);

constexpr uint16_t kOptionApplication = 0xffd0;
constexpr uint16_t kOptionFeatureMap  = 0xffd1;

constexpr const char * kEvseApp = "evse";
constexpr const char * kWhmApp  = "water-heater";

constexpr const char * kValidApps[] = { kEvseApp, kWhmApp };

// Define the chip::ArgParser command line structures for extending the command line to support the
// energy apps
static chip::ArgParser::OptionDef sEnergyAppOptionDefs[] = {
    { "application", chip::ArgParser::kArgumentRequired, kOptionApplication },
    { "featureSet", chip::ArgParser::kArgumentRequired, kOptionFeatureMap },
    { nullptr }
};

static chip::ArgParser::OptionSet sCmdLineOptions = { EnergyAppOptionHandler, // handler function
                                                      sEnergyAppOptionDefs,   // array of option definitions
                                                      "PROGRAM OPTIONS",      // help group
                                                      "-a, --application <evse|water-heater>\n"
                                                      "-f, --featureSet <value>\n" };

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

// Keep track of the parsed featureMap option
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment, Feature::kPowerForecastReporting,
                                          Feature::kStateForecastReporting, Feature::kStartTimeAdjustment, Feature::kPausable,
                                          Feature::kForecastAdjustment, Feature::kConstraintBasedAdjustment);

// Make EVSE the default app
static const char * spApp = kEvseApp;

chip::BitMask<Feature> GetFeatureMapFromCmdLine()
{
    return sFeatureMap;
}

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

static uint32_t ParseNumber(const char * pString)
{
    uint32_t num = 0;
    if (strlen(pString) > 2 && pString[0] == '0' && pString[1] == 'x')
    {
        num = (uint32_t) strtoul(&pString[2], nullptr, 16);
    }
    else
    {
        num = (uint32_t) strtoul(pString, nullptr, 10);
    }

    return num;
}

void ApplicationInit()
{
    ChipLogDetail(AppServer, "Energy Management App: ApplicationInit()");
    if (strcmp(spApp, kEvseApp) == 0)
    {
        EvseApplicationInit();
    }
    else if (strcmp(spApp, kWhmApp) == 0)
    {
        FullWhmApplicationInit();
    }
    else
    {
        ChipLogError(Support, "Unexpected application %s", spApp);
    }
}

void ApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App: ApplicationShutdown()");

    EvseApplicationShutdown();
    FullWhmApplicationShutdown();
}

static bool EnergyAppOptionHandler(const char * aProgram, chip::ArgParser::OptionSet * aOptions, int aIdentifier,
                                   const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {
    case kOptionApplication:
        spApp = nullptr;
        for (size_t idx = 0; idx < (sizeof(kValidApps) / sizeof(kValidApps[0])); idx++)
        {
            if (strcmp(kValidApps[idx], aValue) == 0)
            {
                spApp = kValidApps[idx];
                break;
            }
        }

        if (spApp != nullptr)
        {
            ChipLogDetail(Support, "Running application %s", spApp);
        }
        else
        {
            retval = false;
        }
        break;
    case kOptionFeatureMap:
        sFeatureMap = BitMask<chip::app::Clusters::DeviceEnergyManagement::Feature>(ParseNumber(aValue));
        ChipLogDetail(Support, "Using FeatureMap 0x%04x", sFeatureMap.Raw());
        break;
    default:
        ChipLogError(Support, "%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv, &sCmdLineOptions) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}
