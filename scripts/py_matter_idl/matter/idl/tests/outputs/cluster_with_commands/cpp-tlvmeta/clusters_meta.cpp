#include <tlv/meta/clusters_meta.h>

namespace chip {
namespace TLVMeta {
namespace {

using namespace chip::FlatTree;
using namespace chip::TLV;

const Entry<ItemInfo> _OnOff[] = {
  { { AttributeTag(0), "onOff", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::boolean
  { { AttributeTag(65532), "featureMap", ItemType::kBitmap }, 8 }, // OnOff::OnOffFeature
  { { AttributeTag(65533), "clusterRevision", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::int16u
  { { CommandTag(0), "Off", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::Off::()
  { { CommandTag(1), "On", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::On::()
  { { CommandTag(2), "Toggle", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::Toggle::()
};

const Entry<ItemInfo> _OnOff_DelayedAllOffEffectVariantEnum[] = {
  { { ConstantValueTag(0x0), "kDelayedOffFastFade", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade
  { { ConstantValueTag(0x1), "kNoFade", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::DelayedAllOffEffectVariantEnum::kNoFade
  { { ConstantValueTag(0x2), "kDelayedOffSlowFade", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade
};

const Entry<ItemInfo> _OnOff_DyingLightEffectVariantEnum[] = {
  { { ConstantValueTag(0x0), "kDyingLightFadeOff", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::DyingLightEffectVariantEnum::kDyingLightFadeOff
};

const Entry<ItemInfo> _OnOff_EffectIdentifierEnum[] = {
  { { ConstantValueTag(0x0), "kDelayedAllOff", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::EffectIdentifierEnum::kDelayedAllOff
  { { ConstantValueTag(0x1), "kDyingLight", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::EffectIdentifierEnum::kDyingLight
};

const Entry<ItemInfo> _OnOff_StartUpOnOffEnum[] = {
  { { ConstantValueTag(0x0), "kOff", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::StartUpOnOffEnum::kOff
  { { ConstantValueTag(0x1), "kOn", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::StartUpOnOffEnum::kOn
  { { ConstantValueTag(0x2), "kToggle", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::StartUpOnOffEnum::kToggle
};

const Entry<ItemInfo> _OnOff_OnOffControl[] = {
  { { ConstantValueTag(0x1), "kAcceptOnlyWhenOn", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffControl::kAcceptOnlyWhenOn
};

const Entry<ItemInfo> _OnOff_OnOffFeature[] = {
  { { ConstantValueTag(0x1), "kLighting", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffFeature::kLighting
};

const Entry<ItemInfo> _OnOff_ScenesFeature[] = {
  { { ConstantValueTag(0x1), "kSceneNames", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::ScenesFeature::kSceneNames
};

const Entry<ItemInfo> _all_clusters[] = {
  { { ClusterTag(0x06), "OnOff", ItemType::kDefault }, 2 },

};

// For any non-structure list like u64[] or similar.
const Entry<ItemInfo> _primitive_type_list[] = {
  { { AnonymousTag(), "Anonymous<>", ItemType::kDefault }, kInvalidNodeIndex },
};

} // namespace

#define _ENTRY(n) { sizeof(n) / sizeof(n[0]), n}

const std::array<const Node<ItemInfo>, 8 + 2> clusters_meta = { {
  _ENTRY(_all_clusters), // 0
  _ENTRY(_primitive_type_list), // 1
  _ENTRY(_OnOff), // 2
  _ENTRY(_OnOff_DelayedAllOffEffectVariantEnum), // 3
  _ENTRY(_OnOff_DyingLightEffectVariantEnum), // 4
  _ENTRY(_OnOff_EffectIdentifierEnum), // 5
  _ENTRY(_OnOff_StartUpOnOffEnum), // 6
  _ENTRY(_OnOff_OnOffControl), // 7
  _ENTRY(_OnOff_OnOffFeature), // 8
  _ENTRY(_OnOff_ScenesFeature), // 9
} };

} // namespace TLVMeta
} // namespace chip
