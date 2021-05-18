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

const basePath = '../../../../';
const testPath = 'src/app/tests/yaml/';
const YAML     = require(basePath + 'third_party/zap/repo/node_modules/yaml');
const fs       = require('fs');
const path     = require('path');

const kClusterName        = 'cluster';
const kEndpointName       = 'endpoint';
const kTypeName           = 'type';
const kIndexName          = 'index';
const kResponseName       = 'response';
const kResponseErrorName  = 'error';
const kResponseValuesName = 'values';

function setDefault(test, name, defaultValue)
{
  if (!(name in test)) {
    if (defaultValue == null) {
      const errorStr = 'Test with label "' + test.label + '" does not have any "' + name + '" defined.';
      throw new Error(errorStr);
    }

    test[name] = defaultValue;
  }
}

function setDefaultType(test)
{
  const defaultType = 'command';
  setDefault(test, kTypeName, defaultType);

  const type = test[kTypeName];
  switch (type) {
  case 'command':
    test.isCommand = true;
    break;

  case 'readAttribute':
    test.isAttribute     = true;
    test.isReadAttribute = true;
    break;

  case 'writeTestAttribute':
    test.isAttribute      = true;
    test.isWriteAttribute = true;
    break;
  }

  delete test[kTypeName];
}

function setDefaultResponse(test)
{
  const defaultResponse = {};
  setDefault(test, kResponseName, defaultResponse);

  const defaultResponseError = 0;
  setDefault(test.response, kResponseErrorName, defaultResponseError);

  const defaultResponseValues = [];
  setDefault(test.response, kResponseValuesName, defaultResponseValues);

  if (!test.isReadAttribute) {
    return;
  }

  if (!('value' in test.response)) {
    const errorStr = 'Test with label "' + test.label + '" does not have a "value" defined.';
    throw new Error(errorStr);
  }

  if (!('type' in test.response)) {
    const errorStr = 'Test with label "' + test.label + '" does not have a "type" defined.';
    throw new Error(errorStr);
  }

  test.response.values.push({ name : 'value', value : test.response.value, type : test.response.type });
  delete test.response.value;
  delete test.response.type;
}

function setDefaults(test, index, defaultConfig)
{
  const defaultClusterName = defaultConfig[kClusterName] || null;
  const defaultEndpointId  = defaultConfig[kEndpointName] || null;

  setDefaultType(test);
  setDefault(test, kIndexName, index);
  setDefault(test, kClusterName, defaultClusterName);
  setDefault(test, kEndpointName, defaultEndpointId);
  setDefaultResponse(test);
}

function parse(name)
{
  const filepath = path.resolve(__dirname, basePath + testPath + name + '.yaml');
  const data     = fs.readFileSync(filepath, { encoding : 'utf8', flag : 'r' });
  const yaml     = YAML.parse(data);

  const defaultConfig = yaml.config || [];
  yaml.tests.forEach((test, index) => {
    setDefaults(test, index, defaultConfig);
  });

  return yaml;
}

//
// Module exports
//
exports.parse = parse;
