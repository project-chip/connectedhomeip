/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/MTROTAProviderDelegate.h>

#include <app/clusters/ota-provider/ota-provider-delegate.h>

NS_ASSUME_NONNULL_BEGIN

class MTROTAProviderDelegateBridge : public chip::app::Clusters::OTAProviderDelegate
{
public:
    MTROTAProviderDelegateBridge();
    ~MTROTAProviderDelegateBridge();

    CHIP_ERROR Init(chip::System::Layer * systemLayer, chip::Messaging::ExchangeManager * exchangeManager);

    // Shutdown must be called after the event loop has been stopped, since it
    // touches Matter objects.
    void Shutdown();

    // ControllerShuttingDown must be called on the Matter work queue, since it
    // touches Matter objects.
    void ControllerShuttingDown(MTRDeviceController * controller);

    void HandleQueryImage(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData) override;

    void HandleApplyUpdateRequest(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData) override;

    void HandleNotifyUpdateApplied(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData) override;

private:
    static CHIP_ERROR ConvertToQueryImageParams(
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * commandParams);
    static void ConvertFromQueryImageResponseParams(
        const MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * responseParams,
        chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Type & response);
    static void ConvertToApplyUpdateRequestParams(
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData,
        MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams * commandParams);
    static void ConvertFromApplyUpdateRequestResponseParms(
        const MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * responseParams,
        chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Type & response);
    static void ConvertToNotifyUpdateAppliedParams(
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData,
        MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams * commandParams);
};

NS_ASSUME_NONNULL_END
