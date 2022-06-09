#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/attribute-id.h>

#include <new>

namespace clusters {

struct DemoClusterCluster : public CommonCluster
{
  struct ArmFailSafeRequest
  {
    PrimitiveType<uint16_t, 2, ZCL_INT16U_ATTRIBUTE_TYPE> expiryLengthSeconds;
    PrimitiveType<uint64_t, 8, ZCL_INT64U_ATTRIBUTE_TYPE> breadcrumb;
    PrimitiveType<uint32_t, 4, ZCL_INT32U_ATTRIBUTE_TYPE> timeoutMs;
  };


  static constexpr chip::ClusterId kClusterId = ZCL_DEMO_CLUSTER_CLUSTER_ID;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  EmberAfStatus Read(const EmberAfAttributeMetadata * am, uint8_t * buffer, uint16_t maxReadLength) override
  {
    switch(am->attributeId) {
    case 5:
      return mSingleFailSafe.Read(am, buffer, maxReadLength);
    case 100:
      return mArmFailsafes.Read(am, buffer, maxReadLength);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  EmberAfStatus Write(const EmberAfAttributeMetadata * am, uint8_t * buffer) override
  {
    switch(am->attributeId) {
    case 5:
      return mSingleFailSafe.WriteFromMatter(am, buffer, this);
    case 100:
      return mArmFailsafes.WriteFromMatter(am, buffer, this);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  void WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer) override
  {
    switch(attributeId) {
    case 5:
      mSingleFailSafe.WriteFromBridge(buffer, this);
      break;
    case 100:
      mArmFailsafes.WriteFromBridge(buffer, this);
      break;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSingleFailSafe);
    list->Add(mArmFailsafes);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<5, ATTRIBUTE_MASK_WRITABLE, StructType<ArmFailSafeRequest>> mSingleFailSafe;
  Attribute<100, ATTRIBUTE_MASK_WRITABLE, ArrayType<1, StructType<ArmFailSafeRequest>>> mArmFailsafes;

  static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
    { 5, ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(ArmFailSafeRequest), ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
    { 100, ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(ArmFailSafeRequest) * 1 + 2, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
  };
};

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  CommonCluster* (*ctor)(void*);
} const kKnownClusters[] = {

  {
    ZCL_DEMO_CLUSTER_CLUSTER_ID,
    "DemoCluster",
    sizeof(DemoClusterCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) DemoClusterCluster();
    },
  },
};

}