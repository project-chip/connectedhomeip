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

const testPath  = '../../../../src/app/tests/suites/';
const TestSuite = require(testPath + 'tests.js');

function getManualTests()
{
  return [];
}

// clang-format off

function getTests() {

  let tests = TestSuite.getTests();

  // TODO: This test needs FindCommissionable
  tests.disable('Test_TC_SC_4_2');

  // TestClusterComplexTypes requires representing nullable optionals in ways
  // that can differentiate missing and null, which Darwin can't right now.
  tests.disable('TestClusterComplexTypes');

  // TODO: TestEvents not supported in the codegen yet.
  tests.disable('TestEvents');

  // TODO: TestDiscovery needs FindCommissionable
  tests.disable('TestDiscovery');

  // TODO: TestGroupMessaging does not work on Darwin for now.
  tests.disable('TestGroupMessaging');

  // TODO: Test_TC_DIAG_TH_NW_2_1 does not work on Darwin for now.
  tests.disable('Test_TC_DGTHREAD_2_1');

  // TODO: Test_TC_DIAG_TH_NW_2_2 does not work on Darwin for now.
  tests.disable('Test_TC_DGTHREAD_2_2');

  // TODO: Test_TC_DIAG_TH_NW_2_3 does not work on Darwin for now.
  tests.disable('Test_TC_DGTHREAD_2_3');

  // TODO: Test_TC_DIAG_TH_NW_2_4 does not work on Darwin for now.
  tests.disable('Test_TC_DGTHREAD_2_4');

  // TODO: Test_TC_CC_7_2 seems to rely on pretty tight timing that seem to not
  // work right in the darwin tests.
  tests.disable('Test_TC_CC_7_2');

  // TODO: Test_TC_CC_9_1 does not work on Darwin for now.
  // But is disabled in CI, so we can't disable it here.
  //tests.disable('Test_TC_CC_9_1');

  // TODO: Test_TC_CC_9_2 does not work on Darwin for now.
  // But is disabled in CI, so we can't disable it here.
  //tests.disable('Test_TC_CC_9_2');

  // TODO: Test_TC_CC_9_3 does not work on Darwin for now.
  // But is disabled in CI, so we can't disable it here.
  //tests.disable('Test_TC_CC_9_3');

  // TODO: Test_TC_MC_3_7 does not work on Darwin for now.
  tests.disable('Test_TC_APPLAUNCHER_3_7');

  // TODO: Test_TC_MC_3_8 does not work on Darwin for now.
  tests.disable('Test_TC_APPLAUNCHER_3_8');

  // TODO: Test_TC_MC_3_9 does not work on Darwin for now.
  tests.disable('Test_TC_APPLAUNCHER_3_9');

  // TODO: Test_TC_BINFO_2_1 does not work on Darwin for now.
  tests.disable('Test_TC_BINFO_2_1');

  // TODO: Test_TC_SWTCH_2_1 does not work on Darwin for now.
  tests.disable('Test_TC_SWTCH_2_1');

  return tests;
}

//
// Module exports
//
exports.getTests       = getTests;
exports.getManualTests = getManualTests;
