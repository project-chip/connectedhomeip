/**
 *    Copyright (c) 2024 Project CHIP Authors
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

import Foundation

class MTRSwiftCertificateTests : XCTestCase {
    func testGenerateIntermediateCert() {
        let rootKeys = MTRTestKeys()
        do {
            let rootCert = try MTRCertificates.createRootCertificate(rootKeys, issuerID: nil, fabricID: nil)
            XCTAssertNotNil(rootCert)

            let intermediateKeys = MTRTestKeys()
            XCTAssertNotNil(intermediateKeys)

            // NB: `copyPublicKey` returns Unmanaged<SecKey> if `copyPublicKey` lacks CF_RETURNS_RETAINED annotation.
            let intermediatePublicKey = intermediateKeys.copyPublicKey()
            XCTAssertNotNil(intermediatePublicKey)

            let intermediateCert = try MTRCertificates.createIntermediateCertificate(rootKeys, rootCertificate: rootCert, intermediatePublicKey: intermediatePublicKey, issuerID: nil as NSNumber?, fabricID: nil)
            XCTAssertNotNil(intermediateCert)

            guard let tlvCert = MTRCertificates.convertX509Certificate(intermediateCert) else {
                XCTFail()
                return
            }
            XCTAssertNotNil(tlvCert)

            guard let derCert = MTRCertificates.convertMatterCertificate(tlvCert) else {
                XCTFail()
                return
            }
            XCTAssertNotNil(derCert)

            XCTAssertEqual(intermediateCert, derCert)

        } catch {
            XCTFail()
        }
    }
}
