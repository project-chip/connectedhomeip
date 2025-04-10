#pragma once

#include "commodity-tariff-server.h"
#include <app/util/af-types.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

// Centralized attribute declaration (Primary attrs)
#define COMMODITY_TARIFF_PRIMARY_ATTRIBUTES \
    X(TariffInfo, DataModel::Nullable<Structs::TariffInformationStruct::Type>) \
    X(TariffUnit, Globals::TariffUnitEnum) \
    X(StartDate, DataModel::Nullable<epoch_s>) \
    X(DefaultRandomizationOffset, DataModel::Nullable<int16_t>) \
    X(DefaultRandomizationType, DataModel::Nullable<DayEntryRandomizationTypeEnum>) \
    X(CalendarPeriods, DataModel::List<Structs::CalendarPeriodStruct::Type>) \
    X(DayPatterns, DataModel::List<Structs::DayPatternStruct::Type>) \
    X(IndividualDays, DataModel::List<Structs::DayStruct::Type>) \
    X(DayEntries, DataModel::List<Structs::DayEntryStruct::Type>) \
    X(TariffPeriods, DataModel::List<Structs::TariffPeriodStruct::Type>) \
    X(TariffComponents, DataModel::List<Structs::TariffComponentStruct::Type>)

// Centralized attribute declaration (Current attrs)
#define COMMODITY_TARIFF_CURRENT_ATTRIBUTES \
    X(CurrentDay, DataModel::Nullable<Structs::DayStruct::Type>) \
    X(NextDay, DataModel::Nullable<Structs::DayStruct::Type>) \
    X(CurrentDayEntry, DataModel::Nullable<Structs::DayEntryStruct::Type>) \
    X(NextDayEntry, DataModel::Nullable<Structs::DayEntryStruct::Type>) \
    X(CurrentDayEntryDate, DataModel::Nullable<epoch_s>) \
    X(NextDayEntryDate, DataModel::Nullable<epoch_s>) \
    X(CurrentTariffComponents, DataModel::List<Structs::TariffComponentStruct::Type>) \
    X(NextTariffComponents, DataModel::List<Structs::TariffComponentStruct::Type>)

typedef DataModel::Nullable<uint32_t> epoch_s;

class CommodityTariffPrimaryData {  
    public:
    #define X(attrName, attrType) \
        attrType attrName;
        COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
    #undef X
};
    
class CommodityTariffCurrentData {
    public:
    #define X(attrName, attrType) \
        attrType attrName;
        COMMODITY_TARIFF_CURRENT_ATTRIBUTES
    #undef X
};

// Attribute accessor template template declaration (needed before any specializations)
template <typename T>
struct AttributeAccessor {
    static bool HasChanged(const T& current, const T& newValue) {
        return current != newValue;
    }
    
    static void Update(T& current, const T& newValue) {
        current = newValue;
    }
};

// Specialization for Nullable types
template <typename T>
struct AttributeAccessor<DataModel::Nullable<T>> {
    static bool HasChanged(const DataModel::Nullable<T>& current, const DataModel::Nullable<T>& newValue) {
        return false;//current.IsNull() != newValue.IsNull() || 
              //(!current.IsNull() && current.Value() != newValue.Value());
    }
    
    static void Update(DataModel::Nullable<T>& current, const DataModel::Nullable<T>& newValue) {
        if (newValue.IsNull()) {
            current.SetNull();
        } else {
            current.SetNonNull(newValue.Value());
        }
    }
};

// Specialization for List types
template <typename T>
struct AttributeAccessor<DataModel::List<T>> {
    static bool HasChanged(const DataModel::List<T>& current, const DataModel::List<T>& newValue) {
        // Simplified comparison - in practice you might want a deeper comparison
        return current.size() != newValue.size();
    }
    
    static void Update(DataModel::List<T>& current, const DataModel::List<T>& newValue) {
        // TODO: Implement proper list copy
    }
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip