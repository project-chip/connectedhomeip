#include "CodeGenIntegration.h"

UnitLocalizationServer & UnitLocalizationCluster::Instance()
{
    static UnitLocalizationServer UnitLocalizationCluster::mInstance;
    return UnitLocalizationServer::mInstance;
}
