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

function getManualTests()
{
  const ExampleManualCataCategory = [
    'examples/Test_Example',
  ];

  const tests = [
    ExampleManualCataCategory, //

  ];
  return tests.flat(1);
}

function getTests()
{
  const TestExample = [
    'examples/Test_Example_1',
    'examples/Test_Example_2',
    'examples/Test_Example_3',
  ];

  const tests = [
    TestExample, //
  ];
  return tests.flat(1);
}

//
// Module exports
//
exports.getTests       = getTests;
exports.getManualTests = getManualTests;
