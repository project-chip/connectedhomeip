"""Check targeted ZAP constraints and SDK compatibility settings."""

import shlex
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

SDK_ROOT = Path(__file__).resolve().parents[3]
ZAP_ROOT = SDK_ROOT / "src/app/zap-templates/zcl/data-model/chip"


class TestClusterConstraints(unittest.TestCase):
    def test_sdk_type_and_domain_compatibility(self):
        global_structs = ET.parse(ZAP_ROOT / "global-structs.xml").getroot()
        area_type = global_structs.find("struct[@name='LocationDescriptorStruct']/item[@name='AreaType']")
        with self.subTest(field="LocationDescriptorStruct.AreaType"):
            self.assertEqual(area_type.get("type"), "AreaTypeTag")
        for filename, domain in (("closure-dimension-cluster.xml", "Closures"),
                                 ("electrical-protection-alarm-cluster.xml", "Measurement & Sensing")):
            with self.subTest(filename=filename):
                self.assertEqual(ET.parse(ZAP_ROOT / filename).findtext("cluster/domain"), domain)

    def test_targeted_generation_uses_portable_roots(self):
        for filename in ("global-enums.xml", "global-structs.xml", "closure-dimension-cluster.xml",
                         "electrical-protection-alarm-cluster.xml"):
            with self.subTest(filename=filename):
                comments = (element.text for _, element in ET.iterparse(ZAP_ROOT / filename, events=("comment",)))
                parameters = [line.removeprefix("Parameters: ") for comment in comments
                              for line in comment.splitlines() if line.startswith("Parameters: ")]
                self.assertEqual(len(parameters), 1)
                arguments = dict(argument.split("=", 1) for argument in shlex.split(parameters[0]) if "=" in argument)
                for root in ("sdk-root", "spec-root"):
                    self.assertIn(root, arguments)
                    self.assertFalse(Path(arguments[root]).is_absolute(), arguments[root])

    def test_closure_dimension_positioning_bounds_and_defaults(self):
        cluster = ET.parse(ZAP_ROOT / "closure-dimension-cluster.xml").getroot().find("cluster")
        for name in ("Resolution", "StepValue"):
            with self.subTest(attribute=name):
                attribute = cluster.find(f"attribute[@name='{name}']")
                self.assertIsNotNone(attribute)
                self.assertEqual(attribute.get("type"), "percent100ths")
                self.assertEqual(attribute.get("min"), "1")
                self.assertEqual(attribute.get("max"), "10000")
                self.assertEqual(attribute.get("default"), "1")
                self.assertEqual(attribute.get("optional"), "true")
                self.assertNotEqual(attribute.get("isNullable"), "true")
                self.assertEqual(attribute.find("mandatoryConform/feature").get("name"), "PS")

    def test_electrical_protection_rating_minima(self):
        reference = ET.parse(SDK_ROOT / "data_model/1.7/clusters/ElectricalProtectionAlarm.xml").getroot()
        definition = ET.parse(ZAP_ROOT / "electrical-protection-alarm-cluster.xml").getroot()
        rating_structs = reference.findall("dataTypes/struct")
        self.assertEqual(len(rating_structs), 6)
        for rating in rating_structs:
            for field in rating.findall("field"):
                minimum = field.find("constraint/min")
                if minimum is None:
                    continue
                with self.subTest(struct=rating.get("name"), field=field.get("name")):
                    item = definition.find(f"struct[@name='{rating.get('name')}']/item[@name='{field.get('name')}']")
                    self.assertIsNotNone(item)
                    self.assertEqual(item.get("min"), minimum.get("value"))

    def test_electrical_protection_nullable_ratings(self):
        reference = ET.parse(SDK_ROOT / "data_model/1.7/clusters/ElectricalProtectionAlarm.xml").getroot()
        definition = ET.parse(ZAP_ROOT / "electrical-protection-alarm-cluster.xml").getroot()
        nullable_attributes = [attribute for attribute in reference.findall("attributes/attribute")
                               if attribute.find("quality[@nullable='true']") is not None]
        self.assertEqual(len(nullable_attributes), 7)
        for rating in nullable_attributes:
            with self.subTest(attribute=rating.get("name")):
                attribute = definition.find(f"cluster/attribute[@name='{rating.get('name')}']")
                self.assertIsNotNone(attribute)
                self.assertEqual(attribute.get("isNullable"), "true")


if __name__ == "__main__":
    unittest.main()
