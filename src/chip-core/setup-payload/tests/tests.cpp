/*
<COPYRIGHT>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "../QRCodeSetupPayload.h"
#include "../QRCodeSetupPayloadGenerator.h"
#include "../SetupCodeUtils.h"


#include <iostream>

using namespace chip;
using namespace std;


bool testPayloadByteArrayRep()
{
  QRCodeSetupPayload payload;

  payload.version = 5;
  payload.vendorID = 12;
  payload.productID = 1;
  payload.requiresCustomFlow = 0;
  payload.rendezvousInformation = 1;
  payload.discriminator = 128;
  payload.setUpPINCode = 2048;

  QRCodeSetupPayloadGenerator generator(payload);
  string result = generator.getQRCodePayloadBinaryRepresentation();
  string expectedResult = "10100000000000011000000000000000001000000001100000000000000000000001000000000000";
  
  return result.compare(expectedResult) == 0; 
}

bool testPayloadBase45Rep()
{
  QRCodeSetupPayload payload;

  payload.version = 5;
  payload.vendorID = 12;
  payload.productID = 1;
  payload.requiresCustomFlow = 0;
  payload.rendezvousInformation = 1;
  payload.discriminator = 128;
  payload.setUpPINCode = 2048;

  QRCodeSetupPayloadGenerator generator(payload);
  generator.enableDebugLogs = false;
  string result = generator.getQRCodePayloadBase45Representation();
  string expectedResult = "KABG8842Q000211";
  
  return result.compare(expectedResult) == 0; 
}


bool testBase45Encoding()
{
  uint16_t input = 10;
  string result = base45EncodedString(input, 3);
  string expectedResult = "00A";
  return result.compare(expectedResult) == 0;
}

void callTestFunction(int &numberOfTests, int &failingTests, bool &testPassed, bool (*testFunction)(void), char *functionName)
{
  numberOfTests++;
  if (false == testFunction()) {
    failingTests++;
    printf("\n** %s failed. **", functionName);
    testPassed = false;
  }
  else {
    printf("\n%s passed!", functionName);
  }
}

int main(int argc, char** argv) 
{
  printf("\nRunning QRCodeSetupPayload Tests\n");
  bool allTestsPass = true;
  int numberOfTests = 0;
  int failingTests = 0;

  callTestFunction(numberOfTests, failingTests, allTestsPass, &testPayloadByteArrayRep, "testPayloadByteArrayRep");
  callTestFunction(numberOfTests, failingTests, allTestsPass, &testPayloadBase45Rep, "testPayloadBase45Rep");
  callTestFunction(numberOfTests, failingTests, allTestsPass, &testBase45Encoding, "testBase45Encoding");


  cout << "\n\nNumber of tests run " << (numberOfTests) << endl;
  cout << "Number of passing tests " << (numberOfTests - failingTests) << endl;
  cout << "Number of failing tests " << failingTests << endl;
  if (allTestsPass) {
    cout << "\nAll QRCodeSetupPayload tests passed\n";
  } else {
    cout << "\nCodeSetupPayload Tests failed\n";
  }
  cout << endl;
}