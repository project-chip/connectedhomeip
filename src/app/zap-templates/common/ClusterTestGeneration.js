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
const testPath = 'src/app/tests/suites/';
const zapPath  = basePath + 'third_party/zap/repo/';
const YAML     = require(zapPath + 'node_modules/yaml');
const fs       = require('fs');
const path     = require('path');

// Import helpers from zap core
const templateUtil = require(zapPath + 'src-electron/generator/template-util.js')

const { Clusters, asBlocks, asPromise } = require('./ClustersHelper.js');

const kClusterName       = 'cluster';
const kEndpointName      = 'endpoint';
const kCommandName       = 'command';
const kIndexName         = 'index';
const kValuesName        = 'values';
const kArgumentsName     = 'arguments';
const kResponseName      = 'response';
const kResponseErrorName = 'error';

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
  const type = test[kCommandName];
  switch (type) {
  case 'readAttribute':
    test.isAttribute     = true;
    test.isReadAttribute = true;
    break;

  case 'writeAttribute':
    test.isAttribute      = true;
    test.isWriteAttribute = true;
    break;

  default:
    test.isCommand = true;
    break;
  }
}

function setDefaultArguments(test)
{
  const defaultArguments = {};
  setDefault(test, kArgumentsName, defaultArguments);

  const defaultArgumentsValues = [];
  setDefault(test[kArgumentsName], kValuesName, defaultArgumentsValues);

  if (!test.isWriteAttribute) {
    return;
  }

  if (!('value' in test[kArgumentsName])) {
    const errorStr = 'Test with label "' + test.label + '" does not have a "value" defined.';
    throw new Error(errorStr);
  }

  test[kArgumentsName].values.push({ name : test.attribute, value : test[kArgumentsName].value });
  delete test[kArgumentsName].value;
}

function setDefaultResponse(test)
{
  const defaultResponse = {};
  setDefault(test, kResponseName, defaultResponse);

  const defaultResponseError = 0;
  setDefault(test[kResponseName], kResponseErrorName, defaultResponseError);

  const defaultResponseValues = [];
  setDefault(test[kResponseName], kValuesName, defaultResponseValues);

  if (!test.isReadAttribute) {
    return;
  }

  if (!('value' in test[kResponseName])) {
    const errorStr = 'Test with label "' + test.label + '" does not have a "value" defined.';
    throw new Error(errorStr);
  }

  test[kResponseName].values.push({ name : test.attribute, value : test[kResponseName].value });
  delete test[kResponseName].value;
}

function setDefaults(test, index, defaultConfig)
{
  const defaultClusterName = defaultConfig[kClusterName] || null;
  const defaultEndpointId  = defaultConfig[kEndpointName] || null;

  setDefaultType(test);
  setDefault(test, kIndexName, index);
  setDefault(test, kClusterName, defaultClusterName);
  setDefault(test, kEndpointName, defaultEndpointId);
  setDefaultArguments(test);
  setDefaultResponse(test);
}

function parse(filename)
{
  const filepath = path.resolve(__dirname, basePath + testPath + filename + '.yaml');
  const data     = fs.readFileSync(filepath, { encoding : 'utf8', flag : 'r' });
  const yaml     = YAML.parse(data);

  const defaultConfig = yaml.config || [];
  yaml.tests.forEach((test, index) => {
    setDefaults(test, index, defaultConfig);
  });

  yaml.filename   = filename;
  yaml.totalTests = yaml.tests.length;

  return yaml;
}

//
// Templates
//

function chip_tests(items, options)
{
  const names = items.split(',').map(name => name.trim());
  const tests = names.map(item => parse(item));
  return templateUtil.collectBlocks(tests, options, this);
}

function chip_tests_items(options)
{
  return templateUtil.collectBlocks(this.tests, options, this);
}

function chip_tests_item_parameters(options)
{
  const clusterName   = this.cluster;
  const commandValues = this.arguments.values;

  let filterName;
  let items;

  if (this.isCommand) {
    filterName = this.command;
    items      = Clusters.getClientCommands(clusterName);
  } else if (this.isAttribute) {
    filterName = this.attribute;
    items      = Clusters.getServerAttributes(clusterName);
  } else {
    throw new Error("Unsupported command type: ", this);
  }

  const promise = items.then(items => {
    const filter = item => item.name.toLowerCase() == filterName.toLowerCase();
    const commandArgs   = items.find(filter).arguments;

    const commands = commandArgs.map(commandArg => {
      commandArg = JSON.parse(JSON.stringify(commandArg));

      const expected          = commandValues.find(value => value.name == commandArg.name);
      commandArg.definedValue = expected.value;

      return commandArg;
    });

    return commands;
  });

  return asBlocks.call(this, promise, options);
}

function chip_tests_item_response_parameters(options)
{
  const clusterName    = this.cluster;
  const responseValues = this.response.values;

  let filterName;
  let items;

  if (this.isCommand) {
    filterName = this.command;
    items      = Clusters.getClientCommands(clusterName);
  } else if (this.isAttribute) {
    filterName = this.attribute;
    items      = Clusters.getServerAttributes(clusterName);
  } else {
    throw new Error("Unsupported command type: ", this);
  }

  const promise = items.then(items => {
    const filter = item => item.name.toLowerCase() == filterName.toLowerCase();
    const responseArgs  = items.find(filter).response.arguments;

    const responses = responseArgs.map(responseArg => {
      responseArg = JSON.parse(JSON.stringify(responseArg));

      const expected = responseValues.find(value => value.name == responseArg.name);
      if (expected) {
        responseArg.hasExpectedValue = true;
        responseArg.expectedValue    = expected.value;
      }

      return responseArg;
    });

    return responses;
  });

  return asBlocks.call(this, promise, options);
}

//
// Module exports
//
exports.chip_tests                          = chip_tests;
exports.chip_tests_items                    = chip_tests_items;
exports.chip_tests_item_parameters          = chip_tests_item_parameters;
exports.chip_tests_item_response_parameters = chip_tests_item_response_parameters;
