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

#import "MTROTAProviderDelegate.h"

#include <app/clusters/ota-provider/ota-provider-delegate.h>

NS_ASSUME_NONNULL_BEGIN

class MTROTAProviderDelegateBridge : public chip::app::Clusters::OTAProviderDelegate
{
public:
    MTROTAProviderDelegateBridge();
    ~MTROTAProviderDelegateBridge();

    void setDelegate(id<MTROTAProviderDelegate> delegate, dispatch_queue_t queue);

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
    void ConvertToQueryImageParams(
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData,
        MTROtaSoftwareUpdateProviderClusterQueryImageParams * commandParams);
    void ConvertFromQueryImageResponseParms(
        const MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * responseParams,
        chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Type & response);
    void ConvertToApplyUpdateRequestParams(
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData,
        MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * commandParams);
    void ConvertFromApplyUpdateRequestResponseParms(
        const MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * responseParams,
        chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Type & response);
    void ConvertToNotifyUpdateAppliedParams(
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData,
        MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * commandParams);

    _Nullable id<MTROTAProviderDelegate> mDelegate;
    _Nullable dispatch_queue_t mQueue;
};

NS_ASSUME_NONNULL_END
