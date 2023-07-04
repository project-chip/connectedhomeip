#include <tlv/meta/clusters_meta.h>

namespace chip {
namespace TLVMeta {
namespace {

using namespace chip::FlatTree;
using namespace chip::TLV;

const Entry<ItemInfo> _OnOff[] = {
  { { AttributeTag(0), "onOff", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::boolean
  { { AttributeTag(65532), "featureMap", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::bitmap32
  { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::int16u
  { { CommandTag(0), "Off", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::Off::()
  { { CommandTag(1), "On", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::On::()
  { { CommandTag(2), "Toggle", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::Toggle::()
};

const Entry<ItemInfo> _all_clusters[] = {

};

// For any non-structure list like u64[] or similar.
const Entry<ItemInfo> _primitive_type_list[] = {
  { { AnonymousTag(), "[]", ItemType::kDefault }, kInvalidNodeIndex },
};

} // namespace

#define _ENTRY(n) { sizeof(n) / sizeof(n[0]), n}

const std::array<const Node<ItemInfo>, 1 + 2> clusters_meta = { {
  _ENTRY(_all_clusters), // 0
  _ENTRY(_primitive_type_list), // 1
  _ENTRY(_OnOff), // 2
} };

} // namespace TLVMeta
} // namespace chip
