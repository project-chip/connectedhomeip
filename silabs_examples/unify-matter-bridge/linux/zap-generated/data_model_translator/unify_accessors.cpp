
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

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace IdentifyTime

            namespace IdentifyType {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace IdentifyType

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace Identify

    namespace Groups {
        namespace Attributes {

            namespace NameSupport {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NameSupport

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace Groups

    namespace Scenes {
        namespace Attributes {

            namespace SceneCount {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SceneCount

            namespace CurrentScene {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentScene

            namespace CurrentGroup {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::GroupId& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::GroupId>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::GroupId& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::GroupId>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentGroup

            namespace SceneValid {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SceneValid

            namespace NameSupport {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NameSupport

            namespace LastConfiguredBy {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<chip::NodeId>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::NodeId>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<chip::NodeId>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::NodeId>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LastConfiguredBy

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace Scenes

    namespace OnOff {
        namespace Attributes {

            namespace OnOff {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OnOff

            namespace GlobalSceneControl {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace GlobalSceneControl

            namespace OnTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OnTime

            namespace OffWaitTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OffWaitTime

            namespace StartUpOnOff {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffStartUpOnOff>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::app::Clusters::OnOff::OnOffStartUpOnOff>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::app::DataModel::Nullable<chip::app::Clusters::OnOff::OnOffStartUpOnOff>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::app::Clusters::OnOff::OnOffStartUpOnOff>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace StartUpOnOff

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace OnOff

    namespace LevelControl {
        namespace Attributes {

            namespace CurrentLevel {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentLevel

            namespace RemainingTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RemainingTime

            namespace MinLevel {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinLevel

            namespace MaxLevel {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxLevel

            namespace CurrentFrequency {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentFrequency

            namespace MinFrequency {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinFrequency

            namespace MaxFrequency {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxFrequency

            namespace Options {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions>>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions>>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Options

            namespace OnOffTransitionTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OnOffTransitionTime

            namespace OnLevel {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OnLevel

            namespace OnTransitionTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OnTransitionTime

            namespace OffTransitionTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OffTransitionTime

            namespace DefaultMoveRate {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DefaultMoveRate

            namespace StartUpCurrentLevel {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace StartUpCurrentLevel

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace LevelControl

    namespace DoorLock {
        namespace Attributes {

            namespace LockState {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlLockState>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlLockState>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LockState

            namespace LockType {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::DoorLock::DlLockType& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlLockType>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::DoorLock::DlLockType& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlLockType>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LockType

            namespace ActuatorEnabled {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActuatorEnabled

            namespace DoorState {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlDoorState>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlDoorState>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlDoorState>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlDoorState>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DoorState

            namespace DoorOpenEvents {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DoorOpenEvents

            namespace DoorClosedEvents {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DoorClosedEvents

            namespace OpenPeriod {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OpenPeriod

            namespace NumberOfTotalUsersSupported {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfTotalUsersSupported

            namespace NumberOfPINUsersSupported {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfPINUsersSupported

            namespace NumberOfRFIDUsersSupported {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfRFIDUsersSupported

            namespace NumberOfWeekDaySchedulesSupportedPerUser {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfWeekDaySchedulesSupportedPerUser

            namespace NumberOfYearDaySchedulesSupportedPerUser {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfYearDaySchedulesSupportedPerUser

            namespace NumberOfHolidaySchedulesSupported {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfHolidaySchedulesSupported

            namespace MaxPINCodeLength {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxPINCodeLength

            namespace MinPINCodeLength {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinPINCodeLength

            namespace MaxRFIDCodeLength {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxRFIDCodeLength

            namespace MinRFIDCodeLength {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinRFIDCodeLength

            namespace CredentialRulesSupport {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlCredentialRuleMask>>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CredentialRulesSupport

            namespace NumberOfCredentialsSupportedPerUser {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfCredentialsSupportedPerUser

            namespace Language {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::CharSpan>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    cache.set<chip::CharSpan>(endpoint, value);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Language

            namespace LEDSettings {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LEDSettings

            namespace AutoRelockTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AutoRelockTime

            namespace SoundVolume {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SoundVolume

            namespace OperatingMode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::DoorLock::DlOperatingMode& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlOperatingMode>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::DoorLock::DlOperatingMode& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::app::Clusters::DoorLock::DlOperatingMode>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OperatingMode

            namespace SupportedOperatingModes {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlSupportedOperatingModes>>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SupportedOperatingModes

            namespace DefaultConfigurationRegister {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlDefaultConfigurationRegister>>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DefaultConfigurationRegister

            namespace EnableLocalProgramming {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace EnableLocalProgramming

            namespace EnableOneTouchLocking {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace EnableOneTouchLocking

            namespace EnableInsideStatusLED {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace EnableInsideStatusLED

            namespace EnablePrivacyModeButton {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace EnablePrivacyModeButton

            namespace LocalProgrammingFeatures {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::DoorLock::DlLocalProgrammingFeatures>>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LocalProgrammingFeatures

            namespace WrongCodeEntryLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace WrongCodeEntryLimit

            namespace UserCodeTemporaryDisableTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UserCodeTemporaryDisableTime

            namespace SendPINOverTheAir {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SendPINOverTheAir

            namespace RequirePINforRemoteOperation {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const bool& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<bool>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RequirePINforRemoteOperation

            namespace ExpiringUserTimeout {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ExpiringUserTimeout

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace DoorLock

    namespace BarrierControl {
        namespace Attributes {

            namespace BarrierMovingState {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierMovingState

            namespace BarrierSafetyStatus {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierSafetyStatus

            namespace BarrierCapabilities {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierCapabilities

            namespace BarrierOpenEvents {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierOpenEvents

            namespace BarrierCloseEvents {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierCloseEvents

            namespace BarrierCommandOpenEvents {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierCommandOpenEvents

            namespace BarrierCommandCloseEvents {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierCommandCloseEvents

            namespace BarrierOpenPeriod {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierOpenPeriod

            namespace BarrierClosePeriod {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierClosePeriod

            namespace BarrierPosition {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace BarrierPosition

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace BarrierControl

    namespace Thermostat {
        namespace Attributes {

            namespace LocalTemperature {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LocalTemperature

            namespace OutdoorTemperature {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OutdoorTemperature

            namespace Occupancy {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Occupancy

            namespace AbsMinHeatSetpointLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AbsMinHeatSetpointLimit

            namespace AbsMaxHeatSetpointLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AbsMaxHeatSetpointLimit

            namespace AbsMinCoolSetpointLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AbsMinCoolSetpointLimit

            namespace AbsMaxCoolSetpointLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AbsMaxCoolSetpointLimit

            namespace PICoolingDemand {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PICoolingDemand

            namespace PIHeatingDemand {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PIHeatingDemand

            namespace HVACSystemTypeConfiguration {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace HVACSystemTypeConfiguration

            namespace LocalTemperatureCalibration {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LocalTemperatureCalibration

            namespace OccupiedCoolingSetpoint {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OccupiedCoolingSetpoint

            namespace OccupiedHeatingSetpoint {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OccupiedHeatingSetpoint

            namespace UnoccupiedCoolingSetpoint {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UnoccupiedCoolingSetpoint

            namespace UnoccupiedHeatingSetpoint {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UnoccupiedHeatingSetpoint

            namespace MinHeatSetpointLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinHeatSetpointLimit

            namespace MaxHeatSetpointLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxHeatSetpointLimit

            namespace MinCoolSetpointLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinCoolSetpointLimit

            namespace MaxCoolSetpointLimit {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxCoolSetpointLimit

            namespace MinSetpointDeadBand {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinSetpointDeadBand

            namespace RemoteSensing {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RemoteSensing

            namespace ControlSequenceOfOperation {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint,
                    chip::app::Clusters::Thermostat::ThermostatControlSequence& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::app::Clusters::Thermostat::ThermostatControlSequence>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::app::Clusters::Thermostat::ThermostatControlSequence& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::app::Clusters::Thermostat::ThermostatControlSequence>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ControlSequenceOfOperation

            namespace SystemMode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SystemMode

            namespace ThermostatRunningMode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ThermostatRunningMode

            namespace StartOfWeek {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace StartOfWeek

            namespace NumberOfWeeklyTransitions {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfWeeklyTransitions

            namespace NumberOfDailyTransitions {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfDailyTransitions

            namespace TemperatureSetpointHold {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace TemperatureSetpointHold

            namespace TemperatureSetpointHoldDuration {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace TemperatureSetpointHoldDuration

            namespace ThermostatProgrammingOperationMode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ThermostatProgrammingOperationMode

            namespace ThermostatRunningState {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ThermostatRunningState

            namespace SetpointChangeSource {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SetpointChangeSource

            namespace SetpointChangeAmount {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SetpointChangeAmount

            namespace SetpointChangeSourceTimestamp {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SetpointChangeSourceTimestamp

            namespace OccupiedSetback {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OccupiedSetback

            namespace OccupiedSetbackMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OccupiedSetbackMin

            namespace OccupiedSetbackMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OccupiedSetbackMax

            namespace UnoccupiedSetback {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UnoccupiedSetback

            namespace UnoccupiedSetbackMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UnoccupiedSetbackMin

            namespace UnoccupiedSetbackMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UnoccupiedSetbackMax

            namespace EmergencyHeatDelta {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace EmergencyHeatDelta

            namespace ACType {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ACType

            namespace ACCapacity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ACCapacity

            namespace ACRefrigerantType {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ACRefrigerantType

            namespace ACCompressorType {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ACCompressorType

            namespace ACErrorCode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ACErrorCode

            namespace ACLouverPosition {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ACLouverPosition

            namespace ACCoilTemperature {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ACCoilTemperature

            namespace ACCapacityformat {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ACCapacityformat

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace Thermostat

    namespace FanControl {
        namespace Attributes {

            namespace FanMode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::FanControl::FanModeType& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::app::Clusters::FanControl::FanModeType>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::Clusters::FanControl::FanModeType& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::app::Clusters::FanControl::FanModeType>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FanMode

            namespace FanModeSequence {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::Clusters::FanControl::FanModeSequenceType& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::app::Clusters::FanControl::FanModeSequenceType>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint,
                    const chip::app::Clusters::FanControl::FanModeSequenceType& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<chip::app::Clusters::FanControl::FanModeSequenceType>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FanModeSequence

            namespace PercentSetting {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PercentSetting

            namespace PercentCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PercentCurrent

            namespace SpeedMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SpeedMax

            namespace SpeedSetting {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SpeedSetting

            namespace SpeedCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace SpeedCurrent

            namespace RockSupport {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RockSupport

            namespace RockSetting {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RockSetting

            namespace WindSupport {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace WindSupport

            namespace WindSetting {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace WindSetting

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace FanControl

    namespace ThermostatUserInterfaceConfiguration {
        namespace Attributes {

            namespace TemperatureDisplayMode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace TemperatureDisplayMode

            namespace KeypadLockout {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace KeypadLockout

            namespace ScheduleProgrammingVisibility {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ScheduleProgrammingVisibility

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace ThermostatUserInterfaceConfiguration

    namespace ColorControl {
        namespace Attributes {

            namespace CurrentHue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentHue

            namespace CurrentSaturation {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentSaturation

            namespace RemainingTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RemainingTime

            namespace CurrentX {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentX

            namespace CurrentY {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentY

            namespace DriftCompensation {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DriftCompensation

            namespace CompensationText {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::CharSpan& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<chip::CharSpan>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::CharSpan& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    cache.set<chip::CharSpan>(endpoint, value);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CompensationText

            namespace ColorTemperatureMireds {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorTemperatureMireds

            namespace ColorMode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorMode

            namespace Options {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Options

            namespace NumberOfPrimaries {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NumberOfPrimaries

            namespace Primary1X {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary1X

            namespace Primary1Y {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary1Y

            namespace Primary1Intensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary1Intensity

            namespace Primary2X {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary2X

            namespace Primary2Y {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary2Y

            namespace Primary2Intensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary2Intensity

            namespace Primary3X {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary3X

            namespace Primary3Y {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary3Y

            namespace Primary3Intensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary3Intensity

            namespace Primary4X {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary4X

            namespace Primary4Y {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary4Y

            namespace Primary4Intensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary4Intensity

            namespace Primary5X {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary5X

            namespace Primary5Y {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary5Y

            namespace Primary5Intensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary5Intensity

            namespace Primary6X {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary6X

            namespace Primary6Y {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary6Y

            namespace Primary6Intensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Primary6Intensity

            namespace WhitePointX {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace WhitePointX

            namespace WhitePointY {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace WhitePointY

            namespace ColorPointRX {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointRX

            namespace ColorPointRY {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointRY

            namespace ColorPointRIntensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointRIntensity

            namespace ColorPointGX {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointGX

            namespace ColorPointGY {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointGY

            namespace ColorPointGIntensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointGIntensity

            namespace ColorPointBX {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointBX

            namespace ColorPointBY {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointBY

            namespace ColorPointBIntensity {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorPointBIntensity

            namespace EnhancedCurrentHue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace EnhancedCurrentHue

            namespace EnhancedColorMode {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace EnhancedColorMode

            namespace ColorLoopActive {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorLoopActive

            namespace ColorLoopDirection {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorLoopDirection

            namespace ColorLoopTime {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorLoopTime

            namespace ColorLoopStartEnhancedHue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorLoopStartEnhancedHue

            namespace ColorLoopStoredEnhancedHue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorLoopStoredEnhancedHue

            namespace ColorCapabilities {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorCapabilities

            namespace ColorTempPhysicalMinMireds {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorTempPhysicalMinMireds

            namespace ColorTempPhysicalMaxMireds {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ColorTempPhysicalMaxMireds

            namespace CoupleColorTempToLevelMinMireds {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CoupleColorTempToLevelMinMireds

            namespace StartUpColorTemperatureMireds {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace StartUpColorTemperatureMireds

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace ColorControl

    namespace IlluminanceMeasurement {
        namespace Attributes {

            namespace MeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredValue

            namespace MinMeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinMeasuredValue

            namespace MaxMeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxMeasuredValue

            namespace Tolerance {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Tolerance

            namespace LightSensorType {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint8_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LightSensorType

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace IlluminanceMeasurement

    namespace TemperatureMeasurement {
        namespace Attributes {

            namespace MeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredValue

            namespace MinMeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinMeasuredValue

            namespace MaxMeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxMeasuredValue

            namespace Tolerance {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Tolerance

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace TemperatureMeasurement

    namespace PressureMeasurement {
        namespace Attributes {

            namespace MeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredValue

            namespace MinMeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinMeasuredValue

            namespace MaxMeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxMeasuredValue

            namespace Tolerance {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Tolerance

            namespace ScaledValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ScaledValue

            namespace MinScaledValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinScaledValue

            namespace MaxScaledValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<int16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxScaledValue

            namespace ScaledTolerance {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ScaledTolerance

            namespace Scale {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Scale

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace PressureMeasurement

    namespace RelativeHumidityMeasurement {
        namespace Attributes {

            namespace MeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredValue

            namespace MinMeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MinMeasuredValue

            namespace MaxMeasuredValue {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value.SetNonNull(tmp);
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const chip::app::DataModel::Nullable<uint16_t>& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    if (value.HasValidValue()) {
                        Traits::WorkingToStorage(value.Value(), storageValue);
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MaxMeasuredValue

            namespace Tolerance {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Tolerance

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace RelativeHumidityMeasurement

    namespace OccupancySensing {
        namespace Attributes {

            namespace Occupancy {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Occupancy

            namespace OccupancySensorType {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OccupancySensorType

            namespace OccupancySensorTypeBitmap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OccupancySensorTypeBitmap

            namespace PirOccupiedToUnoccupiedDelay {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PirOccupiedToUnoccupiedDelay

            namespace PirUnoccupiedToOccupiedDelay {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PirUnoccupiedToOccupiedDelay

            namespace PirUnoccupiedToOccupiedThreshold {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PirUnoccupiedToOccupiedThreshold

            namespace UltrasonicOccupiedToUnoccupiedDelay {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UltrasonicOccupiedToUnoccupiedDelay

            namespace UltrasonicUnoccupiedToOccupiedDelay {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UltrasonicUnoccupiedToOccupiedDelay

            namespace UltrasonicUnoccupiedToOccupiedThreshold {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace UltrasonicUnoccupiedToOccupiedThreshold

            namespace PhysicalContactOccupiedToUnoccupiedDelay {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PhysicalContactOccupiedToUnoccupiedDelay

            namespace PhysicalContactUnoccupiedToOccupiedDelay {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PhysicalContactUnoccupiedToOccupiedDelay

            namespace PhysicalContactUnoccupiedToOccupiedThreshold {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PhysicalContactUnoccupiedToOccupiedThreshold

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace OccupancySensing

    namespace ElectricalMeasurement {
        namespace Attributes {

            namespace MeasurementType {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasurementType

            namespace DcVoltage {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcVoltage

            namespace DcVoltageMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcVoltageMin

            namespace DcVoltageMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcVoltageMax

            namespace DcCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcCurrent

            namespace DcCurrentMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcCurrentMin

            namespace DcCurrentMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcCurrentMax

            namespace DcPower {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcPower

            namespace DcPowerMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcPowerMin

            namespace DcPowerMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcPowerMax

            namespace DcVoltageMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcVoltageMultiplier

            namespace DcVoltageDivisor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcVoltageDivisor

            namespace DcCurrentMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcCurrentMultiplier

            namespace DcCurrentDivisor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcCurrentDivisor

            namespace DcPowerMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcPowerMultiplier

            namespace DcPowerDivisor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace DcPowerDivisor

            namespace AcFrequency {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcFrequency

            namespace AcFrequencyMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcFrequencyMin

            namespace AcFrequencyMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcFrequencyMax

            namespace NeutralCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace NeutralCurrent

            namespace TotalActivePower {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace TotalActivePower

            namespace TotalReactivePower {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace TotalReactivePower

            namespace TotalApparentPower {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace TotalApparentPower

            namespace Measured1stHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Measured1stHarmonicCurrent

            namespace Measured3rdHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Measured3rdHarmonicCurrent

            namespace Measured5thHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Measured5thHarmonicCurrent

            namespace Measured7thHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Measured7thHarmonicCurrent

            namespace Measured9thHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Measured9thHarmonicCurrent

            namespace Measured11thHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace Measured11thHarmonicCurrent

            namespace MeasuredPhase1stHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredPhase1stHarmonicCurrent

            namespace MeasuredPhase3rdHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredPhase3rdHarmonicCurrent

            namespace MeasuredPhase5thHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredPhase5thHarmonicCurrent

            namespace MeasuredPhase7thHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredPhase7thHarmonicCurrent

            namespace MeasuredPhase9thHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredPhase9thHarmonicCurrent

            namespace MeasuredPhase11thHarmonicCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace MeasuredPhase11thHarmonicCurrent

            namespace AcFrequencyMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcFrequencyMultiplier

            namespace AcFrequencyDivisor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcFrequencyDivisor

            namespace PowerMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PowerMultiplier

            namespace PowerDivisor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PowerDivisor

            namespace HarmonicCurrentMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace HarmonicCurrentMultiplier

            namespace PhaseHarmonicCurrentMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PhaseHarmonicCurrentMultiplier

            namespace InstantaneousVoltage {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace InstantaneousVoltage

            namespace InstantaneousLineCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace InstantaneousLineCurrent

            namespace InstantaneousActiveCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace InstantaneousActiveCurrent

            namespace InstantaneousReactiveCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace InstantaneousReactiveCurrent

            namespace InstantaneousPower {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace InstantaneousPower

            namespace RmsVoltage {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltage

            namespace RmsVoltageMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageMin

            namespace RmsVoltageMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageMax

            namespace RmsCurrent {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrent

            namespace RmsCurrentMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrentMin

            namespace RmsCurrentMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrentMax

            namespace ActivePower {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePower

            namespace ActivePowerMin {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePowerMin

            namespace ActivePowerMax {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePowerMax

            namespace ReactivePower {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ReactivePower

            namespace ApparentPower {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ApparentPower

            namespace PowerFactor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PowerFactor

            namespace AverageRmsVoltageMeasurementPeriod {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsVoltageMeasurementPeriod

            namespace AverageRmsUnderVoltageCounter {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsUnderVoltageCounter

            namespace RmsExtremeOverVoltagePeriod {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsExtremeOverVoltagePeriod

            namespace RmsExtremeUnderVoltagePeriod {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsExtremeUnderVoltagePeriod

            namespace RmsVoltageSagPeriod {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageSagPeriod

            namespace RmsVoltageSwellPeriod {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageSwellPeriod

            namespace AcVoltageMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcVoltageMultiplier

            namespace AcVoltageDivisor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcVoltageDivisor

            namespace AcCurrentMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcCurrentMultiplier

            namespace AcCurrentDivisor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcCurrentDivisor

            namespace AcPowerMultiplier {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcPowerMultiplier

            namespace AcPowerDivisor {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcPowerDivisor

            namespace OverloadAlarmsMask {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace OverloadAlarmsMask

            namespace VoltageOverload {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace VoltageOverload

            namespace CurrentOverload {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace CurrentOverload

            namespace AcOverloadAlarmsMask {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcOverloadAlarmsMask

            namespace AcVoltageOverload {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcVoltageOverload

            namespace AcCurrentOverload {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcCurrentOverload

            namespace AcActivePowerOverload {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcActivePowerOverload

            namespace AcReactivePowerOverload {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AcReactivePowerOverload

            namespace AverageRmsOverVoltage {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsOverVoltage

            namespace AverageRmsUnderVoltage {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsUnderVoltage

            namespace RmsExtremeOverVoltage {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsExtremeOverVoltage

            namespace RmsExtremeUnderVoltage {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsExtremeUnderVoltage

            namespace RmsVoltageSag {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageSag

            namespace RmsVoltageSwell {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageSwell

            namespace LineCurrentPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LineCurrentPhaseB

            namespace ActiveCurrentPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActiveCurrentPhaseB

            namespace ReactiveCurrentPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ReactiveCurrentPhaseB

            namespace RmsVoltagePhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltagePhaseB

            namespace RmsVoltageMinPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageMinPhaseB

            namespace RmsVoltageMaxPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageMaxPhaseB

            namespace RmsCurrentPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrentPhaseB

            namespace RmsCurrentMinPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrentMinPhaseB

            namespace RmsCurrentMaxPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrentMaxPhaseB

            namespace ActivePowerPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePowerPhaseB

            namespace ActivePowerMinPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePowerMinPhaseB

            namespace ActivePowerMaxPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePowerMaxPhaseB

            namespace ReactivePowerPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ReactivePowerPhaseB

            namespace ApparentPowerPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ApparentPowerPhaseB

            namespace PowerFactorPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PowerFactorPhaseB

            namespace AverageRmsVoltageMeasurementPeriodPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsVoltageMeasurementPeriodPhaseB

            namespace AverageRmsOverVoltageCounterPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsOverVoltageCounterPhaseB

            namespace AverageRmsUnderVoltageCounterPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsUnderVoltageCounterPhaseB

            namespace RmsExtremeOverVoltagePeriodPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsExtremeOverVoltagePeriodPhaseB

            namespace RmsExtremeUnderVoltagePeriodPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsExtremeUnderVoltagePeriodPhaseB

            namespace RmsVoltageSagPeriodPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageSagPeriodPhaseB

            namespace RmsVoltageSwellPeriodPhaseB {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageSwellPeriodPhaseB

            namespace LineCurrentPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace LineCurrentPhaseC

            namespace ActiveCurrentPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActiveCurrentPhaseC

            namespace ReactiveCurrentPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ReactiveCurrentPhaseC

            namespace RmsVoltagePhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltagePhaseC

            namespace RmsVoltageMinPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageMinPhaseC

            namespace RmsVoltageMaxPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageMaxPhaseC

            namespace RmsCurrentPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrentPhaseC

            namespace RmsCurrentMinPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrentMinPhaseC

            namespace RmsCurrentMaxPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsCurrentMaxPhaseC

            namespace ActivePowerPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePowerPhaseC

            namespace ActivePowerMinPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePowerMinPhaseC

            namespace ActivePowerMaxPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ActivePowerMaxPhaseC

            namespace ReactivePowerPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ReactivePowerPhaseC

            namespace ApparentPowerPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ApparentPowerPhaseC

            namespace PowerFactorPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const int8_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<int8_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace PowerFactorPhaseC

            namespace AverageRmsVoltageMeasurementPeriodPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsVoltageMeasurementPeriodPhaseC

            namespace AverageRmsOverVoltageCounterPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsOverVoltageCounterPhaseC

            namespace AverageRmsUnderVoltageCounterPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace AverageRmsUnderVoltageCounterPhaseC

            namespace RmsExtremeOverVoltagePeriodPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsExtremeOverVoltagePeriodPhaseC

            namespace RmsExtremeUnderVoltagePeriodPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsExtremeUnderVoltagePeriodPhaseC

            namespace RmsVoltageSagPeriodPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageSagPeriodPhaseC

            namespace RmsVoltageSwellPeriodPhaseC {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace RmsVoltageSwellPeriodPhaseC

            namespace FeatureMap {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint32_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint32_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace FeatureMap

            namespace ClusterRevision {

                EmberAfStatus Get(const chip::app::ConcreteAttributePath& endpoint, uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();

                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType temp_storage;

                    if (cache.get<Traits::StorageType>(endpoint, temp_storage)) {
                        auto tmp = Traits::StorageToWorking(temp_storage);

                        value = tmp;
                        return EMBER_ZCL_STATUS_SUCCESS;
                    } else {
                        return EMBER_ZCL_STATUS_FAILURE;
                    }
                }

                EmberAfStatus Set(const chip::app::ConcreteAttributePath& endpoint, const uint16_t& value)
                {
                    attribute_state_cache& cache = attribute_state_cache::get_instance();
                    using Traits = NumericAttributeTraits<uint16_t>;
                    Traits::StorageType storageValue;
                    Traits::WorkingToStorage(value, storageValue);
                    cache.set(endpoint, storageValue);
                    return EMBER_ZCL_STATUS_SUCCESS;
                }

            } // namespace ClusterRevision

        } // namespace Attributes
    } // namespace ElectricalMeasurement

} // namespace matter_bridge
} // namespace unify
