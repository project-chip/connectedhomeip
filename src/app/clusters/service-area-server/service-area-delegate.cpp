#include "service-area-delegate.h"
#include "service-area-server.h"

using namespace chip::app::Clusters::ServiceArea;

/**
 * When we cannot guarantee that the restrictions imposed on the SupportedLocations, CurrentLocation and Progress attributes
 * can be upheld, these attributes are set to null.
 *
 * The user is free the redefine this method as their device may have more information on what has changed and may be able to
 * maintain the restrictions on these attributes by selectively editing them.
 */
void Delegate::HandleSupportedLocationsUpdated()  {
    mInstance->ClearSelectedLocations();
    mInstance->SetCurrentLocation(DataModel::Nullable<uint32_t>());
    mInstance->ClearProgress();
};
