//
// Created by Ding, Li-an on 10/21/21.
//
#include <app/clusters/mode-select-server/static-supported-modes-manager.h>
#include <map>
#include <vector>

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelectCluster;

using ModeOptionStructType = Structs::ModeOptionStruct::Type;
using storage_value_type   = const ModeOptionStructType *;

Structs::ModeOptionStruct::Type buildModeOptionStruct(const char * label, uint8_t mode, uint32_t semanticTag)
{
    Structs::ModeOptionStruct::Type option;
    option.label       = CharSpan(label, strlen(label));
    option.mode        = mode;
    option.semanticTag = semanticTag;
    return option;
}

const Structs::ModeOptionStruct::Type StaticSupportedModesManager::blackOption      = buildModeOptionStruct("Black", 0, 0);
const Structs::ModeOptionStruct::Type StaticSupportedModesManager::cappuccinoOption = buildModeOptionStruct("Cappuccino", 4, 0);
const Structs::ModeOptionStruct::Type StaticSupportedModesManager::espressoOption   = buildModeOptionStruct("Espresso", 7, 0);
storage_value_type StaticSupportedModesManager::coffeeOptions[] = { &blackOption, &cappuccinoOption, &espressoOption };
const Span<storage_value_type> StaticSupportedModesManager::coffeeOptionsSpan =
    Span<storage_value_type>(StaticSupportedModesManager::coffeeOptions, 3);
const map<EndpointId, Span<storage_value_type>> StaticSupportedModesManager::optionsByEndpoints = {
    { 0, StaticSupportedModesManager::coffeeOptionsSpan }
};

const StaticSupportedModesManager StaticSupportedModesManager::instance = StaticSupportedModesManager();

const StaticSupportedModesManager::IteratorFactory * StaticSupportedModesManager::getIteratorFactory(EndpointId endpointId) const
{
    const auto & it = _iteratorFactoriesByEndpoints.find(endpointId);
    return (it == _iteratorFactoriesByEndpoints.end()) ? nullptr : &(it->second);
}

EmberAfStatus StaticSupportedModesManager::getModeOptionByMode(unsigned short endpointId, unsigned char mode,
                                                               const ModeOptionStructType *& dataPtr) const
{
    auto * iteratorFactory = this->getIteratorFactory(endpointId);
    if (iteratorFactory == nullptr)
    {
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    }
    const StaticSupportedModesManager::Iterator & begin = *(this->getIteratorFactory(endpointId)->begin());
    const StaticSupportedModesManager::Iterator & end   = *(this->getIteratorFactory(endpointId)->end());
    for (auto it = begin; it != end; ++it)
    {
        auto & modeOption = *it;
        if (modeOption.mode == mode)
        {
            dataPtr = &modeOption;
            return EMBER_ZCL_STATUS_SUCCESS;
        }
    }
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Cannot find the mode %c", mode);
    return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
}
