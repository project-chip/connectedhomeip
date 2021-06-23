/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

function atomicType(arg)
{
  switch (arg.name) {
  case 'cluster_id':
  case 'field_id':
  case 'event_id':
  case 'command_id':
  case 'action_id':
  case 'transaction_id':
  case 'node_id':
  case 'vendor_id':
  case 'fabric_id':
  case 'group_id':
    const parts = arg.name.split('_');
    return 'chip::' + parts[0][0].toUpperCase() + parts[0].substring(1) + parts[1][0].toUpperCase() + parts[1].substring(1);

  case 'attrib_id':
    return 'chip::AttributeId';
  case 'devtype_id':
    return 'chip::DeviceTypeId';
  case 'fabric_idx':
    return 'chip::FabricIndex';
  case 'status':
    return 'chip::StatusCode';
  case 'data_ver':
    return 'chip::DataVersion';
  case 'event_no':
    return 'chip::EventNumber';
  case 'endpoint_no':
    return 'chip::EndpointId';
  default:
    throw 'not overriding';
  }
}

exports.atomicType = atomicType
