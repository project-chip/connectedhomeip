from dataclasses import dataclass

from mobly import asserts

from matter.testing.matter_testing import MatterBaseTest
from matter.testing.spec_parsing import Clusters, XmlAttribute, build_xml_clusters, dm_from_spec_version

# Maps a scenable attribute's spec data type to the typed value field of the
# Scenes cluster AttributeValuePairStruct used to carry its value.
_SCENE_VALUE_FIELDS = {
    "bool": "valueUnsigned8",
    "uint8": "valueUnsigned8",
    "percent": "valueUnsigned8",
    "uint16": "valueUnsigned16",
    "percent100ths": "valueUnsigned16",
    "uint24": "valueUnsigned32",
    "uint32": "valueUnsigned32",
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
    # Will discard Scenable attributes of enum and map types for now
    # because their constraints are not defined in the datamodel xml
    # Additionally, their range of values might be feature dependent
    # "enum8": "valueUnsigned8",
    # "map8": "valueUnsigned8",
    # "enum16": "valueUnsigned16",
    # "map16": "valueUnsigned16",
    # "map32": "valueUnsigned32",
    # "enum32": "valueUnsigned32",
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
class ScenableAttribute:
    """A concrete scenable attribute discovered on the DUT's scene endpoint."""

    cluster: Clusters.ClusterObjects.Cluster
    attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor
    xml_attribute: XmlAttribute
    value_field: str
    min_value: int = 0
    max_value: int = 0


def _scene_value_field(datatype: str) -> str | None:
    """Returns the AttributeValuePairStruct value field for a spec data type, or None if unsupported."""
    return _SCENE_VALUE_FIELDS.get(datatype.lower())


async def select_scenable_attribute(test: MatterBaseTest, scene_endpoint: int) -> ScenableAttribute:
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

    fallback: ScenableAttribute | None = None
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
                candidate = ScenableAttribute(
                    cluster=Clusters.ClusterObjects.ALL_CLUSTERS[cluster.id],
                    attribute=Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster.id][attr_id],
                    xml_attribute=xml_attribute,
                    value_field=value_field,
                )
            except KeyError:
                continue
            # Boolean attributes are preferred because two distinct valid values are always available and unambiguous.
            if xml_attribute.datatype.lower() == "bool":
                _set_scenable_attribute_value_bounds_from_spec_contraints(candidate)
                return candidate
            if fallback is None:
                fallback = candidate

    asserts.assert_true(fallback is not None, f"No scenable attribute found on endpoint {scene_endpoint}")

    # There are two instances of scenable attributes which their contraints depends other attributes in their cluster
    # LevelControl currentLevel is bounded by the cluster's MinLevel and MaxLevel attributes
    # ColorControl ColorTemperatureMireds is bounded by the cluster's ColorTempPhysicalMinMireds and ColorTempPhysicalMaxMireds attributes
    if fallback.cluster == Clusters.LevelControl and fallback.attribute == Clusters.LevelControl.Attributes.CurrentLevel:
        min_level = await test.read_single_attribute_check_success(
            endpoint=scene_endpoint, cluster=Clusters.LevelControl, attribute=Clusters.LevelControl.Attributes.MinLevel
        )
        max_level = await test.read_single_attribute_check_success(
            endpoint=scene_endpoint, cluster=Clusters.LevelControl, attribute=Clusters.LevelControl.Attributes.MaxLevel
        )
        _set_scenable_attribute_value_bounds(fallback, int(min_level), int(max_level))
    elif (
        fallback.cluster == Clusters.ColorControl and fallback.attribute == Clusters.ColorControl.Attributes.ColorTemperatureMireds
    ):
        min_color_temp = await test.read_single_attribute_check_success(
            endpoint=scene_endpoint,
            cluster=Clusters.ColorControl,
            attribute=Clusters.ColorControl.Attributes.ColorTempPhysicalMinMireds,
        )
        max_color_temp = await test.read_single_attribute_check_success(
            endpoint=scene_endpoint,
            cluster=Clusters.ColorControl,
            attribute=Clusters.ColorControl.Attributes.ColorTempPhysicalMaxMireds,
        )
        _set_scenable_attribute_value_bounds(fallback, int(min_color_temp), int(max_color_temp))
    else:
        _set_scenable_attribute_value_bounds_from_spec_contraints(fallback)
    return fallback


def is_writable(scenable: ScenableAttribute) -> bool:
    return scenable.xml_attribute.write_access != Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue


async def read_scenable_value(test: MatterBaseTest, scenable: ScenableAttribute, scene_endpoint: int) -> int:
    """Reads the current value of the selected scenable attribute as an int."""
    value = await test.read_single_attribute_check_success(
        endpoint=scene_endpoint, cluster=scenable.cluster, attribute=scenable.attribute
    )
    return int(value)


def build_extension_fields(scenable: ScenableAttribute, value: int) -> list:
    """Builds the ExtensionFieldSetStructs setting the selected scenable attribute to value."""
    pair = Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=scenable.attribute.attribute_id)
    setattr(pair, scenable.value_field, int(value))
    return [Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct(clusterID=scenable.cluster.id, attributeValueList=[pair])]


def _set_scenable_attribute_value_bounds_from_spec_contraints(scenable: ScenableAttribute):
    """Set the value constraints for the scenable attribute.

    Uses the spec constraints when available, otherwise uses the data type's
    representable range (excluding the maximum unsigned value, which is frequently reserved).
    """
    if scenable.xml_attribute.datatype.lower() == "bool":
        lo, hi = 0, 1
    else:
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
    _set_scenable_attribute_value_bounds(scenable, lo, hi)


def _set_scenable_attribute_value_bounds(scenable: ScenableAttribute, lo: int, hi: int):
    """Override the value constraints for the scenable attribute.

    Used when the Scenable attribute value bounds might differ from its spec constraints or type range
    e.g. LevelControl currentLevel is bounded by the cluster's MinLevel and MaxLevel attributes
    """
    scenable.min_value, scenable.max_value = lo, hi


def value_other_than(scenable: ScenableAttribute, value: int) -> int:
    """Returns an in-range value adjacent to (and different from) the given value within the attribute's value constraints."""
    if value + 1 <= scenable.max_value:
        return value + 1
    if value - 1 >= scenable.min_value:
        return value - 1
    return scenable.min_value if value != scenable.min_value else scenable.max_value
