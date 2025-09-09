/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/localization-configuration-server/localization-configuration-logic.h>
#include <platform/PlatformManager.h>

namespace chip::app::Clusters {

Protocols::InteractionModel::Status LocalizationConfigurationServerLogic::SetActiveLocale(CharSpan activeLocale)
{
    if (!IsSupportedLocale(activeLocale))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    mActiveLocale.SetContent(activeLocale);
    return Protocols::InteractionModel::Status::Success;
}

CharSpan LocalizationConfigurationServerLogic::GetActiveLocale()
{
    return mActiveLocale.Content();
}

CHIP_ERROR LocalizationConfigurationServerLogic::ReadSupportedLocales(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err                                                 = CHIP_NO_ERROR;
    DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it = mDeviceInfoProvider.IterateSupportedLocales();
    if (it)
    {
        err = aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
            CharSpan supportedLocale;

            while (it->Next(supportedLocale))
            {
                ReturnErrorOnFailure(encoder.Encode(supportedLocale));
            }

            return CHIP_NO_ERROR;
        });

        it->Release();
    }
    else
    {
        err = aEncoder.EncodeEmptyList();
    }
    return err;
}

bool LocalizationConfigurationServerLogic::IsSupportedLocale(CharSpan newLangTag)
{
    DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it;
    if ((it = mDeviceInfoProvider.IterateSupportedLocales()))
    {
        CharSpan outLocale;

        while (it->Next(outLocale))
        {
            if (outLocale.data_equal(newLangTag))
            {
                it->Release();
                return true;
            }
        }

        it->Release();
    }

    return false;
}

} // namespace chip::app::Clusters
