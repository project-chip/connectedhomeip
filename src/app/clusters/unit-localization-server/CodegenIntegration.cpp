#include "CodegenIntegration.h"
#include <app/AttributeAccessInterfaceRegistry.h>

using namespace chip::app;
using namespace chip::app::Clusters::UnitLocalization;

UnitLocalizationServer & UnitLocalizationServer::Instance()
{
    static UnitLocalizationServer mInstance;
    return mInstance;
}

void MatterUnitLocalizationPluginServerInitCallback()
{
    TEMPORARY_RETURN_IGNORED UnitLocalizationServer::Instance().Init();
    AttributeAccessInterfaceRegistry::Instance().Register(&UnitLocalizationServer::Instance());
}

void MatterUnitLocalizationPluginServerShutdownCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(&UnitLocalizationServer::Instance());
}
