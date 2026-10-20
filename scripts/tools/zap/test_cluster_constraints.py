"""Check Closure Dimension and Electrical Protection Alarm ZAP constraints."""

import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

SDK_ROOT = Path(__file__).resolve().parents[3]
ZAP_ROOT = SDK_ROOT / "src/app/zap-templates/zcl/data-model/chip"


class TestClusterConstraints(unittest.TestCase):
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
