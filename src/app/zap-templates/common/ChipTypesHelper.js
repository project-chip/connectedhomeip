/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

function asBasicType(type)
{
  switch (type) {
  case 'chip::CommandId':
  case 'chip::EndpointId':
    return 'uint8_t';
  case 'chip::AttributeId':
  case 'chip::ClusterId':
  case 'chip::EventId':
  case 'chip::GroupId':
    return 'uint16_t';
  case 'chip::NodeId':
    return 'uint64_t';
  default:
    return type;
  }
}

//
// Module exports
//
exports.asBasicType = asBasicType;
