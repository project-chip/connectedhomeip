//
// Created by Ding, Li-an on 10/21/21.
//
#include <app/clusters/mode-select-server/heap-based-supported-modes-manager.h>
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

const Structs::ModeOptionStruct::Type HeapBasedSupportedModesManager::blackOption      = buildModeOptionStruct("Black", 0, 0);
const Structs::ModeOptionStruct::Type HeapBasedSupportedModesManager::cappuccinoOption = buildModeOptionStruct("Cappuccino", 4, 0);
const Structs::ModeOptionStruct::Type HeapBasedSupportedModesManager::espressoOption   = buildModeOptionStruct("Espresso", 7, 0);
storage_value_type HeapBasedSupportedModesManager::coffeeOptions[] = { &blackOption, &cappuccinoOption, &espressoOption };
const Span<storage_value_type> HeapBasedSupportedModesManager::coffeeOptionsSpan =
    Span<storage_value_type>(HeapBasedSupportedModesManager::coffeeOptions, 3);
const map<EndpointId, Span<storage_value_type>> HeapBasedSupportedModesManager::optionsByEndpoints = {
    { 0, HeapBasedSupportedModesManager::coffeeOptionsSpan }
};

const HeapBasedSupportedModesManager HeapBasedSupportedModesManager::instance = HeapBasedSupportedModesManager();

const HeapBasedSupportedModesManager::IteratorFactory *
HeapBasedSupportedModesManager::getIteratorFactory(EndpointId endpointId) const
{
    return &(_iteratorFactoriesByEndpoints.at(endpointId));
}

EmberAfStatus HeapBasedSupportedModesManager::getModeOptionByMode(unsigned short endpointId, unsigned char mode,
                                                                  const ModeOptionStructType *& dataPtr) const
{
    const HeapBasedSupportedModesManager::Iterator & begin = *(this->getIteratorFactory(endpointId)->begin());
    const HeapBasedSupportedModesManager::Iterator & end   = *(this->getIteratorFactory(endpointId)->end());
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
