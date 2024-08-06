#!/usr/bin/env python3
# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import io
import unittest
from difflib import unified_diff
from typing import List, Optional, Union

try:
    from matter_idl.data_model_xml import ParseSource, ParseXmls
except ImportError:
    import os
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.data_model_xml import ParseSource, ParseXmls

from matter_idl.generators import GeneratorStorage
from matter_idl.generators.idl import IdlGenerator
from matter_idl.matter_idl_parser import CreateParser
from matter_idl.matter_idl_types import Idl


class GeneratorContentStorage(GeneratorStorage):
    def __init__(self):
        super().__init__()
        self.content: Optional[str] = None

    def get_existing_data(self, relative_path: str):
        # Force re-generation each time
        return None

    def write_new_data(self, relative_path: str, content: str):
        if self.content:
            raise Exception(
                "Unexpected extra data: single file generation expected")
        self.content = content


def RenderAsIdlTxt(idl: Idl) -> str:
    storage = GeneratorContentStorage()
    IdlGenerator(storage=storage, idl=idl).render(dry_run=False)
    return storage.content or ""


def XmlToIdl(what: Union[str, List[str]]) -> Idl:
    if not isinstance(what, list):
        what = [what]

    sources = []
    for idx, txt in enumerate(what):
        sources.append(ParseSource(source=io.StringIO(
            txt), name=("Input %d" % (idx + 1))))

    return ParseXmls(sources, include_meta_data=False)


def IdlTextToIdl(what: str) -> Idl:
    return CreateParser(skip_meta=True).parse(what)


class TestXmlParser(unittest.TestCase):

    def __init__(self, *args, **kargs):
        super().__init__(*args, **kargs)
        self.maxDiff = None

    def assertIdlEqual(self, a: Idl, b: Idl):
        if a == b:
            # seems the same. This will just pass
            self.assertEqual(a, b)
            return

        # Not the same. Try to make a human readable diff:
        a_txt = RenderAsIdlTxt(a)
        b_txt = RenderAsIdlTxt(b)

        delta = unified_diff(a_txt.splitlines(keepends=True),
                             b_txt.splitlines(keepends=True),
                             fromfile='actual.matter',
                             tofile='expected.matter',
                             )
        self.assertEqual(a, b, '\n' + ''.join(delta))
        self.fail("IDLs are not equal (above delta should have failed)")

    def testBasicInput(self):

        xml_idl = XmlToIdl('''
            <cluster id="123" name="Test" revision="1"/>
        ''')

        expected_idl = IdlTextToIdl('''
            client cluster Test = 123 {
               readonly attribute attrib_id attributeList[] = 65531;
               readonly attribute event_id eventList[] = 65530;
               readonly attribute command_id acceptedCommandList[] = 65529;
               readonly attribute command_id generatedCommandList[] = 65528;
               readonly attribute bitmap32 featureMap = 65532;
               readonly attribute int16u clusterRevision = 65533;
           }
        ''')

        self.assertIdlEqual(xml_idl, expected_idl)

    def testClusterDerivation(self):
        # This test is based on a subset of ModeBase and Mode_Dishwasher original xml files

        xml_idl = XmlToIdl([
            # base ...
            '''
<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="" name="Mode Base" revision="2">
  <revisionHistory>
    <revision revision="1" summary="Initial version"/>
    <revision revision="2" summary="ChangeToModeResponse command: StatusText must be provided for InvalidInMode status"/>
  </revisionHistory>
  <classification hierarchy="base" role="application" picsCode="MODB" scope="Endpoint"/>
  <features>
    <feature name="base reserved" summary="This range of bits is reserved for this base cluster">
      <optionalConform/>
    </feature>
    <feature name="derived reserved" summary="This range of bits is reserved for derived clusters">
      <optionalConform/>
    </feature>
    <feature bit="0" code="DEPONOFF" name="OnOff" summary="Dependency with the OnOff cluster">
      <optionalConform/>
    </feature>
  </features>
  <dataTypes>
    <struct name="ModeOptionStruct">
      <field id="0" name="Label" type="string" default="MS">
        <access read="true"/>
        <mandatoryConform/>
        <constraint type="maxLength" value="64"/>
      </field>
      <field id="1" name="Mode" type="uint8" default="MS">
        <access read="true"/>
        <mandatoryConform/>
      </field>
      <field id="2" name="ModeTags" type="list[ModeTagStruct Type]" default="MS">
        <access read="true"/>
        <mandatoryConform/>
        <constraint type="max" value="8"/>
      </field>
    </struct>
  </dataTypes>
  <attributes>
    <attribute name="base reserved">
      <mandatoryConform/>
    </attribute>
    <attribute name="derived reserved">
      <mandatoryConform/>
    </attribute>
    <attribute id="0x0000" name="SupportedModes" type="list[ModeOptionStruct Type]" default="MS">
      <access read="true" readPrivilege="view"/>
      <quality changeOmitted="false" nullable="false" scene="false" persistence="fixed" reportable="false"/>
      <mandatoryConform/>
      <constraint type="between" from="2" to="255"/>
    </attribute>
  </attributes>
</cluster>
        ''',
            # derived ...
            '''
<cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="0x0059" name="Dishwasher Mode" revision="2">
  <revisionHistory>
    <revision revision="1" summary="Initial Release"/>
    <revision revision="2" summary="ChangeToModeResponse command: StatusText must be provided for InvalidInMode status"/>
  </revisionHistory>
  <classification hierarchy="derived" baseCluster="Mode Base" role="application" picsCode="DISHM" scope="Endpoint"/>
  <dataTypes>
    <struct name="ModeOptionStruct">
      <field id="0" name="Label">
        <mandatoryConform/>
      </field>
      <field id="1" name="Mode">
        <mandatoryConform/>
      </field>
      <field id="2" name="ModeTags">
        <mandatoryConform/>
        <constraint type="between" from="1" to="8"/>
      </field>
    </struct>
  </dataTypes>
  <attributes>
    <attribute id="0x0000" name="SupportedModes">
      <mandatoryConform/>
    </attribute>
  </attributes>
</cluster>
        ''',
        ])

        expected_idl = IdlTextToIdl('''
            client cluster DishwasherMode = 89 {
               revision 2;

               bitmap Feature: bitmap32 {
                   kOnOff = 0x1;
               }

               struct ModeOptionStruct {
                  char_string<64> label = 0;
                  int8u mode = 1;
                  ModeTagStruct modeTags[] = 2;
               }

               readonly attribute ModeOptionStruct supportedModes[] = 0;
               readonly attribute attrib_id attributeList[] = 65531;
               readonly attribute event_id eventList[] = 65530;
               readonly attribute command_id acceptedCommandList[] = 65529;
               readonly attribute command_id generatedCommandList[] = 65528;
               readonly attribute bitmap32 featureMap = 65532;
               readonly attribute int16u clusterRevision = 65533;
           }
        ''')

        self.assertIdlEqual(xml_idl, expected_idl)

    def testSignedTypes(self):

        xml_idl = XmlToIdl('''
            <cluster id="123" name="Test" revision="1">
              <attributes>
                <attribute id="0x0000" name="First" type="int16">
                  <access read="true" readPrivilege="view"/>
                  <mandatoryConform/>
                </attribute>
                <attribute id="0x0001" name="Second" type="int16s">
                  <access read="true" readPrivilege="view"/>
                  <mandatoryConform/>
                </attribute>
                <attribute id="0x0002" name="Third" type="int32u">
                  <access read="true" readPrivilege="view"/>
                  <mandatoryConform/>
                </attribute>
              </attributes>
            </cluster>
        ''')

        expected_idl = IdlTextToIdl('''
            client cluster Test = 123 {
               readonly attribute int16s first = 0;
               readonly attribute int16s second = 1;
               readonly attribute int32u third = 2;

               readonly attribute attrib_id attributeList[] = 65531;
               readonly attribute event_id eventList[] = 65530;
               readonly attribute command_id acceptedCommandList[] = 65529;
               readonly attribute command_id generatedCommandList[] = 65528;
               readonly attribute bitmap32 featureMap = 65532;
               readonly attribute int16u clusterRevision = 65533;
           }
        ''')

        self.assertIdlEqual(xml_idl, expected_idl)

    def testEnumRange(self):
        # Check heuristic for enum ranges

        xml_idl = XmlToIdl('''
            <cluster id="123" name="Test" revision="1">
              <dataTypes>
                <bitmap name="Basic">
                  <bitfield name="One" bit="0">
                    <mandatoryConform/>
                  </bitfield>
                  <bitfield name="Two" bit="1">
                    <mandatoryConform/>
                  </bitfield>
                  <bitfield name="Three" bit="2">
                    <mandatoryConform/>
                  </bitfield>
                </bitmap>
                <bitmap name="OneLarge">
                  <bitfield name="Ten" bit="10">
                    <mandatoryConform/>
                  </bitfield>
                </bitmap>
                <bitmap name="LargeBitmap">
                  <bitfield name="One" bit="0">
                    <mandatoryConform/>
                  </bitfield>
                  <bitfield name="Ten" bit="10">
                    <mandatoryConform/>
                  </bitfield>
                  <bitfield name="Twenty" bit="20">
                    <mandatoryConform/>
                  </bitfield>
                </bitmap>
                <bitmap name="HugeBitmap">
                  <bitfield name="Forty" bit="40">
                    <mandatoryConform/>
                  </bitfield>
                </bitmap>
              </dataTypes>
            </cluster>
        ''')

        expected_idl = IdlTextToIdl('''
            client cluster Test = 123 {
               bitmap Basic: bitmap8 {
                  kOne = 0x01;
                  kTwo = 0x02;
                  kThree = 0x04;
               }

               bitmap OneLarge: bitmap16 {
                  kTen = 0x400;
               }

               bitmap LargeBitmap: bitmap32 {
                  kOne = 0x1;
                  kTen = 0x400;
                  kTwenty = 0x100000;
               }

               bitmap HugeBitmap: bitmap64 {
                  kForty = 0x10000000000;
               }

               readonly attribute attrib_id attributeList[] = 65531;
               readonly attribute event_id eventList[] = 65530;
               readonly attribute command_id acceptedCommandList[] = 65529;
               readonly attribute command_id generatedCommandList[] = 65528;
               readonly attribute bitmap32 featureMap = 65532;
               readonly attribute int16u clusterRevision = 65533;
           }
        ''')

        self.assertIdlEqual(xml_idl, expected_idl)

    def testAttributes(self):
        # Validate an attribute with a type list
        # This is a very stripped down version from the original AudioOutput.xml

        xml_idl = XmlToIdl('''
            <cluster id="123" name="Test" revision="1">
              <dataTypes>
                <struct name="OutputInfoStruct">
                  <field id="0" name="Index" type="uint8">
                    <access read="true" write="true"/>
                    <mandatoryConform/>
                  </field>
                </struct>
              </dataTypes>
              <attributes>
                <attribute id="0x0000" name="OutputList" type="list[OutputInfoStruct Type]">
                  <access read="true" readPrivilege="view"/>
                  <mandatoryConform/>
                </attribute>
                <attribute id="0x0001" name="TestConform" type="enum8">
                  <access read="true" readPrivilege="view"/>
                  <otherwiseConform>
                    <mandatoryConform>
                      <feature name="PRSCONST"/>
                    </mandatoryConform>
                    <optionalConform>
                      <feature name="AUTO"/>
                    </optionalConform>
                  </otherwiseConform>
                </attribute>
              </attributes>
            </cluster>
        ''')

        expected_idl = IdlTextToIdl('''
            client cluster Test = 123 {
               struct OutputInfoStruct {
                  int8u index = 0;
               }

               readonly attribute OutputInfoStruct outputList[] = 0;
               readonly attribute optional enum8 testConform = 1;

               readonly attribute attrib_id attributeList[] = 65531;
               readonly attribute event_id eventList[] = 65530;
               readonly attribute command_id acceptedCommandList[] = 65529;
               readonly attribute command_id generatedCommandList[] = 65528;
               readonly attribute bitmap32 featureMap = 65532;
               readonly attribute int16u clusterRevision = 65533;
           }
        ''')

        self.assertIdlEqual(xml_idl, expected_idl)

    def testXmlNameWorkarounds(self):
        # Validate an attribute with a type list
        # This is a manually-edited copy of an attribute test (not real data)

        xml_idl = XmlToIdl('''
            <cluster id="123" name="Test" revision="1">
              <dataTypes>
                <struct name="OutputInfoStruct">
                  <field id="0" name="ID" type="&lt;&lt;ref_DataTypeString&gt;&gt;">
                    <access read="true" write="true"/>
                    <mandatoryConform/>
                  </field>
                  <field id="1" name="items" type="&lt;&lt;ref_DataTypeList&gt;&gt;[uint8]">
                    <access read="true" write="true"/>
                    <mandatoryConform/>
                  </field>
                  <field id="2" name="endpoints" type="&lt;&lt;ref_DataTypeList&gt;&gt;[&lt;&lt;ref_DataTypeEndpointNumber&gt;&gt; Type]">
                    <access read="true" write="true"/>
                    <mandatoryConform/>
                  </field>
                  <field id="3" name="NominalPower" type="power-mW">
                    <access read="true" write="true"/>
                    <mandatoryConform>
                      <feature name="PFR"/>
                    </mandatoryConform>
                    <constraint type="desc"/>
                  </field>
                  <field id="4" name="MaximumEnergy" type="energy-mWh">
                    <access read="true" write="true"/>
                    <mandatoryConform>
                      <feature name="PFR"/>
                    </mandatoryConform>
                  </field>
                </struct>
              </dataTypes>
              <attributes>
                <attribute id="0x0000" name="OutputList" type="list[OutputInfoStruct Type]">
                  <access read="true" readPrivilege="view"/>
                  <mandatoryConform/>
                </attribute>
                <attribute id="0x0001" name="TestConform" type="enum8">
                  <access read="true" readPrivilege="view"/>
                  <otherwiseConform>
                    <mandatoryConform>
                      <feature name="PRSCONST"/>
                    </mandatoryConform>
                    <optionalConform>
                      <feature name="AUTO"/>
                    </optionalConform>
                  </otherwiseConform>
                </attribute>
              </attributes>
            </cluster>
        ''')

        expected_idl = IdlTextToIdl('''
            client cluster Test = 123 {
               struct OutputInfoStruct {
                  char_string id = 0;
                  int8u items[] = 1;
                  endpoint_no endpoints[] = 2;
                  optional power_mw nominalPower = 3;
                  optional energy_mwh maximumEnergy = 4;
               }

               readonly attribute OutputInfoStruct outputList[] = 0;
               readonly attribute optional enum8 testConform = 1;


               readonly attribute attrib_id attributeList[] = 65531;
               readonly attribute event_id eventList[] = 65530;
               readonly attribute command_id acceptedCommandList[] = 65529;
               readonly attribute command_id generatedCommandList[] = 65528;
               readonly attribute bitmap32 featureMap = 65532;
               readonly attribute int16u clusterRevision = 65533;
           }
        ''')

        self.assertIdlEqual(xml_idl, expected_idl)

    def testComplexInput(self):
        # This parses a known copy of Switch.xml which happens to be fully
        # spec-conformant (so assuming it as a good input)
        xml_idl = XmlToIdl('''
             <cluster xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="types types.xsd cluster cluster.xsd" id="0x003b" name="Switch" revision="1">
                <revisionHistory>
                    <revision revision="1" summary="Initial Release"/>
                </revisionHistory>
                <classification hierarchy="base" role="application" picsCode="SWTCH" scope="Endpoint"/>
                <features>
                    <feature bit="0" code="LS" name="LatchingSwitch" summary="Switch is latching">
                    <optionalConform choice="a"/>
                    </feature>
                    <feature bit="1" code="MS" name="MomentarySwitch" summary="Switch is momentary">
                    <optionalConform choice="a"/>
                    </feature>
                    <feature bit="2" code="MSR" name="MomentarySwitchRelease" summary="Switch supports release">
                    <optionalConform>
                        <feature name="MS"/>
                    </optionalConform>
                    </feature>
                    <feature bit="3" code="MSL" name="MomentarySwitchLongPress" summary="Switch supports long press">
                    <optionalConform>
                        <andTerm>
                        <feature name="MS"/>
                        <feature name="MSR"/>
                        </andTerm>
                    </optionalConform>
                    </feature>
                    <feature bit="4" code="MSM" name="MomentarySwitchMultiPress" summary="Switch supports multi-press">
                    <optionalConform>
                        <andTerm>
                        <feature name="MS"/>
                        <feature name="MSR"/>
                        </andTerm>
                    </optionalConform>
                    </feature>
                </features>
                <attributes>
                    <attribute id="0x0000" name="NumberOfPositions" type="uint8" default="2">
                    <access read="true" readPrivilege="view"/>
                    <quality changeOmitted="false" nullable="false" scene="false" persistence="fixed" reportable="false"/>
                    <mandatoryConform/>
                    <constraint type="between" from="2" to="max"/>
                    </attribute>
                    <attribute id="0x0001" name="CurrentPosition" type="uint8" default="0">
                    <access read="true" readPrivilege="view"/>
                    <quality changeOmitted="false" nullable="false" scene="false" persistence="nonVolatile" reportable="false"/>
                    <mandatoryConform/>
                    <constraint type="between" from="0" to="NumberOfPositions-1"/>
                    </attribute>
                    <attribute id="0x0002" name="MultiPressMax" type="uint8" default="2">
                    <access read="true" readPrivilege="view"/>
                    <quality changeOmitted="false" nullable="false" scene="false" persistence="fixed" reportable="false"/>
                    <mandatoryConform>
                        <feature name="MSM"/>
                    </mandatoryConform>
                    <constraint type="between" from="2" to="max"/>
                    </attribute>
                </attributes>
                <events>
                    <event id="0x00" name="SwitchLatched" priority="info">
                    <access readPrivilege="view"/>
                    <mandatoryConform>
                        <feature name="LS"/>
                    </mandatoryConform>
                    <field id="0" name="NewPosition" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="0" to="NumberOfPositions-1"/>
                    </field>
                    </event>
                    <event id="0x01" name="InitialPress" priority="info">
                    <access readPrivilege="view"/>
                    <mandatoryConform>
                        <feature name="MS"/>
                    </mandatoryConform>
                    <field id="0" name="NewPosition" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="0" to="NumberOfPositions-1"/>
                    </field>
                    </event>
                    <event id="0x02" name="LongPress" priority="info">
                    <access readPrivilege="view"/>
                    <mandatoryConform>
                        <feature name="MSL"/>
                    </mandatoryConform>
                    <field id="0" name="NewPosition" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="0" to="NumberOfPositions-1"/>
                    </field>
                    </event>
                    <event id="0x03" name="ShortRelease" priority="info">
                    <access readPrivilege="view"/>
                    <mandatoryConform>
                        <feature name="MSR"/>
                    </mandatoryConform>
                    <field id="0" name="PreviousPosition" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="0" to="NumberOfPositions-1"/>
                    </field>
                    </event>
                    <event id="0x04" name="LongRelease" priority="info">
                    <access readPrivilege="view"/>
                    <mandatoryConform>
                        <feature name="MSL"/>
                    </mandatoryConform>
                    <field id="0" name="PreviousPosition" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="0" to="NumberOfPositions-1"/>
                    </field>
                    </event>
                    <event id="0x05" name="MultiPressOngoing" priority="info">
                    <access readPrivilege="view"/>
                    <mandatoryConform>
                        <feature name="MSM"/>
                    </mandatoryConform>
                    <field id="0" name="NewPosition" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="0" to="NumberOfPositions-1"/>
                    </field>
                    <field id="1" name="CurrentNumberOfPressesCounted" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="2" to="MultiPressMax"/>
                    </field>
                    </event>
                    <event id="0x06" name="MultiPressComplete" priority="info">
                    <access readPrivilege="view"/>
                    <mandatoryConform>
                        <feature name="MSM"/>
                    </mandatoryConform>
                    <field id="0" name="PreviousPosition" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="0" to="NumberOfPositions-1"/>
                    </field>
                    <field id="1" name="TotalNumberOfPressesCounted" type="uint8">
                        <mandatoryConform/>
                        <constraint type="between" from="1" to="MultiPressMax"/>
                    </field>
                    </event>
                </events>
             </cluster>
        ''')

        expected_idl = IdlTextToIdl('''
            client cluster Switch = 59 {
              bitmap Feature : bitmap32 {
                kLatchingSwitch = 0x1;
                kMomentarySwitch = 0x2;
                kMomentarySwitchRelease = 0x4;
                kMomentarySwitchLongPress = 0x8;
                kMomentarySwitchMultiPress = 0x10;
              }

              info event SwitchLatched = 0 {
                int8u newPosition = 0;
              }

              info event InitialPress = 1 {
                int8u newPosition = 0;
              }

              info event LongPress = 2 {
                int8u newPosition = 0;
              }

              info event ShortRelease = 3 {
                int8u previousPosition = 0;
              }

              info event LongRelease = 4 {
                int8u previousPosition = 0;
              }

              info event MultiPressOngoing = 5 {
                int8u newPosition = 0;
                int8u currentNumberOfPressesCounted = 1;
              }

              info event MultiPressComplete = 6 {
                int8u previousPosition = 0;
                int8u totalNumberOfPressesCounted = 1;
              }

              readonly attribute int8u numberOfPositions = 0;
              readonly attribute int8u currentPosition = 1;
              readonly attribute optional int8u multiPressMax = 2;
              readonly attribute attrib_id attributeList[] = 65531;
              readonly attribute event_id eventList[] = 65530;
              readonly attribute command_id acceptedCommandList[] = 65529;
              readonly attribute command_id generatedCommandList[] = 65528;
              readonly attribute bitmap32 featureMap = 65532;
              readonly attribute int16u clusterRevision = 65533;
            }
            ''')

        self.assertIdlEqual(xml_idl, expected_idl)


if __name__ == '__main__':
    unittest.main()
