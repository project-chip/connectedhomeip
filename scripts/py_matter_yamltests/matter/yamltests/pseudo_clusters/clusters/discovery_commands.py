#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

from ..pseudo_cluster import PseudoCluster

_DEFINITION = '''<?xml version="1.0"?>
<configurator>
<cluster>
    <name>DiscoveryCommands</name>
    <code>0xFFF1FD05</code>

    <command source="client" code="0" name="FindCommissionable" response="FindResponse">
    </command>

    <command source="client" code="1" name="FindCommissionableByShortDiscriminator" response="FindResponse">
        <arg name="value" type="int16u"/>
    </command>

    <command source="client" code="2" name="FindCommissionableByLongDiscriminator" response="FindResponse">
        <arg name="value" type="int16u"/>
    </command>

    <command source="client" code="3" name="FindCommissionableByCommissioningMode" response="FindResponse">
    </command>

    <command source="client" code="4" name="FindCommissionableByVendorId" response="FindResponse">
        <arg name="value" type="vendor_id"/>
    </command>

    <command source="client" code="5" name="FindCommissionableByDeviceType" response="FindResponse">
        <arg name="value" type="devtype_id"/>
    </command>

    <command source="client" code="6" name="FindCommissioner" response="FindCommissionerResponse">
    </command>

    <command source="client" code="7" name="FindCommissionerByVendorId" response="FindCommissionerResponse">
        <arg name="value" type="vendor_id"/>
    </command>

    <command source="client" code="8" name="FindCommissionerByDeviceType" response="FindCommissionerResponse">
        <arg name="value" type="devtype_id"/>
    </command>

    <command source="server" code="9" name="FindResponse">
        <arg name="hostName" type="char_string"/>
        <arg name="instanceName" type="char_string"/>
        <arg name="longDiscriminator" type="int16u"/>
        <arg name="shortDiscriminator" type="int16u"/>
        <arg name="vendorId" type="vendor_id"/>
        <arg name="productId" type="int16u"/>
        <arg name="commissioningMode" type="int8u"/>
        <arg name="deviceType" type="devtype_id"/>
        <arg name="deviceName" type="char_string"/>
        <arg name="rotatingId" type="octet_string"/>
        <arg name="rotatingIdLen" type="int64u"/>
        <arg name="pairingHint" type="int16u"/>
        <arg name="pairingInstruction" type="char_string"/>
        <arg name="supportsTcpClient" type="boolean"/>
        <arg name="supportsTcpServer" type="boolean"/>
        <arg name="numIPs" type="int8u"/>
        <arg name="port" type="int16u"/>
        <arg name="mrpRetryIntervalIdle" type="int32u" optional="true"/>
        <arg name="mrpRetryIntervalActive" type="int32u" optional="true"/>
        <arg name="mrpRetryActiveThreshold" type="int16u" optional="true"/>
        <arg name="isICDOperatingAsLIT" type="boolean" optional="true"/>
    </command>
</cluster>
</configurator>
'''


class DiscoveryCommands(PseudoCluster):
    name = 'DiscoveryCommands'
    definition = _DEFINITION
