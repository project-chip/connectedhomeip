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
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>

namespace chip::app::Clusters {

CHIP_ERROR LocalizationConfigurationServerLogic::SetActiveLocale(CharSpan activeLocale)
{
    char validLocaleBuffer[35];
    MutableCharSpan validLocale(validLocaleBuffer, sizeof(validLocaleBuffer));

    CHIP_ERROR error = IsSupportedLocale(activeLocale, validLocale);
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }

    mActiveLocale.SetContent(activeLocale);
    return CHIP_NO_ERROR;
}

CharSpan LocalizationConfigurationServerLogic::GetActiveLocale()
{
    return mActiveLocale.Content();
}

CHIP_ERROR LocalizationConfigurationServerLogic::ReadSupportedLocales(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err                             = CHIP_NO_ERROR;
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it = provider->IterateSupportedLocales();
        if (it)
        {
            err = aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
                CharSpan activeLocale;

                while (it->Next(activeLocale))
                {
                    ReturnErrorOnFailure(encoder.Encode(activeLocale));
                }

                return CHIP_NO_ERROR;
            });

            it->Release();
        }
        else
        {
            err = aEncoder.EncodeEmptyList();
        }
    }
    else
    {
        err = aEncoder.EncodeEmptyList();
    }
    return err;
}

CHIP_ERROR LocalizationConfigurationServerLogic::IsSupportedLocale(CharSpan newLangtag, MutableCharSpan & validLocale)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it;
    bool firstValidLocale = false;
    if (provider && (it = provider->IterateSupportedLocales()))
    {
        CharSpan outLocale;

        while (it->Next(outLocale))
        {
            if (outLocale.data_equal(newLangtag))
            {
                it->Release();
                return CHIP_NO_ERROR;
            }
            if (!firstValidLocale)
            {
                CopyCharSpanToMutableCharSpan(outLocale, validLocale);
                firstValidLocale = true;
            }
        }

        it->Release();
    }

    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip::app::Clusters
