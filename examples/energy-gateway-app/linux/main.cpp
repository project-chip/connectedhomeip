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

#include <AppMain.h>
#include <EnergyGatewayAppCommonMain.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/BitMask.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

// Define the chip::ArgParser command line structures for extending the command line to support the
// energy apps
static bool EnergyGatewayAppOptionHandler(const char * aProgram, chip::ArgParser::OptionSet * aOptions, int aIdentifier,
                                          const char * aName, const char * aValue);

static chip::ArgParser::OptionDef sEnergyGatewayAppOptionDefs[] = { { nullptr } };

static chip::ArgParser::OptionSet sCmdLineOptions = { EnergyGatewayAppOptionHandler, // handler function
                                                      sEnergyGatewayAppOptionDefs,   // array of option definitions
                                                      "PROGRAM OPTIONS",             // help group
                                                      "\n" };

void ApplicationInit()
{
    ChipLogDetail(AppServer, "Energy Gateway App: ApplicationInit()");

    // If we are emulating Electrical Energy Tariff device type then call this
    // TODO consider how other clusters / endpoints should be used in this example app
    ElectricalEnergyTariffInit();
}

void ApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Gateway App: ApplicationShutdown()");

    ElectricalEnergyTariffShutdown();
}

static bool EnergyGatewayAppOptionHandler(const char * aProgram, chip::ArgParser::OptionSet * aOptions, int aIdentifier,
                                          const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {
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
