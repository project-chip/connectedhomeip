//
// Created by Ding, Li-an on 10/21/21.
//
#include <app/clusters/mode-select-server/heap-based-supported-modes-manager.h>
#include <vector>
#include <map>

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelectCluster;

using ModeOptionStructType = Structs::ModeOptionStruct::Type;

HeapBasedSupportedModesManager::Builder &
HeapBasedSupportedModesManager::Builder::addSupportedMode(EndpointId endpointId, ModeOptionStructType & modeOptionStruct)
{
    if (mSupportedModesByEndpoints.end() == mSupportedModesByEndpoints.find(endpointId))
    {
        mSupportedModesByEndpoints.insert(
            pair<uint8_t, vector<ModeOptionStructType> *>(endpointId, new vector<ModeOptionStructType>()));
    }
    mSupportedModesByEndpoints[endpointId]->push_back(modeOptionStruct);

    return *this;
}

HeapBasedSupportedModesManager::Builder &
HeapBasedSupportedModesManager::Builder::addSupportedMode(EndpointId endpointId,
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

HeapBasedSupportedModesManager ModeSelectCluster::HeapBasedSupportedModesManager::Builder::build()
{
    map<EndpointId, const vector<ModeOptionStructType>> supportedOptions;
    for (map<EndpointId, vector<ModeOptionStructType> *>::iterator it = mSupportedModesByEndpoints.begin();
        it != mSupportedModesByEndpoints.end(); ++it)
    {
        EndpointId endpointId = it->first;

        const vector<ModeOptionStructType> modeOptionsForEndpoint(*(it->second));
        supportedOptions.insert(pair<EndpointId, const vector<ModeOptionStructType>>(endpointId, modeOptionsForEndpoint));
    }

    return HeapBasedSupportedModesManager(supportedOptions);
}

const vector<ModeOptionStructType>
HeapBasedSupportedModesManager::getSupportedModesForEndpoint(EndpointId endpoint) const
{
    return _supportedModesForAllEndpoints.at(endpoint);
};

EmberAfStatus HeapBasedSupportedModesManager::getModeOptionByMode(
    unsigned short endpointId, unsigned char mode, const ModeOptionStructType *& dataPtr) const
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
