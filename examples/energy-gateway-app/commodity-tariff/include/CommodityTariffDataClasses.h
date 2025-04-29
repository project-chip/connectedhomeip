#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

static constexpr size_t kDefaultStringValuesMaxBufLength = 128u;

//#define LOG_VAR_CH(changed) ChipLogProgress(NotSpecified, "EGW-CTC: Attr value %s", changed ? "changed": "not changed" )

template <typename T>
class CTC_BaseDataClass {
public:
    explicit CTC_BaseDataClass(T& aValueStorage) : mValue(aValueStorage) { CleanupValue(); }
    virtual ~CTC_BaseDataClass() { CleanupValue(); };

    enum class Error : uint8_t {
        kSuccess            = 0x00,
        kInvalidJson,
        kAllocationFailed,
        kPeriodTooLarge,
        kInvalidPeriodData,
        kInvalidDayEntryIDs,
        kInvalidTariffComponentIDs,        
    };

    T& GetValue() { return mValue; };
    const T& GetValue() const { return mValue; };

    bool Update(const T& aValue) {
        if ( HasChanged(aValue) && (UpdateValue(aValue) == CHIP_NO_ERROR) )
        {
            return true;
        }

        return false;
    };

    virtual CHIP_ERROR LoadFromJson(const Json::Value& json)
    {
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR UpdateValue(const T& aValue) {
        mValue = aValue;
        return CHIP_NO_ERROR;
    }

    virtual void CleanupValue() {
        if constexpr (is_nullable<T>::value) {
            mValue.SetNull();
        } else if constexpr (is_list<T>::value) {
            mValue = T();
        }
    }

    virtual bool IsValid(const T& newValue) const {
        return true;
    }
protected:
    T & mValue;


    static bool NullableNotEqual(const T& a, const T& b) {
        bool is_neq = false;

        if (a.IsNull() || b.IsNull()) {
            is_neq = a.IsNull() != b.IsNull();
        }

        //LOG_VAR_CH(is_neq);

       return is_neq;
       //return a.Value() != b.Value();
    }

    bool HasChanged(const T& newValue) const {
        if constexpr (is_nullable<T>::value) {
            return NullableNotEqual(mValue, newValue);
        } else if constexpr (is_list<T>::value) {
            return mValue.size() != newValue.size();
        }
        return true;
    }
private:
    // Type traits for conditional behavior
    template <typename U>
    struct is_nullable : std::false_type {};

    template <typename U>
    struct is_nullable<DataModel::Nullable<U>> : std::true_type {};

    template <typename U>
    struct is_list : std::false_type {};

    template <typename U>
    struct is_list<DataModel::List<U>> : std::true_type {};
};

template <typename T>
struct SpanCopier {
    static bool Copy(const chip::Span<const T>& source, 
                   DataModel::List<const T>& destination,
                   size_t maxElements = std::numeric_limits<size_t>::max()) 
    {
        if (source.empty()) {
            destination = DataModel::List<const T>();
            return true;
        }

        size_t elementsToCopy = std::min(source.size(), maxElements);
        auto* buffer = static_cast<T*>(chip::Platform::MemoryCalloc(elementsToCopy, sizeof(T)));
        
        if (!buffer) {
            return false;
        }

        std::copy(source.begin(), source.begin() + elementsToCopy, buffer);
        destination = DataModel::List<const T>(chip::Span<const T>(buffer, elementsToCopy));
        return true;
    }
};

template <>
struct SpanCopier<char> {
    static bool Copy(const chip::CharSpan& source,
                   DataModel::Nullable<chip::CharSpan>& destination,
                   size_t maxLength = std::numeric_limits<size_t>::max()) 
    {
        if (source.size() > maxLength) {
            return false;
        }

        if (source.empty()) {
            destination.SetNull();
            return true;
        }

        size_t bytesToCopy = std::min(source.size(), maxLength);
        char* buffer = static_cast<char*>(chip::Platform::MemoryCalloc(1, bytesToCopy + 1)); // +1 for null terminator
        
        if (!buffer) {
            return false;
        }

        std::copy(source.begin(), source.begin() + bytesToCopy, buffer);
        buffer[bytesToCopy] = '\0';
        destination.SetNonNull(chip::CharSpan(buffer, bytesToCopy));
        return true;
    }
};

struct StrToSpan {
    static CHIP_ERROR Copy(const std::string& source,
                         chip::CharSpan& destination,
                         size_t maxLength = std::numeric_limits<size_t>::max())
    {
        // Handle empty string case
        if (source.empty()) {
            destination = chip::CharSpan();
            return CHIP_NO_ERROR;
        }

        // Check length limit
        if (source.size() > maxLength) {
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }

        // Allocate memory (including null terminator)
        char* buffer = static_cast<char*>(chip::Platform::MemoryAlloc(source.size() + 1));
        if (buffer == nullptr) {
            return CHIP_ERROR_NO_MEMORY;
        }

        // Copy data and null-terminate
        memcpy(buffer, source.data(), source.size());
        buffer[source.size()] = '\0';

        // Set output span
        destination = chip::CharSpan(buffer, source.size());
        return CHIP_NO_ERROR;
    }

    // Optional: Memory cleanup helper
    static void Release(chip::CharSpan& span) {
        if (!span.empty()) {
            chip::Platform::MemoryFree(const_cast<char*>(span.data()));
            span = chip::CharSpan();
        }
    }
};

using TariffInformationStructType = DataModel::Nullable<Structs::TariffInformationStruct::Type>;

class TariffInfoDataClass : public CTC_BaseDataClass<TariffInformationStructType> {
public:
    TariffInfoDataClass(TariffInformationStructType & aValueStorage)
        : CTC_BaseDataClass<TariffInformationStructType>(aValueStorage) {mValue.SetNull(); };
    //~TariffInfoDataClass() override = default;
    CHIP_ERROR LoadFromJson(const Json::Value& json) override; 
    CHIP_ERROR UpdateValue(const TariffInformationStructType& aValue) override;
    bool IsValid(const TariffInformationStructType& tariffInfo) const override;    
protected:
    char mTariffLabelValueBuffer[kDefaultStringValuesMaxBufLength];
    char mProviderNameValueBuffer[kDefaultStringValuesMaxBufLength];
};

using TariffPeriodStructType = Structs::TariffPeriodStruct::Type;

class TariffPeriodItemDataClass : public CTC_BaseDataClass<TariffPeriodStructType> {
public:
    TariffPeriodItemDataClass(TariffPeriodStructType& aValueStorage)
        : CTC_BaseDataClass<TariffPeriodStructType>(aValueStorage) {}

    //~TariffPeriodItemDataClass() override = default;

    static void CleanupTariffPeriod(TariffPeriodStructType& period);
    static CHIP_ERROR ParseFromJson(const Json::Value& json, TariffPeriodStructType& output);
    CHIP_ERROR UpdateValue(const TariffPeriodStructType& aValue) override;
    void CleanupValue() override { CleanupTariffPeriod(mValue); }
    bool IsValid(const TariffPeriodStructType& period) const override;    
protected:
    static constexpr size_t kMaxIDsEntries = 20;
};

using TariffPeriodsListType = DataModel::List<Structs::TariffPeriodStruct::Type>;

class TariffPeriodsDataClass : public CTC_BaseDataClass<TariffPeriodsListType> {
public:
    TariffPeriodsDataClass(TariffPeriodsListType& aValueStorage)
        : CTC_BaseDataClass<TariffPeriodsListType>(aValueStorage) {}
    //~TariffPeriodsDataClass() override { CleanupValue(); }
    CHIP_ERROR LoadFromJson(const Json::Value& json) override;
    CHIP_ERROR UpdateValue(const TariffPeriodsListType& aValue) override;
    void CleanupValue() override;
private:
    static constexpr size_t kMaxPeriods = 128;
    static constexpr size_t kMaxIDsPerPeriod = 20;
};

using DayEntryStructType = Structs::DayEntryStruct::Type;

class DayEntryItemDataClass : public CTC_BaseDataClass<DayEntryStructType> {
public:
    DayEntryItemDataClass(DayEntryStructType& aValueStorage)
        : CTC_BaseDataClass<DayEntryStructType>(aValueStorage) {}

    //~DayEntryItemDataClass() override = default;

    static void CleanupDayEntry(DayEntryStructType& entry);
    static CHIP_ERROR ParseFromJson(const Json::Value& json, DayEntryStructType& output);

    CHIP_ERROR UpdateValue(const DayEntryStructType& aValue) override;
    void CleanupValue() override { CleanupDayEntry(mValue); }

    bool IsValid(const DayEntryStructType& entry) const override;
};

using DayEntryListType = DataModel::List<Structs::DayEntryStruct::Type>;

class DayEntriesDataClass : public CTC_BaseDataClass<DayEntryListType> {
public:
    DayEntriesDataClass(DayEntryListType& aValueStorage)
        : CTC_BaseDataClass<DayEntryListType>(aValueStorage) {}

    //~DayEntriesDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;

    CHIP_ERROR UpdateValue(const DayEntryListType& aValue) override;
    void CleanupValue() override;

protected:
    static constexpr size_t kMaxDayEntries = 96; // Max entries per day
};

using TariffComponentStructType = Structs::TariffComponentStruct::Type;

class TariffComponentItemDataClass : public CTC_BaseDataClass<TariffComponentStructType> {
public:
    TariffComponentItemDataClass(TariffComponentStructType& aValueStorage)
        : CTC_BaseDataClass<TariffComponentStructType>(aValueStorage) {}

    //~TariffComponentItemDataClass() override = default;

    static void CleanupTariffComponent(TariffComponentStructType& component);
    static CHIP_ERROR ParseFromJson(const Json::Value& json, TariffComponentStructType& output);
    CHIP_ERROR UpdateValue(const TariffComponentStructType& aValue) override;
    void CleanupValue() override { CleanupTariffComponent(mValue); }
    bool IsValid(const TariffComponentStructType& component) const override;
};

using TariffComponentsListType = DataModel::List<Structs::TariffComponentStruct::Type>;

class TariffComponentsDataClass : public CTC_BaseDataClass<TariffComponentsListType> {
public:
    TariffComponentsDataClass(TariffComponentsListType& aValueStorage)
        : CTC_BaseDataClass<TariffComponentsListType>(aValueStorage) {}

    //~TariffComponentsDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;
    CHIP_ERROR UpdateValue(const TariffComponentsListType& aValue) override;
    void CleanupValue() override;

protected:
    static constexpr size_t kMaxComponents = 20; // Example max components
};

using DayStructType = Structs::DayStruct::Type;
class DayStructItemDataClass : public CTC_BaseDataClass<DayStructType> {
public:
    DayStructItemDataClass(DayStructType& aValueStorage)
        : CTC_BaseDataClass<DayStructType>(aValueStorage) {}

    //~DayStructItemDataClass() override = default;

    static void CleanupTariffComponent(DayStructType& component);
    static CHIP_ERROR ParseFromJson(const Json::Value& json, DayStructType& output);
    CHIP_ERROR UpdateValue(const DayStructType& aValue) override;
    void CleanupValue() override { CleanupTariffComponent(mValue); }
    bool IsValid(const DayStructType& component) const override;
};

using DayStructsListType = DataModel::List<Structs::DayStruct::Type>;
class IndividualDaysDataClass : public CTC_BaseDataClass<DayStructsListType> {
public:
    IndividualDaysDataClass(DayStructsListType& aValueStorage)
        : CTC_BaseDataClass<DayStructsListType>(aValueStorage) {}

    //~IndividualDaysDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;
    CHIP_ERROR UpdateValue(const DayStructsListType& aValue) override;
    void CleanupValue() override;
};

using DayPatternStructType = Structs::DayPatternStruct::Type;
class DayPatternStructItemDataClass : public CTC_BaseDataClass<DayPatternStructType> {
public:
    DayPatternStructItemDataClass(DayPatternStructType& aValueStorage)
        : CTC_BaseDataClass<DayPatternStructType>(aValueStorage) {}

    //~DayPatternStructItemDataClass() override = default;

    static void CleanupTariffComponent(DayPatternStructType& component);
    static CHIP_ERROR ParseFromJson(const Json::Value& json, DayPatternStructType& output);
protected:
    CHIP_ERROR UpdateValue(const DayPatternStructType& aValue) override;
    void CleanupValue() override { CleanupTariffComponent(mValue); }
private:
    bool IsValid(const DayPatternStructType& component) const override;
};

using DayPatternsListType = DataModel::List<DayPatternStructType>;

class DayPatternsDataClass : public CTC_BaseDataClass<DayPatternsListType> {
public:
    DayPatternsDataClass(DayPatternsListType& aValueStorage)
        : CTC_BaseDataClass<DayPatternsListType>(aValueStorage) {}

    //~DayPatternsDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;
    CHIP_ERROR UpdateValue(const DayPatternsListType& aValue) override;
    void CleanupValue() override;
};

using CalendarPeriodStructType = Structs::CalendarPeriodStruct::Type;

class CalendarPeriodItemDataClass : public CTC_BaseDataClass<CalendarPeriodStructType> {
public:
    CalendarPeriodItemDataClass(CalendarPeriodStructType& aValueStorage)
        : CTC_BaseDataClass<CalendarPeriodStructType>(aValueStorage) {}

    //~CalendarPeriodItemDataClass() override = default;

    static void CleanupTariffComponent(CalendarPeriodStructType& component);
    static CHIP_ERROR ParseFromJson(const Json::Value& json, CalendarPeriodStructType& output);
    CHIP_ERROR UpdateValue(const CalendarPeriodStructType& aValue) override;
    void CleanupValue() override { CleanupTariffComponent(mValue); }
    bool IsValid(const CalendarPeriodStructType& component) const override;
};

using CalendarPeriodsListType = DataModel::List<CalendarPeriodStructType>;
class CalendarPeriodsDataClass : public CTC_BaseDataClass<CalendarPeriodsListType> {
public:
    CalendarPeriodsDataClass(CalendarPeriodsListType& aValueStorage)
        : CTC_BaseDataClass<CalendarPeriodsListType>(aValueStorage) {}

    //~CalendarPeriodsDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;
    CHIP_ERROR UpdateValue(const CalendarPeriodsListType& aValue) override;
    void CleanupValue() override;
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip