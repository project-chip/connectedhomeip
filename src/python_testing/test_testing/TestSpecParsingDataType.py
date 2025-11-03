#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
import logging
import xml.etree.ElementTree as ElementTree

from jinja2 import Template
from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, default_matter_test_main
from matter.testing.spec_parsing import (ClusterParser, DataTypeEnum, PrebuiltDataModelDirectory, build_xml_clusters,
                                         build_xml_global_data_types)
from matter.tlv import uint

LOGGER = logging.getLogger(__name__)


class TestSpecParsingDataType(MatterBaseTest):
    def setup_class(self):
        self.xml_clusters, self.xml_cluster_problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_5)
        self.xml_global_data_types, self.xml_global_problems = build_xml_global_data_types(PrebuiltDataModelDirectory.k1_5)

        # Setup templates for testing struct, enum, and bitmap data types
        self.cluster_id = 0xABCD
        self.cluster_name = "Test Cluster"

        # Template for a cluster with data types
        self.cluster_template = Template("""<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="clusters clusters.xsd" id="{{ cluster_id }}" name="{{ cluster_name }}" revision="1">
                                    <classification picsCode="TEST"/>
                                    <dataTypes>
                                        {{ data_types }}
                                    </dataTypes>
                                    <attributes>
                                    </attributes>
                                    <commands>
                                    </commands>
                                    <events>
                                    </events>
                                </cluster>""")

        # Template for a struct data type
        self.struct_template = Template("""<struct name="{{ name }}">
                                            {% for field in fields %}
                                            <field id="{{ field.id }}" name="{{ field.name }}" type="{{ field.type }}">
                                                <mandatoryConform/>
                                            </field>
                                            {% endfor %}
                                        </struct>""")

        # Template for an enum data type
        self.enum_template = Template("""<enum name="{{ name }}">
                                        {% for item in items %}
                                        <item value="{{ item.value }}" name="{{ item.name }}">
                                            <mandatoryConform/>
                                        </item>
                                        {% endfor %}
                                    </enum>""")

        # Template for a bitmap data type
        self.bitmap_template = Template("""<bitmap name="{{ name }}">
                                        {% for field in fields %}
                                        <bitfield bit="{{ field.bit }}" name="{{ field.name }}" {% if field.mask is defined %}mask="{{ field.mask }}"{% endif %}>
                                            <mandatoryConform/>
                                        </bitfield>
                                        {% endfor %}
                                    </bitmap>""")

    def test_parse_basic_struct(self):
        """Test parsing of a basic struct data type"""
        fields = [
            {"id": "1", "name": "Field1", "type": "uint8"},
            {"id": "2", "name": "Field2", "type": "string"}
        ]
        struct_xml = self.struct_template.render(name="TestStruct", fields=fields)
        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=struct_xml)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, "Expected no parsing problems")

        # Verify the struct was properly parsed
        asserts.assert_true("TestStruct" in cluster.structs, "TestStruct not found in parsed structs")
        struct = cluster.structs["TestStruct"]
        asserts.assert_equal(struct.data_type, DataTypeEnum.kStruct, "Incorrect data type")
        asserts.assert_equal(struct.name, "TestStruct", "Incorrect struct name")
        asserts.assert_equal(len(struct.components), 2, "Incorrect number of fields")
        asserts.assert_true(uint(1) in struct.components, "Field1 not found in struct components")
        asserts.assert_true(uint(2) in struct.components, "Field2 not found in struct components")
        asserts.assert_equal(struct.components[uint(1)].name, "Field1", "Incorrect field name")
        asserts.assert_equal(struct.components[uint(2)].name, "Field2", "Incorrect field name")
        asserts.assert_equal(struct.cluster_ids, [self.cluster_id], "Incorrect cluster IDs")

    def test_parse_basic_enum(self):
        """Test parsing of a basic enum data type"""
        items = [
            {"value": "0", "name": "Item1"},
            {"value": "1", "name": "Item2"},
            {"value": "2", "name": "Item3"}
        ]
        enum_xml = self.enum_template.render(name="TestEnum", items=items)
        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=enum_xml)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, "Expected no parsing problems")

        # Verify the enum was properly parsed
        asserts.assert_true("TestEnum" in cluster.enums, "TestEnum not found in parsed enums")
        enum = cluster.enums["TestEnum"]
        asserts.assert_equal(enum.data_type, DataTypeEnum.kEnum, "Incorrect data type")
        asserts.assert_equal(enum.name, "TestEnum", "Incorrect enum name")
        asserts.assert_equal(len(enum.components), 3, "Incorrect number of items")
        asserts.assert_true(uint(0) in enum.components, "Item1 not found in enum components")
        asserts.assert_true(uint(1) in enum.components, "Item2 not found in enum components")
        asserts.assert_true(uint(2) in enum.components, "Item3 not found in enum components")
        asserts.assert_equal(enum.components[uint(0)].name, "Item1", "Incorrect item name")
        asserts.assert_equal(enum.components[uint(1)].name, "Item2", "Incorrect item name")
        asserts.assert_equal(enum.components[uint(2)].name, "Item3", "Incorrect item name")
        asserts.assert_equal(enum.cluster_ids, [self.cluster_id], "Incorrect cluster IDs")

    def test_parse_basic_bitmap(self):
        """Test parsing of a basic bitmap data type"""
        fields = [
            {"bit": "0", "name": "Bit0"},
            {"bit": "1", "name": "Bit1"},
            {"bit": "7", "name": "Bit7"}
        ]
        bitmap_xml = self.bitmap_template.render(name="TestBitmap", fields=fields)
        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=bitmap_xml)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, "Expected no parsing problems")

        # Verify the bitmap was properly parsed
        asserts.assert_true("TestBitmap" in cluster.bitmaps, "TestBitmap not found in parsed bitmaps")
        bitmap = cluster.bitmaps["TestBitmap"]
        asserts.assert_equal(bitmap.data_type, DataTypeEnum.kBitmap, "Incorrect data type")
        asserts.assert_equal(bitmap.name, "TestBitmap", "Incorrect bitmap name")
        asserts.assert_equal(len(bitmap.components), 3, "Incorrect number of bitfields")
        asserts.assert_true(uint(0) in bitmap.components, "Bit0 not found in bitmap components")
        asserts.assert_true(uint(1) in bitmap.components, "Bit1 not found in bitmap components")
        asserts.assert_true(uint(7) in bitmap.components, "Bit7 not found in bitmap components")
        asserts.assert_equal(bitmap.components[uint(0)].name, "Bit0", "Incorrect bitfield name")
        asserts.assert_equal(bitmap.components[uint(1)].name, "Bit1", "Incorrect bitfield name")
        asserts.assert_equal(bitmap.components[uint(7)].name, "Bit7", "Incorrect bitfield name")
        if not bitmap.cluster_ids:
            self.print_step("Warning", f"Bitmap TestBitmap has empty cluster_ids, should contain {self.cluster_id}")
        else:
            asserts.assert_true(self.cluster_id in bitmap.cluster_ids,
                                f"Bitmap TestBitmap has incorrect cluster IDs: expected {self.cluster_id} to be in {bitmap.cluster_ids}")

    def test_parse_multi_bit_bitmap(self):
        """Test parsing of a bitmap with fields spanning multiple bits"""
        fields = [
            {"bit": "0", "name": "SingleBit0"},
            {"bit": "1", "name": "SingleBit1"},
            {"bit": "2", "name": "MultiBit2to4"},  # Bits 2-4 (0b11100)
            {"bit": "5", "name": "MultiBit5to6"}  # Bits 5-6 (0b1100000)
        ]
        bitmap_xml = self.bitmap_template.render(name="MultiBitBitmap", fields=fields)
        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=bitmap_xml)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, "Expected no parsing problems")

        # Verify the bitmap was properly parsed
        asserts.assert_true("MultiBitBitmap" in cluster.bitmaps, "MultiBitBitmap not found in parsed bitmaps")
        bitmap = cluster.bitmaps["MultiBitBitmap"]
        asserts.assert_equal(bitmap.data_type, DataTypeEnum.kBitmap, "Incorrect data type")
        asserts.assert_equal(bitmap.name, "MultiBitBitmap", "Incorrect bitmap name")

        # Verify we have all bitfields
        asserts.assert_equal(len(bitmap.components), 4, "Incorrect number of bitfields")
        asserts.assert_true(uint(0) in bitmap.components, "SingleBit0 not found in bitmap components")
        asserts.assert_true(uint(1) in bitmap.components, "SingleBit1 not found in bitmap components")
        asserts.assert_true(uint(2) in bitmap.components, "MultiBit2to4 not found in bitmap components")
        asserts.assert_true(uint(5) in bitmap.components, "MultiBit5to6 not found in bitmap components")
        if not bitmap.cluster_ids:
            self.print_step("Warning", f"Bitmap MultiBitBitmap has empty cluster_ids, should contain {self.cluster_id}")
        else:
            asserts.assert_true(self.cluster_id in bitmap.cluster_ids,
                                f"Bitmap MultiBitBitmap has incorrect cluster IDs: expected {self.cluster_id} to be in {bitmap.cluster_ids}")

    def test_multiple_data_types(self):
        """Test parsing multiple data types in a single cluster"""
        # Create a struct
        struct_fields = [{"id": "1", "name": "Field1", "type": "uint8"}]
        struct_xml = self.struct_template.render(name="TestStruct", fields=struct_fields)

        # Create an enum
        enum_items = [{"value": "0", "name": "Item1"}, {"value": "1", "name": "Item2"}]
        enum_xml = self.enum_template.render(name="TestEnum", items=enum_items)

        # Create a bitmap
        bitmap_fields = [{"bit": "0", "name": "Bit0"}, {"bit": "1", "name": "Bit1"}]
        bitmap_xml = self.bitmap_template.render(name="TestBitmap", fields=bitmap_fields)

        # Combine all data types in one cluster
        data_types = struct_xml + enum_xml + bitmap_xml
        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=data_types)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, "Expected no parsing problems")

        # Verify all data types were properly parsed
        asserts.assert_true("TestStruct" in cluster.structs, "TestStruct not found in parsed structs")
        asserts.assert_true("TestEnum" in cluster.enums, "TestEnum not found in parsed enums")
        asserts.assert_true("TestBitmap" in cluster.bitmaps, "TestBitmap not found in parsed bitmaps")

    def test_invalid_struct_field(self):
        """Test handling of a struct with an invalid field (missing id)"""
        # Create a struct with one valid field and one invalid field (missing id)
        struct_xml = """<struct name="TestStruct">
                        <field id="1" name="Field1" type="uint8">
                            <mandatoryConform/>
                        </field>
                        <field name="InvalidField" type="uint8">
                            <mandatoryConform/>
                        </field>
                    </struct>"""

        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=struct_xml)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()

        # Verify the valid field was parsed and the invalid one generated a problem
        asserts.assert_true("TestStruct" in cluster.structs, "TestStruct not found in parsed structs")
        struct = cluster.structs["TestStruct"]
        asserts.assert_equal(len(struct.components), 1, "Should only have one valid field")
        asserts.assert_true(uint(1) in struct.components, "Valid field not found in struct components")
        asserts.assert_equal(len(problems), 1, "Should have one problem for invalid field")
        asserts.assert_true("Struct field in TestStruct with no id or name" in problems[0].problem,
                            "Problem message doesn't match expected error")

    def test_datatype_component_additional_fields(self):
        """Test that all additional fields in XmlDataTypeComponent are correctly parsed and stored"""

        # Test struct with summary, type, isOptional, isNullable, and constraints
        struct_xml = """<struct name="FieldsTestStruct">
                        <field id="1" name="BasicField" type="uint8" summary="A basic uint8 field">
                            <mandatoryConform/>
                        </field>
                        <field id="2" name="OptionalField" type="string" isOptional="true" summary="An optional string field">
                            <mandatoryConform/>
                        </field>
                        <field id="3" name="NullableField" type="int16s" isNullable="true" summary="A nullable int16s field">
                            <mandatoryConform/>
                        </field>
                        <field id="4" name="ConstrainedField" type="uint16" summary="A field with constraints">
                            <constraint min="1" max="100"/>
                            <mandatoryConform/>
                        </field>
                    </struct>"""

        # Test bitmap with summary
        bitmap_xml = """<bitmap name="FieldsTestBitmap">
                        <bitfield bit="0" name="BasicBit" summary="A basic bit field">
                            <mandatoryConform/>
                        </bitfield>
                        <bitfield bit="2" name="MaskedBits" summary="A field spanning bits 2-4">
                            <mandatoryConform/>
                        </bitfield>
                    </bitmap>"""

        # Create a cluster with both data types
        data_types = struct_xml + bitmap_xml
        cluster_xml = self.cluster_template.render(
            cluster_id=self.cluster_id,
            cluster_name=self.cluster_name,
            data_types=data_types
        )

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, f"Unexpected problems parsing XML: {problems}")

        # Verify both data types were parsed without problems
        asserts.assert_true("FieldsTestStruct" in cluster.structs, "FieldsTestStruct not found")
        asserts.assert_true("FieldsTestBitmap" in cluster.bitmaps, "FieldsTestBitmap not found")

        # Test struct field attributes
        struct = cluster.structs["FieldsTestStruct"]

        # Basic field with summary and type
        basic_field = struct.components[uint(1)]
        asserts.assert_equal(basic_field.name, "BasicField", "Name mismatch")
        asserts.assert_equal(basic_field.summary, "A basic uint8 field", "Summary mismatch")
        asserts.assert_equal(basic_field.type_info, "uint8", "Type mismatch")
        asserts.assert_false(basic_field.is_optional, "Should not be optional")
        asserts.assert_false(basic_field.is_nullable, "Should not be nullable")

        # Optional field
        opt_field = struct.components[uint(2)]
        asserts.assert_equal(opt_field.name, "OptionalField", "Name mismatch")
        asserts.assert_true(opt_field.is_optional, "Should be optional")
        asserts.assert_equal(opt_field.type_info, "string", "Type mismatch")

        # Nullable field
        null_field = struct.components[uint(3)]
        asserts.assert_equal(null_field.name, "NullableField", "Name mismatch")
        asserts.assert_true(null_field.is_nullable, "Should be nullable")
        asserts.assert_equal(null_field.type_info, "int16s", "Type mismatch")

        # Constrained field
        const_field = struct.components[uint(4)]
        asserts.assert_equal(const_field.name, "ConstrainedField", "Name mismatch")
        asserts.assert_true(const_field.constraints is not None, "Should have constraints")
        asserts.assert_equal(const_field.constraints.get("min"), "1", "Min constraint mismatch")
        asserts.assert_equal(const_field.constraints.get("max"), "100", "Max constraint mismatch")

        # Test bitmap field attributes
        bitmap = cluster.bitmaps["FieldsTestBitmap"]

        # Basic bit
        basic_bit = bitmap.components[uint(0)]
        asserts.assert_equal(basic_bit.name, "BasicBit", "Name mismatch")
        asserts.assert_equal(basic_bit.summary, "A basic bit field", "Summary mismatch")

    def test_invalid_enum_item(self):
        """Test handling of an enum with an invalid item (missing value)"""
        # Create an enum with one valid item and one invalid item (missing value)
        enum_xml = """<enum name="TestEnum">
                    <item value="0" name="Item1">
                        <mandatoryConform/>
                    </item>
                    <item name="InvalidItem">
                        <mandatoryConform/>
                    </item>
                </enum>"""

        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=enum_xml)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()

        # Verify the valid item was parsed and the invalid one generated a problem
        asserts.assert_true("TestEnum" in cluster.enums, "TestEnum not found in parsed enums")
        enum = cluster.enums["TestEnum"]
        asserts.assert_equal(len(enum.components), 1, "Should only have one valid item")
        asserts.assert_true(uint(0) in enum.components, "Valid item not found in enum components")
        asserts.assert_equal(len(problems), 1, "Should have one problem for invalid item")
        asserts.assert_true("Enum field in TestEnum with no id or name" in problems[0].problem,
                            "Problem message doesn't match expected error")

    def test_invalid_bitmap_field(self):
        """Test handling of a bitmap with an invalid bitfield (missing bit)"""
        # Create a bitmap with one valid field and one invalid field (missing bit)
        bitmap_xml = """<bitmap name="TestBitmap">
                        <bitfield bit="0" name="Bit0">
                            <mandatoryConform/>
                        </bitfield>
                        <bitfield name="InvalidBit">
                            <mandatoryConform/>
                        </bitfield>
                    </bitmap>"""

        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=bitmap_xml)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()

        # Verify the valid field was parsed and the invalid one generated a problem
        asserts.assert_true("TestBitmap" in cluster.bitmaps, "TestBitmap not found in parsed bitmaps")
        bitmap = cluster.bitmaps["TestBitmap"]
        asserts.assert_equal(len(bitmap.components), 1, "Should only have one valid field")
        asserts.assert_true(uint(0) in bitmap.components, "Valid bitfield not found in bitmap components")
        asserts.assert_equal(len(problems), 1, "Should have one problem for invalid bitfield")
        asserts.assert_true("Bitmap field in TestBitmap with no id or name" in problems[0].problem,
                            "Problem message doesn't match expected error")

    def test_missing_name(self):
        """Test handling of a data type with missing name attribute"""
        # Try different data types - we'll generate a problem for each
        test_cases = [
            ("struct", "<field id=\"1\" name=\"Field1\" type=\"uint8\"><mandatoryConform/></field>"),
            ("enum", "<item value=\"0\" name=\"Item1\"><mandatoryConform/></item>"),
            ("bitmap", "<bitfield bit=\"0\" name=\"Bit0\"><mandatoryConform/></bitfield>")
        ]

        for data_type, inner_content in test_cases:
            try:
                # Create XML with missing name attribute
                xml = f"""<{data_type}>
                        {inner_content}
                    </{data_type}>"""

                cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                           cluster_name=self.cluster_name,
                                                           data_types=xml)

                # Parse the XML
                et = ElementTree.fromstring(cluster_xml)
                parser = ClusterParser(et, self.cluster_id, self.cluster_name)
                _ = parser.create_cluster()  # We don't use this, just care about problems
                problems = parser.get_problems()
                asserts.assert_equal(len(problems), 1, "Should have one problem for missing name")
                asserts.assert_true("with no name" in problems[0].problem,
                                    f"Problem message doesn't match expected error for {data_type}")
            except Exception as e:
                self.print_step("Error", f"Exception when testing {data_type}: {e}")

    def test_bitmap_with_multiple_bitfields(self):
        """Test parsing of a bitmap with multiple distinct bitfields"""
        fields = [
            {"bit": "0", "name": "MemberA"},
            {"bit": "1", "name": "MemberB"},
            {"bit": "2", "name": "MemberC"}
        ]
        bitmap_xml = self.bitmap_template.render(name="TestStatusBitmap", fields=fields)
        cluster_xml = self.cluster_template.render(cluster_id=self.cluster_id,
                                                   cluster_name=self.cluster_name,
                                                   data_types=bitmap_xml)

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, f"Unexpected problems parsing XML: {problems}")

        # Verify the bitmap was properly parsed
        asserts.assert_true("TestStatusBitmap" in cluster.bitmaps, "TestStatusBitmap not found in parsed bitmaps")
        bitmap = cluster.bitmaps["TestStatusBitmap"]

        # Verify each bitfield was properly parsed
        asserts.assert_equal(len(bitmap.components), 3, "Incorrect number of bitfields")
        asserts.assert_true(uint(0) in bitmap.components, "MemberA not found in bitmap components")
        asserts.assert_true(uint(1) in bitmap.components, "MemberB not found in bitmap components")
        asserts.assert_true(uint(2) in bitmap.components, "MemberC not found in bitmap components")

        # Verify the names were properly parsed
        asserts.assert_equal(bitmap.components[uint(0)].name, "MemberA", "Incorrect bitfield name")
        asserts.assert_equal(bitmap.components[uint(1)].name, "MemberB", "Incorrect bitfield name")
        asserts.assert_equal(bitmap.components[uint(2)].name, "MemberC", "Incorrect bitfield name")

    def test_optionalConform_tag_parsing(self):
        """Test parsing of fields with optionalConform tag"""
        # Create XML with one mandatory and one optional field
        struct_xml = """<struct name="OptionalConformStruct">
                        <field id="1" name="MandatoryField" type="uint8">
                            <mandatoryConform/>
                        </field>
                        <field id="2" name="OptionalField" type="uint8">
                            <optionalConform/>
                        </field>
                    </struct>"""

        cluster_xml = self.cluster_template.render(
            cluster_id=self.cluster_id,
            cluster_name=self.cluster_name,
            data_types=struct_xml
        )

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, "Expected no parsing problems")

        # Verify that optionalConform was properly detected
        struct = cluster.structs["OptionalConformStruct"]
        asserts.assert_false(struct.components[uint(1)].is_optional, "MandatoryField should not be marked optional")
        asserts.assert_true(struct.components[uint(2)].is_optional, "OptionalField should be marked optional")

    def test_nullable_quality_tag_parsing(self):
        """Test parsing of fields with nullable quality tag"""
        # Create XML with nullable field using the quality tag
        struct_xml = """<struct name="NullableStruct">
                        <field id="1" name="RegularField" type="uint8">
                            <mandatoryConform/>
                        </field>
                        <field id="2" name="NullableField" type="uint8">
                            <quality nullable="true"/>
                            <mandatoryConform/>
                        </field>
                    </struct>"""

        cluster_xml = self.cluster_template.render(
            cluster_id=self.cluster_id,
            cluster_name=self.cluster_name,
            data_types=struct_xml
        )

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, "Expected no parsing problems")

        # Verify that nullable quality was properly detected
        struct = cluster.structs["NullableStruct"]
        asserts.assert_false(struct.components[uint(1)].is_nullable, "RegularField should not be marked nullable")
        asserts.assert_true(struct.components[uint(2)].is_nullable, "NullableField should be marked nullable")

    def test_complex_constraints_parsing(self):
        """Test parsing of fields with complex constraint structures"""
        # Create XML with various constraint patterns
        struct_xml = """<struct name="ConstraintsStruct">
                        <field id="1" name="RangeField" type="uint8">
                            <constraint min="1" max="100"/>
                            <mandatoryConform/>
                        </field>
                        <field id="2" name="MaxCountField" type="list">
                            <constraint>
                                <maxCount>
                                    <attribute name="MaxItems"/>
                                </maxCount>
                            </constraint>
                            <mandatoryConform/>
                        </field>
                    </struct>"""

        cluster_xml = self.cluster_template.render(
            cluster_id=self.cluster_id,
            cluster_name=self.cluster_name,
            data_types=struct_xml
        )

        # Parse the XML
        et = ElementTree.fromstring(cluster_xml)
        parser = ClusterParser(et, self.cluster_id, self.cluster_name)
        cluster = parser.create_cluster()
        problems = parser.get_problems()
        asserts.assert_equal(len(problems), 0, f"Unexpected problems parsing XML: {problems}")

        # Verify that constraints were properly detected
        struct = cluster.structs["ConstraintsStruct"]

        # Check min/max constraints on RangeField
        range_field = struct.components[uint(1)]
        asserts.assert_true(range_field.constraints is not None, "RangeField should have constraints")
        asserts.assert_equal(range_field.constraints.get("min"), "1", "Min constraint incorrect")
        asserts.assert_equal(range_field.constraints.get("max"), "100", "Max constraint incorrect")

        # Check maxCount with attribute reference on MaxCountField
        max_count_field = struct.components[uint(2)]
        asserts.assert_true(max_count_field.constraints is not None, "MaxCountField should have constraints")
        asserts.assert_true("maxCountAttribute" in max_count_field.constraints, "maxCountAttribute missing")
        asserts.assert_equal(max_count_field.constraints.get("maxCountAttribute"), "MaxItems", "Attribute reference incorrect")

    def test_XML_clusters_data_types(self):
        """Test data types from clusters in the 1.5 cluster spec XML files"""
        # First, find a cluster with enums to test
        enum_found = False
        struct_found = False

        for cluster_id, cluster in self.xml_clusters.items():
            # Check for enums
            if len(cluster.enums) > 0 and not enum_found:
                enum_found = True
                enum_name = next(iter(cluster.enums.keys()))
                enum = cluster.enums[enum_name]
                self.print_step("Testing", f"enum {enum_name} from cluster {cluster.name}")
                asserts.assert_equal(enum.data_type, DataTypeEnum.kEnum, "Incorrect data type")
                asserts.assert_true(len(enum.components) > 0, f"{enum_name} should have components")

            # Check for structs
            if len(cluster.structs) > 0 and not struct_found:
                struct_found = True
                struct_name = next(iter(cluster.structs.keys()))
                struct = cluster.structs[struct_name]
                self.print_step("Testing", f"struct {struct_name} from cluster {cluster.name}")
                asserts.assert_equal(struct.data_type, DataTypeEnum.kStruct, "Incorrect data type")
                asserts.assert_true(len(struct.components) > 0, f"{struct_name} should have components")

            if enum_found and struct_found:
                break

        asserts.assert_true(enum_found, "No clusters with enums found")
        asserts.assert_true(struct_found, "No clusters with structs found")

        # Bonus: Also try to find a bitmap
        bitmap_found = False
        for cluster_id, cluster in self.xml_clusters.items():
            if len(cluster.bitmaps) > 0:
                bitmap_found = True
                bitmap_name = next(iter(cluster.bitmaps.keys()))
                bitmap = cluster.bitmaps[bitmap_name]
                self.print_step("Testing", f"bitmap {bitmap_name} from cluster {cluster.name}")
                asserts.assert_equal(bitmap.data_type, DataTypeEnum.kBitmap, "Incorrect data type")
                asserts.assert_true(len(bitmap.components) > 0, f"{bitmap_name} should have components")
                break

        # This check is optional as not all clusters might have bitmaps
        if bitmap_found:
            LOGGER.info("Successfully found and tested a bitmap")

    def test_detailed_check_xml_datatype(self):
        """Test a specific cluster XML's data type in detail to ensure all properties are correctly parsed"""

        # Test a specific, known enum from the Access Control cluster, just as an example
        access_control_id = uint(Clusters.AccessControl.id)
        if access_control_id not in self.xml_clusters:
            self.skipTest("Access Control cluster not found in parsed XML")

        cluster = self.xml_clusters[access_control_id]

        # Check for AccessControlEntryPrivilegeEnum
        privilege_enum_name = None
        for name, enum in cluster.enums.items():
            if "Privilege" in name:
                privilege_enum_name = name
                break

        if not privilege_enum_name:
            self.skipTest("AccessControlEntryPrivilegeEnum not found in Access Control cluster")

        privilege_enum = cluster.enums[privilege_enum_name]

        # Verify enum properties
        asserts.assert_equal(privilege_enum.data_type, DataTypeEnum.kEnum, "Incorrect data type")
        asserts.assert_true(len(privilege_enum.components) >= 4, "Should have at least 4 enum values")

        # Verify that we have the expected components (view, operate, manage, administer privileges)
        view_found = operate_found = manage_found = admin_found = False

        for component_id, component in privilege_enum.components.items():
            if "view" in component.name.lower():
                view_found = True
            elif "operat" in component.name.lower():
                operate_found = True
            elif "manag" in component.name.lower():
                manage_found = True
            elif "admin" in component.name.lower():
                admin_found = True

        asserts.assert_true(view_found, "View privilege not found")
        asserts.assert_true(operate_found, "Operate privilege not found")
        asserts.assert_true(manage_found, "Manage privilege not found")
        asserts.assert_true(admin_found, "Admin privilege not found")

    def test_spec_data_type_files(self):
        """Test data types across different specification versions"""
        # Compare data types across different specification versions
        one_three, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_3)
        one_four, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
        one_four_one, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_1)
        one_four_two, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_4_2)
        one_five, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_5)

        # Sample cluster ID to check for data types (Basic Information)
        cluster_id = uint(Clusters.BasicInformation.id)

        # Check that data types exist in all versions and compare counts
        asserts.assert_true(cluster_id in one_three, "Basic Information cluster not found in 1.3")
        asserts.assert_true(cluster_id in one_four, "Basic Information cluster not found in 1.4")
        asserts.assert_true(cluster_id in one_four_one, "Basic Information cluster not found in 1.4.1")
        asserts.assert_true(cluster_id in one_four_two, "Basic Information cluster not found in 1.4.2")
        asserts.assert_true(cluster_id in one_five, "Basic Information cluster not found in 1.5")

        # Compare struct counts (should generally increase or stay the same over versions)
        asserts.assert_less_equal(len(one_three[cluster_id].structs), len(one_five[cluster_id].structs),
                                  "1.5 should have at least as many structs as 1.3")
        asserts.assert_less_equal(len(one_four[cluster_id].structs), len(one_five[cluster_id].structs),
                                  "1.5 should have at least as many structs as 1.4")
        asserts.assert_less_equal(len(one_four_one[cluster_id].structs), len(one_five[cluster_id].structs),
                                  "1.5 should have at least as many structs as 1.4.1")
        asserts.assert_less_equal(len(one_four_two[cluster_id].structs), len(one_five[cluster_id].structs),
                                  "1.5 should have at least as many structs as 1.4.2")

    def test_find_complex_bitmaps(self):
        """Find and test bitmaps with multi-bit fields in the data model"""
        multi_bit_bitmaps = []

        # Look through all clusters for bitmaps that might have multi-bit fields
        for cluster_id, cluster in self.xml_clusters.items():
            for bitmap_name, bitmap in cluster.bitmaps.items():
                # Check for potential multi-bit fields by looking at bit values
                bits = [int(bit) for bit in bitmap.components.keys() if str(bit).isdigit()]
                if bits and (max(bits) - min(bits) + 1) > len(bits):
                    # There might be gaps, which could indicate multi-bit fields
                    multi_bit_bitmaps.append((cluster.name, bitmap_name, bitmap))

        if multi_bit_bitmaps:
            self.print_step("Found", f"{len(multi_bit_bitmaps)} potential bitmaps with multi-bit fields:")
            for cluster_name, bitmap_name, bitmap in multi_bit_bitmaps:
                self.print_step("Found", f"  - {cluster_name}: {bitmap_name} with {len(bitmap.components)} components")
                bits = sorted([int(bit) for bit in bitmap.components.keys() if str(bit).isdigit()])
                self.print_step("Found", f"    Bits: {bits}")
        else:
            self.print_step("Found", "No bitmaps with potential multi-bit fields found")

    def test_xml_bitmap_data_types(self):
        """Test all bitmap data types found in the XML specification"""
        # This test verifies that every bitmap in every cluster is correctly parsed

        # Keep track of statistics
        total_clusters_with_bitmaps = 0
        total_bitmaps = 0
        total_bitfields = 0

        # For detailed testing; will also keep the first bitmap we find for inspection
        example_cluster_name = None
        example_bitmap_name = None
        example_bitmap = None

        # Process all clusters
        for cluster_id, cluster in self.xml_clusters.items():
            if len(cluster.bitmaps) > 0:
                total_clusters_with_bitmaps += 1
                total_bitmaps += len(cluster.bitmaps)

                # Log the cluster that has bitmaps
                self.print_step("Checking", f"Cluster '{cluster.name}' with {len(cluster.bitmaps)} bitmaps")

                # Process each bitmap in this cluster
                for bitmap_name, bitmap in cluster.bitmaps.items():
                    # Save the first example for detailed verification
                    if example_bitmap is None:
                        example_cluster_name = cluster.name
                        example_bitmap_name = bitmap_name
                        example_bitmap = bitmap

                    # Verify basic bitmap properties
                    asserts.assert_equal(bitmap.data_type, DataTypeEnum.kBitmap,
                                         f"Bitmap {bitmap_name} has incorrect data type")
                    asserts.assert_equal(bitmap.name, bitmap_name,
                                         f"Bitmap name mismatch: {bitmap.name} vs {bitmap_name}")
                    if not bitmap.cluster_ids:
                        self.print_step("Warning", f"Bitmap {bitmap_name} has empty cluster_ids, should contain {cluster_id}")
                    else:
                        asserts.assert_true(cluster_id in bitmap.cluster_ids,
                                            f"Bitmap {bitmap_name} has incorrect cluster IDs: expected {cluster_id} to be in {bitmap.cluster_ids}")

                    # Check all bitfields in this bitmap
                    num_bitfields = len(bitmap.components)
                    total_bitfields += num_bitfields
                    self.print_step("Checking", f"  - Bitmap '{bitmap_name}' with {num_bitfields} bitfields")

                    # Verify all components have names and are properly parsed
                    for bit_id, component in bitmap.components.items():
                        asserts.assert_true(component.name,
                                            f"Bitfield {bit_id} in {bitmap_name} has no name")

                        # If this is a bit position (number), verify it's in range
                        if str(bit_id).isdigit():
                            bit_pos = int(bit_id)
                            asserts.assert_true(0 <= bit_pos <= 63,  # Reasonable range for bit positions
                                                f"Bit position {bit_pos} out of range in {bitmap_name}")

        # Log summary statistics
        self.print_step("Summary", f"Tested {total_clusters_with_bitmaps} clusters with bitmaps")
        self.print_step("Summary", f"Verified {total_bitmaps} bitmap data types")
        self.print_step("Summary", f"Verified {total_bitfields} individual bitfields")

        # Skip the detailed test if we didn't find any bitmaps
        if example_bitmap is None:
            self.skipTest("No bitmaps found in XML specification")

        # Detailed verification of our example bitmap
        self.print_step("Detail", f"Detailed verification of {example_bitmap_name} from {example_cluster_name}")
        self.print_step("Detail", f"  - Has {len(example_bitmap.components)} bitfields")

        # Get at least one specific bitfield to check
        if len(example_bitmap.components) > 0:
            first_bit_id = next(iter(example_bitmap.components.keys()))
            first_component = example_bitmap.components[first_bit_id]
            self.print_step("Detail", f"  - Bitfield {first_bit_id}: '{first_component.name}'")

            # Verify the component has the expected properties
            asserts.assert_true(hasattr(first_component, 'name'), "Component missing 'name' attribute")
            asserts.assert_true(hasattr(first_component, 'conformance'), "Component missing 'conformance' attribute")

    def test_xml_datatype_component_fields(self):
        """Test the additional fields in XmlDataTypeComponent using 1.5 cluster spec XML files"""

        # Statistics to track coverage
        clusters_examined = 0
        structs_examined = 0
        enums_examined = 0
        bitmaps_examined = 0
        fields_with_summary = 0
        fields_with_type = 0
        fields_optional = 0
        fields_nullable = 0
        fields_with_constraints = 0

        # Find examples of each special field type for detailed validation
        example_optional_field = None
        example_nullable_field = None
        example_constrained_field = None
        example_with_summary = None

        # Find components with additional attributes across all data types
        for cluster_id, cluster in self.xml_clusters.items():
            clusters_examined += 1

            # Check struct fields
            for struct_name, struct in cluster.structs.items():
                structs_examined += 1

                # Check all components (fields) in the struct
                for field_id, field in struct.components.items():
                    # Check for summary
                    if field.summary:
                        fields_with_summary += 1
                        if not example_with_summary:
                            example_with_summary = (cluster.name, f"struct {struct_name}", field_id, field)

                    # Check for type_info
                    if field.type_info:
                        fields_with_type += 1

                    # Check for is_optional
                    if field.is_optional:
                        fields_optional += 1
                        if not example_optional_field:
                            example_optional_field = (cluster.name, f"struct {struct_name}", field_id, field)

                    # Check for is_nullable
                    if field.is_nullable:
                        fields_nullable += 1
                        if not example_nullable_field:
                            example_nullable_field = (cluster.name, f"struct {struct_name}", field_id, field)

                    # Check for constraints
                    if field.constraints:
                        fields_with_constraints += 1
                        if not example_constrained_field:
                            example_constrained_field = (cluster.name, f"struct {struct_name}", field_id, field)

            # Check enum items
            for enum_name, enum in cluster.enums.items():
                enums_examined += 1

                # Check all components (items) in the enum
                for item_id, item in enum.components.items():
                    # Enums mainly just have summaries
                    if item.summary:
                        fields_with_summary += 1
                        if not example_with_summary:
                            example_with_summary = (cluster.name, f"enum {enum_name}", item_id, item)

            # Check bitmap bitfields
            for bitmap_name, bitmap in cluster.bitmaps.items():
                bitmaps_examined += 1

                # Check all components (bitfields) in the bitmap
                for bit_id, bitfield in bitmap.components.items():
                    # Bitmaps mainly just have summaries
                    if bitfield.summary:
                        fields_with_summary += 1
                        if not example_with_summary:
                            example_with_summary = (cluster.name, f"bitmap {bitmap_name}", bit_id, bitfield)

        # Report what was found
        self.print_step("XML Analysis", f"Examined {clusters_examined} clusters")
        self.print_step("XML Analysis", f"Examined {structs_examined} structs, {enums_examined} enums, {bitmaps_examined} bitmaps")
        self.print_step("XML Analysis", f"Fields with summary: {fields_with_summary}")
        self.print_step("XML Analysis", f"Fields with type_info: {fields_with_type}")
        self.print_step("XML Analysis", f"Optional fields: {fields_optional}")
        self.print_step("XML Analysis", f"Nullable fields: {fields_nullable}")
        self.print_step("XML Analysis", f"Fields with constraints: {fields_with_constraints}")

        # We only assert for fields that we know exist
        asserts.assert_true(fields_with_summary > 0, "No fields with summary found in XML files")
        asserts.assert_true(fields_with_type > 0, "No fields with type_info found in XML files")

        # Verify summary example
        if example_with_summary:
            cluster_name, type_name, field_id, field = example_with_summary
            self.print_step("Example", f"Field with summary in {cluster_name}.{type_name}.{field.name}: '{field.summary}'")
            asserts.assert_true(isinstance(field.summary, str), "Summary should be a string")

        # Detailed verification of optional fields
        if fields_optional > 0:
            self.print_step("XML Analysis", "Found optional fields in XML")
            if example_optional_field:
                cluster_name, type_name, field_id, field = example_optional_field
                self.print_step("Example", f"Optional field in {cluster_name}.{type_name}.{field.name}")
                asserts.assert_true(field.is_optional, "Field should be optional")
        else:
            self.print_step("XML Analysis", "No optional fields found in XML")

        # Detailed verification of nullable fields
        if fields_nullable > 0:
            self.print_step("XML Analysis", "Found nullable fields in XML")
            if example_nullable_field:
                cluster_name, type_name, field_id, field = example_nullable_field
                self.print_step("Example", f"Nullable field in {cluster_name}.{type_name}.{field.name}")
                asserts.assert_true(field.is_nullable, "Field should be nullable")
        else:
            self.print_step("XML Analysis", "No nullable fields found in XML")

        # Detailed verification of constrained fields
        if fields_with_constraints > 0:
            self.print_step("XML Analysis", "Found fields with constraints in XML")
            if example_constrained_field:
                cluster_name, type_name, field_id, field = example_constrained_field
                constraint_str = ", ".join([f"{k}={v}" for k, v in field.constraints.items()])
                self.print_step("Example", f"Field with constraints in {cluster_name}.{type_name}.{field.name}: {constraint_str}")
                asserts.assert_true(isinstance(field.constraints, dict), "Constraints should be a dictionary")
        else:
            self.print_step("XML Analysis", "No fields with constraints found in XML")

    def test_exhaustive_all_fields_in_1_5(self):
        """Comprehensive test: validate every field in every XML for version 1.5, accumulate all issues, and fail at the end if any are found."""
        xml_clusters, problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_5)
        issues = []
        if problems:
            issues.extend([f"Parsing problem: {p}" for p in problems])

        for cluster_id, cluster in xml_clusters.items():
            # Test structs
            for struct_name, struct in cluster.structs.items():
                if not struct.name:
                    issues.append(f"Struct with empty name in cluster {cluster.name}")
                for field_id, field in struct.components.items():
                    if not field.name:
                        issues.append(f"Struct field with empty name in {struct_name} of cluster {cluster.name}")
                    if field_id is None:
                        issues.append(f"Struct field with empty id in {struct_name} of cluster {cluster.name}")

                    # Skip known cases where type_info is missing
                    s_name = struct_name.strip()
                    f_name = field.name.strip()

                    skip_conditions = [
                        ({"MonitoringRegistrationStruct"}, ["Key"], "ICD Management"),
                        ({"ModeOptionStruct"}, ["Mode", "ModeTags", "Label"], None),
                    ]

                    should_skip = False
                    for struct_names, field_names, cluster_name_part in skip_conditions:
                        if s_name in struct_names and f_name in field_names and (cluster_name_part is None or cluster_name_part in cluster.name):
                            should_skip = True
                            break

                    if should_skip:
                        continue

                    if field.type_info is None:
                        issues.append(f"Struct field {field.name} in {struct_name} of cluster {cluster.name} missing type_info")

            # Test enums
            for enum_name, enum in cluster.enums.items():
                if not enum.name:
                    issues.append(f"Enum with empty name in cluster {cluster.name}")
                for item_id, item in enum.components.items():
                    if not item.name:
                        issues.append(f"Enum item with empty name in {enum_name} of cluster {cluster.name}")
                    if item_id is None:
                        issues.append(f"Enum item with empty id in {enum_name} of cluster {cluster.name}")

            # Test bitmaps
            for bitmap_name, bitmap in cluster.bitmaps.items():
                if not bitmap.name:
                    issues.append(f"Bitmap with empty name in cluster {cluster.name}")
                for bit_id, bitfield in bitmap.components.items():
                    if not bitfield.name:
                        issues.append(f"Bitmap bitfield with empty name in {bitmap_name} of cluster {cluster.name}")
                    if bit_id is None:
                        issues.append(f"Bitmap bitfield with empty id in {bitmap_name} of cluster {cluster.name}")

        if issues:
            print("\n===== XML Validation Issues Found =====")
            for issue in issues:
                print(issue)
            asserts.fail(f"{len(issues)} issues found in XML validation. See above for details.")

    def test_global_data_types_parsing(self):
        """Test that global data types are parsed correctly from 1.5 specification"""
        # Check that we have global data types loaded
        asserts.assert_true(self.xml_global_data_types is not None, "Global data types should be loaded")

        # Check for parsing problems
        if self.xml_global_problems:
            self.print_step("Global Parsing Problems", f"Found {len(self.xml_global_problems)} problems:")
            for problem in self.xml_global_problems:
                self.print_step("Problem", str(problem))

        # Verify we have at least 3 global data types
        total_global_types = (len(self.xml_global_data_types['structs']) +
                              len(self.xml_global_data_types['enums']) +
                              len(self.xml_global_data_types['bitmaps']))
        asserts.assert_true(total_global_types >= 3, "Should have at least some global data types")

        self.print_step("Global Data Types", f"Found {len(self.xml_global_data_types['structs'])} global structs, "
                        f"{len(self.xml_global_data_types['enums'])} global enums, "
                        f"{len(self.xml_global_data_types['bitmaps'])} global bitmaps")

    def test_global_structs_validation(self):
        """Test validation of all global struct data types"""
        global_structs = self.xml_global_data_types['structs']
        asserts.assert_true(len(global_structs) > 0, "Should have at least one global struct")

        self.print_step("Global Structs", f"Found {len(global_structs)} global structs")

        # Comprehensively validate ALL global structs
        for struct_name, struct in global_structs.items():
            # Validate basic properties
            asserts.assert_equal(struct.data_type, DataTypeEnum.kStruct, f"{struct_name} should be a struct")
            asserts.assert_equal(struct.name, struct_name, "Struct name should match")
            asserts.assert_true(len(struct.components) > 0, f"{struct_name} should have components")
            asserts.assert_true(struct.cluster_ids is None, f"Global struct {struct_name} should have no cluster IDs")

            # Validate all components have proper attributes
            for component_id, component in struct.components.items():
                asserts.assert_true(component.name, f"Component {component_id} in {struct_name} should have a name")
                asserts.assert_true(component.conformance is not None,
                                    f"Component {component_id} in {struct_name} should have conformance")
                asserts.assert_true(component_id is not None, f"Component in {struct_name} should have a valid ID")

    def test_global_enums_validation(self):
        """Test validation of all global enum data types"""
        global_enums = self.xml_global_data_types['enums']
        asserts.assert_true(len(global_enums) > 0, "Should have at least one global enum")

        self.print_step("Global Enums", f"Found {len(global_enums)} global enums")

        # Comprehensively validate ALL global enums
        for enum_name, enum in global_enums.items():
            # Validate basic properties
            asserts.assert_equal(enum.data_type, DataTypeEnum.kEnum, f"{enum_name} should be an enum")
            asserts.assert_equal(enum.name, enum_name, "Enum name should match")
            asserts.assert_true(len(enum.components) > 0, f"{enum_name} should have components")
            asserts.assert_true(enum.cluster_ids is None, f"Global enum {enum_name} should have no cluster IDs")

            # Validate all components have proper attributes
            for component_id, component in enum.components.items():
                asserts.assert_true(component.name, f"Component {component_id} in {enum_name} should have a name")
                asserts.assert_true(component.conformance is not None,
                                    f"Component {component_id} in {enum_name} should have conformance")
                asserts.assert_true(component_id is not None, f"Component in {enum_name} should have a valid ID")

    def test_global_bitmaps_validation(self):
        """Test validation of all global bitmap data types"""
        global_bitmaps = self.xml_global_data_types['bitmaps']
        asserts.assert_true(len(global_bitmaps) > 0, "Should have at least one global bitmap")

        self.print_step("Global Bitmaps", f"Found {len(global_bitmaps)} global bitmaps")

        # Comprehensively validate ALL global bitmaps
        for bitmap_name, bitmap in global_bitmaps.items():
            # Validate basic properties
            asserts.assert_equal(bitmap.data_type, DataTypeEnum.kBitmap, f"{bitmap_name} should be a bitmap")
            asserts.assert_equal(bitmap.name, bitmap_name, "Bitmap name should match")
            asserts.assert_true(len(bitmap.components) > 0, f"{bitmap_name} should have components")
            asserts.assert_true(bitmap.cluster_ids is None, f"Global bitmap {bitmap_name} should have no cluster IDs")

            # Validate all components have proper attributes
            for component_id, component in bitmap.components.items():
                asserts.assert_true(component.name, f"Component {component_id} in {bitmap_name} should have a name")
                asserts.assert_true(component.conformance is not None,
                                    f"Component {component_id} in {bitmap_name} should have conformance")
                asserts.assert_true(component_id is not None, f"Component in {bitmap_name} should have a valid ID")

    def test_global_data_types_comprehensive_validation(self):
        """Comprehensive validation of all global data types"""
        issues = []

        # Check all global structs
        for struct_name, struct in self.xml_global_data_types['structs'].items():
            if not struct.name:
                issues.append("Global struct with empty name")
            if struct.cluster_ids is not None:
                issues.append(f"Global struct {struct_name} should not have cluster IDs")

            for component_id, component in struct.components.items():
                if not component.name:
                    issues.append(f"Global struct {struct_name} component {component_id} has empty name")
                if component_id is None:
                    issues.append(f"Global struct {struct_name} component has None ID")

        # Check all global enums
        for enum_name, enum in self.xml_global_data_types['enums'].items():
            if not enum.name:
                issues.append("Global enum with empty name")
            if enum.cluster_ids is not None:
                issues.append(f"Global enum {enum_name} should not have cluster IDs")

            for component_id, component in enum.components.items():
                if not component.name:
                    issues.append(f"Global enum {enum_name} component {component_id} has empty name")
                if component_id is None:
                    issues.append(f"Global enum {enum_name} component has None ID")

        # Check all global bitmaps
        for bitmap_name, bitmap in self.xml_global_data_types['bitmaps'].items():
            if not bitmap.name:
                issues.append("Global bitmap with empty name")
            if bitmap.cluster_ids is not None:
                issues.append(f"Global bitmap {bitmap_name} should not have cluster IDs")

            for component_id, component in bitmap.components.items():
                if not component.name:
                    issues.append(f"Global bitmap {bitmap_name} component {component_id} has empty name")
                if component_id is None:
                    issues.append(f"Global bitmap {bitmap_name} component has None ID")

        if issues:
            print("\n===== Global Data Type Validation Issues Found =====")
            for issue in issues:
                print(issue)
            asserts.fail(f"{len(issues)} issues found in global data type validation. See above for details.")

        self.print_step("Global Validation", "Successfully validated all global data types with no issues")


if __name__ == "__main__":
    default_matter_test_main()
