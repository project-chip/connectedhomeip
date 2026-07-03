from dataclasses import dataclass

from matter.testing.spec_parsing import Clusters, XmlAttribute

# Maps a scenable attribute's spec data type to the typed value field of the
# Scenes cluster AttributeValuePairStruct used to carry its value.
_SCENE_VALUE_FIELDS = {
    "bool": "valueUnsigned8",
    "uint8": "valueUnsigned8", "enum8": "valueUnsigned8", "map8": "valueUnsigned8", "percent": "valueUnsigned8",
    "uint16": "valueUnsigned16", "enum16": "valueUnsigned16", "map16": "valueUnsigned16", "percent100ths": "valueUnsigned16",
    "uint24": "valueUnsigned32", "uint32": "valueUnsigned32", "map32": "valueUnsigned32", "enum32": "valueUnsigned32",
    "uint40": "valueUnsigned64", "uint48": "valueUnsigned64", "uint56": "valueUnsigned64", "uint64": "valueUnsigned64",
    "map64": "valueUnsigned64",
    "int8": "valueSigned8", "int16": "valueSigned16", "int24": "valueSigned32", "int32": "valueSigned32",
    "int40": "valueSigned64", "int48": "valueSigned64", "int56": "valueSigned64", "int64": "valueSigned64",
    "temperature": "valueSigned16",
}

# Bit width and signedness of each AttributeValuePairStruct value field.
_VALUE_FIELD_BITS = {
    "valueUnsigned8": (8, False), "valueUnsigned16": (16, False), "valueUnsigned32": (32, False),
    "valueUnsigned64": (64, False), "valueSigned8": (8, True), "valueSigned16": (16, True),
    "valueSigned32": (32, True), "valueSigned64": (64, True),
}


@dataclass
class _ScenableAttribute:
    """A concrete scenable attribute discovered on the DUT's scene endpoint."""
    cluster: Clusters.ClusterObjects.Cluster
    attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor
    xml_attribute: XmlAttribute
    value_field: str


def _scene_value_field(datatype: str) -> str | None:
    """Returns the AttributeValuePairStruct value field for a spec data type, or None if unsupported."""
    return _SCENE_VALUE_FIELDS.get(datatype.lower())


def _value_bounds(scenable: _ScenableAttribute) -> tuple[int, int]:
    """Returns a conservative [min, max] range of valid values for the scenable attribute.

    Uses the spec constraints when available, otherwise falls back to the data type's
    representable range (excluding the maximum unsigned value, which is frequently reserved).
    """
    if scenable.xml_attribute.datatype.lower() == "bool":
        return 0, 1
    bits, signed = _VALUE_FIELD_BITS[scenable.value_field]
    if signed:
        lo, hi = -(2 ** (bits - 1)), 2 ** (bits - 1) - 1
    else:
        lo, hi = 0, 2 ** bits - 2
    constraints = scenable.xml_attribute.constraints
    if constraints is not None:
        if isinstance(constraints.min_value, int):
            lo = constraints.min_value
        if isinstance(constraints.max_value, int):
            hi = constraints.max_value
    return lo, hi


def _value_other_than(value: int, lo: int, hi: int) -> int:
    """Returns an in-range value adjacent to (and different from) the given value."""
    value = int(value)
    if value + 1 <= hi:
        return value + 1
    if value - 1 >= lo:
        return value - 1
    return hi if value != hi else lo
