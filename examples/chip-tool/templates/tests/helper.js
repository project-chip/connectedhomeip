/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

const { zapTypeToDecodableClusterObjectType, zapTypeToEncodableClusterObjectType, asUpperCamelCase, asLowerCamelCase }
= require('../../../../src/app/zap-templates/templates/app/helper.js');
const { isTestOnlyCluster } = require('../../../../src/app/zap-templates/common/simulated-clusters/SimulatedClusters.js');

function utf8StringLength(str)
{
  return new TextEncoder().encode(str).length
}

/*
 * Returns the name to use for accessing a given property of
 * a decodable type.
 *
 */
function asPropertyValue(options)
{
  let rootObject = 'value';

  let context = options.hash.context || this;

  // The decodable type for commands is a struct by default, even if the
  // command just returns a single value.
  if (context.parent.isCommand) {
    rootObject += '.' + asLowerCamelCase(context.name);
  }

  if (context.isOptional && !options.hash.dontUnwrapValue) {
    rootObject += '.Value()';
  }

  return rootObject;
}

async function asEncodableType()
{
  // Copy some properties needed by zapTypeToEncodableClusterObjectType
  let target = { global : this.global, entryType : this.entryType };

  let type;
  if ('commandObject' in this) {
    type = this.commandObject.name;
  } else if ('attributeObject' in this) {
    type              = this.attributeObject.type;
    target.isArray    = this.attributeObject.isArray;
    target.isOptional = this.attributeObject.isOptional;
    target.isNullable = this.attributeObject.isNullable;
  } else {
    throw new Error("Unsupported encodable type");
  }

  if (isTestOnlyCluster(this.cluster) || 'commandObject' in this) {
    return `chip::app::Clusters::${asUpperCamelCase(this.cluster)}::Commands::${asUpperCamelCase(type)}::Type`;
  }

  const options = { 'hash' : { ns : this.cluster } };
  return await zapTypeToEncodableClusterObjectType.call(target, type, options);
}

async function asDecodableType()
{
  // Copy some properties needed by zapTypeToDecodableClusterObjectType
  let target = { global : this.global, entryType : this.entryType };

  let type;
  if ('commandObject' in this) {
    type = this.commandObject.responseName;
  } else if ('attributeObject' in this) {
    type              = this.attributeObject.type;
    target.isArray    = this.attributeObject.isArray;
    target.isOptional = this.attributeObject.isOptional;
    target.isNullable = this.attributeObject.isNullable;
  } else if ('eventObject' in this) {
    type = this.event;
  } else {
    throw new Error("Unsupported decodable type");
  }

  if (isTestOnlyCluster(this.cluster) || 'commandObject' in this) {
    return `chip::app::Clusters::${asUpperCamelCase(this.cluster)}::Commands::${asUpperCamelCase(type)}::DecodableType`;
  }

  const options = { 'hash' : { ns : this.cluster } };
  return await zapTypeToDecodableClusterObjectType.call(target, type, options);
}

//
// Module exports
//
exports.utf8StringLength = utf8StringLength;
exports.asPropertyValue  = asPropertyValue;
exports.asDecodableType  = asDecodableType;
exports.asEncodableType  = asEncodableType;
