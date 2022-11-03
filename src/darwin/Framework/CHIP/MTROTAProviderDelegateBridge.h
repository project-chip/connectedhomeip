/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTROTAProviderDelegate.h"

#include <app/clusters/ota-provider/ota-provider-delegate.h>

NS_ASSUME_NONNULL_BEGIN

class MTROTAProviderDelegateBridge : public chip::app::Clusters::OTAProviderDelegate
{
public:
    MTROTAProviderDelegateBridge(id<MTROTAProviderDelegate> delegate);
    ~MTROTAProviderDelegateBridge();

    CHIP_ERROR Init(chip::System::Layer * systemLayer, chip::Messaging::ExchangeManager * exchangeManager);
    void Shutdown();

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
        MTROtaSoftwareUpdateProviderClusterQueryImageParams * commandParams);
    static void ConvertFromQueryImageResponseParms(
        const MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * responseParams,
        chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Type & response);
    static void ConvertToApplyUpdateRequestParams(
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData,
        MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * commandParams);
    static void ConvertFromApplyUpdateRequestResponseParms(
        const MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * responseParams,
        chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Type & response);
    static void ConvertToNotifyUpdateAppliedParams(
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData,
        MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * commandParams);

    _Nullable id<MTROTAProviderDelegate> mDelegate;
    dispatch_queue_t mWorkQueue;
};

NS_ASSUME_NONNULL_END
