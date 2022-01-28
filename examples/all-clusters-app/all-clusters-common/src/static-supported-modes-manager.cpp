#include <static-supported-modes-manager.h>

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;

using ModeOptionStructType = Structs::ModeOptionStruct::Type;
using storage_value_type   = const ModeOptionStructType;
namespace {
Structs::ModeOptionStruct::Type buildModeOptionStruct(const char * label, uint8_t mode, uint32_t semanticTag)
{
    Structs::ModeOptionStruct::Type option;
    option.label       = CharSpan::fromCharString(label);
    option.mode        = mode;
    option.semanticTag = semanticTag;
    return option;
}
} // namespace

// TODO: Configure your options for each endpoint
storage_value_type StaticSupportedModesManager::coffeeOptions[] = { buildModeOptionStruct("Black", 0, 0),
                                                                    buildModeOptionStruct("Cappuccino", 4, 0),
                                                                    buildModeOptionStruct("Espresso", 7, 0) };
const StaticSupportedModesManager::EndpointSpanPair
    StaticSupportedModesManager::supportedOptionsByEndpoints[EMBER_AF_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT] = {
        EndpointSpanPair(1, Span<storage_value_type>(StaticSupportedModesManager::coffeeOptions)) // Options for Endpoint 1
    };

const StaticSupportedModesManager StaticSupportedModesManager::instance = StaticSupportedModesManager();

const SupportedModesManager::ModeOptionsProvider StaticSupportedModesManager::getModeOptionsProvider(EndpointId endpointId) const
{
    for (auto & endpointSpanPair : supportedOptionsByEndpoints)
    {
        if (endpointSpanPair.mEndpointId == endpointId)
        {
            return ModeOptionsProvider(endpointSpanPair.mSpan.data(), endpointSpanPair.mSpan.end());
        }
    }
    return ModeOptionsProvider(nullptr, nullptr);
}

EmberAfStatus StaticSupportedModesManager::getModeOptionByMode(unsigned short endpointId, unsigned char mode,
                                                               const ModeOptionStructType ** dataPtr) const
{
    auto modeOptionsProvider = this->getModeOptionsProvider(endpointId);
    if (modeOptionsProvider.begin() == nullptr)
    {
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    }
    auto * begin = this->getModeOptionsProvider(endpointId).begin();
    auto * end   = this->getModeOptionsProvider(endpointId).end();

    for (auto * it = begin; it != end; ++it)
    {
        auto & modeOption = *it;
        if (modeOption.mode == mode)
        {
            *dataPtr = &modeOption;
            return EMBER_ZCL_STATUS_SUCCESS;
        }
    }
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "Cannot find the mode %" PRIu8, mode);
    return EMBER_ZCL_STATUS_INVALID_VALUE;
}

const ModeSelect::SupportedModesManager * ModeSelect::getSupportedModesManager()
{
    return &StaticSupportedModesManager::instance;
}
