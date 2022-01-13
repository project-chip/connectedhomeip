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

const { getClusters, getCommands, getAttributes, getEvents, isTestOnlyCluster }
= require('./simulated-clusters/SimulatedClusters.js');
const { asBlocks, ensureClusters } = require('./ClustersHelper.js');
const { Variables }                = require('./variables/Variables.js');

const kIdentityName           = 'identity';
const kClusterName            = 'cluster';
const kEndpointName           = 'endpoint';
const kGroupId                = 'groupId';
const kCommandName            = 'command';
const kWaitCommandName        = 'wait';
const kIndexName              = 'index';
const kValuesName             = 'values';
const kConstraintsName        = 'constraints';
const kArgumentsName          = 'arguments';
const kResponseName           = 'response';
const kDisabledName           = 'disabled';
const kResponseErrorName      = 'error';
const kResponseWrongErrorName = 'errorWrongValue';
const kPICSName               = 'PICS';
const kSaveAsName             = 'saveAs';

class NullObject {
  toString()
  {
    return "YOU SHOULD HAVE CHECKED (isLiteralNull definedValue)"
  }
};

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
  if (kWaitCommandName in test) {
    setDefaultTypeForWaitCommand(test);
  } else {
    setDefaultTypeForCommand(test);
  }
}

function setDefaultTypeForWaitCommand(test)
{
  const type = test[kWaitCommandName];
  switch (type) {
  case 'readEvent':
    test.isEvent     = true;
    test.isReadEvent = true;
    break;
  case 'subscribeEvent':
    test.isEvent          = true;
    test.isSubscribe      = true;
    test.isSubscribeEvent = true;
    break;
  case 'readAttribute':
    test.isAttribute     = true;
    test.isReadAttribute = true;
    break;
  case 'writeAttribute':
    test.isAttribute      = true;
    test.isWriteAttribute = true;
    break;
  case 'subscribeAttribute':
    test.isAttribute          = true;
    test.isSubscribe          = true;
    test.isSubscribeAttribute = true;
    break;
  default:
    test.isCommand = true;
    test.command   = test.wait
    break;
  }

  test.isWait = true;
}

function setDefaultTypeForCommand(test)
{
  const type = test[kCommandName];
  switch (type) {
  case 'readEvent':
    test.commandName = 'Read';
    test.isEvent     = true;
    test.isReadEvent = true;
    break;

  case 'subscribeEvent':
    test.commandName      = 'Subscribe';
    test.isEvent          = true;
    test.isSubscribe      = true;
    test.isSubscribeEvent = true;
    break;

  case 'readAttribute':
    test.commandName     = 'Read';
    test.isAttribute     = true;
    test.isReadAttribute = true;
    break;

  case 'writeAttribute':
    test.commandName      = 'Write';
    test.isAttribute      = true;
    test.isWriteAttribute = true;
    if ((kGroupId in test)) {
      test.isGroupCommand = true;
      test.groupId        = parseInt(test[kGroupId], 10);
    }
    break;

  case 'subscribeAttribute':
    test.commandName          = 'Subscribe';
    test.isAttribute          = true;
    test.isSubscribe          = true;
    test.isSubscribeAttribute = true;
    break;

  case 'waitForReport':
    test.commandName     = 'Report';
    test.isAttribute     = true;
    test.isWaitForReport = true;
    break;

  default:
    test.commandName = test.command;
    test.isCommand   = true;
    if ((kGroupId in test)) {
      test.isGroupCommand = true;
      test.groupId        = parseInt(test[kGroupId], 10);
    }
    break;
  }

  test.isWait = false;
}

function setDefaultPICS(test)
{
  const defaultPICS = '';
  setDefault(test, kPICSName, defaultPICS);

  if (test[kPICSName] == '') {
    return;
  }

  const items = test[kPICSName].split(/[&|() !]+/g).filter(item => item.length);
  items.forEach(key => {
    if (!PICS.has(key)) {
      const errorStr = 'PICS database does not contains any defined value for: ' + key;
      throwError(test, errorStr);
    }
  })
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

function ensureValidError(response, errorName)
{
  if (isNaN(response[errorName])) {
    response[errorName] = "EMBER_ZCL_STATUS_" + response[errorName];
  }
}

function setDefaultResponse(test)
{
  const defaultResponse = {};
  setDefault(test, kResponseName, defaultResponse);

  const hasResponseError = (kResponseErrorName in test[kResponseName]) || (kResponseWrongErrorName in test[kResponseName]);

  const defaultResponseError = 0;
  setDefault(test[kResponseName], kResponseErrorName, defaultResponseError);
  setDefault(test[kResponseName], kResponseWrongErrorName, defaultResponseError);

  const defaultResponseValues = [];
  setDefault(test[kResponseName], kValuesName, defaultResponseValues);

  const defaultResponseConstraints = {};
  setDefault(test[kResponseName], kConstraintsName, defaultResponseConstraints);

  const defaultResponseSaveAs = '';
  setDefault(test[kResponseName], kSaveAsName, defaultResponseSaveAs);

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

  ensureValidError(test[kResponseName], kResponseErrorName);
  ensureValidError(test[kResponseName], kResponseWrongErrorName);

  // Step that waits for a particular event does not requires constraints nor expected values.
  if (test.isWait) {
    return;
  }

  if (!test.isAttribute && !test.isEvent) {
    return;
  }

  if (test.isWriteAttribute || test.isSubscribe) {
    if (hasResponseValueOrConstraints) {
      const errorStr = 'Test has a "value" or a "constraints" defined.';
      throwError(test, errorStr);
    }

    return;
  }

  if (!hasResponseValueOrConstraints && !hasResponseError) {
    console.log(test);
    console.log(test[kResponseName]);
    const errorStr = 'Test does not have a "value" or a "constraints" defined and is not expecting an error.';
    throwError(test, errorStr);
  }

  const name = test.isAttribute ? test.attribute : test.event;
  if (hasResponseValue) {
    test[kResponseName].values.push({ name : name, value : test[kResponseName].value, saveAs : test[kResponseName].saveAs });
  }

  if (hasResponseConstraints) {
    test[kResponseName].values.push(
        { name : name, constraints : test[kResponseName].constraints, saveAs : test[kResponseName].saveAs });
  }

  delete test[kResponseName].value;
}

function setDefaults(test, defaultConfig)
{
  const defaultIdentityName = kIdentityName in defaultConfig ? defaultConfig[kIdentityName] : "alpha";
  const defaultClusterName  = defaultConfig[kClusterName] || null;
  const defaultEndpointId   = kEndpointName in defaultConfig ? defaultConfig[kEndpointName] : null;
  const defaultDisabled     = false;

  setDefaultType(test);
  setDefault(test, kIdentityName, defaultIdentityName);
  setDefault(test, kClusterName, defaultClusterName);
  setDefault(test, kEndpointName, defaultEndpointId);
  setDefault(test, kDisabledName, defaultDisabled);
  setDefaultPICS(test);
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

  // "subscribeAttribute" command expects a report to be acked before
  // it got a success response.
  // In order to validate that the report has been received with the proper value
  // a "subscribeAttribute" command can have a response configured into the test step
  // definition. In this case, a new async "waitForReport" test step will be synthesized
  // and added to the list of tests.
  yaml.tests.forEach((test, index) => {
    if (test.command == "subscribeAttribute" && test.response) {
      // Create a new report test where the expected response is the response argument
      // for the "subscribeAttributeTest"
      const reportTest = {
        label : "Report: " + test.label,
        command : "waitForReport",
        attribute : test.attribute,
        response : test.response,
        async : true,
        allocateSubscribeDataCallback : true,
      };
      delete test.response;

      // insert the new report test into the tests list
      yaml.tests.splice(index, 0, reportTest);

      // Associate the "subscribeAttribute" test with the synthesized report test
      test.hasWaitForReport              = true;
      test.waitForReport                 = reportTest;
      test.allocateSubscribeDataCallback = !test.hasWaitForReport;
    }
  });

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
  yaml.timeout    = yaml.config.timeout;
  yaml.totalTests = yaml.tests.length;

  return yaml;
}

function printErrorAndExit(context, msg)
{
  console.log(context.testName, ': ', context.label);
  console.log(msg);
  process.exit(1);
}

function assertCommandOrAttributeOrEvent(context)
{
  const clusterName = context.cluster;
  return getClusters(context).then(clusters => {
    if (!clusters.find(cluster => cluster.name == clusterName)) {
      const names = clusters.map(item => item.name);
      printErrorAndExit(context, 'Missing cluster "' + clusterName + '" in: \n\t* ' + names.join('\n\t* '));
    }

    let filterName;
    let items;

    if (context.isCommand) {
      filterName = context.command;
      items      = getCommands(context, clusterName);
    } else if (context.isAttribute) {
      filterName = context.attribute;
      items      = getAttributes(context, clusterName);
    } else if (context.isEvent) {
      filterName = context.event;
      items      = getEvents(context, clusterName);
    } else {
      printErrorAndExit(context, 'Unsupported command type: ', context);
    }

    return items.then(items => {
      const filter = item => item.name.toLowerCase() == filterName.toLowerCase();
      const item          = items.find(filter);
      const itemType      = (context.isCommand ? 'Command' : context.isAttribute ? 'Attribute' : 'Event');

      // If the command/attribute/event is not found, it could be because of a typo in the test
      // description, or an attribute/event name not matching the spec, or a wrongly configured zap
      // file.
      if (!item) {
        const names = items.map(item => item.name);
        printErrorAndExit(context, 'Missing ' + itemType + ' "' + filterName + '" in: \n\t* ' + names.join('\n\t* '));
      }

      // If the command/attribute/event has been found but the response can not be found, it could be
      // because of a wrongly configured cluster definition.
      if (!item.response) {
        printErrorAndExit(context, 'Missing ' + itemType + ' "' + filterName + '" response');
      }

      return item;
    });
  });
}

const PICS = (() => {
  let filepath = path.resolve(__dirname, basePath + certificationPath + 'PICS.yaml');
  const data   = fs.readFileSync(filepath, { encoding : 'utf8', flag : 'r' });
  const yaml   = YAML.parse(data);

  const getAll = () => yaml.PICS;
  const get = (id) => has(id) ? yaml.PICS.filter(pics => pics.id == id)[0] : null;
  const has = (id) => !!(yaml.PICS.filter(pics => pics.id == id)).length;

  const PICS = {
    getAll : getAll,
    get : get,
    has : has,
  };
  return PICS;
})();

//
// Templates
//
function chip_tests_pics(options)
{
  return templateUtil.collectBlocks(PICS.getAll(), options, this);
}

async function chip_tests(list, options)
{
  // Set a global on our items so assertCommandOrAttributeOrEvent can work.
  let global  = this.global;
  const items = Array.isArray(list) ? list : list.split(',');
  const names = items.map(name => name.trim());
  let tests   = names.map(item => parse(item));

  const context = this;
  tests         = await Promise.all(tests.map(async function(test) {
    test.tests = await Promise.all(test.tests.map(async function(item) {
      item.global = global;
      if (item.isCommand) {
        let command        = await assertCommandOrAttributeOrEvent(item);
        item.commandObject = command;
      } else if (item.isAttribute) {
        let attr             = await assertCommandOrAttributeOrEvent(item);
        item.attributeObject = attr;
      } else if (item.isEvent) {
        let evt          = await assertCommandOrAttributeOrEvent(item);
        item.eventObject = evt;
      }
      return item;
    }));

    const variables = await Variables(context, test);
    test.variables  = {
      config : variables.config,
      tests : variables.tests,
    };
    return test;
  }));
  return templateUtil.collectBlocks(tests, options, this);
}

function chip_tests_items(options)
{
  return templateUtil.collectBlocks(this.tests, options, this);
}

function chip_tests_config(options)
{
  return templateUtil.collectBlocks(this.variables.config, options, this);
}

function getConfigVariable(context, name)
{
  while (!('variables' in context) && context.parent) {
    context = context.parent;
  }

  if (typeof context === 'undefined' || !('variables' in context)) {
    return null;
  }

  return context.variables.config.find(variable => variable.name == name);
}

function getConfigVariableOrThrow(context, name)
{
  const variable = getConfigVariable(context, name);
  if (variable == null) {
    throw new Error(`Variable ${name} can not be found`);
  }
  return variable;
}

function chip_tests_config_has(name, options)
{
  const variable = getConfigVariable(this, name);
  return !!variable;
}

function chip_tests_config_get_default_value(name, options)
{
  const variable = getConfigVariableOrThrow(this, name);
  return variable.defaultValue;
}

function chip_tests_config_get_type(name, options)
{
  const variable = getConfigVariableOrThrow(this, name);
  return variable.type;
}

// test_cluster_command_value and test_cluster_value-equals are recursive partials using #each. At some point the |global|
// context is lost and it fails. Make sure to attach the global context as a property of the | value |
// that is evaluated.
function attachGlobal(global, value)
{
  if (Array.isArray(value)) {
    value = value.map(v => attachGlobal(global, v));
  } else if (value instanceof Object) {
    for (key in value) {
      if (key == "global") {
        continue;
      }
      value[key] = attachGlobal(global, value[key]);
    }
  } else if (value === null) {
    value = new NullObject();
  } else {
    switch (typeof value) {
    case 'number':
      value = new Number(value);
      break;
    case 'string':
      value = new String(value);
      break;
    case 'boolean':
      value = new Boolean(value);
      break;
    default:
      throw new Error('Unsupported value: ' + JSON.stringify(value));
    }
  }

  value.global = global;
  return value;
}

function chip_tests_item_parameters(options)
{
  const commandValues = this.arguments.values;

  const promise = assertCommandOrAttributeOrEvent(this).then(item => {
    if ((this.isAttribute || this.isEvent) && !this.isWriteAttribute) {
      if (this.isSubscribe) {
        const minInterval = { name : 'minInterval', type : 'int16u', chipType : 'uint16_t', definedValue : this.minInterval };
        const maxInterval = { name : 'maxInterval', type : 'int16u', chipType : 'uint16_t', definedValue : this.maxInterval };
        return [ minInterval, maxInterval ];
      }
      return [];
    }

    const commandArgs = item.arguments;
    const commands    = commandArgs.map(commandArg => {
      commandArg = JSON.parse(JSON.stringify(commandArg));

      const expected = commandValues.find(value => value.name.toLowerCase() == commandArg.name.toLowerCase());
      if (!expected) {
        if (commandArg.isOptional) {
          return undefined;
        }
        printErrorAndExit(this,
            'Missing "' + commandArg.name + '" in arguments list: \n\t* '
                + commandValues.map(command => command.name).join('\n\t* '));
      }
      commandArg.definedValue = attachGlobal(this.global, expected.value);

      return commandArg;
    });

    return commands.filter(item => item !== undefined);
  });

  return asBlocks.call(this, promise, options);
}

function chip_tests_item_response_parameters(options)
{
  const responseValues = this.response.values.slice();

  const promise = assertCommandOrAttributeOrEvent(this).then(item => {
    if (this.isWriteAttribute) {
      return [];
    }
    const responseArgs = item.response.arguments;

    const responses = responseArgs.map(responseArg => {
      responseArg = JSON.parse(JSON.stringify(responseArg));

      const expectedIndex = responseValues.findIndex(value => value.name.toLowerCase() == responseArg.name.toLowerCase());
      if (expectedIndex != -1) {
        const expected = responseValues.splice(expectedIndex, 1)[0];
        if ('value' in expected) {
          responseArg.hasExpectedValue = true;
          responseArg.expectedValue    = attachGlobal(this.global, expected.value);
        }

        if ('constraints' in expected) {
          responseArg.hasExpectedConstraints = true;
          responseArg.expectedConstraints    = expected.constraints;
        }

        if ('saveAs' in expected) {
          responseArg.saveAs = expected.saveAs;
        }
      }

      return responseArg;
    });

    const unusedResponseValues = responseValues.filter(response => 'value' in response);
    unusedResponseValues.forEach(unusedResponseValue => {
      printErrorAndExit(this,
          'Missing "' + unusedResponseValue.name + '" in response arguments list:\n\t* '
              + responseArgs.map(response => response.name).join('\n\t* '));
    });

    return responses;
  });

  return asBlocks.call(this, promise, options);
}

function isLiteralNull(value, options)
{
  // Literal null might look different depending on whether it went through
  // attachGlobal or not.
  return (value === null) || (value instanceof NullObject);
}

function octetStringEscapedForCLiteral(value)
{
  // Escape control characters, things outside the ASCII range, and single
  // quotes (because that's our string terminator).
  return value.replace(/\p{Control}|\P{ASCII}|"/gu, ch => {
    let code = ch.charCodeAt(0);
    code     = code.toString(16);
    if (code.length == 1) {
      code = "0" + code;
    }
    return "\\x" + code;
  });
}

// Structs may not always provide values for optional members.
function if_include_struct_item_value(structValue, name, options)
{
  let hasValue = (name in structValue);
  if (hasValue) {
    return options.fn(this);
  }

  if (!this.isOptional) {
    throw new Error(`Value not provided for ${name} where one is expected`);
  }

  return options.inverse(this);
}

//
// Module exports
//
exports.chip_tests                          = chip_tests;
exports.chip_tests_items                    = chip_tests_items;
exports.chip_tests_item_parameters          = chip_tests_item_parameters;
exports.chip_tests_item_response_parameters = chip_tests_item_response_parameters;
exports.chip_tests_pics                     = chip_tests_pics;
exports.chip_tests_config                   = chip_tests_config;
exports.chip_tests_config_has               = chip_tests_config_has;
exports.chip_tests_config_get_default_value = chip_tests_config_get_default_value;
exports.chip_tests_config_get_type          = chip_tests_config_get_type;
exports.isTestOnlyCluster                   = isTestOnlyCluster;
exports.isLiteralNull                       = isLiteralNull;
exports.octetStringEscapedForCLiteral       = octetStringEscapedForCLiteral;
exports.if_include_struct_item_value        = if_include_struct_item_value;
