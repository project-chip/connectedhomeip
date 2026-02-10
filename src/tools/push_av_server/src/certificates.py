"""
Certificate Authority hierarchy management service.
"""

import datetime
import ipaddress
import logging
import random
import string
import sys
from pathlib import Path
from typing import Literal, Optional

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives.asymmetric.types import CertificateIssuerPrivateKeyTypes, CertificatePublicKeyTypes
from cryptography.x509.oid import ExtendedKeyUsageOID, NameOID

log = logging.getLogger(__name__)


class CAHierarchy:
    """
    Utilities to manage a CA hierarchy on disk.
    """
    default_ca_duration = datetime.timedelta(days=365.25*20)
    client_key_usage_cert = x509.KeyUsage(
        digital_signature=True,
        content_commitment=False,
        key_encipherment=True,
        data_encipherment=False,
        key_agreement=False,
        key_cert_sign=False,
        crl_sign=False,
        encipher_only=False,
        decipher_only=False,
    )
    server_key_usage_cert = x509.KeyUsage(
        digital_signature=True,
        content_commitment=False,
        key_encipherment=False,
        data_encipherment=False,
        key_agreement=False,
        key_cert_sign=False,
        crl_sign=False,
        encipher_only=False,
        decipher_only=False,
    )

    def __init__(self, base: Path, name: str, kind: Literal['server', 'client']) -> None:
        self.name = name
        self.kind = kind
        self.directory = base
        self.root_cert_path = self.directory / "root.pem"
        self.root_key_path = self.directory / "root.key"

        if self.root_key_path.exists() and self.root_cert_path.exists():
            # Root certificate already exists, re-using them
            self.root_cert = x509.load_pem_x509_certificate(
                self.root_cert_path.read_bytes()
            )
            self.root_key = serialization.load_pem_private_key(
                self.root_key_path.read_bytes(), None
            )
            log.info(f"CA Hierarchy loaded from disk: {self.name}")
        elif self.root_key_path.exists() or self.root_cert_path.exists():
            # Only one of the two file exists, bailing out
            log.error("root certificate partially exist on disk, stopping early")
            sys.exit(1)
        else:
            # Start generating the root certificate
            self._generate_root_certificate()
            log.info(f"CA Hierarchy generated: {self.name}")

    def _generate_root_certificate(self):
        """Generate a new root CA certificate."""
        self.root_key = rsa.generate_private_key(
            public_exponent=65537, key_size=2048
        )
        rand_suffix = "".join(
            random.choices(string.ascii_letters + string.digits, k=16)
        )
        root_cert_subject = x509.Name(
            [
                x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                x509.NameAttribute(
                    NameOID.COMMON_NAME, "TC_PAVS root " + rand_suffix
                ),
            ]
        )
        self.root_cert = (
            x509.CertificateBuilder()
            .subject_name(root_cert_subject)
            .issuer_name(root_cert_subject)
            .public_key(self.root_key.public_key())
            .serial_number(x509.random_serial_number())
            .not_valid_before(datetime.datetime.now(datetime.timezone.utc))
            .not_valid_after(
                datetime.datetime.now(datetime.timezone.utc) + self.default_ca_duration
            )
            .add_extension(
                # We make it so that our root can only issue leaf certificates, no intermediate here.
                x509.BasicConstraints(ca=True, path_length=0), critical=True
            )
            .add_extension(
                x509.KeyUsage(
                    digital_signature=True,
                    content_commitment=False,
                    key_encipherment=False,
                    data_encipherment=False,
                    key_agreement=False,
                    key_cert_sign=True,
                    crl_sign=True,
                    encipher_only=False,
                    decipher_only=False,
                ),
                critical=True,
            )
            .add_extension(
                x509.SubjectKeyIdentifier.from_public_key(
                    self.root_key.public_key()
                ),
                critical=False,
            )
            .sign(self.root_key, hashes.SHA256())
        )
        self._save_cert("root", self.root_cert, self.root_key, False)

    def _save_cert(
        self,
        name: str,
        cert: x509.Certificate,
        key: Optional[CertificateIssuerPrivateKeyTypes],
        bundle_root: bool,
    ) -> tuple[Optional[Path], Path]:
        """
        Private method that help with saving certificate and key to the hierarchy folder.
        This tool isn't meant to be used in production, but instead to help with development
        and as such have the goal to make the CA hierarchy as available as possible, which in
        turn make it very unsecure.
        """
        cert_path = self.directory / f"{name}.pem"
        key_path = self.directory / f"{name}.key" if key else None

        if key and key_path:
            with open(key_path, "wb") as f:
                f.write(
                    key.private_bytes(
                        encoding=serialization.Encoding.PEM,
                        format=serialization.PrivateFormat.TraditionalOpenSSL,
                        encryption_algorithm=serialization.NoEncryption(),
                    )
                )

        with open(cert_path, "wb") as f:
            f.write(cert.public_bytes(serialization.Encoding.PEM))
            if bundle_root:
                f.write(b"\n")
                f.write(self.root_cert.public_bytes(serialization.Encoding.PEM))

        return (key_path, cert_path)

    def _sign_cert(
        self,
        dns: str,
        public_key: CertificatePublicKeyTypes,
        duration: datetime.timedelta,
        ip_address: Optional[str] = None
    ) -> x509.Certificate:
        """
        Generate and sign a certificate.
        """
        # Use ip_address for Common Name if provided, otherwise use dns
        common_name = ip_address if ip_address else dns

        # Sign certificate
        subject = x509.Name(
            [
                x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME, "TC_PAVS"),
                x509.NameAttribute(NameOID.COMMON_NAME, common_name),
            ]
        )

        extended_key_usage = [ExtendedKeyUsageOID.CLIENT_AUTH] if self.kind == "client" else [
            ExtendedKeyUsageOID.SERVER_AUTH]

        builder = (x509.CertificateBuilder()
                   .subject_name(subject)
                   .issuer_name(self.root_cert.subject)
                   .public_key(public_key)
                   .serial_number(x509.random_serial_number())
                   .not_valid_before(datetime.datetime.now(datetime.timezone.utc))
                   .not_valid_after(
            datetime.datetime.now(datetime.timezone.utc) + duration
        )
            .add_extension(
                x509.BasicConstraints(ca=False, path_length=None),
                critical=False,
        )
            .add_extension(
                self.client_key_usage_cert if self.kind == "client" else self.server_key_usage_cert,
                critical=True,
        )
            .add_extension(
                x509.ExtendedKeyUsage(extended_key_usage),
                critical=False,
        )
            .add_extension(
                x509.SubjectKeyIdentifier.from_public_key(public_key),
                critical=False,
        )
            .add_extension(
                x509.AuthorityKeyIdentifier.from_issuer_subject_key_identifier(
                    self.root_cert.extensions.get_extension_for_class(
                        x509.SubjectKeyIdentifier
                    ).value
                ),
                critical=False,
        )
            .add_extension(x509.CRLDistributionPoints([x509.DistributionPoint(
                full_name=[x509.UniformResourceIdentifier("http://not.a.valid.website.com/some/path/to/a.crl")],
                relative_name=None,
                reasons=None,
                crl_issuer=None
            )]), critical=False)
        )

        if self.kind == 'server':
            san_names: list[x509.DNSName | x509.IPAddress] = [x509.DNSName(dns)]
            if ip_address:
                san_names.append(x509.IPAddress(ipaddress.ip_address(ip_address)))
            builder.add_extension(
                x509.SubjectAlternativeName(san_names),
                critical=False,
            )

        return builder.sign(self.root_key, hashes.SHA256())

    def gen_cert(self, dns: str, csr: str, override=False, duration: datetime.timedelta = datetime.timedelta(hours=1)) -> tuple[Path, Path, bool]:
        """
        Generate a certificate signed by this CA hierarchy using the provided CSR.
        Returns the path to the key, cert, and whether it was reused or not.
        """
        signing_request = x509.load_pem_x509_csr(csr.encode('utf-8'))
        signing_request.public_key()

        # If we don't always override, first check if an existing keypair already exists
        if not override:
            cert_path = self.directory / f"{dns}.pem"
            key_path = self.directory / f"{dns}.key"
            if cert_path.exists() and key_path.exists():
                return (key_path, cert_path, True)

        # Sign certificate
        cert = self._sign_cert(dns, signing_request.public_key(), duration)

        # Save that information to disk
        (key_path, cert_bundle_path) = self._save_cert(
            dns, cert, None, bundle_root=True
        )
        log.debug("leaf generated. dns=%s; path=%s", dns, cert_bundle_path)
        return (key_path, cert_bundle_path, False)

    def gen_keypair(self, dns: str,
                    override=False,
                    duration: datetime.timedelta = datetime.timedelta(hours=1),
                    ip_address: Optional[str] = None) -> tuple[Path, Path, bool]:
        """
        Generate a private key as well as the associated certificate signed by this CA
        hierarchy. Returns the path to the key, cert, and whether it was reused or not.
        """
        # If we don't always override, first check if an existing keypair already exists
        if not override:
            cert_path = self.directory / f"{dns}.pem"
            key_path = self.directory / f"{dns}.key"
            if cert_path.exists() and key_path.exists():
                cert = x509.load_pem_x509_certificate(cert_path.read_bytes())
                if datetime.datetime.now(datetime.timezone.utc) < cert.not_valid_after:
                    # We only reuse the certificate/key if the cert is still valid
                    return (key_path, cert_path, True)

        # Generate private key
        key = rsa.generate_private_key(public_exponent=65537, key_size=2048)

        # Sign certificate
        cert = self._sign_cert(dns, key.public_key(), duration, ip_address=ip_address)

        # Save that information to disk
        (key_path, cert_bundle_path) = self._save_cert(dns, cert, key, bundle_root=True)
        if key_path is None:
            raise ValueError("Key path should always be set")
        log.debug("leaf generated. dns=%s; path=%s", dns, cert_bundle_path)
        return (key_path, cert_bundle_path, False)
