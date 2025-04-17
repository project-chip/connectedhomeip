#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

static constexpr size_t kDefaultStringValuesMaxBufLength = 128u;

template <typename T>
class CTC_BaseDataClass {
public:
    explicit CTC_BaseDataClass(T& aValueStorage) : mValue(aValueStorage) {}
    virtual ~CTC_BaseDataClass() = default;

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
private:
protected:
    T & mValue;

    virtual bool IsValid(const T& newValue) const {
        return true;
    }

    virtual bool HasChanged(const T& newValue) const {
        return true;
    }
    
    virtual CHIP_ERROR UpdateValue(const T& aValue) {
        mValue = aValue;
        return CHIP_NO_ERROR;
    }

    virtual void CleanupValue() {
        return;
    }
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

bool ParseIDArray(const Json::Value& json, DataModel::List<const uint32_t>& output, size_t maxSize) {
    if (json.empty() || !json.isArray() || json.size() > maxSize) return false;
    
    std::vector<uint32_t> ids;
    for (const auto& id : json) {
        if (id.isUInt()) ids.push_back(id.asUInt());
    }
    
    return SpanCopier<uint32_t>::Copy(
        chip::Span<const uint32_t>(ids.data(), ids.size()),
        output,
        maxSize
    );
}

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

using TariffInformationStructType = DataModel::Nullable<Structs::TariffInformationStruct::Type>;

class TariffInfoDataClass : public CTC_BaseDataClass<TariffInformationStructType> {
public:
    TariffInfoDataClass(TariffInformationStructType & aValueStorage)
        : CTC_BaseDataClass<TariffInformationStructType>(aValueStorage) {mValue.SetNull(); };
    ~TariffInfoDataClass() override = default;
    CHIP_ERROR LoadFromJson(const Json::Value& json) override; 
protected:
    char mTariffLabelValueBuffer[kDefaultStringValuesMaxBufLength];
    char mProviderNameValueBuffer[kDefaultStringValuesMaxBufLength];
    CHIP_ERROR UpdateValue(const TariffInformationStructType& aValue) override;
    bool IsValid(const TariffInformationStructType& period) const override;
};

using TariffPeriodStructType = Structs::TariffPeriodStruct::Type;

class TariffPeriodItemDataClass : public CTC_BaseDataClass<TariffPeriodStructType> {
public:
    TariffPeriodItemDataClass(TariffPeriodStructType& aValueStorage)
        : CTC_BaseDataClass<TariffPeriodStructType>(aValueStorage) {}

    ~TariffPeriodItemDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;
    static void CleanupTariffPeriod(TariffPeriodStructType& period);    
protected:
    CHIP_ERROR UpdateValue(const TariffPeriodStructType& aValue) override;
private:
    static constexpr size_t kMaxIDsEntries = 20;

    CHIP_ERROR ParseLabelFromJson(const Json::Value& json, TariffPeriodStructType& output);
    CHIP_ERROR ParseIDsFromJson(const Json::Value& json, const char* fieldName,
                              DataModel::List<const uint32_t>& output);
    bool IsValid(const TariffPeriodStructType& period) const override;
    void CleanupValue() override { CleanupTariffPeriod(mValue); }
};

using TariffPeriodsList = DataModel::List<Structs::TariffPeriodStruct::Type>;

class TariffPeriodsDataClass : public CTC_BaseDataClass<TariffPeriodsList> {
public:
    TariffPeriodsDataClass(TariffPeriodsList& aValueStorage)
        : CTC_BaseDataClass<TariffPeriodsList>(aValueStorage) {}

    ~TariffPeriodsDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;
protected:
    CHIP_ERROR UpdateValue(const TariffPeriodsList& aValue) override;
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

    ~DayEntryItemDataClass() override = default;

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;

    static void CleanupDayEntry(DayEntryStructType& entry);

protected:
    CHIP_ERROR UpdateValue(const DayEntryStructType& aValue) override;
    void CleanupValue() override { CleanupDayEntry(mValue); }

private:
    CHIP_ERROR ParseFromJson(const Json::Value& json, DayEntryStructType& output);
    bool IsValid(const DayEntryStructType& entry) const override;
};

using DayEntryListType = DataModel::List<Structs::DayEntryStruct::Type>;

class DayEntriesDataClass : public CTC_BaseDataClass<DayEntryListType> {
public:
    DayEntriesDataClass(DayEntryListType& aValueStorage)
        : CTC_BaseDataClass<DayEntryListType>(aValueStorage) {}

    ~DayEntriesDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;

protected:
    CHIP_ERROR UpdateValue(const DayEntryListType& aValue) override;
    void CleanupValue() override;

private:
    static constexpr size_t kMaxDayEntries = 96; // Max entries per day
};

using TariffComponentStructType = Structs::TariffComponentStruct::Type;

class TariffComponentItemDataClass : public CTC_BaseDataClass<TariffComponentStructType> {
public:
    TariffComponentItemDataClass(TariffComponentStructType& aValueStorage)
        : CTC_BaseDataClass<TariffComponentStructType>(aValueStorage) {}

    ~TariffComponentItemDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;
    static void CleanupTariffComponent(TariffComponentStructType& component);

protected:
    CHIP_ERROR UpdateValue(const TariffComponentStructType& aValue) override;
    void CleanupValue() override { CleanupTariffComponent(mValue); }

private:
    CHIP_ERROR ParseLabelFromJson(
        const Json::Value& json, 
        chip::Optional<DataModel::Nullable<chip::CharSpan>>& output);
    bool IsValid(const TariffComponentStructType& component) const override;
};

using TariffComponentsListType = DataModel::List<Structs::TariffComponentStruct::Type>;

class TariffComponentsDataClass : public CTC_BaseDataClass<TariffComponentsListType> {
public:
    TariffComponentsDataClass(TariffComponentsListType& aValueStorage)
        : CTC_BaseDataClass<TariffComponentsListType>(aValueStorage) {}

    ~TariffComponentsDataClass() override { CleanupValue(); }

    CHIP_ERROR LoadFromJson(const Json::Value& json) override;

protected:
    CHIP_ERROR UpdateValue(const TariffComponentsListType& aValue) override;
    void CleanupValue() override;

private:
    static constexpr size_t kMaxComponents = 20; // Example max components
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip