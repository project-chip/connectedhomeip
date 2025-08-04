/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include <platform/ESP32/ESP32DeviceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

class StaticESP32DeviceInfoProvider : public ESP32DeviceInfoProvider
{
public:
    StaticESP32DeviceInfoProvider() = default;
    ~StaticESP32DeviceInfoProvider() override {}

    // Iterators
    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint);
    SupportedLocalesIterator * IterateSupportedLocales();
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes();

    static StaticESP32DeviceInfoProvider & GetDefaultInstance();

    struct FixedLabelEntry
    {
        EndpointId endpointId;
        CharSpan label;
        CharSpan value;
    };

    /**
     * @brief API to set the supported calendar types
     *
     * @param[in] supportedCalendarTypes Span of type chip::app::Clusters::TimeFormatLocalization::CalendarTypeEnum
     *              containing the supported calendar types. The underlying data must remain allocated throughout
     *              the lifetime of the device, as the API does not make a copy.
     *
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    CHIP_ERROR SetSupportedCalendarTypes(const Span<CalendarType> & supportedCalendarTypes)
    {
        VerifyOrReturnError(!supportedCalendarTypes.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        mSupportedCalendarTypes = supportedCalendarTypes;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief API to set the supported Locales
     *
     * @param[in] supportedLocales Span of type chip::CharSpan containing the supported locales.
     *              The underlying data must remain allocated throughout the lifetime of the device,
     *              as the API does not make a copy.
     *
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    CHIP_ERROR SetSupportedLocales(const Span<CharSpan> & supportedLocales)
    {
        VerifyOrReturnError(!supportedLocales.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        mSupportedLocales = supportedLocales;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief API to set the fixed labels
     *
     * @param[in] fixedLabels Span of type chip::DeviceLayer::StaticESP32DeviceInfoProvider::FixedLabelEntry
     *              containing the fixed labels for supported endpoints.
     *              The underlying data must remain allocated throughout the lifetime of the device,
     *              as the API does not make a copy.
     *
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    CHIP_ERROR SetFixedLabels(const Span<FixedLabelEntry> & supportedFixedLabels)
    {
        VerifyOrReturnError(!supportedFixedLabels.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        mFixedLabels = supportedFixedLabels;
        return CHIP_NO_ERROR;
    }

protected:
    class StaticFixedLabelIteratorImpl : public FixedLabelIterator
    {
    public:
        StaticFixedLabelIteratorImpl(EndpointId endpoint, const Span<FixedLabelEntry> & labels);
        size_t Count();
        bool Next(FixedLabelType & output);
        void Release() { chip::Platform::Delete(this); }

    private:
        EndpointId mEndpoint = 0;
        size_t mIndex        = 0;
        Span<FixedLabelEntry> mLabels;
    };

    class StaticSupportedLocalesIteratorImpl : public SupportedLocalesIterator
    {
    public:
        StaticSupportedLocalesIteratorImpl(const Span<CharSpan> & locales);
        size_t Count();
        bool Next(CharSpan & output);
        void Release() { chip::Platform::Delete(this); }

    private:
        size_t mIndex = 0;
        Span<CharSpan> mLocales;
    };

    class StaticSupportedCalendarTypesIteratorImpl : public SupportedCalendarTypesIterator
    {
    public:
        StaticSupportedCalendarTypesIteratorImpl(const Span<CalendarType> & calendarTypes);
        size_t Count();
        bool Next(CalendarType & output);
        void Release() { chip::Platform::Delete(this); }

    private:
        size_t mIndex = 0;
        Span<CalendarType> mCalendarTypes;
    };

private:
    Span<CalendarType> mSupportedCalendarTypes;
    Span<CharSpan> mSupportedLocales;
    Span<FixedLabelEntry> mFixedLabels;
};

} // namespace DeviceLayer
} // namespace chip
