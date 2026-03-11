from dataclasses import dataclass, field
from os import PathLike
from re import split
from typing import Any, List, Optional, TextIO, Union

from ..tls import (
    CipherSuite,
    SessionTicket,
    load_pem_private_key,
    load_pem_x509_certificates,
)
from .logger import QuicLogger
from .packet import QuicProtocolVersion

SMALLEST_MAX_DATAGRAM_SIZE = 1200


@dataclass
class QuicConfiguration:
    """
    A QUIC configuration.
    """

    alpn_protocols: Optional[List[str]] = None
    """
    A list of supported ALPN protocols.
    """

    congestion_control_algorithm: str = "reno"
    """
    The name of the congestion control algorithm to use.

    Currently supported algorithms: `"reno", `"cubic"`.
    """

    connection_id_length: int = 8
    """
    The length in bytes of local connection IDs.
    """

    idle_timeout: float = 60.0
    """
    The idle timeout in seconds.

    The connection is terminated if nothing is received for the given duration.
    """

    is_client: bool = True
    """
    Whether this is the client side of the QUIC connection.
    """

    max_data: int = 1048576
    """
    Connection-wide flow control limit.
    """

    max_datagram_size: int = SMALLEST_MAX_DATAGRAM_SIZE
    """
    The maximum QUIC payload size in bytes to send, excluding UDP or IP overhead.
    """

    max_stream_data: int = 1048576
    """
    Per-stream flow control limit.
    """

    quic_logger: Optional[QuicLogger] = None
    """
    The :class:`~aioquic.quic.logger.QuicLogger` instance to log events to.
    """

    secrets_log_file: TextIO = None
    """
    A file-like object in which to log traffic secrets.

    This is useful to analyze traffic captures with Wireshark.
    """

    server_name: Optional[str] = None
    """
    The server name to use when verifying the server's TLS certificate, which
    can either be a DNS name or an IP address.

    If it is a DNS name, it is also sent during the TLS handshake in the
    Server Name Indication (SNI) extension.

    .. note:: This is only used by clients.
    """

    session_ticket: Optional[SessionTicket] = None
    """
    The TLS session ticket which should be used for session resumption.
    """

    token: bytes = b""
    """
    The address validation token that can be used to validate future connections.

    .. note:: This is only used by clients.
    """

    cadata: Optional[bytes] = None
    cafile: Optional[str] = None
    capath: Optional[str] = None
    certificate: Any = None
    certificate_chain: List[Any] = field(default_factory=list)
    cipher_suites: Optional[List[CipherSuite]] = None
    initial_rtt: float = 0.1
    max_datagram_frame_size: Optional[int] = None
    private_key: Any = None
    quantum_readiness_test: bool = False
    supported_versions: List[int] = field(
        default_factory=lambda: [
            QuicProtocolVersion.VERSION_1,
            QuicProtocolVersion.DRAFT_32,
            QuicProtocolVersion.DRAFT_31,
            QuicProtocolVersion.DRAFT_30,
            QuicProtocolVersion.DRAFT_29,
        ]
    )
    verify_mode: Optional[int] = None

    def load_cert_chain(
        self,
        certfile: PathLike,
        keyfile: Optional[PathLike] = None,
        password: Optional[Union[bytes, str]] = None,
    ) -> None:
        """
        Load a private key and the corresponding certificate.
        """
        with open(certfile, "rb") as fp:
            boundary = b"-----BEGIN PRIVATE KEY-----\n"
            chunks = split(b"\n" + boundary, fp.read())
            certificates = load_pem_x509_certificates(chunks[0])
            if len(chunks) == 2:
                private_key = boundary + chunks[1]
                self.private_key = load_pem_private_key(private_key)
        self.certificate = certificates[0]
        self.certificate_chain = certificates[1:]

        if keyfile is not None:
            with open(keyfile, "rb") as fp:
                self.private_key = load_pem_private_key(
                    fp.read(),
                    password=password.encode("utf8")
                    if isinstance(password, str)
                    else password,
                )

    def load_verify_locations(
        self,
        cafile: Optional[str] = None,
        capath: Optional[str] = None,
        cadata: Optional[bytes] = None,
    ) -> None:
        """
        Load a set of "certification authority" (CA) certificates used to
        validate other peers' certificates.
        """
        self.cafile = cafile
        self.capath = capath
        self.cadata = cadata
