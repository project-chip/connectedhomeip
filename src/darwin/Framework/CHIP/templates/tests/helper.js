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

/*
 * In order to run the tests into an expected order, an index is
 * added to the generated test step. This is because tests using
 * the XCTest framework are expected to be independent and should
 * normally run as independent steps. But since the current code is
 * dealing with a simulated accessory running into its own process
 * for the whole duration of the tests, its state is not resetted
 * between each steps which defeats XCTest expectation.
 */
function asTestIndex(index)
{
  return index.toString().padStart(6, 0);
}

//
// Module exports
//
exports.asTestIndex = asTestIndex;
