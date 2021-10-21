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

#include <map>
#include <vector>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/basic-types.h>

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;

namespace {
typedef ModeSelectCluster::Structs::ModeOptionStruct::Type ModeOptionStructType;
typedef unsigned long size_type;
} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelectCluster {

class SupportedModesManager
{

public:
    class Builder
    {
    public:
        Builder & addSupportedMode(EndpointId endpointId, ModeOptionStructType & modeOptionStruct);
        Builder & addSupportedMode(EndpointId endpointId, ModeOptionStructType && modeOptionStruct);
        SupportedModesManager build();

    private:
        map<EndpointId, vector<ModeOptionStructType> *> mSupportedModesByEndpoints;
    };

    const vector<ModeOptionStructType> getSupportedModesForEndpoint(EndpointId endpointId) const;

    EmberAfStatus getModeOptionByMode(EndpointId & endpointId, uint8_t & mode, const ModeOptionStructType *& dataPtr) const;

private:
    SupportedModesManager(map<EndpointId, const vector<ModeOptionStructType>> supportedModes) :
        _supportedModesForAllEndpoints(supportedModes)
    {}
    // TODO: Implement move constructor?

    // endpoint index -> **ModeOptionStruct;
    map<EndpointId, const vector<ModeOptionStructType>> _supportedModesForAllEndpoints;
};

SupportedModesManager::Builder &
chip::app::Clusters::ModeSelectCluster::SupportedModesManager::Builder::addSupportedMode(EndpointId endpointId,
                                                                                         ModeOptionStructType & modeOptionStruct)
{
    if (mSupportedModesByEndpoints.end() == mSupportedModesByEndpoints.find(endpointId))
    {
        mSupportedModesByEndpoints.insert(
            pair<uint8_t, vector<ModeOptionStructType> *>(endpointId, new vector<ModeOptionStructType>()));
    }
    mSupportedModesByEndpoints[endpointId]->push_back(modeOptionStruct);

    return *this;
}

SupportedModesManager::Builder &
chip::app::Clusters::ModeSelectCluster::SupportedModesManager::Builder::addSupportedMode(EndpointId endpointId,
                                                                                         ModeOptionStructType && modeOptionStruct)
{
    if (mSupportedModesByEndpoints.end() == mSupportedModesByEndpoints.find(endpointId))
    {
        mSupportedModesByEndpoints.insert(
            pair<uint8_t, vector<ModeOptionStructType> *>(endpointId, new vector<ModeOptionStructType>()));
    }
    mSupportedModesByEndpoints[endpointId]->push_back(forward<ModeOptionStructType>(modeOptionStruct));

    return *this;
}

SupportedModesManager chip::app::Clusters::ModeSelectCluster::SupportedModesManager::Builder::build()
{
    map<EndpointId, const vector<ModeOptionStructType>> supportedOptions;
    for (map<EndpointId, vector<ModeOptionStructType> *>::iterator it = mSupportedModesByEndpoints.begin();
         it != mSupportedModesByEndpoints.end(); ++it)
    {
        EndpointId endpointId = it->first;

        const vector<ModeOptionStructType> modeOptionsForEndpoint(*(it->second));
        supportedOptions.insert(pair<EndpointId, const vector<ModeOptionStructType>>(endpointId, modeOptionsForEndpoint));
    }

    return SupportedModesManager(supportedOptions);
}

const vector<ModeOptionStructType>
chip::app::Clusters::ModeSelectCluster::SupportedModesManager::getSupportedModesForEndpoint(EndpointId endpoint) const
{
    return _supportedModesForAllEndpoints.at(endpoint);
};

EmberAfStatus chip::app::Clusters::ModeSelectCluster::SupportedModesManager::getModeOptionByMode(
    unsigned short & endpointId, unsigned char & mode, const ModeOptionStructType *& dataPtr) const
{
    const vector<ModeOptionStructType> & supportedModeOptions = this->getSupportedModesForEndpoint(endpointId);
    for (uint i = 0u; i < supportedModeOptions.size(); i++)
    {
        if (supportedModeOptions.at(i).mode == mode)
        {
            dataPtr = &(supportedModeOptions.at(i));
            return EMBER_ZCL_STATUS_SUCCESS;
        }
    }
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Cannot find the mode %c", mode);
    return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
}

} // namespace ModeSelectCluster
} // namespace Clusters
} // namespace app
} // namespace chip
