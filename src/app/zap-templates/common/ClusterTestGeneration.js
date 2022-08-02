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
const zclHelper    = require(zapPath + 'dist/src-electron/generator/helper-zcl.js');
const queryEnum    = require(zapPath + 'dist/src-electron/db/query-enum');
const queryBitmap  = require(zapPath + 'dist/src-electron/db/query-bitmap');

const { getClusters, getCommands, getAttributes, getEvents, isTestOnlyCluster }
= require('./simulated-clusters/SimulatedClusters.js');
const { asBlocks, ensureClusters } = require('./ClustersHelper.js');
const { Variables }                = require('./variables/Variables.js');

const kIdentityName      = 'identity';
const kClusterName       = 'cluster';
const kEndpointName      = 'endpoint';
const kGroupId           = 'groupId';
const kCommandName       = 'command';
const kWaitCommandName   = 'wait';
const kIndexName         = 'index';
const kValuesName        = 'values';
const kConstraintsName   = 'constraints';
const kArgumentsName     = 'arguments';
const kResponseName      = 'response';
const kDisabledName      = 'disabled';
const kResponseErrorName = 'error';
const kPICSName          = 'PICS';
const kSaveAsName        = 'saveAs';
const kFabricFiltered    = 'fabricFiltered';

const kHexPrefix = 'hex:';

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
    test.commandName = 'WaitEvent';
    test.isEvent     = true;
    test.isReadEvent = true;
    break;
  case 'subscribeEvent':
    test.commandName      = 'WaitEvent';
    test.isEvent          = true;
    test.isSubscribe      = true;
    test.isSubscribeEvent = true;
    break;
  case 'readAttribute':
    test.commandName     = 'WaitAttribute';
    test.isAttribute     = true;
    test.isReadAttribute = true;
    break;
  case 'writeAttribute':
    test.commandName      = 'WaitAttribute';
    test.isAttribute      = true;
    test.isWriteAttribute = true;
    break;
  case 'subscribeAttribute':
    test.commandName          = 'WaitAttribute';
    test.isAttribute          = true;
    test.isSubscribe          = true;
    test.isSubscribeAttribute = true;
    break;
  default:
    test.commandName = 'WaitCommand';
    test.isCommand   = true;
    test.command     = test.wait
    break;
  }

  test.isWait = true;
}

function setDefaultTypeForCommand(test)
{
  const type = test[kCommandName];
  switch (type) {
  case 'readEvent':
    test.commandName = 'ReadEvent';
    test.isEvent     = true;
    test.isReadEvent = true;
    break;

  case 'subscribeEvent':
    test.commandName      = 'SubscribeEvent';
    test.isEvent          = true;
    test.isSubscribe      = true;
    test.isSubscribeEvent = true;
    break;

  case 'readAttribute':
    test.commandName     = 'ReadAttribute';
    test.isAttribute     = true;
    test.isReadAttribute = true;
    if (!(kFabricFiltered in test)) {
      test[kFabricFiltered] = true;
    }
    break;

  case 'writeAttribute':
    test.commandName      = 'WriteAttribute';
    test.isAttribute      = true;
    test.isWriteAttribute = true;
    if ((kGroupId in test)) {
      test.isGroupCommand        = true;
      test.isWriteGroupAttribute = true;
      test.commandName           = 'WriteGroupAttribute';
      test.groupId               = parseInt(test[kGroupId], 10);
    }
    break;

  case 'subscribeAttribute':
    test.commandName          = 'SubscribeAttribute';
    test.isAttribute          = true;
    test.isSubscribe          = true;
    test.isSubscribeAttribute = true;
    if (!(kFabricFiltered in test)) {
      test[kFabricFiltered] = true;
    }
    break;

  case 'waitForReport':
    test.commandName = 'WaitForReport';
    if ('attribute' in test) {
      test.isAttribute = true;
    } else if ('event' in test) {
      test.isEvent = true;
    }
    test.isWaitForReport = true;
    break;

  default:
    test.commandName = isTestOnlyCluster(test.cluster) ? test.command : 'SendCommand';
    test.isCommand   = true;
    if ((kGroupId in test)) {
      test.isGroupCommand = true;
      test.commandName    = 'SendGroupCommand';
      test.groupId        = parseInt(test[kGroupId], 10);
    }
    break;
  }

  // Sanity Check for GroupId usage
  // Only two types of actions can be send to Group : Write attribute, and Commands
  // Spec : Action 8.2.4
  if ((kGroupId in test) && !test.isGroupCommand) {
    printErrorAndExit(this, 'Wrong Yaml configuration. Action : ' + test.commandName + " can't be sent to group " + test[kGroupId]);
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

function setDefaultResponse(test, useSynthesizeWaitForReport)
{
  // Some of the tests does not have any command defined.
  if (!test.command || test.isWait) {
    setDefault(test, kResponseName, []);
    return;
  }

  test.expectMultipleResponses = test.isEvent;

  const defaultResponse = test.expectMultipleResponses ? [] : {};
  setDefault(test, kResponseName, defaultResponse);

  // There is different syntax for expressing the expected response, but in the
  // end it needs to be converted to an array of responses.
  if (kResponseName in test && !Array.isArray(test[kResponseName])) {
    let testValues = {};

    const response = test[kResponseName];

    if (kValuesName in response) {
      testValues[kValuesName] = response[kValuesName];
    } else if ('value' in response || kConstraintsName in response || kSaveAsName in response) {
      let obj = {};
      if ('value' in response) {
        obj['value'] = response['value'];
      }

      if (kConstraintsName in response) {
        obj[kConstraintsName] = response[kConstraintsName];
      }

      if (kSaveAsName in response) {
        obj[kSaveAsName] = response[kSaveAsName];
      }

      testValues[kValuesName] = [ obj ];
    } else {
      testValues[kValuesName] = [];
    }

    if (kResponseErrorName in response) {
      testValues[kResponseErrorName] = response[kResponseErrorName];
    }

    if ('clusterError' in response) {
      testValues['clusterError'] = response['clusterError'];
    }

    test[kResponseName] = [ testValues ];
  }

  // Ensure only valid keywords are used for response values.
  test[kResponseName].forEach(response => {
    const values = response[kValuesName];
    for (let i = 0; i < values.length; i++) {
      for (let key in values[i]) {
        if (key == "name" || key == "value" || key == kConstraintsName || key == kSaveAsName) {
          continue;
        }

        const errorStr = `Unknown key "${key}" in "${JSON.stringify(values)}"`;
        throwError(test, errorStr);
      }
    }
  });

  let responseType = '';
  if (test.isCommand) {
    responseType = 'command';
  } else if (test.isAttribute) {
    responseType = 'attribute';
  } else if (test.isEvent) {
    responseType = 'event';
  } else {
    const errorStr = 'Unknown response type';
    throwError(response, errorStr);
  }

  const defaultName = test[responseType];

  test[kResponseName].forEach(response => {
    const hasResponseError = (kResponseErrorName in response);

    const defaultResponseError = 0;
    setDefault(response, kResponseErrorName, defaultResponseError);
    ensureValidError(response, kResponseErrorName);

    const values = response[kValuesName];
    values.forEach(expectedValue => {
      const hasResponseValue       = 'value' in expectedValue;
      const hasResponseConstraints = (kConstraintsName in expectedValue) && !!Object.keys(expectedValue.constraints).length;
      const hasResponseSaveAs      = (kSaveAsName in expectedValue);

      if (test.isWriteAttribute || (useSynthesizeWaitForReport && test.isSubscribe)) {
        if (hasResponseValue || hasResponseConstraints) {
          const errorStr = 'Test has a "value" or a "constraints" defined.';
          throwError(test, errorStr);
        }
      }

      if (test.isCommand && !('name' in expectedValue)) {
        const errorStr = 'Test value does not have a named argument.\n' +
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

      setDefault(expectedValue, 'name', defaultName);
    });

    setDefault(response, kCommandName, test.command);
    setDefault(response, responseType, test[responseType]);
    setDefault(response, kClusterName, test.cluster);
    setDefault(response, 'optional', test.optional || false);
    setDefault(response, 'async', test.async || false);
    setDefaultType(response);
  });
}

function setDefaults(test, defaultConfig, useSynthesizeWaitForReport)
{
  const defaultIdentityName = kIdentityName in defaultConfig ? defaultConfig[kIdentityName] : "alpha";
  const defaultClusterName  = defaultConfig[kClusterName] || null;
  const defaultEndpointId   = kEndpointName in defaultConfig ? defaultConfig[kEndpointName] : null;
  const defaultDisabled     = false;

  setDefault(test, kIdentityName, defaultIdentityName);
  setDefault(test, kClusterName, defaultClusterName);
  setDefault(test, kEndpointName, defaultEndpointId);
  setDefault(test, kDisabledName, defaultDisabled);
  setDefaultType(test);
  setDefaultPICS(test);
  setDefaultArguments(test);
  setDefaultResponse(test, useSynthesizeWaitForReport);
}

function parse(filename, useSynthesizeWaitForReport)
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

  if (useSynthesizeWaitForReport) {
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
  }

  const defaultConfig = yaml.config || [];
  yaml.tests.forEach(test => {
    test.filename = filename;
    test.testName = yaml.name;
    setDefaults(test, defaultConfig, useSynthesizeWaitForReport);
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
  console.log("\nERROR:\n", context.testName, ': ', context.label);
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

async function configureTestItem(item)
{
  if (item.isCommand) {
    let command               = await assertCommandOrAttributeOrEvent(item);
    item.commandObject        = command;
    item.hasSpecificArguments = true;
    item.hasSpecificResponse  = command.hasSpecificResponse || false;
  } else if (item.isAttribute) {
    let attr                  = await assertCommandOrAttributeOrEvent(item);
    item.attributeObject      = attr;
    item.hasSpecificArguments = item.isWriteAttribute || false;
    item.hasSpecificResponse  = item.isReadAttribute || item.isSubscribeAttribute || item.isWaitForReport || false;
  } else if (item.isEvent) {
    let evt                   = await assertCommandOrAttributeOrEvent(item);
    item.eventObject          = evt;
    item.hasSpecificArguments = false;
    item.hasSpecificResponse  = true;
  }
}

async function chip_tests(list, options)
{
  // Set a global on our items so assertCommandOrAttributeOrEvent can work.
  let global  = this.global;
  const items = Array.isArray(list) ? list : list.split(',');
  const names = items.map(name => name.trim());
  let tests   = names.map(item => parse(item, options.hash.useSynthesizeWaitForReport));

  const context = this;
  tests         = await Promise.all(tests.map(async function(test) {
    test.tests = await Promise.all(test.tests.map(async function(item) {
      item.global = global;
      await configureTestItem(item);

      if (kResponseName in item) {
        await Promise.all(item[kResponseName].map(response => configureTestItem(response)));
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

function getVariable(context, key, name)
{
  if (!(typeof name == "string" || (typeof name == "object" && (name instanceof String)))) {
    // Non-string key; don't try to look it up.  Could end up looking like a
    // variable name by accident when stringified.
    return null;
  }

  while (!('variables' in context) && context.parent) {
    context = context.parent;
  }

  if (typeof context === 'undefined' || !('variables' in context)) {
    return null;
  }

  return context.variables[key].find(variable => variable.name == name);
}

function getVariableOrThrow(context, key, name)
{
  const variable = getVariable(context, key, name);
  if (variable == null) {
    throw new Error(`Variable ${name} can not be found`);
  }
  return variable;
}

function chip_tests_variables(options)
{
  return templateUtil.collectBlocks(this.variables.tests, options, this);
}

function chip_tests_variables_has(name, options)
{
  const variable = getVariable(this, 'tests', name);
  return !!variable;
}

function chip_tests_variables_get_type(name, options)
{
  const variable = getVariableOrThrow(this, 'tests', name);
  return variable.type;
}

function chip_tests_variables_is_nullable(name, options)
{
  const variable = getVariableOrThrow(this, 'tests', name);
  return variable.isNullable;
}

function chip_tests_config(options)
{
  return templateUtil.collectBlocks(this.variables.config, options, this);
}

function chip_tests_config_has(name, options)
{
  const variable = getVariable(this, 'config', name);
  return !!variable;
}

function chip_tests_config_get_default_value(name, options)
{
  const variable = getVariableOrThrow(this, 'config', name);
  return variable.defaultValue;
}

function chip_tests_config_get_type(name, options)
{
  const variable = getVariableOrThrow(this, 'config', name);
  return variable.type;
}

// test_cluster_command_value and test_cluster_value-equals are recursive partials using #each. At some point the |global|
// context is lost and it fails. Make sure to attach the global context as a property of the | value |
// that is evaluated.
//
// errorContext should have "thisVal" and "name" properties that will be used
// for error reporting via printErrorAndExit.
function attachGlobal(global, value, errorContext)
{
  if (Array.isArray(value)) {
    value = value.map(v => attachGlobal(global, v, errorContext));
  } else if (value instanceof Object) {
    for (key in value) {
      if (key == "global") {
        continue;
      }
      value[key] = attachGlobal(global, value[key], errorContext);
    }
  } else if (value === null) {
    value = new NullObject();
  } else {
    switch (typeof value) {
    case 'number':
      checkNumberSanity(value, errorContext);
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

/**
 * Ensure the given value is not a possibly-corrupted-by-going-through-double
 * integer.  If it is, tell the user (using that errorContext.name to describe
 * it) and die.
 */
function checkNumberSanity(value, errorContext)
{
  // Number.isInteger is false for non-Numbers.
  if (Number.isInteger(value) && !Number.isSafeInteger(value)) {
    printErrorAndExit(errorContext.thisVal,
        `${errorContext.name} value ${
            value} is too large to represent exactly as an integer in YAML.  Put quotes around it to treat it as a string.\n\n`);
  }
}

function chip_tests_item_parameters(options)
{
  if (this.isWait) {
    return asBlocks.call(this, Promise.resolve([]), options);
  }

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

      commandArg.definedValue = attachGlobal(this.global, expected.value, { thisVal : this, name : commandArg.name });

      return commandArg;
    });

    return commands.filter(item => item !== undefined);
  });

  return asBlocks.call(this, promise, options);
}

function chip_tests_item_responses(options)
{
  return templateUtil.collectBlocks(this[kResponseName], options, this);
}

function chip_tests_item_response_parameters(options)
{
  const responseValues = this.values.slice();

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
          responseArg.expectedValue    = attachGlobal(this.global, expected.value, { thisVal : this, name : responseArg.name });
        }

        if ('constraints' in expected) {
          responseArg.hasExpectedConstraints = true;
          responseArg.expectedConstraints
              = attachGlobal(this.global, expected.constraints, { thisVal : this, name : responseArg.name });
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

function isHexString(value)
{
  return value && value.startsWith(kHexPrefix);
}

function octetStringFromHexString(value)
{
  const hexString = value.substring(kHexPrefix.length);

  if (hexString.length % 2) {
    throw new Error("The provided hexadecimal string contains an even number of characters");
  }

  if (!(/^[0-9a-fA-F]+$/.test(hexString))) {
    throw new Error("The provided hexadecimal string contains invalid hexadecimal character.");
  }

  const bytes = hexString.match(/(..)/g);
  return bytes.map(byte => '\\x' + byte).join('');
}

function octetStringLengthFromHexString(value)
{
  const hexString = value.substring(kHexPrefix.length);
  return (hexString.length / 2);
}

function octetStringEscapedForCLiteral(value)
{
  // Escape control characters, things outside the ASCII range, and single
  // quotes (because that's our string terminator).
  return value.replace(/\p{Control}|\P{ASCII}|"/gu, ch => {
    var code = ch.charCodeAt(0).toString(8)
    return "\\" +
        "0".repeat(3 - code.length) + code;
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
    throw new Error(`Value not provided for ${name} where one is expected in ` + JSON.stringify(structValue));
  }

  return options.inverse(this);
}

// To be used to verify that things are actually arrays before trying to use
// #each with them, since that silently treats non-arrays as empty arrays.
function ensureIsArray(value, options)
{
  if (!(value instanceof Array)) {
    printErrorAndExit(this, `Expected array but instead got ${typeof value}: ${JSON.stringify(value)}\n`);
  }
}

function checkIsInsideTestOnlyClusterBlock(conditions, name)
{
  conditions.forEach(condition => {
    if (condition == undefined) {
      const errorStr = `Not inside a ({#${name}}} block.`;
      console.error(errorStr);
      throw new Error(errorStr);
    }
  });
}

/**
 * Creates block iterator over the simulated clusters.
 *
 * @param {*} options
 */
async function chip_tests_only_clusters(options)
{
  const clusters         = await getClusters(this);
  const testOnlyClusters = clusters.filter(cluster => isTestOnlyCluster(cluster.name));
  return asBlocks.call(this, Promise.resolve(testOnlyClusters), options);
}

/**
 * Creates block iterator over the cluster commands for a given simulated cluster.
 *
 * This function is meant to be used inside a {{#chip_tests_only_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
async function chip_tests_only_cluster_commands(options)
{
  const conditions = [ isTestOnlyCluster(this.name) ];
  checkIsInsideTestOnlyClusterBlock(conditions, 'chip_tests_only_clusters');

  const commands = await getCommands(this, this.name);
  return asBlocks.call(this, Promise.resolve(commands), options);
}

/**
 * Creates block iterator over the command arguments for a given simulated cluster command.
 *
 * This function is meant to be used inside a {{#chip_tests_only_cluster_commands}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
async function chip_tests_only_cluster_command_parameters(options)
{
  const conditions = [ isTestOnlyCluster(this.parent.name), this.arguments, this.response ];
  checkIsInsideTestOnlyClusterBlock(conditions, 'chip_tests_only_cluster_commands');

  return asBlocks.call(this, Promise.resolve(this.arguments), options);
}

/**
 * Creates block iterator over the cluster responses for a given simulated cluster.
 *
 * This function is meant to be used inside a {{#chip_tests_only_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
async function chip_tests_only_cluster_responses(options)
{
  const conditions = [ isTestOnlyCluster(this.name) ];
  checkIsInsideTestOnlyClusterBlock(conditions, 'chip_tests_only_clusters');

  const commands  = await getCommands(this, this.name);
  const responses = [];
  commands.forEach(command => {
    if (!command.response.arguments) {
      return;
    }

    if (!('responseName' in command)) {
      return;
    }

    const alreadyExists = responses.some(item => item.responseName == command.responseName);
    if (alreadyExists) {
      return;
    }

    command.response.responseName = command.responseName;
    responses.push(command.response);
  });

  return asBlocks.call(this, Promise.resolve(responses), options);
}

/**
 * Creates block iterator over the response arguments for a given simulated cluster response.
 *
 * This function is meant to be used inside a {{#chip_tests_only_cluster_responses}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
async function chip_tests_only_cluster_response_parameters(options)
{
  const conditions = [ isTestOnlyCluster(this.parent.name), this.arguments, this.responseName ];
  checkIsInsideTestOnlyClusterBlock(conditions, 'chip_tests_only_cluster_responses');

  return asBlocks.call(this, Promise.resolve(this.arguments), options);
}

function chip_tests_iterate_expected_list(values, options)
{
  let context = options.hash.context || this;
  values      = values.map(value => {
    return {
      global: context.global, parent: context.parent, name: context.name, type: context.type, isArray: false, isNullable: false,
          value: value,
    }
  });

  return asBlocks.call(this, Promise.resolve(values), options);
}

function chip_tests_iterate_constraints(constraints, options)
{
  let values = [];
  for (let key of Object.keys(constraints)) {
    // Skip "global", because that's not an actual constraint.
    if (key == "global") {
      continue;
    }
    values.push({ global : this.global, constraint : key, value : constraints[key] })
  }

  return asBlocks.call(this, Promise.resolve(values), options)
}

async function asTestType(type, isList)
{
  if (isList) {
    return 'list';
  }

  const pkgId = await templateUtil.ensureZclPackageId(this);
  const db    = this.global.db;

  const isEnum = await zclHelper.isEnum(db, type, pkgId);
  if (isEnum != 'unknown') {
    const enumObj = await queryEnum.selectEnumByName(db, type, pkgId);
    return 'enum' + (8 * enumObj.size);
  }

  const isBitmap = await zclHelper.isBitmap(db, type, pkgId);
  if (isBitmap != 'unknown') {
    const bitmapObj = await queryBitmap.selectBitmapByName(db, pkgId, type);
    return 'bitmap' + (8 * bitmapObj.size);
  }

  return type;
}

//
// Module exports
//
exports.chip_tests                                  = chip_tests;
exports.chip_tests_items                            = chip_tests_items;
exports.chip_tests_item_parameters                  = chip_tests_item_parameters;
exports.chip_tests_item_responses                   = chip_tests_item_responses;
exports.chip_tests_item_response_parameters         = chip_tests_item_response_parameters;
exports.chip_tests_pics                             = chip_tests_pics;
exports.chip_tests_config                           = chip_tests_config;
exports.chip_tests_config_has                       = chip_tests_config_has;
exports.chip_tests_config_get_default_value         = chip_tests_config_get_default_value;
exports.chip_tests_config_get_type                  = chip_tests_config_get_type;
exports.chip_tests_variables                        = chip_tests_variables;
exports.chip_tests_variables_has                    = chip_tests_variables_has;
exports.chip_tests_variables_get_type               = chip_tests_variables_get_type;
exports.chip_tests_variables_is_nullable            = chip_tests_variables_is_nullable;
exports.isTestOnlyCluster                           = isTestOnlyCluster;
exports.isLiteralNull                               = isLiteralNull;
exports.octetStringEscapedForCLiteral               = octetStringEscapedForCLiteral;
exports.if_include_struct_item_value                = if_include_struct_item_value;
exports.ensureIsArray                               = ensureIsArray;
exports.chip_tests_only_clusters                    = chip_tests_only_clusters;
exports.chip_tests_only_cluster_commands            = chip_tests_only_cluster_commands;
exports.chip_tests_only_cluster_command_parameters  = chip_tests_only_cluster_command_parameters;
exports.chip_tests_only_cluster_responses           = chip_tests_only_cluster_responses;
exports.chip_tests_only_cluster_response_parameters = chip_tests_only_cluster_response_parameters;
exports.isHexString                                 = isHexString;
exports.octetStringLengthFromHexString              = octetStringLengthFromHexString;
exports.octetStringFromHexString                    = octetStringFromHexString;
exports.chip_tests_iterate_expected_list            = chip_tests_iterate_expected_list;
exports.chip_tests_iterate_constraints              = chip_tests_iterate_constraints;
exports.asTestType                                  = asTestType;
