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

const basePath          = '../../../../';
const testPath          = 'src/app/tests/suites/';
const certificationPath = 'src/app/tests/suites/certification/';
const zapPath           = basePath + 'third_party/zap/repo/';
const YAML              = require(zapPath + 'node_modules/yaml');
const fs                = require('fs');
const path              = require('path');

// Import helpers from zap core
const templateUtil = require(zapPath + 'dist/src-electron/generator/template-util.js')

const { DelayCommands }                 = require('./simulated-clusters/TestDelayCommands.js');
const { Clusters, asBlocks, asPromise } = require('./ClustersHelper.js');

const kClusterName       = 'cluster';
const kEndpointName      = 'endpoint';
const kCommandName       = 'command';
const kIndexName         = 'index';
const kValuesName        = 'values';
const kConstraintsName   = 'constraints';
const kArgumentsName     = 'arguments';
const kResponseName      = 'response';
const kDisabledName      = 'disabled';
const kResponseErrorName = 'error';

function throwError(test, errorStr)
{
  console.error('Error in: ' + test.filename + '.yaml for test with label: "' + test.label + '"\n');
  console.error(errorStr);
  throw new Error();
}

function setDefault(test, name, defaultValue)
{
  if (!(name in test)) {
    if (defaultValue == null) {
      const errorStr = 'Test does not have any "' + name + '" defined.';
      throwError(test, errorStr);
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
    const errorStr = 'Test does not have a "value" defined.';
    throwError(test, errorStr);
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

  const defaultResponseConstraints = {};
  setDefault(test[kResponseName], kConstraintsName, defaultResponseConstraints);

  const hasResponseValue              = 'value' in test[kResponseName];
  const hasResponseConstraints        = 'constraints' in test[kResponseName] && Object.keys(test[kResponseName].constraints).length;
  const hasResponseValueOrConstraints = hasResponseValue || hasResponseConstraints;

  if (test.isCommand && hasResponseValueOrConstraints) {
    const errorStr = 'Test has a "value" or a "constraints" defined.\n' +
        '\n' +
        'Command should explicitly use the response argument name. Example: \n' +
        '- label: "Send Test Specific Command"\n' +
        '  command: "testSpecific"\n' +
        '  response: \n' +
        '    values: \n' +
        '      - name: "returnValue"\n' +
        '      - value: 7\n';
    throwError(test, errorStr);
  }

  if (test.isWriteAttribute && hasResponseValueOrConstraints) {
    const errorStr = 'Attribute write test has a "value" or a "constraints" defined.';
    throwError(test, errorStr);
  }

  if (!test.isReadAttribute) {
    return;
  }

  if (!hasResponseValueOrConstraints) {
    console.log(test[kResponseName]);
    const errorStr = 'Test does not have a "value" or a "constraints" defined.';
    throwError(test, errorStr);
  }

  if (hasResponseValue) {
    test[kResponseName].values.push({ name : test.attribute, value : test[kResponseName].value });
  }

  if (hasResponseConstraints) {
    test[kResponseName].values.push({ name : test.attribute, constraints : test[kResponseName].constraints });
  }

  delete test[kResponseName].value;
}

function setDefaults(test, defaultConfig)
{
  const defaultClusterName = defaultConfig[kClusterName] || null;
  const defaultEndpointId  = kEndpointName in defaultConfig ? defaultConfig[kEndpointName] : null;
  const defaultDisabled    = false;

  setDefaultType(test);
  setDefault(test, kClusterName, defaultClusterName);
  setDefault(test, kEndpointName, defaultEndpointId);
  setDefault(test, kDisabledName, defaultDisabled);
  setDefaultArguments(test);
  setDefaultResponse(test);
}

function parse(filename)
{
  let filepath;
  const isCertificationTest = filename.startsWith('Test_TC_');
  if (isCertificationTest) {
    filepath = path.resolve(__dirname, basePath + certificationPath + filename + '.yaml');
  } else {
    filepath = path.resolve(__dirname, basePath + testPath + filename + '.yaml');
  }

  const data = fs.readFileSync(filepath, { encoding : 'utf8', flag : 'r' });
  const yaml = YAML.parse(data);

  const defaultConfig = yaml.config || [];
  yaml.tests.forEach(test => {
    test.filename = filename;
    test.testName = yaml.name;
    setDefaults(test, defaultConfig);
  });

  // Filter disabled tests
  yaml.tests = yaml.tests.filter(test => !test.disabled);
  yaml.tests.forEach((test, index) => {
    setDefault(test, kIndexName, index);
  });

  yaml.filename   = filename;
  yaml.totalTests = yaml.tests.length;

  return yaml;
}

function printErrorAndExit(context, msg)
{
  console.log(context.testName, ': ', context.label);
  console.log(msg);
  process.exit(1);
}

function getClusters()
{
  // Create a new array to merge the configured clusters list and test
  // simulated clusters.
  return Clusters.getClusters().then(clusters => clusters.concat(DelayCommands));
}

function getCommands(clusterName)
{
  return (clusterName == DelayCommands.name) ? Promise.resolve(DelayCommands.commands) : Clusters.getClientCommands(clusterName);
}

function getAttributes(clusterName)
{
  return (clusterName == DelayCommands.name) ? Promise.resolve(DelayCommands.attributes)
                                             : Clusters.getServerAttributes(clusterName);
}

function assertCommandOrAttribute(context)
{
  const clusterName = context.cluster;
  return getClusters().then(clusters => {
    if (!clusters.find(cluster => cluster.name == clusterName)) {
      const names = clusters.map(item => item.name);
      printErrorAndExit(context, 'Missing cluster "' + clusterName + '" in: \n\t* ' + names.join('\n\t* '));
    }

    let filterName;
    let items;

    if (context.isCommand) {
      filterName = context.command;
      items      = getCommands(clusterName);
    } else if (context.isAttribute) {
      filterName = context.attribute;
      items      = getAttributes(clusterName);
    } else {
      printErrorAndExit(context, 'Unsupported command type: ', context);
    }

    return items.then(items => {
      const filter = item => item.name.toLowerCase() == filterName.toLowerCase();
      const item          = items.find(filter);
      const itemType      = (context.isCommand ? 'Command' : 'Attribute');

      // If the command or attribute is not found, it could be because of a typo in the test
      // description, or an attribute name not matching the spec, or a wrongly configured zap
      // file.
      if (!item) {
        const names = items.map(item => item.name);
        printErrorAndExit(context, 'Missing ' + itemType + ' "' + filterName + '" in: \n\t* ' + names.join('\n\t* '));
      }

      // If the command or attribute has been found but the response can not be found, it could be
      // because of a wrongly configured cluster definition.
      if (!item.response) {
        printErrorAndExit(context, 'Missing ' + itemType + ' "' + filterName + '" response');
      }

      return item;
    });
  });
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

function isTestOnlyCluster(name)
{
  return name == DelayCommands.name;
}

function chip_tests_item_parameters(options)
{
  const commandValues = this.arguments.values;

  const promise = assertCommandOrAttribute(this).then(item => {
    const commandArgs = item.arguments;
    const commands    = commandArgs.map(commandArg => {
      commandArg = JSON.parse(JSON.stringify(commandArg));

      const expected = commandValues.find(value => value.name.toLowerCase() == commandArg.name.toLowerCase());
      if (!expected) {
        printErrorAndExit(this,
            'Missing "' + commandArg.name + '" in arguments list: \n\t* '
                + commandValues.map(command => command.name).join('\n\t* '));
      }
      commandArg.definedValue = expected.value;

      return commandArg;
    });

    return commands;
  });

  return asBlocks.call(this, promise, options);
}

function chip_tests_item_response_parameters(options)
{
  const responseValues = this.response.values.slice();

  const promise = assertCommandOrAttribute(this).then(item => {
    const responseArgs = item.response.arguments;

    const responses = responseArgs.map(responseArg => {
      responseArg = JSON.parse(JSON.stringify(responseArg));

      const expectedIndex = responseValues.findIndex(value => value.name.toLowerCase() == responseArg.name.toLowerCase());
      if (expectedIndex != -1) {
        const expected = responseValues.splice(expectedIndex, 1)[0];
        if ('value' in expected) {
          responseArg.hasExpectedValue = true;
          responseArg.expectedValue    = expected.value;
        }

        if ('constraints' in expected) {
          responseArg.hasExpectedConstraints = true;
          responseArg.expectedConstraints    = expected.constraints;
        }
      }

      const unusedResponseValues = responseValues.filter(response => 'value' in response);
      unusedResponseValues.forEach(unusedResponseValue => {
        printErrorAndExit(this,
            'Missing "' + unusedResponseValue.name + '" in response arguments list:\n\t* '
                + responseArgs.map(response => response.name).join('\n\t* '));
      });

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
exports.isTestOnlyCluster                   = isTestOnlyCluster;
