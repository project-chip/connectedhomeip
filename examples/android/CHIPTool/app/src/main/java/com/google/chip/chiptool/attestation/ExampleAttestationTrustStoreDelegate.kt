package com.google.chip.chiptool.attestation

import android.util.Base64
import chip.devicecontroller.AttestationTrustStoreDelegate
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.DeviceAttestation
import java.util.*

class ExampleAttestationTrustStoreDelegate(val chipDeviceController: ChipDeviceController) :
  AttestationTrustStoreDelegate {

  private val paaCerts = arrayListOf(TEST_PAA_FFF1_Cert, TEST_PAA_NOVID_CERT)

  override fun getProductAttestationAuthorityCert(skid: ByteArray): ByteArray? {
    return paaCerts
      .map { Base64.decode(it, Base64.DEFAULT) }
      .firstOrNull { cert -> Arrays.equals(DeviceAttestation.extractSkidFromPaaCert(cert), skid) }
  }

  companion object {
    const val TEST_PAA_FFF1_Cert =
      "MIIBvTCCAWSgAwIBAgIITqjoMYLUHBwwCgYIKoZIzj0EAwIwMDEYMBYGA1UEAwwP\n" +
        "TWF0dGVyIFRlc3QgUEFBMRQwEgYKKwYBBAGConwCAQwERkZGMTAgFw0yMTA2Mjgx\n" +
        "NDIzNDNaGA85OTk5MTIzMTIzNTk1OVowMDEYMBYGA1UEAwwPTWF0dGVyIFRlc3Qg\n" +
        "UEFBMRQwEgYKKwYBBAGConwCAQwERkZGMTBZMBMGByqGSM49AgEGCCqGSM49AwEH\n" +
        "A0IABLbLY3KIfyko9brIGqnZOuJDHK2p154kL2UXfvnO2TKijs0Duq9qj8oYShpQ\n" +
        "NUKWDUU/MD8fGUIddR6Pjxqam3WjZjBkMBIGA1UdEwEB/wQIMAYBAf8CAQEwDgYD\n" +
        "VR0PAQH/BAQDAgEGMB0GA1UdDgQWBBRq/SJ3H1Ef7L8WQZdnENzcMaFxfjAfBgNV\n" +
        "HSMEGDAWgBRq/SJ3H1Ef7L8WQZdnENzcMaFxfjAKBggqhkjOPQQDAgNHADBEAiBQ\n" +
        "qoAC9NkyqaAFOPZTaK0P/8jvu8m+t9pWmDXPmqdRDgIgI7rI/g8j51RFtlM5CBpH\n" +
        "mUkpxyqvChVI1A0DTVFLJd4="

    const val TEST_PAA_NOVID_CERT =
      "MIIBkTCCATegAwIBAgIHC4+6qN2G7jAKBggqhkjOPQQDAjAaMRgwFgYDVQQDDA9N\n" +
        "YXR0ZXIgVGVzdCBQQUEwIBcNMjEwNjI4MTQyMzQzWhgPOTk5OTEyMzEyMzU5NTla\n" +
        "MBoxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBQTBZMBMGByqGSM49AgEGCCqGSM49\n" +
        "AwEHA0IABBDvAqgah7aBIfuo0xl4+AejF+UKqKgoRGgokUuTPejt1KXDnJ/3Gkzj\n" +
        "ZH/X9iZTt9JJX8ukwPR/h2iAA54HIEqjZjBkMBIGA1UdEwEB/wQIMAYBAf8CAQEw\n" +
        "DgYDVR0PAQH/BAQDAgEGMB0GA1UdDgQWBBR4XOcFuGuPTm/Hk6pgy0PqaWiC1TAf\n" +
        "BgNVHSMEGDAWgBR4XOcFuGuPTm/Hk6pgy0PqaWiC1TAKBggqhkjOPQQDAgNIADBF\n" +
        "AiEAue/bPqBqUuwL8B5h2u0sLRVt22zwFBAdq3mPrAX6R+UCIGAGHT411g2dSw1E\n" +
        "ja12EvfoXFguP8MS3Bh5TdNzcV5d"
  }
}
