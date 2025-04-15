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
    CTC_BaseDataClass() {}
    virtual ~CTC_BaseDataClass() = default;

    T& GetValue() { return mValue; };
    const T& GetValue() const { return mValue; };

    bool Update(const T& aValue) {
        if (!IsValid(aValue))
        {
            // TODO - add an indication or ret status
        }
        else if(HasChanged(aValue))
        {
            UpdateValue(aValue);
            return true;
        }
        return false;
    };

    bool LoadFromJson(const Json::Value& json);
private:
protected:
    T mValue;

    virtual bool IsValid(const T& newValue) const {
        return true;
    }

    virtual bool HasChanged(const T& newValue) const {
        return false;
    }
    
    virtual void UpdateValue(const T& aValue) {
        mValue = aValue;
    }
};

using TariffInformationStruct = DataModel::Nullable<Structs::TariffInformationStruct::Type>;

class TariffInfoDataClass : public CTC_BaseDataClass<TariffInformationStruct> {
public:
    TariffInfoDataClass() = default;
    ~TariffInfoDataClass() override = default;
    bool LoadFromJson(const Json::Value& json); 
protected:
    char mTariffLabelValueBuffer[kDefaultStringValuesMaxBufLength];
    char mProviderNameValueBuffer[kDefaultStringValuesMaxBufLength];
    void UpdateValue(const TariffInformationStruct& aValue) override;
};

/*
using TariffInfoStruct = Structs::TariffInformationStruct::Type;

class TariffInfoDataClass : public StructDataClass<TariffInfoStruct> {
public:
    // Constructor
    explicit TariffInfoDataClass(const TariffInfoStruct& initialValue)
        : StructDataClass<TariffInfoStruct>(initialValue, 
            [](const TariffInfoStruct& value) {
                return ValidateTariffInformation(value);
            }) {}

    // Custom validation for TariffInformationStruct
    static bool ValidateTariffInformation(const TariffInfoStruct& value) {
        // Validate string lengths
        if (value.tariffLabel.Value().size() > 128) return false;
        if (value.providerName.Value().size() > 128) return false;
        
        // Validate Currency if present
        //if (value.currency.HasValue() && !CurrencyStructValidator::Validate(value.currency.Value())) {
        //    return false;
        //}
        
        // All checks passed
        return true;
    }

    // Field-specific update methods with additional validation
    //bool UpdatetariffLabel(const chip::CharSpan& newLabel) {
    //    if (newLabel.size() > 128) return false;
    //    return UpdateField(&TariffInformationStruct::tariffLabel, newLabel);
    //}

    //bool UpdateproviderName(const chip::CharSpan& newName) {
    //    if (newName.size() > 128) return false;
    //    return UpdateField(&TariffInformationStruct::providerName, newName);
    //}

    //bool UpdateCurrency(const Globals::Structs::CurrencyStruct::Type>& newCurrency) {
    //    if (newCurrency.HasValue() && !CurrencyStructValidator::Validate(newCurrency.Value())) {
    //        return false;
    //    }
    //    return UpdateField(&TariffInformationStruct::Currency, newCurrency);
    //}

    //bool UpdateBlockMode(const std::optional<BlockModeEnum>& newBlockMode) {
    //    // No additional validation needed for BlockMode
    //    return UpdateField(&TariffInformationStruct::BlockMode, newBlockMode);
    //}

    // Convenience getters
    chip::CharSpan& GetTariffLabel() const {
        return GetValue().tariffLabel.Value();
    }

    chip::CharSpan& GetProviderName() const {
        return GetValue().providerName.Value();
    }

    Globals::Structs::CurrencyStruct::Type& GetCurrency() const {
        return GetValue().currency.Value().Value();
    }

    BlockModeEnum& GetBlockMode() const {
        return GetValue().blockMode.Value();
    }

    DataModel::Nullable<Structs::TariffInformationStruct::Type> & GetValue()
    {
        DataModel::Nullable<Structs::TariffInformationStruct::Type> retval;
        retval.SetNonNull(value);
        return retval;
    }
};
*/
} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip