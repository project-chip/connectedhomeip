#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

"""Shared PQC Phase 1 device attestation helpers for the TC-DA test scripts.

How a crypto profile is interpreted
-----------------------------------
An `AttestationCryptoProfileBitmap` value names the certificate *documents* a device holds,
not a single algorithm within one certificate. A device that advertises `MlDsa65` for the DAC
holds a DAC belonging to the ML-DSA-65 attestation chain; during PQC Phase 1 that DAC still
carries a P-256 subject key so the Device Attestation signature stays `EcdsaMatterLegacy`.
A profile therefore reads as the union ("OR") of the algorithms that may appear on the
certificate, and `CryptoProfile` in `CertificateChainRequest` selects which chain variant the
DUT serves. `TestHarnessDACProvider::GetProfileDocument` and `BuildProfileSupport` in
src/app/tests/suites/credentials/TestHarnessDACProvider.cpp implement exactly this.

The consequence for the tests is that the profile-selected DAC must be requested with the
selected *chain* profile rather than with the DAC's own subject-key profile: requesting the
DAC with `EcdsaMatterLegacy` returns the separate legacy DAC document instead of the DAC that
the PQC PAI issued.

An independent implementation, on purpose
-----------------------------------------
Every certificate check here is implemented in Python against the `cryptography` package rather
than by calling into the SDK's own C++ crypto. That is deliberate: if the test harness validated
with the same code the device under test uses, a defect in that shared code would be invisible to
the test. Keeping a second, independently written implementation gives the SDK a cross-check, so
the C++ and Python results have to agree for a test to pass.

`assert_attestation_certificate_format` mirrors the requirements enforced by
`chip::Crypto::VerifyAttestationCertificateFormat` (src/crypto/CHIPCryptoPALOpenSSL.cpp) and
`validate_attestation_chain` mirrors the sequence chip-cert's `validate-att-cert` applies
(src/tools/chip-cert/Cmd_ValidateAttCert.cpp). When one of those changes, this file has to be
updated to match, and the unit tests in
src/python_testing/test_testing/test_pqc_support.py check both implementations against the
shared certificate vectors in src/crypto/tests/MlDsaAttestationChain_test_vectors.h.

Requires a `cryptography` build providing `hazmat.primitives.asymmetric.mldsa` (46.0.0 or newer,
linked against OpenSSL 3.5 or newer) to validate ML-DSA chains. ECDSA-only chains work with any
supported version.
"""

import enum
import logging
from collections.abc import Awaitable, Callable
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path

from cryptography import x509
from cryptography.exceptions import InvalidSignature, UnsupportedAlgorithm
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from mobly import asserts

import matter.clusters as Clusters

logger = logging.getLogger(__name__)

AttestationCryptoProfile = Clusters.OperationalCredentials.Enums.AttestationCryptoProfileEnum
AttestationCryptoProfileBitmap = Clusters.OperationalCredentials.Bitmaps.AttestationCryptoProfileBitmap
CertificateChainType = Clusters.OperationalCredentials.Enums.CertificateChainTypeEnum
OperationalCredentialsFeature = Clusters.OperationalCredentials.Bitmaps.Feature

# The DUT slices every profile-selected document on a fixed 600 byte boundary and derives the
# read offset as SegmentID * 600, independently of the requested MaxSegmentSize. 600 is also the
# smallest MaxSegmentSize the spec permits, because it is the payload a Matter transport is
# always guaranteed to carry.
kCertificateSegmentSize = 600

# X.509 AlgorithmIdentifier OIDs for each attestation crypto profile. An ML-DSA certificate uses
# its profile-specific OID in whichever of the two AlgorithmIdentifier fields uses ML-DSA.
kOidEcPublicKey = "1.2.840.10045.2.1"
kOidEcdsaWithSha256 = "1.2.840.10045.4.3.2"
kOidMlDsa44 = "2.16.840.1.101.3.4.3.17"
kOidMlDsa65 = "2.16.840.1.101.3.4.3.18"

# Matter distinguished-name attribute OIDs carrying the VID and PID.
kOidMatterVid = "1.3.6.1.4.1.37244.2.1"
kOidMatterPid = "1.3.6.1.4.1.37244.2.2"

# Both key identifiers are a SHA-1 length digest.
kKeyIdentifierLength = 20

_kSignatureProfileForOid = {
    kOidEcdsaWithSha256: AttestationCryptoProfile.kEcdsaMatterLegacy,
    kOidMlDsa44: AttestationCryptoProfile.kMlDsa44,
    kOidMlDsa65: AttestationCryptoProfile.kMlDsa65,
}

_kPublicKeyProfileForOid = {
    kOidEcPublicKey: AttestationCryptoProfile.kEcdsaMatterLegacy,
    kOidMlDsa44: AttestationCryptoProfile.kMlDsa44,
    kOidMlDsa65: AttestationCryptoProfile.kMlDsa65,
}

_kProfileMask = {
    AttestationCryptoProfile.kEcdsaMatterLegacy: AttestationCryptoProfileBitmap.kSupportsEcdsaMatterLegacy,
    AttestationCryptoProfile.kMlDsa44: AttestationCryptoProfileBitmap.kSupportsMlDsa44,
    AttestationCryptoProfile.kMlDsa65: AttestationCryptoProfileBitmap.kSupportsMlDsa65,
}

# Strongest profile first, so the first advertised entry is the one to negotiate. The order also
# defines the algorithm strength ordering that AttestationAlgorithmStrength gives in C++: a
# certificate must never carry a key stronger than the algorithm that signed it.
_kProfilesByDescendingStrength = (
    AttestationCryptoProfile.kMlDsa65,
    AttestationCryptoProfile.kMlDsa44,
    AttestationCryptoProfile.kEcdsaMatterLegacy,
)

_kProfileStrength = {
    AttestationCryptoProfile.kEcdsaMatterLegacy: 1,
    AttestationCryptoProfile.kMlDsa44: 2,
    AttestationCryptoProfile.kMlDsa65: 3,
}


class AttestationCertType(enum.Enum):
    """The role a certificate plays in an attestation chain."""

    PAA = "PAA"
    PAI = "PAI"
    DAC = "DAC"


def profile_mask(profile: AttestationCryptoProfile) -> int:
    """Return the AttestationCryptoProfileBitmap mask for an enum profile."""
    asserts.assert_in(profile, _kProfileMask, f"No profile bitmap mask is defined for {profile!r}")
    return int(_kProfileMask[profile])


def is_pqc_profile(profile: AttestationCryptoProfile) -> bool:
    """Return whether a profile uses post-quantum cryptography."""
    return profile != AttestationCryptoProfile.kEcdsaMatterLegacy


def _ml_dsa_module():
    """Return the cryptography ML-DSA module, or None when the installed build lacks it."""
    try:
        from cryptography.hazmat.primitives.asymmetric import mldsa
    except ImportError:
        return None
    return mldsa


def is_ml_dsa_supported() -> bool:
    """Return whether the installed cryptography package can verify ML-DSA signatures."""
    return _ml_dsa_module() is not None


def assert_profile_supported_by_test_harness(profile: AttestationCryptoProfile) -> None:
    """Assert that the TH can verify the profile it is about to negotiate.

    Pre-condition 2 of TC-DA-1.10 and TC-DA-1.12 requires the TH to support ML-DSA-65 and
    ML-DSA-44. Checking up front keeps a TH limitation from surfacing later as a certificate
    validation failure and being misread as a DUT defect.
    """
    if not is_pqc_profile(profile):
        return

    asserts.assert_true(is_ml_dsa_supported(),
                        f"The TH cannot verify {profile.name}, which pre-condition 2 requires. Install a "
                        "cryptography build providing hazmat.primitives.asymmetric.mldsa (46.0.0 or newer).")


def select_strongest_profile(supported_profiles: int, chain_element: str) -> AttestationCryptoProfile:
    """Pick the highest-security attestation profile advertised for one chain element.

    Both TC-DA-1.10 and TC-DA-1.12 require the TH to negotiate the highest-security profile the
    DUT advertises: MlDsa65 when advertised, otherwise MlDsa44, otherwise EcdsaMatterLegacy.
    """
    for profile in _kProfilesByDescendingStrength:
        if supported_profiles & profile_mask(profile):
            logger.info("Selected %s profile %s from bitmap 0x%04X", chain_element, profile.name, supported_profiles)
            return profile

    asserts.fail(f"{chain_element}SupportedProfiles (0x{supported_profiles:04X}) advertises no known profile")


def assert_profile_advertised(supported_profiles: int, profile: AttestationCryptoProfile, document_name: str,
                              algorithm_field: str) -> None:
    """Assert that a bitmap advertises `profile`."""
    asserts.assert_true(supported_profiles & profile_mask(profile),
                        f"{document_name} {algorithm_field} uses {profile.name}, but "
                        f"{document_name}SupportedProfiles (0x{supported_profiles:04X}) does not advertise it")


@dataclass(frozen=True)
class CertificateAlgorithms:
    """Attestation profiles used by an X.509 certificate's key and signature."""

    subject_key_profile: AttestationCryptoProfile
    signature_profile: AttestationCryptoProfile
    subject_public_key_algorithm_oid: str
    signature_algorithm_oid: str


def certificate_algorithms_for_oids(signature_algorithm_oid: str,
                                    subject_public_key_algorithm_oid: str) -> CertificateAlgorithms:
    """Map the signature and subject-key OIDs to their attestation profiles."""
    asserts.assert_in(signature_algorithm_oid, _kSignatureProfileForOid,
                      f"Certificate uses unsupported signatureAlgorithm OID {signature_algorithm_oid}")
    asserts.assert_in(subject_public_key_algorithm_oid, _kPublicKeyProfileForOid,
                      "Certificate uses unsupported subjectPublicKeyInfo algorithm OID "
                      f"{subject_public_key_algorithm_oid}")
    return CertificateAlgorithms(
        subject_key_profile=_kPublicKeyProfileForOid[subject_public_key_algorithm_oid],
        signature_profile=_kSignatureProfileForOid[signature_algorithm_oid],
        subject_public_key_algorithm_oid=subject_public_key_algorithm_oid,
        signature_algorithm_oid=signature_algorithm_oid,
    )


def _subject_public_key_algorithm_oid(certificate: x509.Certificate, name: str) -> str:
    """Return the subjectPublicKeyInfo algorithm OID of a parsed certificate."""
    mldsa = _ml_dsa_module()

    try:
        public_key = certificate.public_key()
    except UnsupportedAlgorithm as e:
        asserts.fail(f"{name} carries a public key the installed cryptography cannot read: {e}")

    if isinstance(public_key, ec.EllipticCurvePublicKey):
        asserts.assert_equal(public_key.curve.name, "secp256r1",
                             f"{name} carries an ECDSA key on curve {public_key.curve.name}; attestation "
                             "certificates use P-256")
        return kOidEcPublicKey

    if mldsa is not None:
        if isinstance(public_key, mldsa.MLDSA44PublicKey):
            return kOidMlDsa44
        if isinstance(public_key, mldsa.MLDSA65PublicKey):
            return kOidMlDsa65

    asserts.fail(f"{name} carries an unsupported public key type {type(public_key).__name__}")


@dataclass(frozen=True)
class ParsedCertificate:
    """A parsed attestation certificate and the profiles it uses."""

    name: str
    der: bytes
    certificate: x509.Certificate
    algorithms: CertificateAlgorithms

    @property
    def is_self_issued(self) -> bool:
        return self.certificate.subject == self.certificate.issuer

    def matter_id(self, oid: str) -> int | None:
        """Return a Matter VID or PID from the subject DN, or None when it is absent.

        The identifier is carried either as its own DN attribute or, for older certificates, as
        an `Mvid:`/`Mpid:` token inside the common name.
        """
        values = self.certificate.subject.get_attributes_for_oid(x509.ObjectIdentifier(oid))
        if values:
            asserts.assert_equal(len(values), 1, f"{self.name} carries more than one {oid} subject attribute")
            return _parse_matter_id_text(str(values[0].value), self.name, oid)

        token = "Mvid:" if oid == kOidMatterVid else "Mpid:"
        for attribute in self.certificate.subject.get_attributes_for_oid(x509.NameOID.COMMON_NAME):
            for field in str(attribute.value).replace(",", " ").split():
                if field.startswith(token):
                    return _parse_matter_id_text(field[len(token):], self.name, oid)

        return None


def _parse_matter_id_text(text: str, certificate_name: str, oid: str) -> int:
    """Parse a Matter VID or PID, which is encoded as exactly four uppercase hex digits."""
    asserts.assert_equal(len(text), 4, f"{certificate_name} {oid} value {text!r} is not four hex digits")
    asserts.assert_true(all(character in "0123456789ABCDEF" for character in text),
                        f"{certificate_name} {oid} value {text!r} must be uppercase hex")
    return int(text, 16)


def parse_certificate(der: bytes, name: str) -> ParsedCertificate:
    """Parse a DER attestation certificate and determine the profiles it uses."""
    asserts.assert_greater(len(der), 0, f"{name} is empty")
    try:
        certificate = x509.load_der_x509_certificate(der)
    except Exception as e:
        asserts.fail(f"{name} does not parse as an X.509 certificate: {type(e).__name__}: {e}")

    algorithms = certificate_algorithms_for_oids(certificate.signature_algorithm_oid.dotted_string,
                                                 _subject_public_key_algorithm_oid(certificate, name))
    logger.info("%s: signatureAlgorithm %s (%s), subjectPublicKeyInfo algorithm %s (%s)", name,
                algorithms.signature_algorithm_oid, algorithms.signature_profile.name,
                algorithms.subject_public_key_algorithm_oid, algorithms.subject_key_profile.name)
    return ParsedCertificate(name=name, der=der, certificate=certificate, algorithms=algorithms)


def parse_certificate_algorithms(der: bytes, name: str = "certificate") -> CertificateAlgorithms:
    """Parse the attestation profiles used for a certificate's key and signature."""
    return parse_certificate(der, name).algorithms


def _assert_single_extension(certificate: x509.Certificate, extension_class, must_be_critical: bool,
                             name: str) -> object | None:
    """Return an extension's value, asserting it appears at most once with the required criticality."""
    matching = [extension for extension in certificate.extensions if isinstance(extension.value, extension_class)]
    if not matching:
        return None

    asserts.assert_equal(len(matching), 1, f"{name} carries {extension_class.__name__} more than once")
    extension = matching[0]
    if must_be_critical:
        asserts.assert_true(extension.critical, f"{name} {extension_class.__name__} must be marked critical")
    else:
        asserts.assert_false(extension.critical, f"{name} {extension_class.__name__} must not be marked critical")
    return extension.value


def assert_attestation_certificate_format(parsed: ParsedCertificate, cert_type: AttestationCertType) -> None:
    """Assert that a certificate satisfies the device attestation certificate format requirements.

    Mirrors chip::Crypto::VerifyAttestationCertificateFormat.
    """
    name = parsed.name
    certificate = parsed.certificate

    asserts.assert_equal(certificate.version, x509.Version.v3, f"{name} must be an X.509 v3 certificate")
    asserts.assert_greater(certificate.serial_number, 0, f"{name} must carry a serial number")
    asserts.assert_greater(len(certificate.issuer), 0, f"{name} must carry an issuer name")
    asserts.assert_greater(len(certificate.subject), 0, f"{name} must carry a subject name")

    key_profile = parsed.algorithms.subject_key_profile
    signature_profile = parsed.algorithms.signature_profile

    # A DAC always carries a P-256 key; only a PAA or PAI may be ML-DSA.
    if cert_type == AttestationCertType.DAC:
        asserts.assert_equal(key_profile, AttestationCryptoProfile.kEcdsaMatterLegacy,
                             f"{name} is a DAC, so its subject key must be ECDSA P-256, not {key_profile.name}")

    if cert_type == AttestationCertType.PAA:
        # A self-signed PAA must use its subject key algorithm for its signature.
        asserts.assert_equal(key_profile, signature_profile,
                             f"{name} is a self-signed PAA, so its signature algorithm "
                             f"({signature_profile.name}) must match its subject key ({key_profile.name})")
    else:
        # A certificate must not be stronger than the one that signed it: an issuer whose own key
        # can be broken offers no protection to a stronger key below it.
        asserts.assert_less_equal(_kProfileStrength[key_profile], _kProfileStrength[signature_profile],
                                  f"{name} carries a {key_profile.name} key but was signed with the weaker "
                                  f"{signature_profile.name}")

    basic_constraints = _assert_single_extension(certificate, x509.BasicConstraints, True, name)
    asserts.assert_is_not_none(basic_constraints, f"{name} must carry the basicConstraints extension")
    if cert_type == AttestationCertType.DAC:
        asserts.assert_false(basic_constraints.ca, f"{name} is a DAC, so basicConstraints CA must be false")
        asserts.assert_is_none(basic_constraints.path_length,
                               f"{name} is a DAC, so basicConstraints must not carry a pathLenConstraint")
    elif cert_type == AttestationCertType.PAI:
        asserts.assert_true(basic_constraints.ca, f"{name} is a PAI, so basicConstraints CA must be true")
        asserts.assert_equal(basic_constraints.path_length, 0,
                             f"{name} is a PAI, so its pathLenConstraint must be 0")
    else:
        asserts.assert_true(basic_constraints.ca, f"{name} is a PAA, so basicConstraints CA must be true")
        asserts.assert_in(basic_constraints.path_length, (None, 1),
                          f"{name} is a PAA, so its pathLenConstraint must be absent or 1")

    key_usage = _assert_single_extension(certificate, x509.KeyUsage, True, name)
    asserts.assert_is_not_none(key_usage, f"{name} must carry the keyUsage extension")
    if cert_type == AttestationCertType.DAC:
        # SHALL only have the digitalSignature bit set.
        asserts.assert_true(key_usage.digital_signature, f"{name} is a DAC, so keyUsage digitalSignature must be set")
        for unexpected in ("content_commitment", "key_encipherment", "data_encipherment", "key_agreement",
                           "key_cert_sign", "crl_sign"):
            asserts.assert_false(getattr(key_usage, unexpected),
                                 f"{name} is a DAC, so keyUsage must only set digitalSignature, but "
                                 f"{unexpected} is set")
    else:
        asserts.assert_true(key_usage.key_cert_sign, f"{name} is a CA, so keyUsage keyCertSign must be set")
        asserts.assert_true(key_usage.crl_sign, f"{name} is a CA, so keyUsage cRLSign must be set")
        # digitalSignature is tolerated on a CA certificate; nothing else is.
        for unexpected in ("content_commitment", "key_encipherment", "data_encipherment", "key_agreement"):
            asserts.assert_false(getattr(key_usage, unexpected),
                                 f"{name} is a CA, so keyUsage must not set {unexpected}")

    subject_key_identifier = _assert_single_extension(certificate, x509.SubjectKeyIdentifier, False, name)
    asserts.assert_is_not_none(subject_key_identifier, f"{name} must carry the subjectKeyIdentifier extension")
    asserts.assert_equal(len(subject_key_identifier.digest), kKeyIdentifierLength,
                         f"{name} subjectKeyIdentifier must be {kKeyIdentifierLength} bytes")

    authority_key_identifier = _assert_single_extension(certificate, x509.AuthorityKeyIdentifier, False, name)
    if cert_type in (AttestationCertType.DAC, AttestationCertType.PAI):
        asserts.assert_is_not_none(authority_key_identifier,
                                   f"{name} must carry the authorityKeyIdentifier extension")
    if authority_key_identifier is not None:
        asserts.assert_is_not_none(authority_key_identifier.key_identifier,
                                   f"{name} authorityKeyIdentifier must carry a key identifier")
        asserts.assert_equal(len(authority_key_identifier.key_identifier), kKeyIdentifierLength,
                             f"{name} authorityKeyIdentifier must be {kKeyIdentifierLength} bytes")

    logger.info("%s satisfies the %s device attestation certificate format requirements", name, cert_type.value)


def assert_certificate_currently_valid(parsed: ParsedCertificate) -> None:
    """Assert that a certificate's validity period covers the current time."""
    now = datetime.now(UTC)
    not_before = parsed.certificate.not_valid_before_utc
    not_after = parsed.certificate.not_valid_after_utc
    asserts.assert_less_equal(not_before, now, f"{parsed.name} is not valid until {not_before.isoformat()}")
    asserts.assert_less_equal(now, not_after, f"{parsed.name} expired at {not_after.isoformat()}")


def verify_certificate_signature(subject: ParsedCertificate, issuer: ParsedCertificate) -> None:
    """Verify that `issuer` signed `subject`."""
    asserts.assert_equal(subject.algorithms.signature_profile, issuer.algorithms.subject_key_profile,
                         f"{subject.name} is signed with {subject.algorithms.signature_profile.name} but "
                         f"{issuer.name} holds a {issuer.algorithms.subject_key_profile.name} key")

    profile = subject.algorithms.signature_profile
    public_key = issuer.certificate.public_key()
    signature = subject.certificate.signature
    signed_data = subject.certificate.tbs_certificate_bytes

    try:
        if profile == AttestationCryptoProfile.kEcdsaMatterLegacy:
            public_key.verify(signature, signed_data, ec.ECDSA(hashes.SHA256()))
        else:
            # ML-DSA signs the message directly rather than a digest, and Matter uses the empty
            # context, which is what cryptography's verify() applies.
            asserts.assert_true(is_ml_dsa_supported(),
                                f"Verifying the {subject.name} {profile.name} signature needs a cryptography "
                                "build providing hazmat.primitives.asymmetric.mldsa (46.0.0 or newer).")
            public_key.verify(signature, signed_data)
    except InvalidSignature:
        asserts.fail(f"The {subject.name} signature does not validate with the {issuer.name} public key "
                     f"({profile.name})")

    logger.info("Verified the %s signature with the %s public key (%s)", subject.name, issuer.name, profile.name)


def assert_dac_and_pai_ids(dac: ParsedCertificate, pai: ParsedCertificate, paa: ParsedCertificate,
                           chain_name: str) -> None:
    """Assert the VID and PID constraints the device attestation certificates must satisfy.

    Mirrors the identifier checks in chip-cert's validate-att-cert: the DAC must carry a VID and
    PID, the PAI VID must match, a PAI PID must match when present, a PAA VID must match when
    present, and a PAA must never carry a PID.
    """
    dac_vid, dac_pid = dac.matter_id(kOidMatterVid), dac.matter_id(kOidMatterPid)
    pai_vid, pai_pid = pai.matter_id(kOidMatterVid), pai.matter_id(kOidMatterPid)
    paa_vid, paa_pid = paa.matter_id(kOidMatterVid), paa.matter_id(kOidMatterPid)

    asserts.assert_is_not_none(dac_vid, f"The {chain_name} DAC subject must carry a VID")
    asserts.assert_is_not_none(dac_pid, f"The {chain_name} DAC subject must carry a PID")
    asserts.assert_equal(pai_vid, dac_vid,
                         f"The {chain_name} PAI VID must match the DAC VID (PAI {pai_vid}, DAC {dac_vid})")
    if pai_pid is not None:
        asserts.assert_equal(pai_pid, dac_pid,
                             f"The {chain_name} PAI PID is present and must match the DAC PID "
                             f"(PAI {pai_pid}, DAC {dac_pid})")
    if paa_vid is not None:
        asserts.assert_equal(paa_vid, dac_vid,
                             f"The {chain_name} PAA VID is present and must match the DAC VID "
                             f"(PAA {paa_vid}, DAC {dac_vid})")
    asserts.assert_is_none(paa_pid, f"The {chain_name} PAA must not carry a PID")

    logger.info("The %s chain agrees on VID 0x%04X / PID 0x%04X", chain_name, dac_vid, dac_pid)


def validate_attestation_chain(paa: ParsedCertificate, pai: ParsedCertificate, dac: ParsedCertificate,
                               chain_name: str) -> None:
    """Run the full device attestation validation of a PAA, PAI and DAC.

    Applies the same sequence as chip-cert's validate-att-cert: the per-certificate format
    requirements and validity, the VID/PID constraints, and the signature chain.
    """
    for parsed, cert_type in ((paa, AttestationCertType.PAA), (pai, AttestationCertType.PAI),
                              (dac, AttestationCertType.DAC)):
        assert_attestation_certificate_format(parsed, cert_type)
        assert_certificate_currently_valid(parsed)

    asserts.assert_true(paa.is_self_issued, f"The {chain_name} PAA must be self-issued")
    asserts.assert_equal(pai.certificate.issuer, paa.certificate.subject,
                         f"The {chain_name} PAI is not issued by the PAA")
    asserts.assert_equal(dac.certificate.issuer, pai.certificate.subject,
                         f"The {chain_name} DAC is not issued by the PAI")

    assert_dac_and_pai_ids(dac, pai, paa, chain_name)

    verify_certificate_signature(paa, paa)
    verify_certificate_signature(pai, paa)
    verify_certificate_signature(dac, pai)

    logger.info("The %s certificate chain validates from the PAA through the PAI to the DAC", chain_name)


def load_paa_certificates(directory: Path) -> list[ParsedCertificate]:
    """Parse every DER PAA certificate in a trust store directory.

    PEM files are skipped because the trust stores in this tree always ship a matching `.der`
    alongside each `.pem`. A file this module cannot parse is skipped rather than failing the
    test, because a trust store may hold certificates using algorithms outside PQC Phase 1.
    """
    asserts.assert_true(directory.is_dir(), f"PAA trust store {directory} is not a directory")

    certificates = []
    for path in sorted(directory.glob("*.der")):
        try:
            certificates.append(parse_certificate(path.read_bytes(), f"PAA {path.name}"))
        except Exception as e:  # noqa: BLE001 - an unparsable candidate is simply not a PAA we can use.
            logger.debug("Skipping PAA candidate %s: %s", path.name, e)

    asserts.assert_greater(len(certificates), 0, f"PAA trust store {directory} holds no usable .der certificate")
    logger.info("Loaded %d PAA candidate(s) from %s", len(certificates), directory)
    return certificates


def find_issuing_paa(pai: ParsedCertificate, candidates: list[ParsedCertificate],
                     expected_profile: AttestationCryptoProfile, chain_name: str) -> ParsedCertificate:
    """Return the trust store PAA that issued `pai` using `expected_profile`.

    Candidates are narrowed by subject DN and key profile and then confirmed by verifying the PAI
    signature, so a trust store holding several PAAs with the same subject still resolves.
    """
    matching_subject = [candidate for candidate in candidates
                        if candidate.certificate.subject == pai.certificate.issuer]
    asserts.assert_greater(len(matching_subject), 0,
                           f"No PAA in the trust store has the subject named by the {chain_name} PAI issuer")

    matching_profile = [candidate for candidate in matching_subject
                        if candidate.algorithms.subject_key_profile == expected_profile]
    asserts.assert_greater(len(matching_profile), 0,
                           f"The trust store holds no {expected_profile.name} PAA for the {chain_name} PAI issuer; "
                           "the PAA for the negotiated profile must be provided externally")

    for candidate in matching_profile:
        try:
            verify_certificate_signature(pai, candidate)
        except Exception:  # noqa: BLE001 - a non-matching candidate simply fails verification.
            continue
        logger.info("Resolved the %s PAI issuer to %s", chain_name, candidate.name)
        return candidate

    asserts.fail(f"None of the {len(matching_profile)} {expected_profile.name} PAA candidate(s) for the "
                 f"{chain_name} PAI issuer validate its signature")


@dataclass(frozen=True)
class SegmentedDocument:
    """A certificate reassembled from one or more CertificateChainResponse segments."""

    der: bytes
    segment_count: int
    total_document_size: int


# A callable that issues one CertificateChainRequest and returns the CertificateChainResponse.
SendCertificateChainRequest = Callable[[CertificateChainType, AttestationCryptoProfile, int, int], Awaitable]


async def retrieve_segmented_document(send_request: SendCertificateChainRequest,
                                      certificate_type: CertificateChainType,
                                      crypto_profile: AttestationCryptoProfile,
                                      document_name: str,
                                      max_segment_size: int = kCertificateSegmentSize) -> SegmentedDocument:
    """Retrieve a profile-selected certificate, following NextSegmentID to the final segment.

    Asserts the segmentation rules along the way: SegmentID numbering is monotonic from zero,
    TotalDocumentSize is stable across every segment, NextSegmentID is present until the final
    segment and absent on it, and the reassembled document matches TotalDocumentSize.
    """
    response = await send_request(certificate_type, crypto_profile, 0, max_segment_size)
    asserts.assert_is_not_none(response.totalDocumentSize,
                               f"DUT omitted TotalDocumentSize from the profile-selected {document_name} response")
    asserts.assert_greater(response.totalDocumentSize, 0, f"DUT reported an empty {document_name} document")

    total_document_size = response.totalDocumentSize
    if total_document_size > kCertificateSegmentSize:
        asserts.assert_is_not_none(response.nextSegmentID,
                                   f"DUT omitted NextSegmentID on a {document_name} document that needs more than "
                                   "one segment")
    else:
        asserts.assert_is_none(response.nextSegmentID,
                               f"DUT returned NextSegmentID on a single-segment {document_name} document")

    document = bytearray(response.certificate)
    # Segment 0 has been retrieved, so the next expected SegmentID is 1.
    expected_segment_id = 1

    while response.nextSegmentID is not None:
        asserts.assert_equal(response.nextSegmentID, expected_segment_id,
                             f"{document_name} NextSegmentID must be monotonic from zero")

        response = await send_request(certificate_type, crypto_profile, expected_segment_id, max_segment_size)
        asserts.assert_equal(response.totalDocumentSize, total_document_size,
                             f"{document_name} TotalDocumentSize must be stable across all segments")
        asserts.assert_greater(len(response.certificate), 0,
                               f"{document_name} segment {expected_segment_id} is empty")
        asserts.assert_less_equal(len(response.certificate), max_segment_size,
                                  f"{document_name} segment {expected_segment_id} exceeds the requested MaxSegmentSize")
        document += response.certificate
        expected_segment_id += 1
        asserts.assert_less_equal(len(document), total_document_size,
                                  f"{document_name} segments returned more data than TotalDocumentSize")

    asserts.assert_is_none(response.nextSegmentID, f"{document_name} final segment must omit NextSegmentID")
    asserts.assert_equal(len(document), total_document_size,
                         f"Reassembled {document_name} does not match the declared TotalDocumentSize")

    logger.info("Reassembled the %s from %d segment(s), %d bytes", document_name, expected_segment_id,
                total_document_size)
    return SegmentedDocument(der=bytes(document), segment_count=expected_segment_id,
                             total_document_size=total_document_size)
