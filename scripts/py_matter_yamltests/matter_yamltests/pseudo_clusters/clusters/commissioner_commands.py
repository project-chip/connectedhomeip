#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

from ..pseudo_cluster import PseudoCluster

_DEFINITION = '''<?xml version="1.0"?>
<configurator>
<cluster>
    <name>CommissionerCommands</name>
    <code>0xFFF1FD04</code>

    <command source="client" code="0" name="PairWithCode">
        <arg name="nodeId" type="node_id"/>
        <arg name="payload" type="char_string"/>
        <arg name="discoverOnce" type="boolean" optional="true"/>
    </command>

    <command source="client" code="1" name="Unpair">
      <arg name="nodeId" type="node_id"/>
    </command>

    <command source="client" code="2" name="GetCommissionerNodeId" response="GetCommissionerNodeIdResponse">
    </command>

    <command source="server" code="0" name="GetCommissionerNodeIdResponse">
      <arg name="nodeId" type="node_id"/>
    </command>

    <command source="client" code="3" name="GetCommissionerRootCertificate" response="GetCommissionerRootCertificateResponse">
    </command>

    <command source="server" code="1" name="GetCommissionerRootCertificateResponse">
      <arg name="RCAC" type="OCTET_STRING"/>
    </command>

    <command source="client" code="4" name="IssueNocChain" response="IssueNocChainResponse">
        <arg name="Elements" type="octet_string"/>
        <arg name="nodeId" type="node_id"/>
    </command>

    <command source="server" code="2" name="IssueNocChainResponse">
      <arg name="NOC" type="octet_string"/>
      <arg name="ICAC" type="octet_string"/>
      <arg name="RCAC" type="octet_string"/>
      <arg name="IPK" type="octet_string"/>
    </command>

</cluster>
</configurator>
'''


class CommissionerCommands(PseudoCluster):
    name = 'CommissionerCommands'
    definition = _DEFINITION
