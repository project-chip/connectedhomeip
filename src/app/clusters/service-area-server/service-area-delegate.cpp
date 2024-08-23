#include "service-area-delegate.h"
#include "service-area-server.h"

using namespace chip::app::Clusters::ServiceArea;

void Delegate::HandleSupportedAreasUpdated()
{
    mInstance->ClearSelectedAreas();
    mInstance->SetCurrentArea(DataModel::NullNullable);
    mInstance->ClearProgress();
}
