/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
import os.log

class ExampleDAC : DeviceAttestationCredentialsHolder {
    let kDevelopmentDAC_Cert_FFF1_8001: Data = Data(base64Encoded: "MIIB5zCCAY6gAwIBAgIIac3xDenlTtEwCgYIKoZIzj0EAwIwPTElMCMGA1UEAwwcTWF0dGVyIERldiBQQUkgMHhGRkYxIG5vIFBJRDEUMBIGCisGAQQBgqJ8AgEMBEZGRjEwIBcNMjIwMjA1MDAwMDAwWhgPOTk5OTEyMzEyMzU5NTlaMFMxJTAjBgNVBAMMHE1hdHRlciBEZXYgREFDIDB4RkZGMS8weDgwMDExFDASBgorBgEEAYKifAIBDARGRkYxMRQwEgYKKwYBBAGConwCAgwEODAwMTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABEY6xpNCkQoOVYj8b/Vrtj5i7M7LFI99TrA+5VJgFBV2fRalxmP3k+SRIyYLgpenzX58/HsxaznZjpDSk3dzjoKjYDBeMAwGA1UdEwEB/wQCMAAwDgYDVR0PAQH/BAQDAgeAMB0GA1UdDgQWBBSI3eezADgpMs/3NMBGJIEPRBaKbzAfBgNVHSMEGDAWgBRjVA5H9kscONE4hKRi0WwZXY/7PDAKBggqhkjOPQQDAgNHADBEAiABJ6J7S0RhDuL83E0reIVWNmC8D3bxchntagjfsrPBzQIga1ngr0Xz6yqFuRnTVzFSjGAoxBUjlUXhCOTlTnCXE1M=")!;
    let kDevelopmentDAC_PrivateKey_FFF1_8001: Data = Data(base64Encoded: "qrYAroroqrfXNifCF7fCBHCcppRq9fL3UwgzpStE+/8=")!;
    let kDevelopmentDAC_PublicKey_FFF1_8001: Data = Data(base64Encoded: "BEY6xpNCkQoOVYj8b/Vrtj5i7M7LFI99TrA+5VJgFBV2fRalxmP3k+SRIyYLgpenzX58/HsxaznZjpDSk3dzjoI=")!;
    let KPAI_FFF1_8000_Cert_Array: Data = Data(base64Encoded: "MIIByzCCAXGgAwIBAgIIVq2CIq2UW2QwCgYIKoZIzj0EAwIwMDEYMBYGA1UEAwwPTWF0dGVyIFRlc3QgUEFBMRQwEgYKKwYBBAGConwCAQwERkZGMTAgFw0yMjAyMDUwMDAwMDBaGA85OTk5MTIzMTIzNTk1OVowPTElMCMGA1UEAwwcTWF0dGVyIERldiBQQUkgMHhGRkYxIG5vIFBJRDEUMBIGCisGAQQBgqJ8AgEMBEZGRjEwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARBmpMVwhc+DIyHbQPM/JRIUmR/f+xeUIL0BZko7KiUxZQVEwmsYx5MsDOSr2hLC6+35ls7gWLC9Sv5MbjneqqCo2YwZDASBgNVHRMBAf8ECDAGAQH/AgEAMA4GA1UdDwEB/wQEAwIBBjAdBgNVHQ4EFgQUY1QOR/ZLHDjROISkYtFsGV2P+zwwHwYDVR0jBBgwFoAUav0idx9RH+y/FkGXZxDc3DGhcX4wCgYIKoZIzj0EAwIDSAAwRQIhALLvJ/Sa6bUPuR7qyUxNC9u415KcbLiPrOUpNo0SBUwMAiBlXckrhr2QmIKmxiF3uCXX0F7b58Ivn+pxIg5+pwP4kQ==")!;
    let kCertificationDeclaration: Data = Data(base64Encoded: "MIICGQYJKoZIhvcNAQcCoIICCjCCAgYCAQMxDTALBglghkgBZQMEAgEwggFxBgkqhkiG9w0BBwGgggFiBIIBXhUkAAElAfH/NgIFAIAFAYAFAoAFA4AFBIAFBYAFBoAFB4AFCIAFCYAFCoAFC4AFDIAFDYAFDoAFD4AFEIAFEYAFEoAFE4AFFIAFFYAFFoAFF4AFGIAFGYAFGoAFG4AFHIAFHYAFHoAFH4AFIIAFIYAFIoAFI4AFJIAFJYAFJoAFJ4AFKIAFKYAFKoAFK4AFLIAFLYAFLoAFL4AFMIAFMYAFMoAFM4AFNIAFNYAFNoAFN4AFOIAFOYAFOoAFO4AFPIAFPYAFPoAFP4AFQIAFQYAFQoAFQ4AFRIAFRYAFRoAFR4AFSIAFSYAFSoAFS4AFTIAFTYAFToAFT4AFUIAFUYAFUoAFU4AFVIAFVYAFVoAFV4AFWIAFWYAFWoAFW4AFXIAFXYAFXoAFX4AFYIAFYYAFYoAFY4AYJAMWLAQTWklHMjAxNDJaQjMzMDAwMy0yNCQFACQGACUHlCYkCAAYMX0wewIBA4AUYvqCM1ms+qmWPhz6FArd9QTzcWAwCwYJYIZIAWUDBAIBMAoGCCqGSM49BAMCBEcwRQIgJOXR9Hp9ew0gaibvaZt8l1e3LUaQid4xkuZ4x0Xn9gwCIQD4qi+nEfy3m5fjl87aZnuuRk4r0//fw8zteqjKX0wafA==")!;


    override func getCertificationDeclaration() -> Data {
        return kCertificationDeclaration
    }

    override func getFirmwareInformation() -> Data {
        return Data()
    }

    override func getDeviceAttestationCert() -> Data {
        return kDevelopmentDAC_Cert_FFF1_8001
    }

    override func getProductAttestationIntermediateCert() -> Data {
        return KPAI_FFF1_8000_Cert_Array
    }
    
    override func getDeviceAttestationCertPrivateKeyRef() -> Unmanaged<SecKey> {
        var privateKey = Data()
        privateKey.append(kDevelopmentDAC_PublicKey_FFF1_8001);
        privateKey.append(kDevelopmentDAC_PrivateKey_FFF1_8001);
        
        let privateKeyRef: SecKey = SecKeyCreateWithData(privateKey as NSData,
                                    [
                                        kSecAttrKeyType: kSecAttrKeyTypeECSECPrimeRandom,
                                        kSecAttrKeyClass: kSecAttrKeyClassPrivate,
                                        kSecAttrKeySizeInBits: 256
                                    ] as NSDictionary, nil)!
        
        return Unmanaged<SecKey>.passRetained(privateKeyRef);
    }
}
