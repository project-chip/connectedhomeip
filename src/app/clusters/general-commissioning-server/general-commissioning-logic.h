#pragma once

#include <app/AttributeValueDecoder.h>
#include <app/CommandHandler.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <lib/core/DataModelTypes.h>

#include <app/AttributeValueEncoder.h>
#include <clusters/GeneralCommissioning/Enums.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>

#include <app/CommandHandler.h>
#include <app/reporting/reporting.h>

#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/server/Server.h>
#include <platform/DeviceControlServer.h>

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
#include <app/server/TermsAndConditionsManager.h>  //nogncheck
#include <app/server/TermsAndConditionsProvider.h> //nogncheck
#endif

using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;

namespace {

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
typedef struct sTermsAndConditionsState
{
    Optional<TermsAndConditions> acceptance;
    bool acknowledgementsRequired;
    Optional<TermsAndConditions> requirements;
    Optional<uint32_t> updateAcceptanceDeadline;
} TermsAndConditionsState;

CHIP_ERROR GetTermsAndConditionsAttributeState(TermsAndConditionsProvider * tcProvider,
                                               TermsAndConditionsState & outTermsAndConditionsState)
{
    TermsAndConditionsState termsAndConditionsState;

    ReturnErrorOnFailure(tcProvider->GetAcceptance(termsAndConditionsState.acceptance));
    ReturnErrorOnFailure(tcProvider->GetAcknowledgementsRequired(termsAndConditionsState.acknowledgementsRequired));
    ReturnErrorOnFailure(tcProvider->GetRequirements(termsAndConditionsState.requirements));
    ReturnErrorOnFailure(tcProvider->GetUpdateAcceptanceDeadline(termsAndConditionsState.updateAcceptanceDeadline));

    outTermsAndConditionsState = termsAndConditionsState;
    return CHIP_NO_ERROR;
}

void NotifyTermsAndConditionsAttributeChangeIfRequired(const TermsAndConditionsState & initialState,
                                                       const TermsAndConditionsState & updatedState)
{
    // Notify on TCAcknowledgementsRequired change
    if (initialState.acknowledgementsRequired != updatedState.acknowledgementsRequired)
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCAcknowledgementsRequired::Id);
    }

    // Notify on TCAcceptedVersion change
    if ((initialState.acceptance.HasValue() != updatedState.acceptance.HasValue()) ||
        (initialState.acceptance.HasValue() &&
         (initialState.acceptance.Value().GetVersion() != updatedState.acceptance.Value().GetVersion())))
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCAcceptedVersion::Id);
    }

    // Notify on TCAcknowledgements change
    if ((initialState.acceptance.HasValue() != updatedState.acceptance.HasValue()) ||
        (initialState.acceptance.HasValue() &&
         (initialState.acceptance.Value().GetValue() != updatedState.acceptance.Value().GetValue())))
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCAcknowledgements::Id);
    }

    // Notify on TCRequirements change
    if ((initialState.requirements.HasValue() != updatedState.requirements.HasValue()) ||
        (initialState.requirements.HasValue() &&
         (initialState.requirements.Value().GetVersion() != updatedState.requirements.Value().GetVersion() ||
          initialState.requirements.Value().GetValue() != updatedState.requirements.Value().GetValue())))
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCMinRequiredVersion::Id);
    }

    // Notify on TCUpdateDeadline change
    if ((initialState.updateAcceptanceDeadline.HasValue() != updatedState.updateAcceptanceDeadline.HasValue()) ||
        (initialState.updateAcceptanceDeadline.HasValue() &&
         (initialState.updateAcceptanceDeadline.Value() != updatedState.updateAcceptanceDeadline.Value())))
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GeneralCommissioning::Id, TCUpdateDeadline::Id);
    }
}
#endif
class GeneralCommissioningFabricTableDelegate : public chip::FabricTable::Delegate
{
public:
    // Gets called when a fabric is deleted
    void OnFabricRemoved(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex)
    {
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        // If the FabricIndex matches the last remaining entry in the Fabrics list, then the device SHALL delete all Matter
        // related data on the node which was created since it was commissioned.
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            ChipLogProgress(Zcl, "general-commissioning-server: Last Fabric index 0x%x was removed",
                            static_cast<unsigned>(fabricIndex));

            TermsAndConditionsProvider * tcProvider = TermsAndConditionsManager::GetInstance();
            TermsAndConditionsState initialState, updatedState;
            VerifyOrReturn(nullptr != tcProvider);
            VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, initialState));
            VerifyOrReturn(CHIP_NO_ERROR == tcProvider->ResetAcceptance());
            VerifyOrReturn(CHIP_NO_ERROR == GetTermsAndConditionsAttributeState(tcProvider, updatedState));
            NotifyTermsAndConditionsAttributeChangeIfRequired(initialState, updatedState);
        }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    }
};
} // namespace

namespace chip {
namespace app {
namespace Clusters {

namespace GeneralCommissioning {

void SetBreadcrumb(Attributes::Breadcrumb::TypeInfo::Type breadcrumb);

} // namespace GeneralCommissioning

class GeneralCommissioningLogic
{
public:
    GeneralCommissioningLogic(BitFlags<GeneralCommissioning::Feature> featureFlags) : mFeatureFlags(featureFlags) {}
    ~GeneralCommissioningLogic() {}

    CHIP_ERROR ReadIfSupported(CHIP_ERROR (ConfigurationManager::*getter)(uint8_t &), AttributeValueEncoder & aEncoder)
    {
        uint8_t data   = 0;
        CHIP_ERROR err = (DeviceLayer::ConfigurationMgr().*getter)(data);
        if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            data = 0;
        }
        else if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        return aEncoder.Encode(data);
    }

    CHIP_ERROR ReadBasicCommissioningInfo(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportsConcurrentConnection(AttributeValueEncoder & aEncoder);
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    CHIP_ERROR ReadTCAcceptedVersion(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadTCMinRequiredVersion(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadTCAcknowledgements(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadTCAcknowledgementsRequired(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadTCUpdateDeadline(AttributeValueEncoder & aEncoder);
#endif

    std::optional<DataModel::ActionReturnStatus>
    HandleArmFailSafe(CommandHandler * commandObj, const ConcreteCommandPath & path,
                      const GeneralCommissioning::Commands::ArmFailSafe::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleCommissioningComplete(CommandHandler * commandObj, const ConcreteCommandPath & path, FabricIndex fabricIndex,
                                const GeneralCommissioning::Commands::CommissioningComplete::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetRegulatoryConfig(CommandHandler * commandObj, const ConcreteCommandPath & path,
                              const GeneralCommissioning::Commands::SetRegulatoryConfig::DecodableType & commandData);

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    std::optional<DataModel::ActionReturnStatus>
    HandleSetTCAcknowledgements(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                const GeneralCommissioning::Commands::SetTCAcknowledgements::DecodableType & commandData);
#endif

    const BitFlags<GeneralCommissioning::Feature> & GetFeatureFlags() const { return mFeatureFlags; }
    static void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

private:
    const BitFlags<GeneralCommissioning::Feature> mFeatureFlags;
};

} // namespace Clusters
} // namespace app
} // namespace chip
