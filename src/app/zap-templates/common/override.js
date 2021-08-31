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

function cleanseUints(uint)
{
  if (uint == 'uint24_t')
    return 'uint32_t'
    if (uint == 'uint48_t') return 'uint8_t *'
    return uint
}

function defaultAtomicType(arg = {
  name : 'unknown',
  size : 0
})
{
  let name = arg.name
  let size = arg.size
  if (name.startsWith('int'))
  {
    let signed if (name.endsWith('s')) signed = true
    else signed                               = false

    let ret = `${signed ? '' : 'u'}int${size * 8}_t`

    // few exceptions
    ret = cleanseUints(ret)
    return ret
  }
  else if (name.startsWith('enum') || name.startsWith('data'))
  {
    return cleanseUints(`uint${name.slice(4)}_t`)
  }
  else if (name.startsWith('bitmap'))
  {
    return cleanseUints(`uint${name.slice(6)}_t`)
  }
  else
  {
    switch (name) {
    case 'utc_time':
    case 'date':
    case 'time_of_day':
    case 'bacnet_oid':
      return 'uint32_t'
      case 'attribute_id': case 'cluster_id': return 'uint16_t'
      case 'no_data': case 'octet_string': case 'char_string': case 'ieee_address': return 'uint8_t *'
      case 'boolean': return 'uint8_t'
      case 'array': return `/* TYPE WARNING: ${name} array defaults to */ uint8_t * ` default: return `/* TYPE WARNING: ${
          name} defaults to */ uint8_t * `
    }
  }
}

function atomicType(arg)
{
  switch (arg.name) {
  case 'boolean':
    return 'bool'
    case 'action_id': case 'cluster_id': case 'command_id': case 'event_id': case 'fabric_id': case 'field_id': case 'group_id': case 'node_id': case 'transaction_id': case 'vendor_id': return (
        'chip::' + arg.name.split('_').map(part => part[0].toUpperCase() + part.substring(1)).join(''))
    case 'attrib_id': return 'chip::AttributeId'
    case 'data_ver': return 'chip::DataVersion'
    case 'devtype_id': return 'chip::DeviceTypeId'
    case 'endpoint_no': return 'chip::EndpointId'
    case 'event_no': return 'chip::EventNumber'
    case 'fabric_idx': return 'chip::FabricIndex'
    case 'status': return 'chip::StatusCode'
    case 'octet_string': case 'long_octet_string': return 'chip::ByteSpan'
    case 'eui64': return 'chip::node_id' default: return defaultAtomicType(arg)
  }
}
/**
 * Returns the name of a fall-through non-atomic type.
 * This method will be used unless the override is
 * providing a different implementation.
 *
 * @param {*} arg
 */
function nonAtomicType(arg = {
  name : 'unknown',
  isStruct : false
})
{
  return `EmberAf${arg.name}`
}

exports.atomicType    = atomicType
exports.nonAtomicType = nonAtomicType
