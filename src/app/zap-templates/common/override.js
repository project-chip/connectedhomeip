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
  case 'node_id':
    return 'chip::NodeId';
  case 'endpoint_id':
    return 'chip::EndpointId';
  case 'group_id':
    return 'chip::GroupId';
  case 'attribute_id':
    return 'chip::AttributeId';
  case 'cluster_id':
    return 'chip::ClusterId';
  case 'ieee_address':
    return 'uint64_t';
  default:
    throw 'not overriding';
  }
}

exports.atomicType = atomicType
