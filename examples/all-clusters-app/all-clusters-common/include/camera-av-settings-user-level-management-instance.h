#pragma once

#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-server.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

class AVSettingsUserLevelManagementDelegate : public Delegate
{
public:
    AVSettingsUserLevelManagementDelegate() {};

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    Protocols::InteractionModel::Status PersistentAttributesLoadedCallback() override;

    bool CanChangeMPTZ() override;

    /**
     * delegate command handlers
     */
    Protocols::InteractionModel::Status MPTZSetPosition(Optional<int16_t> pan, Optional<int16_t> tilt, Optional<uint8_t> zoom) override;
    Protocols::InteractionModel::Status MPTZRelativeMove() override;
    Protocols::InteractionModel::Status MPTZMoveToPreset() override;
    Protocols::InteractionModel::Status MPTZSavePreset() override;
    Protocols::InteractionModel::Status MPTZRemovePreset() override;
    Protocols::InteractionModel::Status DPTZSetViewport() override;
    Protocols::InteractionModel::Status DPTZRelativeMove() override;
};

CameraAvSettingsUserLevelMgmtServer * GetInstance();

void Shutdown();

} // namespace AVSettingsUserLevelManagementDelegate
} // namespace Clusters
} // namespace app
} // namespace chip
