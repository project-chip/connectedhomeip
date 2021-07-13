#pragma once

#include <type_traits>
#include <array>
#include <device/SchemaTypes.h>
#include <basic-types.h>

namespace chip {
namespace app {
namespace Cluster {
namespace NetworkCommissioningCluster {
    constexpr chip::ClusterId kClusterId = 0x0031;

    enum CommandId {
        kScanNetworksRequestCommandId = 0,
        kScanNetworksRespCommandId = 1,
        kAddWifiNetworkRequestCommandId = 2,
        kAddWifiNetworkResponseCommandId = 3,
        kUpdateWifiNetworkRequestCommandId = 4,
        kUpdateWifiNetworkResponseCommandId = 5,
        kAddThreadNetworkRequestCommandId = 6,
        kAddThreadNetworkResponseCommandId = 7,
        kUpdateThreadNetworkRequestCommandId = 8,
        kUpdateThreadNetworkResponseCommandId = 9,
        kRemoveNetworkRequestCommandId = 10,
        kRemoveNetworkResponseCommandId = 11,
        kEnableNetworkRequestCommandId = 12,
        kEnableNetworkResponseCommandId = 13,
        kDisableNetworkRequestCommandId = 14,
        kDisableNetworkResponseCommandId = 15,
        kGetLastNetworkCommissioningResultRequestCommandId = 16
    };

    namespace AddThreadNetworkCommand {
        enum FieldId {
            kOperationalDatasetFieldId = 0,
            kBreadcrumbFieldId = 1,
            kTimeoutMsFieldId = 2
        };

        constexpr FullFieldDescriptor _Schema[] = {
            {kOperationalDatasetFieldId,    BitFlags<Type>(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags<Type>(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags<Type>(Type::TYPE_UINT32),      kNone,   0,       {}}
        };
    };

    namespace AddWifiNetworkCommand {
        enum FieldId {
            kSsidFieldId = 0,
            kCredentialsFieldId = 1,
            kBreadcrumbFieldId = 2,
            kTimeoutMsFieldId = 3
        };

        constexpr FullFieldDescriptor _Schema[] = {
            {kSsidFieldId,                  BitFlags<Type>(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kCredentialsFieldId,           BitFlags<Type>(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags<Type>(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags<Type>(Type::TYPE_UINT32),      kNone,   0,       {}}
        };
    };

    namespace DisableNetworkCommand {
        enum FieldId {
            kNetworkId = 0,
            kBreadcrumbFieldId = 1,
            kTimeoutMsFieldId = 2
        };

        constexpr FullFieldDescriptor _Schema[] = {
            {kNetworkId,                    BitFlags<Type>(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags<Type>(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags<Type>(Type::TYPE_UINT32),      kNone,   0,       {}}
        };
    };

    namespace EnableNetworkCommand {
        enum FieldId {
            kNetworkId = 0,
            kBreadcrumbFieldId = 1,
            kTimeoutMsFieldId = 2
        };

        constexpr FullFieldDescriptor _Schema[] = {
            {kNetworkId,                    BitFlags<Type>(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags<Type>(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags<Type>(Type::TYPE_UINT32),      kNone,   0,       {}}
        };
    };

    namespace RemoveNetworkCommand {
        enum FieldId {
            kNetworkId = 0,
            kBreadcrumbFieldId = 1,
            kTimeoutMsFieldId = 2
        };

        constexpr FullFieldDescriptor _Schema[] = {
            {kNetworkId,                    BitFlags<Type>(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags<Type>(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags<Type>(Type::TYPE_UINT32),      kNone,   0,       {}}
        };
    };


    namespace GetLastNetworkCommissioningResultCommand {
        enum FieldId {
            kTimeoutMsFieldId = 0
        };

        constexpr FullFieldDescriptor _Schema[] = {
            {kTimeoutMsFieldId,             BitFlags<Type>(Type::TYPE_UINT32),      kNone,   0,       {}}
        };
    };
}
}
}
}
