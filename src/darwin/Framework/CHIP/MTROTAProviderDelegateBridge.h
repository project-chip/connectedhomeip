/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
