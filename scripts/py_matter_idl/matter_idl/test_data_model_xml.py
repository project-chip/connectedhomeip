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
from typing import List, Union

try:
    from matter_idl.data_model_xml import ParseSource, ParseXmls
except ImportError:
    import os
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.data_model_xml import ParseSource, ParseXmls

from matter_idl.matter_idl_types import Idl
from matter_idl_parser import CreateParser


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

        self.assertEqual(xml_idl, expected_idl)

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

        self.assertEqual(xml_idl, expected_idl)


if __name__ == '__main__':
    unittest.main()
