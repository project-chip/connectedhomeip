#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/attribute-id.h>

#include <new>

namespace clusters {

struct FirstCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = ZCL_FIRST_CLUSTER_ID;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  EmberAfStatus Read(const EmberAfAttributeMetadata * am, uint8_t * buffer, uint16_t maxReadLength) override
  {
    switch(am->attributeId) {
    case 1:
      return mSomeInteger.Read(am, buffer, maxReadLength);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  EmberAfStatus Write(const EmberAfAttributeMetadata * am, uint8_t * buffer) override
  {
    switch(am->attributeId) {
    case 1:
      return mSomeInteger.WriteFromMatter(am, buffer, this);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  void WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer) override
  {
    switch(attributeId) {
    case 1:
      mSomeInteger.WriteFromBridge(buffer, this);
      break;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSomeInteger);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<1, ATTRIBUTE_MASK_WRITABLE, PrimitiveType<uint16_t, 2, ZCL_INT16U_ATTRIBUTE_TYPE>> mSomeInteger;

  static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
    { 1, ZCL_INT16U_ATTRIBUTE_TYPE, 2, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
  };
};
struct SecondCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = ZCL_SECOND_CLUSTER_ID;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  EmberAfStatus Read(const EmberAfAttributeMetadata * am, uint8_t * buffer, uint16_t maxReadLength) override
  {
    switch(am->attributeId) {
    case 123:
      return mSomeBytes.Read(am, buffer, maxReadLength);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  EmberAfStatus Write(const EmberAfAttributeMetadata * am, uint8_t * buffer) override
  {
    switch(am->attributeId) {
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  void WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer) override
  {
    switch(attributeId) {
    case 123:
      mSomeBytes.WriteFromBridge(buffer, this);
      break;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSomeBytes);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<123, 0, OctetString<32, ZCL_OCTET_STRING_ATTRIBUTE_TYPE>> mSomeBytes;

  static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
    { 123, ZCL_OCTET_STRING_ATTRIBUTE_TYPE, 32, 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
  };
};
struct ThirdCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = ZCL_THIRD_CLUSTER_ID;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  EmberAfStatus Read(const EmberAfAttributeMetadata * am, uint8_t * buffer, uint16_t maxReadLength) override
  {
    switch(am->attributeId) {
    case 10:
      return mSomeEnum.Read(am, buffer, maxReadLength);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  EmberAfStatus Write(const EmberAfAttributeMetadata * am, uint8_t * buffer) override
  {
    switch(am->attributeId) {
    case 10:
      return mSomeEnum.WriteFromMatter(am, buffer, this);
    default:
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  void WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer) override
  {
    switch(attributeId) {
    case 10:
      mSomeEnum.WriteFromBridge(buffer, this);
      break;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSomeEnum);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<10, ATTRIBUTE_MASK_WRITABLE, PrimitiveType<uint8_t, 1, ZCL_ENUM8_ATTRIBUTE_TYPE>> mSomeEnum;

  static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
    { 10, ZCL_ENUM8_ATTRIBUTE_TYPE, 1, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
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
    ZCL_FIRST_CLUSTER_ID,
    "First",
    sizeof(FirstCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) FirstCluster();
    },
  },
  {
    ZCL_SECOND_CLUSTER_ID,
    "Second",
    sizeof(SecondCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) SecondCluster();
    },
  },
  {
    ZCL_THIRD_CLUSTER_ID,
    "Third",
    sizeof(ThirdCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) ThirdCluster();
    },
  },
};

}