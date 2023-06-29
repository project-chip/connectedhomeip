#include <lib/core/TLVTags.h>
#include <lib/format/FlatTree.h>

#include <array>

namespace chip {
namespace TLVMeta {

// TODO: move definition outside
static constexpr uint32_t kAttributeProfile = 1;
static constexpr uint32_t kCommandProfile = 2;
static constexpr uint32_t kEventProfile = 3;

constexpr TLV::Tag ClusterTag(uint32_t cluster_id) {
  return TLV::CommonTag(cluster_id);
}

constexpr TLV::Tag AttributeTag(uint32_t attribute_id) {
  return TLV::ProfileTag(kAttributeProfile, attribute_id);
}

constexpr TLV::Tag CommandTag(uint32_t command_id) {
  return TLV::ProfileTag(kCommandProfile, command_id);
}

constexpr TLV::Tag EventTag(uint32_t event_id) {
  return TLV::ProfileTag(kEventProfile, event_id);
}

enum class ItemType : uint8_t {
  kDefault = 0,
  kList = 1,
  kEnum = 2,
  kBitmap = 4,
};

struct ItemInfo {
  TLV::Tag tag;
  const char *name;
  ItemType type;
};

extern std::array<FlatTree::Node<ItemInfo>, 53 + 1> protocols_meta;

} // namespace TLVMeta
} // namespace chip
