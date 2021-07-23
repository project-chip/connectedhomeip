#pragma once

#include "NetworkCommissioningCluster.h"
#include <array>
#include <type_traits>

namespace chip {
namespace app {
namespace Cluster {
namespace NetworkCommissioningCluster {
namespace AddThreadNetworkCommand {
constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
extern const StructDescriptor<NumImplementedFields> Descriptor;

struct Type
{
    chip::ByteSpan OperationalDataSet;
    uint64_t Breadcrumb;
    uint32_t TimeoutMs;

    chip::ClusterId GetClusterId() { return kClusterId; }
    chip::CommandId GetCommandId() { return kAddThreadNetworkRequestCommandId; }

    static const StructDescriptor<NumImplementedFields> & mDescriptor;
};
} // namespace AddThreadNetworkCommand

namespace AddWifiNetworkCommand {
constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
extern const StructDescriptor<NumImplementedFields> Descriptor;

struct Type
{
    chip::ByteSpan Ssid;
    chip::ByteSpan Credentials;
    uint64_t Breadcrumb;
    uint32_t TimeoutMs;

    static const StructDescriptor<NumImplementedFields> & mDescriptor;
};
} // namespace AddWifiNetworkCommand

namespace EnableNetworkCommand {
constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
extern const StructDescriptor<NumImplementedFields> Descriptor;

struct Type
{
    chip::ByteSpan NetworkId;
    uint64_t Breadcrumb;
    uint32_t TimeoutMs;

    static const StructDescriptor<NumImplementedFields> & mDescriptor;
};
} // namespace EnableNetworkCommand
} // namespace NetworkCommissioningCluster
} // namespace Cluster
} // namespace app
} // namespace chip
