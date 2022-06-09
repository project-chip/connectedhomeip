#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/attribute-id.h>

#include <new>

namespace clusters {

struct MyClusterCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = ZCL_MY_CLUSTER_CLUSTER_ID;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  EmberAfStatus Read(const EmberAfAttributeMetadata * am, uint8_t * buffer, uint16_t maxReadLength) override
  {
    switch(am->attributeId) {
    case 1:
      return mClusterAttr.Read(am, buffer, maxReadLength);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  EmberAfStatus Write(const EmberAfAttributeMetadata * am, uint8_t * buffer) override
  {
    switch(am->attributeId) {
    case 1:
      return mClusterAttr.WriteFromMatter(am, buffer, this);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  void WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer) override
  {
    switch(attributeId) {
    case 1:
      mClusterAttr.WriteFromBridge(buffer, this);
      break;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mClusterAttr);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<1, ATTRIBUTE_MASK_WRITABLE, PrimitiveType<uint16_t, 2, ZCL_INT16U_ATTRIBUTE_TYPE>> mClusterAttr;

  static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
    { 1, ZCL_INT16U_ATTRIBUTE_TYPE, 2, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
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
    ZCL_MY_CLUSTER_CLUSTER_ID,
    "MyCluster",
    sizeof(MyClusterCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) MyClusterCluster();
    },
  },
};

}