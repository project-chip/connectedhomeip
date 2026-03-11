from enum import Enum
from functools import lru_cache

from grapheme.grapheme_property_group import ContainerNode, LeafNode, load_file


class InCBPropertyGroup(Enum):
    CONSONANT = "InCBConsonant"
    EXTEND = "InCBExtend"
    LINKER = "InCBLinker"

    OTHER = "Other"


SINGLE_CHAR_MAPPINGS = {}

RANGE_TREE = ContainerNode([LeafNode(0, 0, None)])


def get_group(char):
    if char in COMMON_OTHER_GROUP_CHARS:
        return InCBPropertyGroup.OTHER
    return get_group_ord(ord(char))


@lru_cache(128)
def get_group_ord(char):
    group = SINGLE_CHAR_MAPPINGS.get(char, None)
    if group:
        return group

    return RANGE_TREE.get_value(char) or InCBPropertyGroup.OTHER


SINGLE_CHAR_MAPPINGS, RANGE_TREE, COMMON_OTHER_GROUP_CHARS = load_file(
    "data/derived_core_property.json", InCBPropertyGroup
)
