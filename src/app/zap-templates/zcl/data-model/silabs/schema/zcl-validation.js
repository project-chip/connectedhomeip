/**
 *
 *    Copyright (c) 2020 Silicon Labs
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

const libxmljs = require('libxmljs')

// validateZclFile will have validationSchema be bound to it.
function validateZclFile(validationSchemaBuffer, zclFileBuffer) {
  let zclFile = libxmljs.parseXml(zclFileBuffer.toString())
  let xsdDoc = libxmljs.parseXml(validationSchemaBuffer.toString())
  let validationStatus = zclFile.validate(xsdDoc)
  let validationErrors = zclFile.validationErrors
  let returnValue = { isValid: validationStatus, errors: validationErrors }
  return returnValue
}

exports.validateZclFile = validateZclFile
