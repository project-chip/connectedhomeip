#include <lib/format/tlv_meta.h>
#include <lib/format/FlatTree.h>

#include <array>

namespace chip {
namespace TLVMeta {

extern const std::array<const FlatTree::Node<ItemInfo>, 8 + 2> clusters_meta;

} // namespace TLVMeta
} // namespace chip
