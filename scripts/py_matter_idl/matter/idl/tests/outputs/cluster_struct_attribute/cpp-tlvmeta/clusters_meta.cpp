#include <tlv/meta/clusters_meta.h>

namespace chip {
namespace TLVMeta {
namespace {

using namespace chip::FlatTree;
using namespace chip::TLV;

const Entry<ItemInfo> _DemoCluster[] = {
  { { AttributeTag(5), "singleFailSafe", ItemType::kDefault }, 3 }, // DemoCluster::ArmFailSafeRequest
  { { AttributeTag(100), "armFailsafes", ItemType::kList }, 4 }, // DemoCluster::ArmFailSafeRequest[]
};

const Entry<ItemInfo> _DemoCluster_ArmFailSafeRequest[] = {
  { { ContextTag(0), "expiryLengthSeconds", ItemType::kDefault }, kInvalidNodeIndex }, // DemoCluster::INT16U
  { { ContextTag(1), "breadcrumb", ItemType::kDefault }, kInvalidNodeIndex }, // DemoCluster::INT64U
  { { ContextTag(2), "timeoutMs", ItemType::kDefault }, kInvalidNodeIndex }, // DemoCluster::INT32U
};

const Entry<ItemInfo> _DemoCluster_ArmFailSafeRequest_list_[] = {
  { { AnonymousTag(), "Anonymous<>", ItemType::kDefault }, 3 }, // DemoCluster_ArmFailSafeRequest[]
};

const Entry<ItemInfo> _all_clusters[] = {
  { { ClusterTag(0x0A), "DemoCluster", ItemType::kDefault }, 2 },

};

// For any non-structure list like u64[] or similar.
const Entry<ItemInfo> _primitive_type_list[] = {
  { { AnonymousTag(), "Anonymous<>", ItemType::kDefault }, kInvalidNodeIndex },
};

} // namespace

#define _ENTRY(n) { sizeof(n) / sizeof(n[0]), n}

const std::array<const Node<ItemInfo>, 3 + 2> clusters_meta = { {
  _ENTRY(_all_clusters), // 0
  _ENTRY(_primitive_type_list), // 1
  _ENTRY(_DemoCluster), // 2
  _ENTRY(_DemoCluster_ArmFailSafeRequest), // 3
  _ENTRY(_DemoCluster_ArmFailSafeRequest_list_), // 4
} };

} // namespace TLVMeta
} // namespace chip
