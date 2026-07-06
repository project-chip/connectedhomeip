from dataclasses import dataclass

from mobly import asserts

from matter.testing.matter_testing import MatterBaseTest
from matter.testing.spec_parsing import Clusters, XmlAttribute, build_xml_clusters, dm_from_spec_version

# Maps a scenable attribute's spec data type to the typed value field of the
# Scenes cluster AttributeValuePairStruct used to carry its value.
_SCENE_VALUE_FIELDS = {
    "bool": "valueUnsigned8",
    "uint8": "valueUnsigned8",
    "enum8": "valueUnsigned8",
    "map8": "valueUnsigned8",
    "percent": "valueUnsigned8",
    "uint16": "valueUnsigned16",
    "enum16": "valueUnsigned16",
    "map16": "valueUnsigned16",
    "percent100ths": "valueUnsigned16",
    "uint24": "valueUnsigned32",
    "uint32": "valueUnsigned32",
    "map32": "valueUnsigned32",
    "enum32": "valueUnsigned32",
    "uint40": "valueUnsigned64",
    "uint48": "valueUnsigned64",
    "uint56": "valueUnsigned64",
    "uint64": "valueUnsigned64",
    "map64": "valueUnsigned64",
    "int8": "valueSigned8",
    "int16": "valueSigned16",
    "int24": "valueSigned32",
    "int32": "valueSigned32",
    "int40": "valueSigned64",
    "int48": "valueSigned64",
    "int56": "valueSigned64",
    "int64": "valueSigned64",
    "temperature": "valueSigned16",
}

# Bit width and signedness of each AttributeValuePairStruct value field.
_VALUE_FIELD_BITS = {
    "valueUnsigned8": (8, False),
    "valueUnsigned16": (16, False),
    "valueUnsigned32": (32, False),
    "valueUnsigned64": (64, False),
    "valueSigned8": (8, True),
    "valueSigned16": (16, True),
    "valueSigned32": (32, True),
    "valueSigned64": (64, True),
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


async def _select_scenable_attribute(test: MatterBaseTest, scene_endpoint: int) -> _ScenableAttribute:
    """Discovers a scenable attribute present on the scene endpoint.

    Cross-references the attributes the DUT actually exposes on the scene endpoint
    with the data model 'scene' (S) quality.
    Boolean attributes (e.g. OnOff) are preferred because two distinct valid values are
    always available and unambiguous.
    """
    wildcard = await test.default_controller.Read(test.dut_node_id, [()])
    attributes = wildcard.attributes
    spec_version = attributes[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.SpecificationVersion]
    xml_clusters, _ = build_xml_clusters(dm_from_spec_version(spec_version))

    endpoint_data = attributes.get(scene_endpoint)
    asserts.assert_true(endpoint_data is not None, f"Scene endpoint {scene_endpoint} not found on the DUT")

    fallback: _ScenableAttribute | None = None
    for cluster, cluster_data in endpoint_data.items():
        xml_cluster = xml_clusters.get(cluster.id)
        if xml_cluster is None:
            continue
        for attr_id in cluster_data.get(cluster.Attributes.AttributeList, []):
            xml_attribute = xml_cluster.attributes.get(attr_id)
            if xml_attribute is None or not xml_attribute.scene:
                continue
            value_field = _scene_value_field(xml_attribute.datatype)
            if value_field is None:
                continue
            try:
                candidate = _ScenableAttribute(
                    cluster=Clusters.ClusterObjects.ALL_CLUSTERS[cluster.id],
                    attribute=Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster.id][attr_id],
                    xml_attribute=xml_attribute,
                    value_field=value_field,
                )
            except KeyError:
                continue
            if xml_attribute.datatype.lower() == "bool":
                return candidate
            if fallback is None:
                fallback = candidate

    asserts.assert_true(fallback is not None, f"No scenable attribute found on endpoint {scene_endpoint}")
    return fallback


def _is_writable(scenable: _ScenableAttribute) -> bool:
    return scenable.xml_attribute.write_access != Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue


async def _read_scenable_value(test: MatterBaseTest, scenable: _ScenableAttribute, scene_endpoint: int) -> int:
    """Reads the current value of the selected scenable attribute as an int."""
    value = await test.read_single_attribute_check_success(
        endpoint=scene_endpoint, cluster=scenable.cluster, attribute=scenable.attribute
    )
    return int(value)


def _build_extension_fields(scenable: _ScenableAttribute, value: int) -> list:
    """Builds the ExtensionFieldSetStructs setting the selected scenable attribute to value."""
    pair = Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=scenable.attribute.attribute_id)
    setattr(pair, scenable.value_field, int(value))
    return [Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct(clusterID=scenable.cluster.id, attributeValueList=[pair])]


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
        lo, hi = 0, 2**bits - 2
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
