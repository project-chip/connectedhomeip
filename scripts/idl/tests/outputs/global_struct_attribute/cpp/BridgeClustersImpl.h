#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/attribute-id.h>

#include <new>

namespace clusters {

struct LabelStruct
{
  OctetString<16, ZCL_CHAR_STRING_ATTRIBUTE_TYPE> label;
  OctetString<16, ZCL_CHAR_STRING_ATTRIBUTE_TYPE> value;
};
struct DemoClusterCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = ZCL_DEMO_CLUSTER_CLUSTER_ID;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  EmberAfStatus Read(const EmberAfAttributeMetadata * am, uint8_t * buffer, uint16_t maxReadLength) override
  {
    switch(am->attributeId) {
    case 32:
      return mSingleLabel.Read(am, buffer, maxReadLength);
    case 33:
      return mSomeLabels.Read(am, buffer, maxReadLength);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  EmberAfStatus Write(const EmberAfAttributeMetadata * am, uint8_t * buffer) override
  {
    switch(am->attributeId) {
    case 32:
      return mSingleLabel.WriteFromMatter(am, buffer, this);
    case 33:
      return mSomeLabels.WriteFromMatter(am, buffer, this);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  void WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer) override
  {
    switch(attributeId) {
    case 32:
      mSingleLabel.WriteFromBridge(buffer, this);
      break;
    case 33:
      mSomeLabels.WriteFromBridge(buffer, this);
      break;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSingleLabel);
    list->Add(mSomeLabels);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<32, ATTRIBUTE_MASK_WRITABLE, StructType<LabelStruct>> mSingleLabel;
  Attribute<33, ATTRIBUTE_MASK_WRITABLE, ArrayType<1, StructType<LabelStruct>>> mSomeLabels;

  static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
    { 32, ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(LabelStruct), ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
    { 33, ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(LabelStruct) * 1 + 2, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
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