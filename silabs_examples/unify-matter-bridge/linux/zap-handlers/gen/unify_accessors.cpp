
/**
 *  @file
 *    This file contains definitions for accessors around clusters attributes.
 */

#define CHIP_USE_ENUM_CLASS_FOR_IM_ENUM
#include <app-common/zap-generated/attributes/Accessors.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/odd-sized-integers.h>
#include <attribute_state_cache.hpp>

using namespace chip::app;
namespace unify {
namespace matter_bridge {

namespace Identify {
namespace Attributes {

namespace IdentifyTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace IdentifyTime

namespace IdentifyType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace IdentifyType

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Identify

namespace Scenes {
namespace Attributes {

namespace SceneCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SceneCount

namespace CurrentScene {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentScene

namespace CurrentGroup {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::GroupId& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::GroupId>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::GroupId& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::GroupId>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentGroup

namespace SceneValid {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SceneValid

namespace NameSupport {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NameSupport

namespace LastConfiguredBy {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::NodeId>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::NodeId>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::NodeId>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::NodeId>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LastConfiguredBy

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Scenes

namespace OnOff {
namespace Attributes {

namespace OnOff {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OnOff

namespace GlobalSceneControl {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace GlobalSceneControl

namespace OnTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OnTime

namespace OffWaitTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OffWaitTime

namespace StartUpOnOff {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffStartUpOnOff>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::OnOff::OnOffStartUpOnOff>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffStartUpOnOff>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::OnOff::OnOffStartUpOnOff>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StartUpOnOff

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // OnOff

namespace OnOffSwitchConfiguration {
namespace Attributes {

namespace SwitchType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SwitchType

namespace SwitchActions {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SwitchActions

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // OnOffSwitchConfiguration

namespace LevelControl {
namespace Attributes {

namespace CurrentLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentLevel

namespace RemainingTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RemainingTime

namespace MinLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinLevel

namespace MaxLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxLevel

namespace CurrentFrequency {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentFrequency

namespace MinFrequency {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinFrequency

namespace MaxFrequency {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxFrequency

namespace Options {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Options

namespace OnOffTransitionTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OnOffTransitionTime

namespace OnLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OnLevel

namespace OnTransitionTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OnTransitionTime

namespace OffTransitionTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OffTransitionTime

namespace DefaultMoveRate {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DefaultMoveRate

namespace StartUpCurrentLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StartUpCurrentLevel

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // LevelControl

namespace BinaryInputBasic {
namespace Attributes {

namespace ActiveText {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActiveText

namespace Description {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Description

namespace InactiveText {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InactiveText

namespace OutOfService {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OutOfService

namespace Polarity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Polarity

namespace PresentValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PresentValue

namespace Reliability {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Reliability

namespace StatusFlags {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StatusFlags

namespace ApplicationType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ApplicationType

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // BinaryInputBasic

namespace PulseWidthModulation {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // PulseWidthModulation

namespace Descriptor {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Descriptor

namespace Binding {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Binding

namespace AccessControl {
namespace Attributes {

namespace SubjectsPerAccessControlEntry {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SubjectsPerAccessControlEntry

namespace TargetsPerAccessControlEntry {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TargetsPerAccessControlEntry

namespace AccessControlEntriesPerFabric {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AccessControlEntriesPerFabric

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // AccessControl

namespace Actions {
namespace Attributes {

namespace SetupURL {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SetupURL

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Actions

namespace Basic {
namespace Attributes {

namespace DataModelRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DataModelRevision

namespace VendorName {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace VendorName

namespace VendorID {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::VendorId& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::VendorId>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::VendorId& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::VendorId>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace VendorID

namespace ProductName {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ProductName

namespace ProductID {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ProductID

namespace NodeLabel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NodeLabel

namespace Location {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Location

namespace HardwareVersion {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace HardwareVersion

namespace HardwareVersionString {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace HardwareVersionString

namespace SoftwareVersion {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SoftwareVersion

namespace SoftwareVersionString {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SoftwareVersionString

namespace ManufacturingDate {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ManufacturingDate

namespace PartNumber {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PartNumber

namespace ProductURL {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ProductURL

namespace ProductLabel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ProductLabel

namespace SerialNumber {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SerialNumber

namespace LocalConfigDisabled {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LocalConfigDisabled

namespace Reachable {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Reachable

namespace UniqueID {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UniqueID

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Basic

namespace OtaSoftwareUpdateProvider {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // OtaSoftwareUpdateProvider

namespace OtaSoftwareUpdateRequestor {
namespace Attributes {

namespace UpdatePossible {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UpdatePossible

namespace UpdateState {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UpdateState

namespace UpdateStateProgress {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UpdateStateProgress

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // OtaSoftwareUpdateRequestor

namespace LocalizationConfiguration {
namespace Attributes {

namespace ActiveLocale {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActiveLocale

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // LocalizationConfiguration

namespace TimeFormatLocalization {
namespace Attributes {

namespace HourFormat {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TimeFormatLocalization::HourFormat& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::TimeFormatLocalization::HourFormat>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TimeFormatLocalization::HourFormat& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::TimeFormatLocalization::HourFormat>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace HourFormat

namespace ActiveCalendarType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TimeFormatLocalization::CalendarType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::TimeFormatLocalization::CalendarType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TimeFormatLocalization::CalendarType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::TimeFormatLocalization::CalendarType>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActiveCalendarType

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // TimeFormatLocalization

namespace UnitLocalization {
namespace Attributes {

namespace TemperatureUnit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::UnitLocalization::TempUnit& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::UnitLocalization::TempUnit>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::UnitLocalization::TempUnit& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::UnitLocalization::TempUnit>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TemperatureUnit

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // UnitLocalization

namespace PowerSourceConfiguration {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // PowerSourceConfiguration

namespace PowerSource {
namespace Attributes {

namespace Status {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::PowerSourceStatus& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::PowerSourceStatus>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::PowerSourceStatus& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::PowerSourceStatus>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Status

namespace Order {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Order

namespace Description {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Description

namespace WiredAssessedInputVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WiredAssessedInputVoltage

namespace WiredAssessedInputFrequency {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WiredAssessedInputFrequency

namespace WiredCurrentType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::WiredCurrentType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::WiredCurrentType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::WiredCurrentType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::WiredCurrentType>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WiredCurrentType

namespace WiredAssessedCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WiredAssessedCurrent

namespace WiredNominalVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WiredNominalVoltage

namespace WiredMaximumCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WiredMaximumCurrent

namespace WiredPresent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WiredPresent

namespace BatVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatVoltage

namespace BatPercentRemaining {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatPercentRemaining

namespace BatTimeRemaining {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatTimeRemaining

namespace BatChargeLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::BatChargeLevel& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::BatChargeLevel>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::BatChargeLevel& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::BatChargeLevel>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatChargeLevel

namespace BatReplacementNeeded {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatReplacementNeeded

namespace BatReplaceability {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::BatReplaceability& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::BatReplaceability>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::BatReplaceability& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::BatReplaceability>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatReplaceability

namespace BatPresent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatPresent

namespace BatReplacementDescription {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatReplacementDescription

namespace BatCommonDesignation {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatCommonDesignation

namespace BatANSIDesignation {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatANSIDesignation

namespace BatIECDesignation {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatIECDesignation

namespace BatApprovedChemistry {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatApprovedChemistry

namespace BatCapacity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatCapacity

namespace BatQuantity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatQuantity

namespace BatChargeState {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PowerSource::BatChargeState& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::BatChargeState>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PowerSource::BatChargeState& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PowerSource::BatChargeState>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatChargeState

namespace BatTimeToFullCharge {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatTimeToFullCharge

namespace BatFunctionalWhileCharging {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatFunctionalWhileCharging

namespace BatChargingCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BatChargingCurrent

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // PowerSource

namespace GeneralCommissioning {
namespace Attributes {

namespace Breadcrumb {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Breadcrumb

namespace RegulatoryConfig {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RegulatoryConfig

namespace LocationCapability {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::GeneralCommissioning::RegulatoryLocationType>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LocationCapability

namespace SupportsConcurrentConnection {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SupportsConcurrentConnection

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // GeneralCommissioning

namespace DiagnosticLogs {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // DiagnosticLogs

namespace GeneralDiagnostics {
namespace Attributes {

namespace RebootCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RebootCount

namespace UpTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UpTime

namespace TotalOperationalHours {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TotalOperationalHours

namespace BootReasons {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BootReasons

namespace TestEventTriggersEnabled {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TestEventTriggersEnabled

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // GeneralDiagnostics

namespace SoftwareDiagnostics {
namespace Attributes {

namespace CurrentHeapFree {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentHeapFree

namespace CurrentHeapUsed {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentHeapUsed

namespace CurrentHeapHighWatermark {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentHeapHighWatermark

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // SoftwareDiagnostics

namespace ThreadNetworkDiagnostics {
namespace Attributes {

namespace Channel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Channel

namespace RoutingRole {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::ThreadNetworkDiagnostics::RoutingRole>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RoutingRole

namespace NetworkName {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::CharSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::CharSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NetworkName

namespace PanId {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PanId

namespace ExtendedPanId {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ExtendedPanId

namespace MeshLocalPrefix {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::ByteSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::ByteSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::ByteSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::ByteSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeshLocalPrefix

namespace OverrunCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OverrunCount

namespace PartitionId {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PartitionId

namespace Weighting {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Weighting

namespace DataVersion {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DataVersion

namespace StableDataVersion {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StableDataVersion

namespace LeaderRouterId {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LeaderRouterId

namespace DetachedRoleCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DetachedRoleCount

namespace ChildRoleCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ChildRoleCount

namespace RouterRoleCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RouterRoleCount

namespace LeaderRoleCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LeaderRoleCount

namespace AttachAttemptCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AttachAttemptCount

namespace PartitionIdChangeCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PartitionIdChangeCount

namespace BetterPartitionAttachAttemptCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BetterPartitionAttachAttemptCount

namespace ParentChangeCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ParentChangeCount

namespace TxTotalCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxTotalCount

namespace TxUnicastCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxUnicastCount

namespace TxBroadcastCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxBroadcastCount

namespace TxAckRequestedCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxAckRequestedCount

namespace TxAckedCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxAckedCount

namespace TxNoAckRequestedCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxNoAckRequestedCount

namespace TxDataCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxDataCount

namespace TxDataPollCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxDataPollCount

namespace TxBeaconCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxBeaconCount

namespace TxBeaconRequestCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxBeaconRequestCount

namespace TxOtherCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxOtherCount

namespace TxRetryCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxRetryCount

namespace TxDirectMaxRetryExpiryCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxDirectMaxRetryExpiryCount

namespace TxIndirectMaxRetryExpiryCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxIndirectMaxRetryExpiryCount

namespace TxErrCcaCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxErrCcaCount

namespace TxErrAbortCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxErrAbortCount

namespace TxErrBusyChannelCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxErrBusyChannelCount

namespace RxTotalCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxTotalCount

namespace RxUnicastCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxUnicastCount

namespace RxBroadcastCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxBroadcastCount

namespace RxDataCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxDataCount

namespace RxDataPollCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxDataPollCount

namespace RxBeaconCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxBeaconCount

namespace RxBeaconRequestCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxBeaconRequestCount

namespace RxOtherCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxOtherCount

namespace RxAddressFilteredCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxAddressFilteredCount

namespace RxDestAddrFilteredCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxDestAddrFilteredCount

namespace RxDuplicatedCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxDuplicatedCount

namespace RxErrNoFrameCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxErrNoFrameCount

namespace RxErrUnknownNeighborCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxErrUnknownNeighborCount

namespace RxErrInvalidSrcAddrCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxErrInvalidSrcAddrCount

namespace RxErrSecCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxErrSecCount

namespace RxErrFcsCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxErrFcsCount

namespace RxErrOtherCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RxErrOtherCount

namespace ActiveTimestamp {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActiveTimestamp

namespace PendingTimestamp {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PendingTimestamp

namespace Delay {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Delay

namespace ChannelPage0Mask {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::ByteSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::ByteSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::ByteSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::ByteSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ChannelPage0Mask

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ThreadNetworkDiagnostics

namespace WiFiNetworkDiagnostics {
namespace Attributes {

namespace Bssid {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::ByteSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::ByteSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::ByteSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::ByteSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Bssid

namespace SecurityType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::WiFiNetworkDiagnostics::SecurityType>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SecurityType

namespace WiFiVersion {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::WiFiNetworkDiagnostics::WiFiVersionType>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WiFiVersion

namespace ChannelNumber {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ChannelNumber

namespace Rssi {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Rssi

namespace BeaconLostCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BeaconLostCount

namespace BeaconRxCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BeaconRxCount

namespace PacketMulticastRxCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PacketMulticastRxCount

namespace PacketMulticastTxCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PacketMulticastTxCount

namespace PacketUnicastRxCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PacketUnicastRxCount

namespace PacketUnicastTxCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PacketUnicastTxCount

namespace CurrentMaxRate {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentMaxRate

namespace OverrunCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OverrunCount

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // WiFiNetworkDiagnostics

namespace EthernetNetworkDiagnostics {
namespace Attributes {

namespace PHYRate {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateType>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateType>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::EthernetNetworkDiagnostics::PHYRateType>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PHYRate

namespace FullDuplex {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<bool>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<bool>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FullDuplex

namespace PacketRxCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PacketRxCount

namespace PacketTxCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PacketTxCount

namespace TxErrCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TxErrCount

namespace CollisionCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CollisionCount

namespace OverrunCount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OverrunCount

namespace CarrierDetect {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<bool>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<bool>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CarrierDetect

namespace TimeSinceReset {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint64_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TimeSinceReset

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // EthernetNetworkDiagnostics

namespace TimeSynchronization {
namespace Attributes {

namespace UTCTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UTCTime

namespace Granularity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TimeSynchronization::GranularityEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::TimeSynchronization::GranularityEnum>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TimeSynchronization::GranularityEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::TimeSynchronization::GranularityEnum>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Granularity

namespace TimeSource {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::TimeSynchronization::TimeSourceEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::TimeSynchronization::TimeSourceEnum>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::TimeSynchronization::TimeSourceEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::TimeSynchronization::TimeSourceEnum>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TimeSource

namespace TrustedTimeNodeId {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::NodeId>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::NodeId>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::NodeId>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::NodeId>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TrustedTimeNodeId

namespace DefaultNtp {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::CharSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::CharSpan>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DefaultNtp

namespace LocalTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LocalTime

namespace TimeZoneDatabase {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TimeZoneDatabase

namespace NtpServerPort {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NtpServerPort

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // TimeSynchronization

namespace BridgedDeviceBasic {
namespace Attributes {

namespace VendorName {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace VendorName

namespace VendorID {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::VendorId& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::VendorId>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::VendorId& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::VendorId>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace VendorID

namespace ProductName {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ProductName

namespace NodeLabel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NodeLabel

namespace HardwareVersion {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace HardwareVersion

namespace HardwareVersionString {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace HardwareVersionString

namespace SoftwareVersion {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SoftwareVersion

namespace SoftwareVersionString {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SoftwareVersionString

namespace ManufacturingDate {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ManufacturingDate

namespace PartNumber {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PartNumber

namespace ProductURL {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ProductURL

namespace ProductLabel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ProductLabel

namespace SerialNumber {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SerialNumber

namespace Reachable {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Reachable

namespace UniqueID {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UniqueID

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // BridgedDeviceBasic

namespace Switch {
namespace Attributes {

namespace NumberOfPositions {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfPositions

namespace CurrentPosition {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentPosition

namespace MultiPressMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MultiPressMax

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Switch

namespace AdministratorCommissioning {
namespace Attributes {

namespace WindowStatus {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatus>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WindowStatus

namespace AdminFabricIndex {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::FabricIndex>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::FabricIndex>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::FabricIndex>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::FabricIndex>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AdminFabricIndex

namespace AdminVendorId {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AdminVendorId

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // AdministratorCommissioning

namespace OperationalCredentials {
namespace Attributes {

namespace SupportedFabrics {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SupportedFabrics

namespace CommissionedFabrics {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CommissionedFabrics

namespace CurrentFabricIndex {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentFabricIndex

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // OperationalCredentials

namespace GroupKeyManagement {
namespace Attributes {

namespace MaxGroupsPerFabric {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxGroupsPerFabric

namespace MaxGroupKeysPerFabric {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxGroupKeysPerFabric

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // GroupKeyManagement

namespace FixedLabel {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // FixedLabel

namespace UserLabel {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // UserLabel

namespace ProxyConfiguration {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ProxyConfiguration

namespace ProxyDiscovery {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ProxyDiscovery

namespace ProxyValid {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ProxyValid

namespace BooleanState {
namespace Attributes {

namespace StateValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StateValue

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // BooleanState

namespace ModeSelect {
namespace Attributes {

namespace Description {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Description

namespace StandardNamespace {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StandardNamespace

namespace CurrentMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentMode

namespace StartUpMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StartUpMode

namespace OnMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OnMode

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ModeSelect

namespace DoorLock {
namespace Attributes {

namespace LockState {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlLockState>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlLockState>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LockState

namespace LockType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::DoorLock::DlLockType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlLockType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::DoorLock::DlLockType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlLockType>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LockType

namespace ActuatorEnabled {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActuatorEnabled

namespace DoorState {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlDoorState>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlDoorState>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlDoorState>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlDoorState>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DoorState

namespace DoorOpenEvents {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DoorOpenEvents

namespace DoorClosedEvents {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DoorClosedEvents

namespace OpenPeriod {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OpenPeriod

namespace NumberOfTotalUsersSupported {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfTotalUsersSupported

namespace NumberOfPINUsersSupported {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfPINUsersSupported

namespace NumberOfRFIDUsersSupported {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfRFIDUsersSupported

namespace NumberOfWeekDaySchedulesSupportedPerUser {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfWeekDaySchedulesSupportedPerUser

namespace NumberOfYearDaySchedulesSupportedPerUser {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfYearDaySchedulesSupportedPerUser

namespace NumberOfHolidaySchedulesSupported {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfHolidaySchedulesSupported

namespace MaxPINCodeLength {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxPINCodeLength

namespace MinPINCodeLength {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinPINCodeLength

namespace MaxRFIDCodeLength {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxRFIDCodeLength

namespace MinRFIDCodeLength {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinRFIDCodeLength

namespace CredentialRulesSupport {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CredentialRulesSupport

namespace NumberOfCredentialsSupportedPerUser {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfCredentialsSupportedPerUser

namespace Language {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Language

namespace LEDSettings {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LEDSettings

namespace AutoRelockTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AutoRelockTime

namespace SoundVolume {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SoundVolume

namespace OperatingMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::DoorLock::DlOperatingMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlOperatingMode>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::DoorLock::DlOperatingMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlOperatingMode>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OperatingMode

namespace SupportedOperatingModes {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SupportedOperatingModes

namespace DefaultConfigurationRegister {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DefaultConfigurationRegister

namespace EnableLocalProgramming {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EnableLocalProgramming

namespace EnableOneTouchLocking {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EnableOneTouchLocking

namespace EnableInsideStatusLED {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EnableInsideStatusLED

namespace EnablePrivacyModeButton {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EnablePrivacyModeButton

namespace LocalProgrammingFeatures {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LocalProgrammingFeatures

namespace WrongCodeEntryLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WrongCodeEntryLimit

namespace UserCodeTemporaryDisableTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UserCodeTemporaryDisableTime

namespace SendPINOverTheAir {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SendPINOverTheAir

namespace RequirePINforRemoteOperation {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RequirePINforRemoteOperation

namespace ExpiringUserTimeout {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ExpiringUserTimeout

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // DoorLock

namespace WindowCovering {
namespace Attributes {

namespace Type {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::WindowCovering::Type& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::WindowCovering::Type>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::WindowCovering::Type& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::WindowCovering::Type>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Type

namespace PhysicalClosedLimitLift {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PhysicalClosedLimitLift

namespace PhysicalClosedLimitTilt {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PhysicalClosedLimitTilt

namespace CurrentPositionLift {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentPositionLift

namespace CurrentPositionTilt {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentPositionTilt

namespace NumberOfActuationsLift {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfActuationsLift

namespace NumberOfActuationsTilt {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfActuationsTilt

namespace ConfigStatus {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::WindowCovering::ConfigStatus>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::WindowCovering::ConfigStatus>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::WindowCovering::ConfigStatus>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::WindowCovering::ConfigStatus>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ConfigStatus

namespace CurrentPositionLiftPercentage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::Percent>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::Percent>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentPositionLiftPercentage

namespace CurrentPositionTiltPercentage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::Percent>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::Percent>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentPositionTiltPercentage

namespace OperationalStatus {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OperationalStatus

namespace TargetPositionLiftPercent100ths {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent100ths>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::Percent100ths>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent100ths>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::Percent100ths>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TargetPositionLiftPercent100ths

namespace TargetPositionTiltPercent100ths {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent100ths>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::Percent100ths>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent100ths>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::Percent100ths>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TargetPositionTiltPercent100ths

namespace EndProductType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::WindowCovering::EndProductType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::WindowCovering::EndProductType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::WindowCovering::EndProductType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::WindowCovering::EndProductType>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EndProductType

namespace CurrentPositionLiftPercent100ths {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent100ths>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::Percent100ths>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent100ths>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::Percent100ths>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentPositionLiftPercent100ths

namespace CurrentPositionTiltPercent100ths {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::Percent100ths>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::Percent100ths>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::Percent100ths>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::Percent100ths>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentPositionTiltPercent100ths

namespace InstalledOpenLimitLift {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstalledOpenLimitLift

namespace InstalledClosedLimitLift {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstalledClosedLimitLift

namespace InstalledOpenLimitTilt {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstalledOpenLimitTilt

namespace InstalledClosedLimitTilt {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstalledClosedLimitTilt

namespace Mode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::WindowCovering::Mode>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::WindowCovering::Mode>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::WindowCovering::Mode>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::WindowCovering::Mode>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Mode

namespace SafetyStatus {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::WindowCovering::SafetyStatus>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::WindowCovering::SafetyStatus>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::WindowCovering::SafetyStatus>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::WindowCovering::SafetyStatus>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SafetyStatus

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // WindowCovering

namespace BarrierControl {
namespace Attributes {

namespace BarrierMovingState {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierMovingState

namespace BarrierSafetyStatus {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierSafetyStatus

namespace BarrierCapabilities {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierCapabilities

namespace BarrierOpenEvents {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierOpenEvents

namespace BarrierCloseEvents {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierCloseEvents

namespace BarrierCommandOpenEvents {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierCommandOpenEvents

namespace BarrierCommandCloseEvents {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierCommandCloseEvents

namespace BarrierOpenPeriod {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierOpenPeriod

namespace BarrierClosePeriod {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierClosePeriod

namespace BarrierPosition {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BarrierPosition

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // BarrierControl

namespace PumpConfigurationAndControl {
namespace Attributes {

namespace MaxPressure {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxPressure

namespace MaxSpeed {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxSpeed

namespace MaxFlow {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxFlow

namespace MinConstPressure {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinConstPressure

namespace MaxConstPressure {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxConstPressure

namespace MinCompPressure {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinCompPressure

namespace MaxCompPressure {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxCompPressure

namespace MinConstSpeed {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinConstSpeed

namespace MaxConstSpeed {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxConstSpeed

namespace MinConstFlow {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinConstFlow

namespace MaxConstFlow {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxConstFlow

namespace MinConstTemp {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinConstTemp

namespace MaxConstTemp {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxConstTemp

namespace PumpStatus {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatus>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatus>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatus>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::PumpConfigurationAndControl::PumpStatus>>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PumpStatus

namespace EffectiveOperationMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EffectiveOperationMode

namespace EffectiveControlMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EffectiveControlMode

namespace Capacity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Capacity

namespace Speed {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Speed

namespace LifetimeRunningHours {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LifetimeRunningHours

namespace Power {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Power

namespace LifetimeEnergyConsumed {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LifetimeEnergyConsumed

namespace OperationMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PumpConfigurationAndControl::PumpOperationMode>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OperationMode

namespace ControlMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::PumpConfigurationAndControl::PumpControlMode>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ControlMode

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // PumpConfigurationAndControl

namespace Thermostat {
namespace Attributes {

namespace LocalTemperature {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LocalTemperature

namespace OutdoorTemperature {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OutdoorTemperature

namespace Occupancy {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Occupancy

namespace AbsMinHeatSetpointLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AbsMinHeatSetpointLimit

namespace AbsMaxHeatSetpointLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AbsMaxHeatSetpointLimit

namespace AbsMinCoolSetpointLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AbsMinCoolSetpointLimit

namespace AbsMaxCoolSetpointLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AbsMaxCoolSetpointLimit

namespace PICoolingDemand {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PICoolingDemand

namespace PIHeatingDemand {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PIHeatingDemand

namespace HVACSystemTypeConfiguration {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace HVACSystemTypeConfiguration

namespace LocalTemperatureCalibration {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LocalTemperatureCalibration

namespace OccupiedCoolingSetpoint {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OccupiedHeatingSetpoint

namespace UnoccupiedCoolingSetpoint {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UnoccupiedCoolingSetpoint

namespace UnoccupiedHeatingSetpoint {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UnoccupiedHeatingSetpoint

namespace MinHeatSetpointLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinHeatSetpointLimit

namespace MaxHeatSetpointLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxHeatSetpointLimit

namespace MinCoolSetpointLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinCoolSetpointLimit

namespace MaxCoolSetpointLimit {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxCoolSetpointLimit

namespace MinSetpointDeadBand {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinSetpointDeadBand

namespace RemoteSensing {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RemoteSensing

namespace ControlSequenceOfOperation {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::Thermostat::ThermostatControlSequence& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::Thermostat::ThermostatControlSequence>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::Thermostat::ThermostatControlSequence& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::Thermostat::ThermostatControlSequence>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ControlSequenceOfOperation

namespace SystemMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SystemMode

namespace ThermostatRunningMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ThermostatRunningMode

namespace StartOfWeek {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StartOfWeek

namespace NumberOfWeeklyTransitions {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfWeeklyTransitions

namespace NumberOfDailyTransitions {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfDailyTransitions

namespace TemperatureSetpointHold {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TemperatureSetpointHold

namespace TemperatureSetpointHoldDuration {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TemperatureSetpointHoldDuration

namespace ThermostatProgrammingOperationMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ThermostatProgrammingOperationMode

namespace ThermostatRunningState {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ThermostatRunningState

namespace SetpointChangeSource {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SetpointChangeSource

namespace SetpointChangeAmount {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SetpointChangeAmount

namespace SetpointChangeSourceTimestamp {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SetpointChangeSourceTimestamp

namespace OccupiedSetback {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OccupiedSetback

namespace OccupiedSetbackMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OccupiedSetbackMin

namespace OccupiedSetbackMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OccupiedSetbackMax

namespace UnoccupiedSetback {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UnoccupiedSetback

namespace UnoccupiedSetbackMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UnoccupiedSetbackMin

namespace UnoccupiedSetbackMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UnoccupiedSetbackMax

namespace EmergencyHeatDelta {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EmergencyHeatDelta

namespace ACType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ACType

namespace ACCapacity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ACCapacity

namespace ACRefrigerantType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ACRefrigerantType

namespace ACCompressorType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ACCompressorType

namespace ACErrorCode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ACErrorCode

namespace ACLouverPosition {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ACLouverPosition

namespace ACCoilTemperature {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ACCoilTemperature

namespace ACCapacityformat {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ACCapacityformat

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Thermostat

namespace FanControl {
namespace Attributes {

namespace FanMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::FanControl::FanModeType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::FanControl::FanModeType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::FanControl::FanModeType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::FanControl::FanModeType>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FanMode

namespace FanModeSequence {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::FanControl::FanModeSequenceType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::FanControl::FanModeSequenceType>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::FanControl::FanModeSequenceType& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::FanControl::FanModeSequenceType>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FanModeSequence

namespace PercentSetting {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PercentSetting

namespace PercentCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PercentCurrent

namespace SpeedMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SpeedMax

namespace SpeedSetting {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SpeedSetting

namespace SpeedCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SpeedCurrent

namespace RockSupport {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RockSupport

namespace RockSetting {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RockSetting

namespace WindSupport {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WindSupport

namespace WindSetting {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WindSetting

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // FanControl

namespace ThermostatUserInterfaceConfiguration {
namespace Attributes {

namespace TemperatureDisplayMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TemperatureDisplayMode

namespace KeypadLockout {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace KeypadLockout

namespace ScheduleProgrammingVisibility {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ScheduleProgrammingVisibility

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ThermostatUserInterfaceConfiguration

namespace ColorControl {
namespace Attributes {

namespace CurrentHue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentHue

namespace CurrentSaturation {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentSaturation

namespace RemainingTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RemainingTime

namespace CurrentX {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentX

namespace CurrentY {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentY

namespace DriftCompensation {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DriftCompensation

namespace CompensationText {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CompensationText

namespace ColorTemperatureMireds {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorTemperatureMireds

namespace ColorMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorMode

namespace Options {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Options

namespace NumberOfPrimaries {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NumberOfPrimaries

namespace Primary1X {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary1X

namespace Primary1Y {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary1Y

namespace Primary1Intensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary1Intensity

namespace Primary2X {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary2X

namespace Primary2Y {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary2Y

namespace Primary2Intensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary2Intensity

namespace Primary3X {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary3X

namespace Primary3Y {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary3Y

namespace Primary3Intensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary3Intensity

namespace Primary4X {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary4X

namespace Primary4Y {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary4Y

namespace Primary4Intensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary4Intensity

namespace Primary5X {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary5X

namespace Primary5Y {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary5Y

namespace Primary5Intensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary5Intensity

namespace Primary6X {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary6X

namespace Primary6Y {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary6Y

namespace Primary6Intensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Primary6Intensity

namespace WhitePointX {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WhitePointX

namespace WhitePointY {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace WhitePointY

namespace ColorPointRX {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointRX

namespace ColorPointRY {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointRY

namespace ColorPointRIntensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointRIntensity

namespace ColorPointGX {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointGX

namespace ColorPointGY {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointGY

namespace ColorPointGIntensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointGIntensity

namespace ColorPointBX {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointBX

namespace ColorPointBY {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointBY

namespace ColorPointBIntensity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorPointBIntensity

namespace EnhancedCurrentHue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EnhancedCurrentHue

namespace EnhancedColorMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace EnhancedColorMode

namespace ColorLoopActive {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorLoopActive

namespace ColorLoopDirection {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorLoopDirection

namespace ColorLoopTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorLoopTime

namespace ColorLoopStartEnhancedHue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorLoopStartEnhancedHue

namespace ColorLoopStoredEnhancedHue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorLoopStoredEnhancedHue

namespace ColorCapabilities {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorCapabilities

namespace ColorTempPhysicalMinMireds {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorTempPhysicalMinMireds

namespace ColorTempPhysicalMaxMireds {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ColorTempPhysicalMaxMireds

namespace CoupleColorTempToLevelMinMireds {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CoupleColorTempToLevelMinMireds

namespace StartUpColorTemperatureMireds {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StartUpColorTemperatureMireds

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ColorControl

namespace BallastConfiguration {
namespace Attributes {

namespace PhysicalMinLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PhysicalMinLevel

namespace PhysicalMaxLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PhysicalMaxLevel

namespace BallastStatus {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BallastStatus

namespace MinLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinLevel

namespace MaxLevel {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxLevel

namespace IntrinsicBalanceFactor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace IntrinsicBalanceFactor

namespace BallastFactorAdjustment {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace BallastFactorAdjustment

namespace LampQuantity {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LampQuantity

namespace LampType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LampType

namespace LampManufacturer {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LampManufacturer

namespace LampRatedHours {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LampRatedHours

namespace LampBurnHours {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LampBurnHours

namespace LampAlarmMode {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LampAlarmMode

namespace LampBurnHoursTripPoint {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint32_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LampBurnHoursTripPoint

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // BallastConfiguration

namespace IlluminanceMeasurement {
namespace Attributes {

namespace MeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredValue

namespace MinMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinMeasuredValue

namespace MaxMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxMeasuredValue

namespace Tolerance {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Tolerance

namespace LightSensorType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LightSensorType

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // IlluminanceMeasurement

namespace TemperatureMeasurement {
namespace Attributes {

namespace MeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredValue

namespace MinMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinMeasuredValue

namespace MaxMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxMeasuredValue

namespace Tolerance {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Tolerance

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // TemperatureMeasurement

namespace PressureMeasurement {
namespace Attributes {

namespace MeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredValue

namespace MinMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinMeasuredValue

namespace MaxMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxMeasuredValue

namespace Tolerance {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Tolerance

namespace ScaledValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ScaledValue

namespace MinScaledValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinScaledValue

namespace MaxScaledValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxScaledValue

namespace ScaledTolerance {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ScaledTolerance

namespace Scale {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Scale

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // PressureMeasurement

namespace FlowMeasurement {
namespace Attributes {

namespace MeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredValue

namespace MinMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinMeasuredValue

namespace MaxMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxMeasuredValue

namespace Tolerance {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Tolerance

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // FlowMeasurement

namespace RelativeHumidityMeasurement {
namespace Attributes {

namespace MeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredValue

namespace MinMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MinMeasuredValue

namespace MaxMeasuredValue {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MaxMeasuredValue

namespace Tolerance {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Tolerance

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // RelativeHumidityMeasurement

namespace OccupancySensing {
namespace Attributes {

namespace Occupancy {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Occupancy

namespace OccupancySensorType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OccupancySensorType

namespace OccupancySensorTypeBitmap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OccupancySensorTypeBitmap

namespace PirOccupiedToUnoccupiedDelay {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PirOccupiedToUnoccupiedDelay

namespace PirUnoccupiedToOccupiedDelay {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PirUnoccupiedToOccupiedDelay

namespace PirUnoccupiedToOccupiedThreshold {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PirUnoccupiedToOccupiedThreshold

namespace UltrasonicOccupiedToUnoccupiedDelay {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UltrasonicOccupiedToUnoccupiedDelay

namespace UltrasonicUnoccupiedToOccupiedDelay {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UltrasonicUnoccupiedToOccupiedDelay

namespace UltrasonicUnoccupiedToOccupiedThreshold {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace UltrasonicUnoccupiedToOccupiedThreshold

namespace PhysicalContactOccupiedToUnoccupiedDelay {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PhysicalContactOccupiedToUnoccupiedDelay

namespace PhysicalContactUnoccupiedToOccupiedDelay {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PhysicalContactUnoccupiedToOccupiedDelay

namespace PhysicalContactUnoccupiedToOccupiedThreshold {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PhysicalContactUnoccupiedToOccupiedThreshold

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // OccupancySensing

namespace WakeOnLan {
namespace Attributes {

namespace MACAddress {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MACAddress

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // WakeOnLan

namespace Channel {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // Channel

namespace TargetNavigator {
namespace Attributes {

namespace CurrentTarget {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentTarget

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // TargetNavigator

namespace MediaPlayback {
namespace Attributes {

namespace CurrentState {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::MediaPlayback::PlaybackStateEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::MediaPlayback::PlaybackStateEnum>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::MediaPlayback::PlaybackStateEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::MediaPlayback::PlaybackStateEnum>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentState

namespace StartTime {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace StartTime

namespace Duration {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Duration

namespace PlaybackSpeed {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, float& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<float>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const float& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<float>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PlaybackSpeed

namespace SeekRangeEnd {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SeekRangeEnd

namespace SeekRangeStart {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value.SetNonNull(tmp);
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint64_t>& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType storageValue;
    if(value.HasValidValue()) {
        Traits::WorkingToStorage(value.Value(), storageValue);
    } else {
        return EMBER_ZCL_STATUS_FAILURE;
    }
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SeekRangeStart

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // MediaPlayback

namespace MediaInput {
namespace Attributes {

namespace CurrentInput {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentInput

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // MediaInput

namespace LowPower {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // LowPower

namespace KeypadInput {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // KeypadInput

namespace ContentLauncher {
namespace Attributes {

namespace SupportedStreamingProtocols {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace SupportedStreamingProtocols

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ContentLauncher

namespace AudioOutput {
namespace Attributes {

namespace CurrentOutput {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentOutput

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // AudioOutput

namespace ApplicationLauncher {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ApplicationLauncher

namespace ApplicationBasic {
namespace Attributes {

namespace VendorName {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace VendorName

namespace VendorID {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::VendorId& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::VendorId>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::VendorId& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::VendorId>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace VendorID

namespace ApplicationName {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ApplicationName

namespace ProductID {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ProductID

namespace Status {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Status

namespace ApplicationVersion {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<chip::CharSpan>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    cache.set<chip::CharSpan>(endpoint,value);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ApplicationVersion

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ApplicationBasic

namespace AccountLogin {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // AccountLogin

namespace ElectricalMeasurement {
namespace Attributes {

namespace MeasurementType {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasurementType

namespace DcVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcVoltage

namespace DcVoltageMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcVoltageMin

namespace DcVoltageMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcVoltageMax

namespace DcCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcCurrent

namespace DcCurrentMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcCurrentMin

namespace DcCurrentMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcCurrentMax

namespace DcPower {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcPower

namespace DcPowerMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcPowerMin

namespace DcPowerMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcPowerMax

namespace DcVoltageMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcVoltageMultiplier

namespace DcVoltageDivisor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcVoltageDivisor

namespace DcCurrentMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcCurrentMultiplier

namespace DcCurrentDivisor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcCurrentDivisor

namespace DcPowerMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcPowerMultiplier

namespace DcPowerDivisor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace DcPowerDivisor

namespace AcFrequency {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcFrequency

namespace AcFrequencyMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcFrequencyMin

namespace AcFrequencyMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcFrequencyMax

namespace NeutralCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace NeutralCurrent

namespace TotalActivePower {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TotalActivePower

namespace TotalReactivePower {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TotalReactivePower

namespace TotalApparentPower {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace TotalApparentPower

namespace Measured1stHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Measured1stHarmonicCurrent

namespace Measured3rdHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Measured3rdHarmonicCurrent

namespace Measured5thHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Measured5thHarmonicCurrent

namespace Measured7thHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Measured7thHarmonicCurrent

namespace Measured9thHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Measured9thHarmonicCurrent

namespace Measured11thHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace Measured11thHarmonicCurrent

namespace MeasuredPhase1stHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredPhase1stHarmonicCurrent

namespace MeasuredPhase3rdHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredPhase3rdHarmonicCurrent

namespace MeasuredPhase5thHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredPhase5thHarmonicCurrent

namespace MeasuredPhase7thHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredPhase7thHarmonicCurrent

namespace MeasuredPhase9thHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredPhase9thHarmonicCurrent

namespace MeasuredPhase11thHarmonicCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace MeasuredPhase11thHarmonicCurrent

namespace AcFrequencyMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcFrequencyMultiplier

namespace AcFrequencyDivisor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcFrequencyDivisor

namespace PowerMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PowerMultiplier

namespace PowerDivisor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PowerDivisor

namespace HarmonicCurrentMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace HarmonicCurrentMultiplier

namespace PhaseHarmonicCurrentMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PhaseHarmonicCurrentMultiplier

namespace InstantaneousVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstantaneousVoltage

namespace InstantaneousLineCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstantaneousLineCurrent

namespace InstantaneousActiveCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstantaneousActiveCurrent

namespace InstantaneousReactiveCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstantaneousReactiveCurrent

namespace InstantaneousPower {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace InstantaneousPower

namespace RmsVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltage

namespace RmsVoltageMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageMin

namespace RmsVoltageMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageMax

namespace RmsCurrent {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrent

namespace RmsCurrentMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrentMin

namespace RmsCurrentMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrentMax

namespace ActivePower {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePower

namespace ActivePowerMin {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePowerMin

namespace ActivePowerMax {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePowerMax

namespace ReactivePower {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ReactivePower

namespace ApparentPower {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ApparentPower

namespace PowerFactor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PowerFactor

namespace AverageRmsVoltageMeasurementPeriod {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsVoltageMeasurementPeriod

namespace AverageRmsUnderVoltageCounter {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsUnderVoltageCounter

namespace RmsExtremeOverVoltagePeriod {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsExtremeOverVoltagePeriod

namespace RmsExtremeUnderVoltagePeriod {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsExtremeUnderVoltagePeriod

namespace RmsVoltageSagPeriod {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageSagPeriod

namespace RmsVoltageSwellPeriod {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageSwellPeriod

namespace AcVoltageMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcVoltageMultiplier

namespace AcVoltageDivisor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcVoltageDivisor

namespace AcCurrentMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcCurrentMultiplier

namespace AcCurrentDivisor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcCurrentDivisor

namespace AcPowerMultiplier {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcPowerMultiplier

namespace AcPowerDivisor {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcPowerDivisor

namespace OverloadAlarmsMask {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace OverloadAlarmsMask

namespace VoltageOverload {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace VoltageOverload

namespace CurrentOverload {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace CurrentOverload

namespace AcOverloadAlarmsMask {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcOverloadAlarmsMask

namespace AcVoltageOverload {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcVoltageOverload

namespace AcCurrentOverload {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcCurrentOverload

namespace AcActivePowerOverload {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcActivePowerOverload

namespace AcReactivePowerOverload {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AcReactivePowerOverload

namespace AverageRmsOverVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsOverVoltage

namespace AverageRmsUnderVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsUnderVoltage

namespace RmsExtremeOverVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsExtremeOverVoltage

namespace RmsExtremeUnderVoltage {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsExtremeUnderVoltage

namespace RmsVoltageSag {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageSag

namespace RmsVoltageSwell {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageSwell

namespace LineCurrentPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LineCurrentPhaseB

namespace ActiveCurrentPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActiveCurrentPhaseB

namespace ReactiveCurrentPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ReactiveCurrentPhaseB

namespace RmsVoltagePhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltagePhaseB

namespace RmsVoltageMinPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageMinPhaseB

namespace RmsVoltageMaxPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageMaxPhaseB

namespace RmsCurrentPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrentPhaseB

namespace RmsCurrentMinPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrentMinPhaseB

namespace RmsCurrentMaxPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrentMaxPhaseB

namespace ActivePowerPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePowerPhaseB

namespace ActivePowerMinPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePowerMinPhaseB

namespace ActivePowerMaxPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePowerMaxPhaseB

namespace ReactivePowerPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ReactivePowerPhaseB

namespace ApparentPowerPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ApparentPowerPhaseB

namespace PowerFactorPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PowerFactorPhaseB

namespace AverageRmsVoltageMeasurementPeriodPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsVoltageMeasurementPeriodPhaseB

namespace AverageRmsOverVoltageCounterPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsOverVoltageCounterPhaseB

namespace AverageRmsUnderVoltageCounterPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsUnderVoltageCounterPhaseB

namespace RmsExtremeOverVoltagePeriodPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsExtremeOverVoltagePeriodPhaseB

namespace RmsExtremeUnderVoltagePeriodPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsExtremeUnderVoltagePeriodPhaseB

namespace RmsVoltageSagPeriodPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageSagPeriodPhaseB

namespace RmsVoltageSwellPeriodPhaseB {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageSwellPeriodPhaseB

namespace LineCurrentPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace LineCurrentPhaseC

namespace ActiveCurrentPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActiveCurrentPhaseC

namespace ReactiveCurrentPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ReactiveCurrentPhaseC

namespace RmsVoltagePhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltagePhaseC

namespace RmsVoltageMinPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageMinPhaseC

namespace RmsVoltageMaxPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageMaxPhaseC

namespace RmsCurrentPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrentPhaseC

namespace RmsCurrentMinPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrentMinPhaseC

namespace RmsCurrentMaxPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsCurrentMaxPhaseC

namespace ActivePowerPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePowerPhaseC

namespace ActivePowerMinPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePowerMinPhaseC

namespace ActivePowerMaxPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ActivePowerMaxPhaseC

namespace ReactivePowerPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ReactivePowerPhaseC

namespace ApparentPowerPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ApparentPowerPhaseC

namespace PowerFactorPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<int8_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace PowerFactorPhaseC

namespace AverageRmsVoltageMeasurementPeriodPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsVoltageMeasurementPeriodPhaseC

namespace AverageRmsOverVoltageCounterPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsOverVoltageCounterPhaseC

namespace AverageRmsUnderVoltageCounterPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace AverageRmsUnderVoltageCounterPhaseC

namespace RmsExtremeOverVoltagePeriodPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsExtremeOverVoltagePeriodPhaseC

namespace RmsExtremeUnderVoltagePeriodPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsExtremeUnderVoltagePeriodPhaseC

namespace RmsVoltageSagPeriodPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageSagPeriodPhaseC

namespace RmsVoltageSwellPeriodPhaseC {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace RmsVoltageSwellPeriodPhaseC

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // ElectricalMeasurement

namespace FaultInjection {
namespace Attributes {

namespace FeatureMap {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace FeatureMap

namespace ClusterRevision {





EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();

    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp_storage;

    cache.get<Traits::StorageType>(endpoint,temp_storage);    
    auto tmp = Traits::StorageToWorking(temp_storage);

    value = tmp;
    return EMBER_ZCL_STATUS_SUCCESS;    
}

EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
{
    attribute_state_cache& cache = attribute_state_cache::get_instance();    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    cache.set(endpoint,storageValue);
    return EMBER_ZCL_STATUS_SUCCESS;
}


} // namespace ClusterRevision

} // namespace Attributes
} // FaultInjection


} // matter_bridge
} // unify
