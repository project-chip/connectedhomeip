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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --camera-test-videosrc
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import contextlib
import datetime
import http.server
import ipaddress
import logging
import os
import random
import ssl
import struct
import subprocess
import sys
import tempfile
import threading
import time
from dataclasses import dataclass

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.x509.oid import ExtendedKeyUsageOID, NameOID
from mobly import asserts
from TC_TLS_Utils import TLSUtils

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


@dataclass(frozen=True)
class _RGBFrame:
    data: bytes
    width: int
    height: int


class _LocalCMAFIngestServer:
    """Lightweight in-process TLS HTTPS server to capture CMAF H.264 video uploads from PushAVStreamTransport."""

    def __init__(self, host_ip: str = "127.0.0.1") -> None:
        self.host_ip = host_ip
        self._lock = threading.Lock()
        self._init_bytes: bytes | None = None
        self._m4s_segments: list[bytes] = []
        self._tmpdir = tempfile.TemporaryDirectory(prefix="tc_zonemgmt_privacy_tls_")

        self._server_ca_key, self._server_ca_cert = self._make_root_ca("TC_ZONEMGMT_Privacy Server Root")
        self._device_ca_key, self._device_ca_cert = self._make_root_ca("TC_ZONEMGMT_Privacy Device Root")
        server_leaf_key, server_leaf_cert = self._make_server_cert()

        server_cert_path = os.path.join(self._tmpdir.name, "server.pem")
        server_key_path = os.path.join(self._tmpdir.name, "server.key")
        device_ca_path = os.path.join(self._tmpdir.name, "device_ca.pem")

        with open(server_cert_path, "wb") as f:
            f.write(server_leaf_cert.public_bytes(serialization.Encoding.PEM))
            f.write(b"\n")
            f.write(self._server_ca_cert.public_bytes(serialization.Encoding.PEM))
        with open(server_key_path, "wb") as f:
            f.write(
                server_leaf_key.private_bytes(
                    encoding=serialization.Encoding.PEM,
                    format=serialization.PrivateFormat.TraditionalOpenSSL,
                    encryption_algorithm=serialization.NoEncryption(),
                )
            )
        with open(device_ca_path, "wb") as f:
            f.write(self._device_ca_cert.public_bytes(serialization.Encoding.PEM))

        parent = self

        class _Handler(http.server.BaseHTTPRequestHandler):
            protocol_version = "HTTP/1.1"

            def log_message(self, fmt: str, *args) -> None:
                msg = fmt % args if args else fmt
                log.debug("CMAFIngestServer: %s", msg)

            def do_PUT(self) -> None:
                transfer_encoding = self.headers.get("Transfer-Encoding", "").lower()
                if "chunked" in transfer_encoding:
                    chunks: list[bytes] = []
                    while True:
                        chunk_header = self.rfile.readline().strip()
                        if not chunk_header:
                            break
                        chunk_size = int(chunk_header.split(b";", 1)[0], 16)
                        if chunk_size == 0:
                            while self.rfile.readline().strip():
                                pass
                            break
                        chunks.append(self.rfile.read(chunk_size))
                        self.rfile.read(2)
                    body = b"".join(chunks)
                else:
                    length = int(self.headers.get("Content-Length", "0"))
                    body = self.rfile.read(length) if length > 0 else b""
                parent._record_upload(self.path, body)
                self.send_response(201)
                self.send_header("Content-Length", "0")
                self.end_headers()

        self._httpd = http.server.ThreadingHTTPServer((self.host_ip, 0), _Handler)
        ssl_ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ssl_ctx.load_cert_chain(certfile=server_cert_path, keyfile=server_key_path)
        ssl_ctx.load_verify_locations(cafile=device_ca_path)
        ssl_ctx.verify_mode = ssl.CERT_OPTIONAL
        self._httpd.socket = ssl_ctx.wrap_socket(self._httpd.socket, server_side=True)
        self.port: int = self._httpd.server_address[1]
        self._thread = threading.Thread(target=self._httpd.serve_forever, daemon=True)
        self._thread.start()

    @staticmethod
    def _make_root_ca(common_name: str):
        key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
        now = datetime.datetime.now(datetime.UTC)
        subject = x509.Name(
            [
                x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                x509.NameAttribute(NameOID.COMMON_NAME, common_name),
            ]
        )
        cert = (
            x509.CertificateBuilder()
            .subject_name(subject)
            .issuer_name(subject)
            .public_key(key.public_key())
            .serial_number(x509.random_serial_number())
            .not_valid_before(now - datetime.timedelta(minutes=5))
            .not_valid_after(now + datetime.timedelta(days=365))
            .add_extension(x509.BasicConstraints(ca=True, path_length=0), critical=True)
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
            .add_extension(x509.SubjectKeyIdentifier.from_public_key(key.public_key()), critical=False)
            .sign(key, hashes.SHA256())
        )
        return key, cert

    def _make_server_cert(self):
        key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
        now = datetime.datetime.now(datetime.UTC)
        subject = x509.Name(
            [
                x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                x509.NameAttribute(NameOID.COMMON_NAME, self.host_ip),
            ]
        )
        san_entries: list[x509.GeneralName] = [x509.DNSName("localhost")]
        try:
            san_entries.append(x509.IPAddress(ipaddress.ip_address(self.host_ip)))
        except ValueError:
            if self.host_ip != "localhost":
                san_entries.append(x509.DNSName(self.host_ip))
        ski = self._server_ca_cert.extensions.get_extension_for_class(x509.SubjectKeyIdentifier).value
        cert = (
            x509.CertificateBuilder()
            .subject_name(subject)
            .issuer_name(self._server_ca_cert.subject)
            .public_key(key.public_key())
            .serial_number(x509.random_serial_number())
            .not_valid_before(now - datetime.timedelta(minutes=5))
            .not_valid_after(now + datetime.timedelta(days=1))
            .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=False)
            .add_extension(
                x509.KeyUsage(
                    digital_signature=True,
                    content_commitment=False,
                    key_encipherment=True,
                    data_encipherment=False,
                    key_agreement=False,
                    key_cert_sign=False,
                    crl_sign=False,
                    encipher_only=False,
                    decipher_only=False,
                ),
                critical=True,
            )
            .add_extension(x509.ExtendedKeyUsage([ExtendedKeyUsageOID.SERVER_AUTH]), critical=False)
            .add_extension(x509.SubjectKeyIdentifier.from_public_key(key.public_key()), critical=False)
            .add_extension(x509.AuthorityKeyIdentifier.from_issuer_subject_key_identifier(ski), critical=False)
            .add_extension(
                x509.SubjectAlternativeName(san_entries),
                critical=False,
            )
            .sign(self._server_ca_key, hashes.SHA256())
        )
        return key, cert

    def get_server_root_der(self) -> bytes:
        return self._server_ca_cert.public_bytes(serialization.Encoding.DER)

    def sign_device_csr(self, csr_der: bytes) -> bytes:
        csr = x509.load_der_x509_csr(csr_der)
        now = datetime.datetime.now(datetime.UTC)
        subject = x509.Name(
            [
                x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                x509.NameAttribute(NameOID.COMMON_NAME, "DUT"),
            ]
        )
        ski = self._device_ca_cert.extensions.get_extension_for_class(x509.SubjectKeyIdentifier).value
        cert = (
            x509.CertificateBuilder()
            .subject_name(subject)
            .issuer_name(self._device_ca_cert.subject)
            .public_key(csr.public_key())
            .serial_number(x509.random_serial_number())
            .not_valid_before(now - datetime.timedelta(minutes=5))
            .not_valid_after(now + datetime.timedelta(days=1))
            .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=False)
            .add_extension(
                x509.KeyUsage(
                    digital_signature=True,
                    content_commitment=False,
                    key_encipherment=True,
                    data_encipherment=False,
                    key_agreement=False,
                    key_cert_sign=False,
                    crl_sign=False,
                    encipher_only=False,
                    decipher_only=False,
                ),
                critical=True,
            )
            .add_extension(x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH]), critical=False)
            .add_extension(x509.SubjectKeyIdentifier.from_public_key(csr.public_key()), critical=False)
            .add_extension(x509.AuthorityKeyIdentifier.from_issuer_subject_key_identifier(ski), critical=False)
            .sign(self._device_ca_key, hashes.SHA256())
        )
        return cert.public_bytes(serialization.Encoding.DER)

    def _record_upload(self, path: str, body: bytes) -> None:
        if not body:
            return
        with self._lock:
            if path.endswith(".init") or path.endswith(".mp4"):
                self._init_bytes = body
            elif path.endswith(".m4s"):
                self._m4s_segments.append(body)

    def reset_segments(self) -> None:
        with self._lock:
            self._init_bytes = None
            self._m4s_segments.clear()

    def get_first_cmaf_segment(self) -> bytes | None:
        with self._lock:
            if self._init_bytes is None or not self._m4s_segments:
                return None
            return self._init_bytes + self._m4s_segments[0]

    def close(self) -> None:
        with contextlib.suppress(Exception):
            self._httpd.shutdown()
            self._httpd.server_close()
            self._thread.join(timeout=2.0)
        with contextlib.suppress(Exception):
            self._tmpdir.cleanup()


def _extract_cmaf_dimensions(cmaf_bytes: bytes, fallback_width: int, fallback_height: int) -> tuple[int, int]:
    """Extract encoded track dimensions from the ISO BMFF tkhd box in CMAF init bytes."""
    idx = cmaf_bytes.find(b"tkhd")
    if idx >= 4:
        box_start = idx - 4
        box_size = struct.unpack(">I", cmaf_bytes[box_start:idx])[0]
        box_end = box_start + box_size
        if box_size in (92, 104) and box_end <= len(cmaf_bytes):
            w_fixed, h_fixed = struct.unpack(">II", cmaf_bytes[box_end - 8 : box_end])
            w = w_fixed >> 16
            h = h_fixed >> 16
            if w > 0 and h > 0:
                return w, h
    return fallback_width, fallback_height


def _decode_cmaf_h264_to_rgb_frames(cmaf_bytes: bytes, fallback_width: int, fallback_height: int) -> list[_RGBFrame]:
    """Decode all H.264 frames of a CMAF (init + m4s) segment into raw RGB24 _RGBFrame objects."""
    width, height = _extract_cmaf_dimensions(cmaf_bytes, fallback_width, fallback_height)

    with tempfile.NamedTemporaryFile(suffix=".mp4") as tmp_mp4:
        tmp_mp4.write(cmaf_bytes)
        tmp_mp4.flush()

        with contextlib.suppress(FileNotFoundError, subprocess.CalledProcessError, ValueError):
            probe = subprocess.run(
                [
                    "ffprobe",
                    "-v",
                    "error",
                    "-select_streams",
                    "v:0",
                    "-show_entries",
                    "stream=width,height",
                    "-of",
                    "csv=p=0:s=x",
                    tmp_mp4.name,
                ],
                capture_output=True,
                check=True,
                text=True,
            )
            dims = probe.stdout.strip().splitlines()
            if dims and "x" in dims[0]:
                pw, ph = (int(v) for v in dims[0].split("x", 1))
                if pw > 0 and ph > 0:
                    width, height = pw, ph

        frame_len = width * height * 3

        with contextlib.suppress(FileNotFoundError, subprocess.CalledProcessError):
            proc = subprocess.run(
                [
                    "ffmpeg",
                    "-v",
                    "error",
                    "-i",
                    tmp_mp4.name,
                    "-vf",
                    f"scale={width}:{height}",
                    "-f",
                    "rawvideo",
                    "-pix_fmt",
                    "rgb24",
                    "pipe:1",
                ],
                capture_output=True,
                check=True,
            )
            if len(proc.stdout) >= frame_len and len(proc.stdout) % frame_len == 0:
                return [
                    _RGBFrame(data=proc.stdout[offset : offset + frame_len], width=width, height=height)
                    for offset in range(0, len(proc.stdout), frame_len)
                ]

        proc = subprocess.run(
            [
                "gst-launch-1.0",
                "-q",
                "filesrc",
                f"location={tmp_mp4.name}",
                "!",
                "decodebin",
                "!",
                "videoconvert",
                "!",
                "videoscale",
                "!",
                f"video/x-raw,format=RGB,width={width},height={height}",
                "!",
                "fdsink",
            ],
            capture_output=True,
            check=True,
        )
        raw_out = proc.stdout
        row_bytes = width * 3
        padded_stride = (row_bytes + 3) & ~3
        padded_frame_len = padded_stride * height
        if padded_stride != row_bytes and len(raw_out) >= padded_frame_len and len(raw_out) % padded_frame_len == 0:
            frames: list[_RGBFrame] = []
            for offset in range(0, len(raw_out), padded_frame_len):
                chunk = raw_out[offset : offset + padded_frame_len]
                unpadded = b"".join(chunk[r * padded_stride : r * padded_stride + row_bytes] for r in range(height))
                frames.append(_RGBFrame(data=unpadded, width=width, height=height))
            return frames

        asserts.assert_greater_equal(len(raw_out), frame_len, "Decoded H.264 RGB24 output shorter than one frame")
        asserts.assert_equal(len(raw_out) % frame_len, 0, "Decoded H.264 RGB24 byte length is not a whole number of frames")
        return [
            _RGBFrame(data=raw_out[offset : offset + frame_len], width=width, height=height)
            for offset in range(0, len(raw_out), frame_len)
        ]


def _decode_jpeg_to_rgb(jpeg_bytes: bytes, expected_width: int, expected_height: int) -> _RGBFrame:
    """Decode JPEG bytes into an _RGBFrame containing raw RGB24 bytes (height * width * 3)."""
    asserts.assert_true(jpeg_bytes.startswith(b"\xff\xd8"), "Captured snapshot does not start with JPEG SOI marker")
    asserts.assert_true(jpeg_bytes.endswith(b"\xff\xd9"), "Captured snapshot does not end with JPEG EOI marker")
    expected_len = expected_width * expected_height * 3

    with contextlib.suppress(ImportError):
        import io

        from PIL import Image

        with Image.open(io.BytesIO(jpeg_bytes)) as img:
            rgb = img.convert("RGB")
            asserts.assert_equal(rgb.size, (expected_width, expected_height), "Decoded JPEG dimensions mismatch")
            raw = rgb.tobytes()
            asserts.assert_equal(len(raw), expected_len, "Decoded RGB24 byte length mismatch")
            return _RGBFrame(data=raw, width=expected_width, height=expected_height)

    # Fallback 1: ffmpeg rawvideo rgb24 pipe
    with contextlib.suppress(FileNotFoundError, subprocess.CalledProcessError):
        proc = subprocess.run(
            [
                "ffmpeg",
                "-v",
                "error",
                "-f",
                "image2pipe",
                "-vcodec",
                "mjpeg",
                "-i",
                "pipe:0",
                "-f",
                "rawvideo",
                "-pix_fmt",
                "rgb24",
                "pipe:1",
            ],
            input=jpeg_bytes,
            capture_output=True,
            check=True,
        )
        if len(proc.stdout) == expected_len:
            return _RGBFrame(data=proc.stdout, width=expected_width, height=expected_height)

    # Fallback 2: gst-launch-1.0 jpegdec pipe
    proc = subprocess.run(
        [
            "gst-launch-1.0",
            "-q",
            "fdsrc",
            "!",
            "jpegdec",
            "!",
            "videoconvert",
            "!",
            f"video/x-raw,format=RGB,width={expected_width},height={expected_height}",
            "!",
            "fdsink",
        ],
        input=jpeg_bytes,
        capture_output=True,
        check=True,
    )
    asserts.assert_equal(len(proc.stdout), expected_len, "Decoded RGB24 byte length mismatch from gst-launch-1.0")
    return _RGBFrame(data=proc.stdout, width=expected_width, height=expected_height)


class TC_ZONEMGMT_Privacy(MatterTestCommissionedDevice):
    def desc_TC_ZONEMGMT_Privacy(self) -> str:
        return "[TC-ZONEMGMT-Privacy] Verify Privacy Zone polygonal pixel black-out and lifecycle with Server as DUT"

    def pics_TC_ZONEMGMT_Privacy(self) -> list[str]:
        return ["ZONEMGMT.S", "AVSM.S"]

    def steps_TC_ZONEMGMT_Privacy(self) -> list[TestStep]:
        return [
            TestStep(
                "1",
                "Read ZoneManagement and CameraAVStreamManagement attributes and allocate snapshot and video streams.",
                "TwoDCartesianMax is valid and snapshot/video streams are allocated.",
                is_commissioning=True,
            ),
            TestStep(
                "2",
                "Capture baseline snapshot with no Privacy Zones active and decode JPEG pixels.",
                "Pixels in target regions A, B, and C are non-black.",
            ),
            TestStep(
                "3",
                "Create a rectangular Privacy Zone over Region A, capture a snapshot, and inspect decoded pixels.",
                "All pixels strictly inside Region A are black (R, G, B <= 8) while pixels outside (Regions B and C) remain non-black.",
            ),
            TestStep(
                "4",
                "Verify negative trigger and focus interactions on Privacy Zones (CreateOrUpdateTrigger on Privacy Zone, UpdateTwoDCartesianZone to Focus without FocusZones feature, and UpdateTwoDCartesianZone on triggered Motion Zone to Privacy).",
                "DUT returns ConstraintError and InvalidInState as required by specification.",
            ),
            TestStep(
                "5",
                "Update the Privacy Zone via UpdateTwoDCartesianZone to disjoint Region B, verify changing its use to Motion removes privacy masking, and restore it to Privacy.",
                "Pixels inside Region B are black when use is Privacy, non-black when updated to Motion, and black again when updated back to Privacy, while former Region A and control Region C remain non-black.",
            ),
            TestStep(
                "6",
                "Create a second concave L-shaped Privacy Zone over Region A, capture a snapshot (and a live H.264 video segment if PushAV is supported), and inspect decoded pixels.",
                "Both L-shape arms in Region A and Region B are black, while the concave cutout in Region A and control Region C remain non-black in both snapshot and H.264 video frames.",
            ),
            TestStep(
                "7",
                "Remove all Privacy Zones via RemoveZone, capture a snapshot (and a live H.264 video segment if PushAV is supported), and verify unmasked restoration.",
                "Pixels in Regions A, B, and C are restored to non-black in both snapshot and H.264 video frames.",
            ),
        ]

    @staticmethod
    def _pixel_to_zone_vertex(px: int, py: int, width: int, height: int, max_x: int, max_y: int):
        zx = int(round(px * max_x / (width - 1)))
        zy = int(round(py * max_y / (height - 1)))
        zx = max(0, min(max_x, zx))
        zy = max(0, min(max_y, zy))
        return Clusters.ZoneManagement.Structs.TwoDCartesianVertexStruct(x=zx, y=zy)

    @staticmethod
    def _assert_region_is_black(frame: _RGBFrame, x0: int, y0: int, x1: int, y1: int, label: str):
        asserts.assert_greater(x1, x0, f"Empty X range for {label}")
        asserts.assert_greater(y1, y0, f"Empty Y range for {label}")
        max_val = 0
        stride = frame.width * 3
        for y in range(y0, y1):
            row_start = y * stride + x0 * 3
            row_end = y * stride + x1 * 3
            row_max = max(frame.data[row_start:row_end])
            if row_max > max_val:
                max_val = row_max
        log.info("Region '%s' [x=%d..%d, y=%d..%d] max RGB channel value = %d", label, x0, x1, y0, y1, max_val)
        asserts.assert_less_equal(
            max_val,
            8,
            f"Expected all pixels in '{label}' [x={x0}..{x1}, y={y0}..{y1}] to be black (R,G,B <= 8), but found max channel {max_val}",
        )

    @staticmethod
    def _assert_region_is_non_black(frame: _RGBFrame, x0: int, y0: int, x1: int, y1: int, label: str):
        asserts.assert_greater(x1, x0, f"Empty X range for {label}")
        asserts.assert_greater(y1, y0, f"Empty Y range for {label}")
        min_brightest_channel = 255
        total_sum = 0
        total_channels = (x1 - x0) * (y1 - y0) * 3
        stride = frame.width * 3
        mv = memoryview(frame.data)
        for y in range(y0, y1):
            row_offset = y * stride
            for x in range(x0, x1):
                idx = row_offset + x * 3
                r = mv[idx]
                g = mv[idx + 1]
                b = mv[idx + 2]
                total_sum += r + g + b
                px_max = r if r >= g and r >= b else (g if g >= b else b)
                if px_max < min_brightest_channel:
                    min_brightest_channel = px_max
        mean_val = total_sum / total_channels
        log.info(
            "Region '%s' [x=%d..%d, y=%d..%d] min(max(R,G,B)) = %d, mean = %.1f",
            label,
            x0,
            x1,
            y0,
            y1,
            min_brightest_channel,
            mean_val,
        )
        asserts.assert_greater(
            min_brightest_channel,
            20,
            f"Expected pixels in '{label}' [x={x0}..{x1}, y={y0}..{y1}] to be non-black (> 20), but found min brightest channel {min_brightest_channel}",
        )

    async def _capture_and_decode_rgb(self, endpoint: int, stream_id: int, resolution) -> _RGBFrame:
        avsm_commands = Clusters.CameraAvStreamManagement.Commands
        response = await self.send_single_cmd(
            cmd=avsm_commands.CaptureSnapshot(
                snapshotStreamID=stream_id,
                requestedResolution=resolution,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )
        asserts.assert_greater(len(response.data), 0, "CaptureSnapshot returned empty data")
        asserts.assert_equal(response.resolution.width, resolution.width, "Snapshot width mismatch")
        asserts.assert_equal(response.resolution.height, resolution.height, "Snapshot height mismatch")
        return _decode_jpeg_to_rgb(response.data, resolution.width, resolution.height)

    async def _capture_and_decode_pushav_video_rgb(
        self,
        endpoint: int,
        stream_usage,
        video_stream_id: int,
        tls_endpoint_id: int,
        cmaf_server: _LocalCMAFIngestServer,
        vid_w: int,
        vid_h: int,
    ) -> list[_RGBFrame]:
        pavst_cluster = Clusters.PushAvStreamTransport
        cmaf_server.reset_segments()

        alloc_push_resp = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=pavst_cluster.Commands.AllocatePushTransport(
                transportOptions={
                    "streamUsage": stream_usage,
                    "videoStreamID": video_stream_id,
                    "TLSEndpointID": tls_endpoint_id,
                    "url": f"https://{cmaf_server.host_ip}:{cmaf_server.port}/streams/1/",
                    "triggerOptions": {
                        "triggerType": pavst_cluster.Enums.TransportTriggerTypeEnum.kCommand,
                        "maxPreRollLen": 0,
                    },
                    "ingestMethod": pavst_cluster.Enums.IngestMethodsEnum.kCMAFIngest,
                    "containerOptions": {
                        "containerType": pavst_cluster.Enums.ContainerFormatEnum.kCmaf,
                        "CMAFContainerOptions": {
                            "CMAFInterface": pavst_cluster.Enums.CMAFInterfaceEnum.kInterface1,
                            "chunkDuration": 500,
                            "segmentDuration": 1000,
                            "sessionGroup": 1,
                            "trackName": "media",
                        },
                    },
                    "expiryTime": 150,
                }
            ),
        )
        connection_id = alloc_push_resp.transportConfiguration.connectionID

        try:
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=pavst_cluster.Commands.SetTransportStatus(
                    connectionID=connection_id,
                    transportStatus=pavst_cluster.Enums.TransportStatusEnum.kActive,
                ),
            )
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=pavst_cluster.Commands.ManuallyTriggerTransport(
                    connectionID=connection_id,
                    activationReason=pavst_cluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
                    timeControl={
                        "initialDuration": 2,
                        "augmentationDuration": 1,
                        "maxDuration": 5,
                        "blindDuration": 0,
                    },
                ),
            )
            deadline = time.monotonic() + 10.0
            cmaf_bytes = None
            while time.monotonic() < deadline:
                cmaf_bytes = cmaf_server.get_first_cmaf_segment()
                if cmaf_bytes is not None:
                    break
                await asyncio.sleep(0.1)
            asserts.assert_is_not_none(cmaf_bytes, "Timed out waiting for CMAF H.264 video upload from PushAVStreamTransport")
            frames = _decode_cmaf_h264_to_rgb_frames(cmaf_bytes, vid_w, vid_h)
            asserts.assert_greater(len(frames), 0, "Decoded CMAF segment contained zero H.264 frames")
            return frames
        finally:
            with contextlib.suppress(InteractionModelError):
                await self.send_single_cmd(
                    endpoint=endpoint,
                    cmd=pavst_cluster.Commands.SetTransportStatus(
                        connectionID=connection_id,
                        transportStatus=pavst_cluster.Enums.TransportStatusEnum.kInactive,
                    ),
                )
            with contextlib.suppress(InteractionModelError):
                await self.send_single_cmd(
                    endpoint=endpoint,
                    cmd=pavst_cluster.Commands.DeallocatePushTransport(connectionID=connection_id),
                )

    @run_if_endpoint_matches(
        lambda wildcard, endpoint: has_cluster(Clusters.ZoneManagement)(wildcard, endpoint)
        and has_cluster(Clusters.CameraAvStreamManagement)(wildcard, endpoint)
    )
    async def test_TC_ZONEMGMT_Privacy(self):
        endpoint = self.get_endpoint()
        zm_cluster = Clusters.ZoneManagement
        zm_attr = zm_cluster.Attributes
        zm_commands = zm_cluster.Commands
        zm_enums = zm_cluster.Enums

        avsm_cluster = Clusters.CameraAvStreamManagement
        avsm_attr = avsm_cluster.Attributes
        avsm_commands = avsm_cluster.Commands

        self.step("1")
        zm_feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.FeatureMap
        )
        two_d_cart_supported = bool(zm_feature_map & zm_cluster.Bitmaps.Feature.kTwoDimensionalCartesianZone)
        user_defined_supported = bool(zm_feature_map & zm_cluster.Bitmaps.Feature.kUserDefined)
        focus_zones_supported = bool(zm_feature_map & zm_cluster.Bitmaps.Feature.kFocusZones)
        per_zone_sense_supported = bool(zm_feature_map & zm_cluster.Bitmaps.Feature.kPerZoneSensitivity)

        asserts.assert_true(
            two_d_cart_supported and user_defined_supported,
            "TwoDCartesianZone and UserDefined features are required for TC_ZONEMGMT_Privacy",
        )

        two_d_max = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.TwoDCartesianMax
        )
        max_x = two_d_max.x
        max_y = two_d_max.y
        log.info("TwoDCartesianMax: (%d, %d)", max_x, max_y)
        asserts.assert_greater(max_x, 0, "TwoDCartesianMax.x must be > 0")
        asserts.assert_greater(max_y, 0, "TwoDCartesianMax.y must be > 0")

        # Clean up any pre-existing triggers and user-defined zones
        existing_triggers = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.Triggers
        )
        for trig in existing_triggers:
            with contextlib.suppress(InteractionModelError):
                await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveTrigger(zoneID=trig.zoneID))

        existing_zones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.Zones
        )
        for z in existing_zones:
            if z.zoneSource == zm_enums.ZoneSourceEnum.kUser:
                with contextlib.suppress(InteractionModelError):
                    await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveZone(zoneID=z.zoneID))

        avsm_feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.FeatureMap
        )
        asserts.assert_true(
            bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kSnapshot),
            "Snapshot feature is required on CameraAVStreamManagement",
        )
        video_supported = bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kVideo)
        watermark_supported = bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kWatermark)
        osd_supported = bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kOnScreenDisplay)

        snapshot_caps = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.SnapshotCapabilities
        )
        asserts.assert_true(len(snapshot_caps) > 0, "SnapshotCapabilities must not be empty")

        stream_id = None
        selected_cap = None
        for cap in snapshot_caps:
            try:
                alloc_args = {
                    "imageCodec": cap.imageCodec,
                    "maxFrameRate": cap.maxFrameRate,
                    "minResolution": cap.resolution,
                    "maxResolution": cap.resolution,
                    "quality": 90,
                }
                if watermark_supported:
                    alloc_args["watermarkEnabled"] = False
                if osd_supported:
                    alloc_args["OSDEnabled"] = False
                resp = await self.send_single_cmd(
                    endpoint=endpoint, cmd=avsm_commands.SnapshotStreamAllocate(**alloc_args)
                )
                stream_id = resp.snapshotStreamID
                selected_cap = cap
                break
            except InteractionModelError as e:
                log.warning("SnapshotStreamAllocate failed for capability %s: %s", cap, e)

        asserts.assert_is_not_none(stream_id, "Failed to allocate snapshot stream")
        snap_w = selected_cap.resolution.width
        snap_h = selected_cap.resolution.height
        log.info("Allocated snapshot stream ID %d at resolution %dx%d", stream_id, snap_w, snap_h)

        # Define test regions in pixel coordinates (for 640x480 or scaled proportionally for other resolutions)
        # Region A: top-left quadrant [80..260, 80..260] (for 640x480)
        # Region B: bottom-right quadrant [360..540, 200..380] (for 640x480)
        # Region C: top-right control region [360..540, 40..140] (for 640x480)
        ax0, ay0 = int(round(80 * snap_w / 640)), int(round(80 * snap_h / 480))
        ax1, ay1 = int(round(260 * snap_w / 640)), int(round(260 * snap_h / 480))
        bx0, by0 = int(round(360 * snap_w / 640)), int(round(200 * snap_h / 480))
        bx1, by1 = int(round(540 * snap_w / 640)), int(round(380 * snap_h / 480))
        cx0, cy0 = int(round(360 * snap_w / 640)), int(round(40 * snap_h / 480))
        cx1, cy1 = int(round(540 * snap_w / 640)), int(round(140 * snap_h / 480))
        inset = max(8, int(round(12 * snap_w / 640)))

        video_stream_id = None
        selected_video_usage = None
        vid_w = snap_w
        vid_h = snap_h
        cmaf_server = None
        tls_utils = None
        tls_caid = None
        tls_ccdid = None
        tls_endpoint_id = None
        created_zone_ids = []

        try:
            if video_supported:
                stream_usages = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.StreamUsagePriorities
                )
                rate_distortion_points = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.RateDistortionTradeOffPoints
                )
                min_viewport_res = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.MinViewportResolution
                )
                sensor_params = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.VideoSensorParams
                )
                asserts.assert_greater(len(stream_usages), 0, "StreamUsagePriorities must not be empty when Video is supported")
                asserts.assert_greater(
                    len(rate_distortion_points), 0, "RateDistortionTradeOffPoints must not be empty when Video is supported"
                )
                selected_video_usage = stream_usages[0]
                compatible_rd_points = [
                    p
                    for p in rate_distortion_points
                    if (
                        min_viewport_res.width <= p.resolution.width <= sensor_params.sensorWidth
                        and min_viewport_res.height <= p.resolution.height <= sensor_params.sensorHeight
                    )
                ]
                selected_rd_point = next(
                    (
                        p
                        for p in compatible_rd_points
                        if p.resolution.width == snap_w and p.resolution.height == snap_h
                    ),
                    compatible_rd_points[0] if compatible_rd_points else rate_distortion_points[0],
                )
                target_video_min_res = (
                    selected_rd_point.resolution
                    if compatible_rd_points
                    else min_viewport_res
                )
                video_alloc_args = {
                    "streamUsage": selected_video_usage,
                    "videoCodec": selected_rd_point.codec,
                    "minFrameRate": min(30, sensor_params.maxFPS),
                    "maxFrameRate": sensor_params.maxFPS,
                    "minResolution": target_video_min_res,
                    "maxResolution": avsm_cluster.Structs.VideoResolutionStruct(
                        width=sensor_params.sensorWidth,
                        height=sensor_params.sensorHeight,
                    ),
                    "minBitRate": selected_rd_point.minBitRate,
                    "maxBitRate": selected_rd_point.minBitRate,
                    "keyFrameInterval": 1000,
                }
                if watermark_supported:
                    video_alloc_args["watermarkEnabled"] = False
                if osd_supported:
                    video_alloc_args["OSDEnabled"] = False
                video_resp = await self.send_single_cmd(
                    endpoint=endpoint, cmd=avsm_commands.VideoStreamAllocate(**video_alloc_args)
                )
                video_stream_id = video_resp.videoStreamID
                asserts.assert_is_not_none(video_stream_id, "Failed to allocate video stream")
                allocated_video_streams = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.AllocatedVideoStreams
                )
                allocated_video = next((s for s in allocated_video_streams if s.videoStreamID == video_stream_id), None)
                asserts.assert_is_not_none(allocated_video, f"Video stream {video_stream_id} missing from AllocatedVideoStreams")
                vid_w = allocated_video.minResolution.width
                vid_h = allocated_video.minResolution.height
                log.info("Allocated video stream ID %d at resolution %dx%d", video_stream_id, vid_w, vid_h)

                ep_servers = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.ServerList
                )
                ep0_servers = await self.read_single_attribute_check_success(
                    endpoint=0, cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.ServerList
                )
                pavst_supported = (
                    Clusters.PushAvStreamTransport.id in ep_servers
                    and Clusters.TlsCertificateManagement.id in ep0_servers
                    and Clusters.TlsClientManagement.id in ep0_servers
                )
                if pavst_supported:
                    pavst_cluster = Clusters.PushAvStreamTransport
                    existing_conns = await self.read_single_attribute_check_success(
                        endpoint=endpoint, cluster=pavst_cluster, attribute=pavst_cluster.Attributes.CurrentConnections
                    )
                    for conn in existing_conns:
                        with contextlib.suppress(InteractionModelError):
                            await self.send_single_cmd(
                                endpoint=endpoint,
                                cmd=pavst_cluster.Commands.DeallocatePushTransport(connectionID=conn.connectionID),
                            )

                    host_ip = self.user_params.get("host_ip", None) or "127.0.0.1"
                    cmaf_server = _LocalCMAFIngestServer(host_ip=host_ip)
                    tls_utils = TLSUtils(self, endpoint=0)
                    prc_res = await tls_utils.send_provision_root_command(certificate=cmaf_server.get_server_root_der())
                    tls_caid = prc_res.caid
                    csr_res = await tls_utils.send_csr_command(nonce=random.randbytes(32))
                    tls_ccdid = csr_res.ccdid
                    await tls_utils.send_provision_client_command(
                        certificate=cmaf_server.sign_device_csr(csr_res.csr), ccdid=tls_ccdid
                    )
                    tls_ep_res = await tls_utils.send_provision_tls_endpoint_command(
                        hostname=cmaf_server.host_ip.encode("utf-8"),
                        port=cmaf_server.port,
                        caid=tls_caid,
                        ccdid=tls_ccdid,
                    )
                    tls_endpoint_id = tls_ep_res.endpointID
                    log.info(
                        "Provisioned PushAV TLS endpoint ID=%d on https://%s:%d/",
                        tls_endpoint_id,
                        cmaf_server.host_ip,
                        cmaf_server.port,
                    )
                else:
                    log.info("DUT does not expose PushAVStreamTransport + TLS clusters; skipping CMAF video decode check")
            else:
                log.info("DUT does not support Video (VDO) feature; skipping video stream allocation")

            self.step("2")
            baseline_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_non_black(baseline_rgb, ax0 + inset, ay0 + inset, ax1 - inset, ay1 - inset, "Baseline Region A")
            self._assert_region_is_non_black(baseline_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Baseline Region B")
            self._assert_region_is_non_black(baseline_rgb, cx0 + inset, cy0 + inset, cx1 - inset, cy1 - inset, "Baseline Region C")

            self.step("3")
            region_a_vertices = [
                self._pixel_to_zone_vertex(ax0, ay0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax1, ay0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax1, ay1, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax0, ay1, snap_w, snap_h, max_x, max_y),
            ]
            create_privacy_cmd = zm_commands.CreateTwoDCartesianZone(
                zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                    name="PrivacyZoneA",
                    use=zm_enums.ZoneUseEnum.kPrivacy,
                    vertices=region_a_vertices,
                    color="#000000",
                )
            )
            create_resp = await self.send_single_cmd(endpoint=endpoint, cmd=create_privacy_cmd)
            privacy_zone_id = create_resp.zoneID
            created_zone_ids.append(privacy_zone_id)
            log.info("Created Privacy Zone ID: %d", privacy_zone_id)

            zones_after_create = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.Zones
            )
            matching_zone = next((z for z in zones_after_create if z.zoneID == privacy_zone_id), None)
            asserts.assert_is_not_none(matching_zone, f"Privacy zone {privacy_zone_id} not found in Zones attribute")
            asserts.assert_equal(matching_zone.zoneType, zm_enums.ZoneTypeEnum.kTwoDCARTZone, "Unexpected zoneType")
            asserts.assert_equal(matching_zone.zoneSource, zm_enums.ZoneSourceEnum.kUser, "Unexpected zoneSource")
            asserts.assert_equal(
                matching_zone.twoDCartesianZone.use, zm_enums.ZoneUseEnum.kPrivacy, "Expected ZoneUseEnum.kPrivacy"
            )

            masked_a_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_black(masked_a_rgb, ax0 + inset, ay0 + inset, ax1 - inset, ay1 - inset, "Masked Region A")
            self._assert_region_is_non_black(
                masked_a_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Unmasked Region B"
            )
            self._assert_region_is_non_black(
                masked_a_rgb, cx0 + inset, cy0 + inset, cx1 - inset, cy1 - inset, "Unmasked Control Region C"
            )

            self.step("4")
            sensitivity = None
            if per_zone_sense_supported:
                sensitivity_max = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.SensitivityMax
                )
                sensitivity = min(4, sensitivity_max)
            privacy_trigger = zm_cluster.Structs.ZoneTriggerControlStruct(
                zoneID=privacy_zone_id,
                initialDuration=10,
                augmentationDuration=2,
                maxDuration=30,
                blindDuration=5,
                sensitivity=sensitivity,
            )
            try:
                await self.send_single_cmd(
                    endpoint=endpoint, cmd=zm_commands.CreateOrUpdateTrigger(trigger=privacy_trigger)
                )
                asserts.fail("Triggers are only allowed on Motion zones; CreateOrUpdateTrigger on a Privacy Zone must fail with ConstraintError")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.ConstraintError,
                    "Triggers are only allowed on Motion zones; CreateOrUpdateTrigger on a Privacy Zone must fail with ConstraintError",
                )

            if not focus_zones_supported:
                try:
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=zm_commands.UpdateTwoDCartesianZone(
                            zoneID=privacy_zone_id,
                            zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                                name="FocusInvalid",
                                use=zm_enums.ZoneUseEnum.kFocus,
                                vertices=region_a_vertices,
                                color="#000000",
                            ),
                        ),
                    )
                    asserts.fail(
                        "Focus use requires the FocusZones feature; UpdateTwoDCartesianZone to Focus without FocusZones must fail with ConstraintError")
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status,
                        Status.ConstraintError,
                        "Focus use requires the FocusZones feature; UpdateTwoDCartesianZone to Focus without FocusZones must fail with ConstraintError",
                    )
            else:
                log.info("DUT supports FocusZones (F03); skipping FocusZones-unsupported negative check")

            # Create a temporary Motion zone, attach an active trigger, and verify updating it to Privacy fails with InvalidInState
            motion_vertices = [
                self._pixel_to_zone_vertex(cx0, cy0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(cx1, cy0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(cx1, cy1, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(cx0, cy1, snap_w, snap_h, max_x, max_y),
            ]
            motion_resp = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=zm_commands.CreateTwoDCartesianZone(
                    zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                        name="TempMotionZone",
                        use=zm_enums.ZoneUseEnum.kMotion,
                        vertices=motion_vertices,
                        color="#00FFFF",
                    )
                ),
            )
            motion_zone_id = motion_resp.zoneID
            created_zone_ids.append(motion_zone_id)

            motion_trigger = zm_cluster.Structs.ZoneTriggerControlStruct(
                zoneID=motion_zone_id,
                initialDuration=10,
                augmentationDuration=2,
                maxDuration=30,
                blindDuration=5,
                sensitivity=sensitivity,
            )
            await self.send_single_cmd(
                endpoint=endpoint, cmd=zm_commands.CreateOrUpdateTrigger(trigger=motion_trigger)
            )
            try:
                await self.send_single_cmd(
                    endpoint=endpoint,
                    cmd=zm_commands.UpdateTwoDCartesianZone(
                        zoneID=motion_zone_id,
                        zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                            name="TempMotionToPrivacy",
                            use=zm_enums.ZoneUseEnum.kPrivacy,
                            vertices=motion_vertices,
                            color="#000000",
                        ),
                    ),
                )
                asserts.fail(
                    "A zone referenced by an active trigger cannot change its use; UpdateTwoDCartesianZone must fail with InvalidInState")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.InvalidInState,
                    "A zone referenced by an active trigger cannot change its use; UpdateTwoDCartesianZone must fail with InvalidInState",
                )
            finally:
                await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveTrigger(zoneID=motion_zone_id))
                await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveZone(zoneID=motion_zone_id))
                created_zone_ids.remove(motion_zone_id)

            self.step("5")
            region_b_vertices = [
                self._pixel_to_zone_vertex(bx0, by0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(bx1, by0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(bx1, by1, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(bx0, by1, snap_w, snap_h, max_x, max_y),
            ]
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=zm_commands.UpdateTwoDCartesianZone(
                    zoneID=privacy_zone_id,
                    zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                        name="PrivacyZoneB",
                        use=zm_enums.ZoneUseEnum.kPrivacy,
                        vertices=region_b_vertices,
                        color="#000000",
                    ),
                ),
            )

            masked_b_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_black(masked_b_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Masked Region B")
            self._assert_region_is_non_black(
                masked_b_rgb, ax0 + inset, ay0 + inset, ax1 - inset, ay1 - inset, "Restored Region A after Update"
            )
            self._assert_region_is_non_black(
                masked_b_rgb, cx0 + inset, cy0 + inset, cx1 - inset, cy1 - inset, "Unmasked Control Region C"
            )

            # Change the existing Privacy zone's use to Motion to exercise the RemovePrivacyZone branch in UpdateTwoDCartesianZone
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=zm_commands.UpdateTwoDCartesianZone(
                    zoneID=privacy_zone_id,
                    zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                        name="MotionZoneB",
                        use=zm_enums.ZoneUseEnum.kMotion,
                        vertices=region_b_vertices,
                        color="#00FF00",
                    ),
                ),
            )

            unmasked_b_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_non_black(
                unmasked_b_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Unmasked Region B after updating use to Motion"
            )

            # Restore zone back to Privacy use on Region B
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=zm_commands.UpdateTwoDCartesianZone(
                    zoneID=privacy_zone_id,
                    zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                        name="PrivacyZoneB",
                        use=zm_enums.ZoneUseEnum.kPrivacy,
                        vertices=region_b_vertices,
                        color="#000000",
                    ),
                ),
            )

            remasked_b_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_black(
                remasked_b_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Re-masked Region B after updating use back to Privacy"
            )

            self.step("6")
            # Create a 6-vertex concave L-shaped Privacy Zone over Region A with cutout in bottom-right [amid_x..ax1, amid_y..ay1]
            amid_x = (ax0 + ax1) // 2
            amid_y = (ay0 + ay1) // 2
            concave_l_vertices = [
                self._pixel_to_zone_vertex(ax0, ay0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax1, ay0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax1, amid_y, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(amid_x, amid_y, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(amid_x, ay1, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax0, ay1, snap_w, snap_h, max_x, max_y),
            ]
            create_l_resp = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=zm_commands.CreateTwoDCartesianZone(
                    zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                        name="PrivacyConcaveL",
                        use=zm_enums.ZoneUseEnum.kPrivacy,
                        vertices=concave_l_vertices,
                        color="#000000",
                    )
                ),
            )
            privacy_zone_2_id = create_l_resp.zoneID
            created_zone_ids.append(privacy_zone_2_id)

            multi_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            # Vertical arm of L-shape: [ax0 + inset .. amid_x - inset, ay0 + inset .. ay1 - inset]
            self._assert_region_is_black(
                multi_rgb, ax0 + inset, ay0 + inset, amid_x - inset, ay1 - inset, "L-shape Vertical Arm"
            )
            # Horizontal arm of L-shape: [ax0 + inset .. ax1 - inset, ay0 + inset .. amid_y - inset]
            self._assert_region_is_black(
                multi_rgb, ax0 + inset, ay0 + inset, ax1 - inset, amid_y - inset, "L-shape Horizontal Arm"
            )
            # Concave cutout of L-shape: [amid_x + inset .. ax1 - inset, amid_y + inset .. ay1 - inset] must remain unmasked
            self._assert_region_is_non_black(
                multi_rgb, amid_x + inset, amid_y + inset, ax1 - inset, ay1 - inset, "L-shape Concave Cutout"
            )
            # Region B (first privacy zone) must also remain black
            self._assert_region_is_black(
                multi_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Simultaneous Masked Region B"
            )

            if video_stream_id is not None:
                allocated_video_streams = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.AllocatedVideoStreams
                )
                asserts.assert_true(
                    any(s.videoStreamID == video_stream_id for s in allocated_video_streams),
                    f"Expected active video stream {video_stream_id} in AllocatedVideoStreams while privacy zones are active",
                )

            if tls_endpoint_id is not None and cmaf_server is not None and video_stream_id is not None:
                video_multi_frames = await self._capture_and_decode_pushav_video_rgb(
                    endpoint, selected_video_usage, video_stream_id, tls_endpoint_id, cmaf_server, vid_w, vid_h
                )
                for frame_idx, video_multi_rgb in enumerate(video_multi_frames):
                    f_w, f_h = video_multi_rgb.width, video_multi_rgb.height
                    vax0, vay0 = int(round(80 * f_w / 640)), int(round(80 * f_h / 480))
                    vax1, vay1 = int(round(260 * f_w / 640)), int(round(260 * f_h / 480))
                    vbx0, vby0 = int(round(360 * f_w / 640)), int(round(200 * f_h / 480))
                    vbx1, vby1 = int(round(540 * f_w / 640)), int(round(380 * f_h / 480))
                    vcx0, vcy0 = int(round(360 * f_w / 640)), int(round(40 * f_h / 480))
                    vcx1, vcy1 = int(round(540 * f_w / 640)), int(round(140 * f_h / 480))
                    vamid_x = (vax0 + vax1) // 2
                    vamid_y = (vay0 + vay1) // 2
                    vinset_x = max(2, min(int(round(12 * f_w / 640)), max(2, (vamid_x - vax0) // 3)))
                    vinset_y = max(2, min(int(round(12 * f_h / 480)), max(2, (vamid_y - vay0) // 3)))

                    self._assert_region_is_black(
                        video_multi_rgb,
                        vax0 + vinset_x,
                        vay0 + vinset_y,
                        vamid_x - vinset_x,
                        vay1 - vinset_y,
                        f"H.264 Video frame[{frame_idx}] L-shape Vertical Arm",
                    )
                    self._assert_region_is_black(
                        video_multi_rgb,
                        vax0 + vinset_x,
                        vay0 + vinset_y,
                        vax1 - vinset_x,
                        vamid_y - vinset_y,
                        f"H.264 Video frame[{frame_idx}] L-shape Horizontal Arm",
                    )
                    self._assert_region_is_non_black(
                        video_multi_rgb,
                        vamid_x + vinset_x,
                        vamid_y + vinset_y,
                        vax1 - vinset_x,
                        vay1 - vinset_y,
                        f"H.264 Video frame[{frame_idx}] L-shape Cutout",
                    )
                    self._assert_region_is_black(
                        video_multi_rgb,
                        vbx0 + vinset_x,
                        vby0 + vinset_y,
                        vbx1 - vinset_x,
                        vby1 - vinset_y,
                        f"H.264 Video frame[{frame_idx}] Masked Region B",
                    )
                    self._assert_region_is_non_black(
                        video_multi_rgb,
                        vcx0 + vinset_x,
                        vcy0 + vinset_y,
                        vcx1 - vinset_x,
                        vcy1 - vinset_y,
                        f"H.264 Video frame[{frame_idx}] Control Region C",
                    )

            self.step("7")
            for zid in list(created_zone_ids):
                await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveZone(zoneID=zid))
                created_zone_ids.remove(zid)

            restored_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_non_black(
                restored_rgb, ax0 + inset, ay0 + inset, ax1 - inset, ay1 - inset, "Restored Region A after RemoveZone"
            )
            self._assert_region_is_non_black(
                restored_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Restored Region B after RemoveZone"
            )
            self._assert_region_is_non_black(
                restored_rgb, cx0 + inset, cy0 + inset, cx1 - inset, cy1 - inset, "Control Region C after RemoveZone"
            )

            if tls_endpoint_id is not None and cmaf_server is not None and video_stream_id is not None:
                video_restored_frames = await self._capture_and_decode_pushav_video_rgb(
                    endpoint, selected_video_usage, video_stream_id, tls_endpoint_id, cmaf_server, vid_w, vid_h
                )
                for frame_idx, video_restored_rgb in enumerate(video_restored_frames):
                    f_w, f_h = video_restored_rgb.width, video_restored_rgb.height
                    vax0, vay0 = int(round(80 * f_w / 640)), int(round(80 * f_h / 480))
                    vax1, vay1 = int(round(260 * f_w / 640)), int(round(260 * f_h / 480))
                    vbx0, vby0 = int(round(360 * f_w / 640)), int(round(200 * f_h / 480))
                    vbx1, vby1 = int(round(540 * f_w / 640)), int(round(380 * f_h / 480))
                    vcx0, vcy0 = int(round(360 * f_w / 640)), int(round(40 * f_h / 480))
                    vcx1, vcy1 = int(round(540 * f_w / 640)), int(round(140 * f_h / 480))
                    vamid_x = (vax0 + vax1) // 2
                    vamid_y = (vay0 + vay1) // 2
                    vinset_x = max(2, min(int(round(12 * f_w / 640)), max(2, (vamid_x - vax0) // 3)))
                    vinset_y = max(2, min(int(round(12 * f_h / 480)), max(2, (vamid_y - vay0) // 3)))

                    self._assert_region_is_non_black(
                        video_restored_rgb,
                        vax0 + vinset_x,
                        vay0 + vinset_y,
                        vax1 - vinset_x,
                        vay1 - vinset_y,
                        f"H.264 Video frame[{frame_idx}] Restored Region A after RemoveZone",
                    )
                    self._assert_region_is_non_black(
                        video_restored_rgb,
                        vbx0 + vinset_x,
                        vby0 + vinset_y,
                        vbx1 - vinset_x,
                        vby1 - vinset_y,
                        f"H.264 Video frame[{frame_idx}] Restored Region B after RemoveZone",
                    )
                    self._assert_region_is_non_black(
                        video_restored_rgb,
                        vcx0 + vinset_x,
                        vcy0 + vinset_y,
                        vcx1 - vinset_x,
                        vcy1 - vinset_y,
                        f"H.264 Video frame[{frame_idx}] Control Region C after RemoveZone",
                    )
        finally:
            tls_cleanup_errors: list[Exception] = []
            for zid in list(created_zone_ids):
                with contextlib.suppress(InteractionModelError):
                    await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveZone(zoneID=zid))
            if tls_utils is not None:
                if tls_endpoint_id is not None:
                    try:
                        await tls_utils.send_remove_tls_endpoint_command(endpoint_id=tls_endpoint_id)
                    except InteractionModelError as err:
                        tls_cleanup_errors.append(err)
                if tls_ccdid is not None:
                    try:
                        await tls_utils.send_remove_client_command(ccdid=tls_ccdid)
                    except InteractionModelError as err:
                        tls_cleanup_errors.append(err)
                if tls_caid is not None:
                    try:
                        await tls_utils.send_remove_root_command(caid=tls_caid)
                    except InteractionModelError as err:
                        tls_cleanup_errors.append(err)
            if cmaf_server is not None:
                cmaf_server.close()
            if video_stream_id is not None:
                with contextlib.suppress(InteractionModelError):
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=avsm_commands.VideoStreamDeallocate(videoStreamID=video_stream_id),
                    )
            if stream_id is not None:
                with contextlib.suppress(InteractionModelError):
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=avsm_commands.SnapshotStreamDeallocate(snapshotStreamID=stream_id),
                    )
            if tls_cleanup_errors and sys.exc_info()[0] is None:
                raise tls_cleanup_errors[0]


if __name__ == "__main__":
    default_matter_test_main()
