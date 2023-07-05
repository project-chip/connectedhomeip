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

const Entry<ItemInfo> _OnOff_OnOffDelayedAllOffEffectVariant[] = {
  { { ConstantValueTag(0x0), "kFadeToOffIn0p8Seconds", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffDelayedAllOffEffectVariant::kFadeToOffIn0p8Seconds
  { { ConstantValueTag(0x1), "kNoFade", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffDelayedAllOffEffectVariant::kNoFade
  { { ConstantValueTag(0x2), "k50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffDelayedAllOffEffectVariant::k50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds
};

const Entry<ItemInfo> _OnOff_OnOffDyingLightEffectVariant[] = {
  { { ConstantValueTag(0x0), "k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffDyingLightEffectVariant::k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second
};

const Entry<ItemInfo> _OnOff_OnOffEffectIdentifier[] = {
  { { ConstantValueTag(0x0), "kDelayedAllOff", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffEffectIdentifier::kDelayedAllOff
  { { ConstantValueTag(0x1), "kDyingLight", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffEffectIdentifier::kDyingLight
};

const Entry<ItemInfo> _OnOff_OnOffStartUpOnOff[] = {
  { { ConstantValueTag(0x0), "kOff", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffStartUpOnOff::kOff
  { { ConstantValueTag(0x1), "kOn", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffStartUpOnOff::kOn
  { { ConstantValueTag(0x2), "kTogglePreviousOnOff", ItemType::kDefault }, kInvalidNodeIndex }, // OnOff::OnOffStartUpOnOff::kTogglePreviousOnOff
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

};

// For any non-structure list like u64[] or similar.
const Entry<ItemInfo> _primitive_type_list[] = {
  { { AnonymousTag(), "[]", ItemType::kDefault }, kInvalidNodeIndex },
};

} // namespace

#define _ENTRY(n) { sizeof(n) / sizeof(n[0]), n}

const std::array<const Node<ItemInfo>, 8 + 2> clusters_meta = { {
  _ENTRY(_all_clusters), // 0
  _ENTRY(_primitive_type_list), // 1
  _ENTRY(_OnOff), // 2
  _ENTRY(_OnOff_OnOffDelayedAllOffEffectVariant), // 3
  _ENTRY(_OnOff_OnOffDyingLightEffectVariant), // 4
  _ENTRY(_OnOff_OnOffEffectIdentifier), // 5
  _ENTRY(_OnOff_OnOffStartUpOnOff), // 6
  _ENTRY(_OnOff_OnOffControl), // 7
  _ENTRY(_OnOff_OnOffFeature), // 8
  _ENTRY(_OnOff_ScenesFeature), // 9
} };

} // namespace TLVMeta
} // namespace chip
