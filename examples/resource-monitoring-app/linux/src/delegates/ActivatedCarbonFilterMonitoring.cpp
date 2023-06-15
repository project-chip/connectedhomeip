#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>
#include <resource-monitoring-delegates.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::HepaFilterMonitoring;
using chip::Protocols::InteractionModel::Status;

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

//-- Hepa filter Monitoring delegate functions
CHIP_ERROR ActivatedCarbonFilterMonitoringDelegate::Init()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::Init()");
    return CHIP_NO_ERROR;
}

Status ActivatedCarbonFilterMonitoringDelegate::HandleResetCondition()
{
    ChipLogError(Zcl, "ActivatedCarbonFilterMonitoringDelegate::HandleResetCondition()");
    return Status::Success;
}
