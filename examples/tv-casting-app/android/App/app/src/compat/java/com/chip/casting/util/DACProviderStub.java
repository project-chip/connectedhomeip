package com.chip.casting.util;

import android.util.Base64;
import com.chip.casting.DACProvider;
import java.math.BigInteger;
import java.security.AlgorithmParameters;
import java.security.KeyFactory;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.ECParameterSpec;
import java.security.spec.ECPrivateKeySpec;

/** @deprecated Refer to com.matter.casting.DACProviderStub. */
@Deprecated
public class DACProviderStub implements DACProvider {

  private String kDevelopmentDAC_Cert_FFF1_8001 =
      "MIIB5zCCAY6gAwIBAgIIac3xDenlTtEwCgYIKoZIzj0EAwIwPTElMCMGA1UEAwwcTWF0dGVyIERldiBQQUkgMHhGRkYxIG5vIFBJRDEUMBIGCisGAQQBgqJ8AgEMBEZGRjEwIBcNMjIwMjA1MDAwMDAwWhgPOTk5OTEyMzEyMzU5NTlaMFMxJTAjBgNVBAMMHE1hdHRlciBEZXYgREFDIDB4RkZGMS8weDgwMDExFDASBgorBgEEAYKifAIBDARGRkYxMRQwEgYKKwYBBAGConwCAgwEODAwMTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABEY6xpNCkQoOVYj8b/Vrtj5i7M7LFI99TrA+5VJgFBV2fRalxmP3k+SRIyYLgpenzX58/HsxaznZjpDSk3dzjoKjYDBeMAwGA1UdEwEB/wQCMAAwDgYDVR0PAQH/BAQDAgeAMB0GA1UdDgQWBBSI3eezADgpMs/3NMBGJIEPRBaKbzAfBgNVHSMEGDAWgBRjVA5H9kscONE4hKRi0WwZXY/7PDAKBggqhkjOPQQDAgNHADBEAiABJ6J7S0RhDuL83E0reIVWNmC8D3bxchntagjfsrPBzQIga1ngr0Xz6yqFuRnTVzFSjGAoxBUjlUXhCOTlTnCXE1M=";

  private String kDevelopmentDAC_PrivateKey_FFF1_8001 =
      "qrYAroroqrfXNifCF7fCBHCcppRq9fL3UwgzpStE+/8=";

  private String kDevelopmentDAC_PublicKey_FFF1_8001 =
      "BEY6xpNCkQoOVYj8b/Vrtj5i7M7LFI99TrA+5VJgFBV2fRalxmP3k+SRIyYLgpenzX58/HsxaznZjpDSk3dzjoI=";

  private String KPAI_FFF1_8000_Cert_Array =
      "MIIByzCCAXGgAwIBAgIIVq2CIq2UW2QwCgYIKoZIzj0EAwIwMDEYMBYGA1UEAwwPTWF0dGVyIFRlc3QgUEFBMRQwEgYKKwYBBAGConwCAQwERkZGMTAgFw0yMjAyMDUwMDAwMDBaGA85OTk5MTIzMTIzNTk1OVowPTElMCMGA1UEAwwcTWF0dGVyIERldiBQQUkgMHhGRkYxIG5vIFBJRDEUMBIGCisGAQQBgqJ8AgEMBEZGRjEwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARBmpMVwhc+DIyHbQPM/JRIUmR/f+xeUIL0BZko7KiUxZQVEwmsYx5MsDOSr2hLC6+35ls7gWLC9Sv5MbjneqqCo2YwZDASBgNVHRMBAf8ECDAGAQH/AgEAMA4GA1UdDwEB/wQEAwIBBjAdBgNVHQ4EFgQUY1QOR/ZLHDjROISkYtFsGV2P+zwwHwYDVR0jBBgwFoAUav0idx9RH+y/FkGXZxDc3DGhcX4wCgYIKoZIzj0EAwIDSAAwRQIhALLvJ/Sa6bUPuR7qyUxNC9u415KcbLiPrOUpNo0SBUwMAiBlXckrhr2QmIKmxiF3uCXX0F7b58Ivn+pxIg5+pwP4kQ==";

  /**
   * format_version = 1 vendor_id = 0xFFF1 product_id_array = [ 0x8000,0x8001...0x8063]
   * device_type_id = 0x1234 certificate_id = "ZIG20141ZB330001-24" security_level = 0
   * security_information = 0 version_number = 0x2694 certification_type = 0 dac_origin_vendor_id is
   * not present dac_origin_product_id is not present
   */
  private String kCertificationDeclaration =
      "MIICGQYJKoZIhvcNAQcCoIICCjCCAgYCAQMxDTALBglghkgBZQMEAgEwggFxBgkqhkiG9w0BBwGgggFiBIIBXhUkAAElAfH/NgIFAIAFAYAFAoAFA4AFBIAFBYAFBoAFB4AFCIAFCYAFCoAFC4AFDIAFDYAFDoAFD4AFEIAFEYAFEoAFE4AFFIAFFYAFFoAFF4AFGIAFGYAFGoAFG4AFHIAFHYAFHoAFH4AFIIAFIYAFIoAFI4AFJIAFJYAFJoAFJ4AFKIAFKYAFKoAFK4AFLIAFLYAFLoAFL4AFMIAFMYAFMoAFM4AFNIAFNYAFNoAFN4AFOIAFOYAFOoAFO4AFPIAFPYAFPoAFP4AFQIAFQYAFQoAFQ4AFRIAFRYAFRoAFR4AFSIAFSYAFSoAFS4AFTIAFTYAFToAFT4AFUIAFUYAFUoAFU4AFVIAFVYAFVoAFV4AFWIAFWYAFWoAFW4AFXIAFXYAFXoAFX4AFYIAFYYAFYoAFY4AYJAMWLAQTWklHMjAxNDJaQjMzMDAwMy0yNCQFACQGACUHlCYkCAAYMX0wewIBA4AUYvqCM1ms+qmWPhz6FArd9QTzcWAwCwYJYIZIAWUDBAIBMAoGCCqGSM49BAMCBEcwRQIgJOXR9Hp9ew0gaibvaZt8l1e3LUaQid4xkuZ4x0Xn9gwCIQD4qi+nEfy3m5fjl87aZnuuRk4r0//fw8zteqjKX0wafA==";

  @Override
  public byte[] GetCertificationDeclaration() {
    return Base64.decode(kCertificationDeclaration, Base64.DEFAULT);
  }

  @Override
  public byte[] GetFirmwareInformation() {
    return new byte[0];
  }

  @Override
  public byte[] GetDeviceAttestationCert() {
    return Base64.decode(kDevelopmentDAC_Cert_FFF1_8001, Base64.DEFAULT);
  }

  @Override
  public byte[] GetProductAttestationIntermediateCert() {
    return Base64.decode(KPAI_FFF1_8000_Cert_Array, Base64.DEFAULT);
  }

  @Override
  public byte[] SignWithDeviceAttestationKey(byte[] message) {

    try {
      byte[] privateKeyBytes = Base64.decode(kDevelopmentDAC_PrivateKey_FFF1_8001, Base64.DEFAULT);

      AlgorithmParameters algorithmParameters = AlgorithmParameters.getInstance("EC");
      algorithmParameters.init(new ECGenParameterSpec("secp256r1"));
      ECParameterSpec parameterSpec = algorithmParameters.getParameterSpec(ECParameterSpec.class);
      ECPrivateKeySpec ecPrivateKeySpec =
          new ECPrivateKeySpec(new BigInteger(1, privateKeyBytes), parameterSpec);

      KeyFactory keyFactory = KeyFactory.getInstance("EC");
      PrivateKey privateKey = keyFactory.generatePrivate(ecPrivateKeySpec);

      Signature signature = Signature.getInstance("SHA256withECDSA");
      signature.initSign(privateKey);

      signature.update(message);

      return signature.sign();

    } catch (Exception e) {
      return null;
    }
  }
}
