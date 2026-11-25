#!/usr/bin/env python3
"""Copyright (c) 2026 Project CHIP Authors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import logging
import os
import re
import shlex
import socket
import struct
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from collections.abc import Iterator
from dataclasses import dataclass

from cirque.nodes.androiddockernode import UiAutomatorHelper
from helper.AndroidCirqueTestBase import (DEFAULT_BOOT_TIMEOUT_SEC, DEFAULT_COMMISSION_TIMEOUT_SEC, AndroidCirqueTestBase,
                                          check_android_preconditions)
from helper.CHIPTestBase import TestResult
from helper.paths import CHIP_REPO_STR

logger = logging.getLogger('AndroidTvCastingTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(logging.Formatter('%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CIRQUE_URL = os.environ.get('CIRQUE_URL', 'http://localhost:5000')
TEST_WIFI_SSID = 'CHIP-VirtualWiFi-AP'
TEST_WIFI_PSK = 'ChipWiFiPassword123'

ANDROID_BOOT_TIMEOUT_SEC = DEFAULT_BOOT_TIMEOUT_SEC
ANDROID_COMMISSION_TIMEOUT_SEC = DEFAULT_COMMISSION_TIMEOUT_SEC

TV_APP_APK_PATH = os.path.join(
    CHIP_REPO_STR, 'out', 'android-x64-tv-server', 'platform-app', 'outputs', 'apk', 'debug', 'platform-app-debug.apk'
)
CONTENT_APP_APK_PATH = os.path.join(
    CHIP_REPO_STR, 'out', 'android-x64-tv-server', 'content-app', 'outputs', 'apk', 'debug', 'content-app-debug.apk'
)
CASTING_APP_APK_PATH = os.path.join(
    CHIP_REPO_STR, 'out', 'android-x64-tv-casting-app', 'outputs', 'apk', 'debug', 'app-debug.apk'
)
ANDROID_RUNNER_IMAGE = os.environ.get('CIRQUE_ANDROID_RUNNER_IMAGE', 'cirque-android-runner:latest')

DEVICE_CONFIG = {
    'device0': {
        'type': 'wifi_ap',
        'base_image': '@default',
        'ssid': TEST_WIFI_SSID,
        'psk': TEST_WIFI_PSK,
        'use_virtual_wifi_tcp': True,
        'capability': ['TrafficControl'],
        'traffic_control': {'latencyMs': 10},
    },
    'device1': {
        'type': 'android_emulator',
        'base_image': ANDROID_RUNNER_IMAGE,
        'capability': ['Bluetooth', 'WiFi', 'Mount'],
        'use_virtual_bt_tcp': True,
        'use_virtual_wifi_tcp': True,
        'wifi_auto_connect': True,
        'ssid': TEST_WIFI_SSID,
        'psk': TEST_WIFI_PSK,
        'tap_interface': 'cirque_tap0',
        'mac_address': '02:15:b2:00:00:01',
        'ipv4_addr': '10.0.1.11',
        'avd_name': 'Pixel_6_API_34',
        'chiptool_apk': str(TV_APP_APK_PATH),
        'extra_apks': [{'apk_path': str(CONTENT_APP_APK_PATH), 'package_name': 'com.example.contentapp'}],
        'mount_pairs': [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device2': {
        'type': 'android_emulator',
        'base_image': ANDROID_RUNNER_IMAGE,
        'capability': ['Bluetooth', 'WiFi', 'Mount'],
        'use_virtual_bt_tcp': True,
        'use_virtual_wifi_tcp': True,
        'wifi_auto_connect': True,
        'ssid': TEST_WIFI_SSID,
        'psk': TEST_WIFI_PSK,
        'tap_interface': 'cirque_tap0',
        'mac_address': '02:15:b2:00:00:02',
        'ipv4_addr': '10.0.1.12',
        'avd_name': 'Pixel_6_API_34',
        'chiptool_apk': str(CASTING_APP_APK_PATH),
        'mount_pairs': [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
}


@dataclass(frozen=True, slots=True)
class UdpSegment:
    """Decoded Ethernet IPv4/IPv6 UDP segment fields.

    Attributes:
        src_mac: Lowercase colon-delimited source Ethernet MAC.
        dst_mac: Lowercase colon-delimited destination Ethernet MAC.
        src_ip: Formatted IPv4 or IPv6 source address.
        dst_ip: Formatted IPv4 or IPv6 destination address.
        src_port: UDP source port.
        dst_port: UDP destination port.
        payload: Raw UDP payload bytes.
    """

    src_mac: str
    dst_mac: str
    src_ip: str
    dst_ip: str
    src_port: int
    dst_port: int
    payload: bytes

    @property
    def is_mdns_port(self) -> bool:
        return self.src_port == 5353 or self.dst_port == 5353

    def __iter__(self) -> Iterator[object]:
        yield self.src_mac
        yield self.dst_mac
        yield self.src_ip
        yield self.dst_ip
        yield self.src_port
        yield self.dst_port
        yield self.payload


@dataclass(frozen=True, slots=True)
class WifiPcapOracles:
    """Counts of Matter casting protocol frames observed in wifi_medium.pcap.

    Attributes:
        mdns_answers: Answer-section PTR records for _matterd._udp.local from TV.
        udc_datagrams: Unencrypted UDC IdentificationDeclaration frames to TV.
        operational_datagrams: Encrypted operational Matter UDP frames between pair.
    """

    mdns_answers: int = 0
    udc_datagrams: int = 0
    operational_datagrams: int = 0

    @property
    def has_mdns_and_udc(self) -> bool:
        return self.mdns_answers > 0 and self.udc_datagrams > 0

    def __iter__(self) -> Iterator[int]:
        yield self.mdns_answers
        yield self.udc_datagrams
        yield self.operational_datagrams


@dataclass(frozen=True, slots=True)
class _L3HeaderSlice:
    """Parsed L3 protocol, IP endpoints, and L4 payload offset."""

    proto: int
    src_ip: str
    dst_ip: str
    l4_off: int

    @property
    def is_udp(self) -> bool:
        return self.proto == 17

    def has_bytes(self, frame_len: int, needed: int) -> bool:
        return frame_len >= self.l4_off + needed


@dataclass(frozen=True, slots=True)
class _MatterPacketHeader:
    """Parsed unencrypted/encrypted Matter message header fields."""

    payload_offset: int
    session_id: int
    sec_flags: int

    @property
    def is_encrypted_session(self) -> bool:
        return self.session_id != 0 or (self.sec_flags & 0x03) != 0

    def privacy_enabled(self) -> bool:
        return bool(self.sec_flags & 0x80)


@dataclass(frozen=True, slots=True)
class _CastingEndpoints:
    """Expected IPv4 and MAC endpoints for TV server and Casting client."""

    tv_ip: str | None = None
    casting_ip: str | None = None
    tv_mac: str | None = None
    casting_mac: str | None = None

    def is_from_tv(self, udp: UdpSegment) -> bool:
        return _endpoint_matches(udp.src_ip, udp.src_mac, expected_ip=self.tv_ip, expected_mac=self.tv_mac)

    def is_casting_to_tv(self, udp: UdpSegment) -> bool:
        return _endpoint_matches(
            udp.src_ip, udp.src_mac, expected_ip=self.casting_ip, expected_mac=self.casting_mac
        ) and _endpoint_matches(udp.dst_ip, udp.dst_mac, expected_ip=self.tv_ip, expected_mac=self.tv_mac)


def _parse_ui_hierarchy_root(xml_content: str) -> ET.Element | None:
    start = xml_content.find('<?xml')
    end = xml_content.rfind('</hierarchy>')
    if start < 0 or end < start:
        return None
    try:
        return ET.fromstring(xml_content[start:end + len('</hierarchy>')])
    except ET.ParseError:
        return None


def casting_ui_shows_connected(xml_content):
    """Returns True if a casting app uiautomator dump shows connection success."""
    root = _parse_ui_hierarchy_root(xml_content)
    if root is None:
        return False
    for node in root.iter('node'):
        resource_id = node.get('resource-id', '')
        if resource_id.endswith('connectionFragmentStatusText') and 'Successfully connected' in node.get('text', ''):
            return True
        if resource_id.endswith('connectionFragmentNextButton') and node.get('enabled') == 'true':
            return True
    return False


def casting_ui_shows_discovered(xml_content):
    """Returns True if uiautomator dump shows a discovered CastingPlayer item."""
    root = _parse_ui_hierarchy_root(xml_content)
    if root is None:
        return False
    return any(
        node.get('resource-id', '').endswith('commissionable_player_description')
        and bool(node.get('text', '').strip())
        for node in root.iter('node')
    )


def connectivity_has_wifi_connected(dumpsys_output):
    """Returns True if dumpsys connectivity shows an active WIFI network."""
    active_section = re.split(
        r'\n\s*(?:Network Connectivity Log|ConnectivityService events|mEventLog|Historical)',
        dumpsys_output,
        maxsplit=1,
    )[0]
    return bool(
        re.search(
            r'(?:NetworkAgentInfo\{[^\n]*\bWIFI\b[^\n]*\bCONNECTED\b|'
            r'NetworkAgentInfo\s*\[WIFI[^\n]*\bCONNECTED\b|'
            r'\bWIFI\b[^\n]*state:\s*CONNECTED)',
            active_section,
        )
    )


def _decode_dns_name(buf, offset):
    """Decodes an RFC 1035 domain name, returning (name, next_offset) or ('', -1)."""
    labels = []
    pos = offset
    next_offset = -1
    jumps = 0
    while True:
        if pos >= len(buf) or jumps > 64:
            return '', -1
        length = buf[pos]
        if length == 0:
            return '.'.join(labels), (pos + 1 if next_offset < 0 else next_offset)
        if (length & 0xC0) == 0xC0:
            if pos + 1 >= len(buf):
                return '', -1
            if next_offset < 0:
                next_offset = pos + 2
            pos = ((length & 0x3F) << 8) | buf[pos + 1]
            jumps += 1
            continue
        if (length & 0xC0) != 0 or pos + 1 + length > len(buf):
            return '', -1
        labels.append(buf[pos + 1:pos + 1 + length].decode('latin1'))
        pos += 1 + length


def _mdns_response_has_service(dns_payload, service_name='_matterd._udp.local'):
    """Returns True if a DNS response has an Answer-section PTR RR for service_name."""
    if len(dns_payload) < 12:
        return False
    _txid, flags, qdcount, ancount, _nscount, _arcount = struct.unpack('!HHHHHH', dns_payload[:12])
    if (flags & 0x8000) == 0 or ancount == 0:
        return False
    expected = service_name.rstrip('.').lower()
    if not expected.endswith('.local'):
        expected = f'{expected}.local'
    pos = 12
    for _ in range(qdcount):
        _, pos = _decode_dns_name(dns_payload, pos)
        if pos < 0 or pos + 4 > len(dns_payload):
            return False
        pos += 4
    for _ in range(ancount):
        if pos >= len(dns_payload):
            break
        rr_name, pos = _decode_dns_name(dns_payload, pos)
        if pos < 0 or pos + 10 > len(dns_payload):
            break
        rr_type, rr_class, _ttl, rdlen = struct.unpack('!HHIH', dns_payload[pos:pos + 10])
        rdata_start, rdata_end = pos + 10, pos + 10 + rdlen
        if rdata_end > len(dns_payload):
            break
        if rr_type == 12 and (rr_class & 0x7FFF) == 1 and rr_name.rstrip('.').lower() == expected:
            ptr_target, next_pos = _decode_dns_name(dns_payload, rdata_start)
            if next_pos == rdata_end and ptr_target:
                return True
        pos = rdata_end
    return False


def _parse_ipv4_l3(eth_frame: bytes, l3_off: int) -> _L3HeaderSlice | None:
    if len(eth_frame) < l3_off + 20 or (eth_frame[l3_off] >> 4) != 4:
        return None
    ihl = (eth_frame[l3_off] & 0x0F) * 4
    if ihl < 20 or len(eth_frame) < l3_off + ihl:
        return None
    if (struct.unpack('!H', eth_frame[l3_off + 6:l3_off + 8])[0] & 0x1FFF) != 0:
        return None
    return _L3HeaderSlice(
        proto=eth_frame[l3_off + 9],
        src_ip=socket.inet_ntoa(eth_frame[l3_off + 12:l3_off + 16]),
        dst_ip=socket.inet_ntoa(eth_frame[l3_off + 16:l3_off + 20]),
        l4_off=l3_off + ihl,
    )


def _parse_ipv6_l3(eth_frame: bytes, l3_off: int) -> _L3HeaderSlice | None:
    if len(eth_frame) < l3_off + 40 or (eth_frame[l3_off] >> 4) != 6:
        return None
    proto = eth_frame[l3_off + 6]
    src_ip = socket.inet_ntop(socket.AF_INET6, eth_frame[l3_off + 8:l3_off + 24])
    dst_ip = socket.inet_ntop(socket.AF_INET6, eth_frame[l3_off + 24:l3_off + 40])
    l4_off = l3_off + 40
    while proto in (0, 43, 60) and len(eth_frame) >= l4_off + 2:
        proto, hdr_ext_len = eth_frame[l4_off], eth_frame[l4_off + 1]
        l4_off += (hdr_ext_len + 1) * 8
    if proto == 44:
        if len(eth_frame) < l4_off + 8:
            return None
        if (struct.unpack('!H', eth_frame[l4_off + 2:l4_off + 4])[0] & 0xFFF8) != 0:
            return None
        proto = eth_frame[l4_off]
        l4_off += 8
    return _L3HeaderSlice(proto=proto, src_ip=src_ip, dst_ip=dst_ip, l4_off=l4_off)


def _extract_udp_segment(eth_frame) -> UdpSegment | None:
    """Extracts a UdpSegment from an Ethernet IPv4/IPv6 UDP frame, or returns None."""
    if len(eth_frame) < 14:
        return None
    dst_mac = ':'.join(f'{b:02x}' for b in eth_frame[0:6])
    src_mac = ':'.join(f'{b:02x}' for b in eth_frame[6:12])
    ethertype = struct.unpack('!H', eth_frame[12:14])[0]
    l3_off = 14
    while ethertype in (0x8100, 0x88A8) and len(eth_frame) >= l3_off + 4:
        ethertype = struct.unpack('!H', eth_frame[l3_off + 2:l3_off + 4])[0]
        l3_off += 4

    if ethertype == 0x0800:
        l3 = _parse_ipv4_l3(eth_frame, l3_off)
    elif ethertype == 0x86DD:
        l3 = _parse_ipv6_l3(eth_frame, l3_off)
    else:
        return None

    if l3 is None or not l3.is_udp or not l3.has_bytes(len(eth_frame), 8):
        return None
    src_port, dst_port, udp_len, _csum = struct.unpack('!HHHH', eth_frame[l3.l4_off:l3.l4_off + 8])
    end_off = min(len(eth_frame), l3.l4_off + max(8, udp_len))
    return UdpSegment(
        src_mac=src_mac,
        dst_mac=dst_mac,
        src_ip=l3.src_ip,
        dst_ip=l3.dst_ip,
        src_port=src_port,
        dst_port=dst_port,
        payload=eth_frame[l3.l4_off + 8:end_off],
    )


def _parse_matter_packet_header(payload: bytes) -> _MatterPacketHeader | None:
    if len(payload) < 8:
        return None
    msg_flags = payload[0]
    version, dsiz = (msg_flags >> 4) & 0x0F, msg_flags & 0x03
    if version != 0 or dsiz == 3:
        return None
    session_id = struct.unpack_from('<H', payload, 1)[0]
    sec_flags = payload[3]
    session_type = sec_flags & 0x03
    if session_type not in (0, 1):
        return None
    pos = 8 + (8 if (msg_flags & 0x04) else 0)
    if dsiz == 1:
        pos += 8
    elif dsiz == 2:
        if session_type != 1:
            return None
        pos += 2
    if sec_flags & 0x20:
        if pos + 2 > len(payload):
            return None
        pos += 2 + struct.unpack_from('<H', payload, pos)[0]
    if pos >= len(payload):
        return None
    return _MatterPacketHeader(payload_offset=pos, session_id=session_id, sec_flags=sec_flags)


def _classify_matter_udp_payload(payload):
    """Classifies a Matter UDP payload as 'udc', 'operational', 'secure_channel', or None."""
    pkt = _parse_matter_packet_header(payload)
    if pkt is None:
        return None
    pos = pkt.payload_offset
    if pkt.is_encrypted_session:
        return 'operational' if len(payload) >= pos + 22 else None
    if pkt.privacy_enabled() or pos + 6 > len(payload):
        return None

    ex_flags, opcode = payload[pos], payload[pos + 1]
    pos += 4
    vendor_id = 0x0000
    if ex_flags & 0x10:
        if pos + 2 > len(payload):
            return None
        vendor_id = struct.unpack_from('<H', payload, pos)[0]
        pos += 2
    if pos + 2 > len(payload):
        return None
    protocol_id = struct.unpack_from('<H', payload, pos)[0]
    pos += 2 + (4 if (ex_flags & 0x02) else 0)
    if ex_flags & 0x08:
        if pos + 2 > len(payload):
            return None
        pos += 2 + struct.unpack_from('<H', payload, pos)[0]
    if pos > len(payload):
        return None
    if vendor_id == 0x0000 and protocol_id == 0x0003 and opcode == 0x00:
        return 'udc'
    if vendor_id == 0x0000 and protocol_id == 0x0000:
        return 'secure_channel'
    return None


def _endpoint_matches(pkt_ip, pkt_mac, expected_ip=None, expected_mac=None):
    """Matches a packet endpoint by IPv4 address when IPv4, else by Ethernet MAC."""
    if not expected_ip and not expected_mac:
        return True
    if expected_ip and '.' in pkt_ip:
        return pkt_ip == expected_ip
    if expected_mac:
        return pkt_mac.lower() == expected_mac.lower()
    return False


def _parse_ethernet_pcap_endian(data: bytes) -> str | None:
    if len(data) < 24:
        return None
    magic = struct.unpack('<I', data[:4])[0]
    if magic in (0xA1B2C3D4, 0xA1B23C4D):
        endian = '<'
    elif magic in (0xD4C3B2A1, 0x4D3CB2A1):
        endian = '>'
    else:
        return None
    return endian if struct.unpack(endian + 'I', data[20:24])[0] == 1 else None


def _classify_casting_udp_segment(udp: UdpSegment, endpoints: _CastingEndpoints) -> str | None:
    if udp.is_mdns_port:
        if endpoints.is_from_tv(udp) and _mdns_response_has_service(udp.payload, '_matterd._udp.local'):
            return 'mdns'
        return None

    kind = _classify_matter_udp_payload(udp.payload)
    if kind == 'udc' and endpoints.is_casting_to_tv(udp):
        return 'udc'
    if kind == 'operational':
        reverse = _endpoint_matches(
            udp.src_ip, udp.src_mac, expected_ip=endpoints.tv_ip, expected_mac=endpoints.tv_mac
        ) and _endpoint_matches(
            udp.dst_ip, udp.dst_mac, expected_ip=endpoints.casting_ip, expected_mac=endpoints.casting_mac
        )
        if endpoints.is_casting_to_tv(udp) or reverse:
            return 'operational'
    return None


def _scan_wifi_pcap_oracles(
    pcap_path,
    tv_ip=None,
    casting_ip=None,
    tv_mac=None,
    casting_mac=None,
) -> WifiPcapOracles:
    """Counts TV _matterd._udp PTR answers, UDC requests, and operational frames in PCAP."""
    with open(pcap_path, 'rb') as fp:
        data = fp.read()
    endian = _parse_ethernet_pcap_endian(data)
    if endian is None:
        return WifiPcapOracles()

    endpoints = _CastingEndpoints(
        tv_ip=tv_ip, casting_ip=casting_ip, tv_mac=tv_mac, casting_mac=casting_mac
    )
    mdns_answers = 0
    udc_datagrams = 0
    operational_datagrams = 0
    off = 24
    while off + 16 <= len(data):
        _ts_s, _ts_u, incl_len, _orig_len = struct.unpack(endian + 'IIII', data[off:off + 16])
        off += 16
        if off + incl_len > len(data):
            break
        udp = _extract_udp_segment(data[off:off + incl_len])
        off += incl_len
        if udp is None:
            continue
        bucket = _classify_casting_udp_segment(udp, endpoints)
        if bucket == 'mdns':
            mdns_answers += 1
        elif bucket == 'udc':
            udc_datagrams += 1
        elif bucket == 'operational':
            operational_datagrams += 1
    return WifiPcapOracles(
        mdns_answers=mdns_answers,
        udc_datagrams=udc_datagrams,
        operational_datagrams=operational_datagrams,
    )


def _check_android_preconditions() -> bool:
    return check_android_preconditions(ANDROID_RUNNER_IMAGE, run_fn=subprocess.run)


class AndroidTvCastingTest(AndroidCirqueTestBase):

    def __init__(self, device_config=None):
        super().__init__(CIRQUE_URL, DEVICE_CONFIG if device_config is None else device_config)
        self.logger = logger

    def _check_preconditions(self) -> bool:
        return _check_android_preconditions()

    def _start_emulator_screen_recording(self, device_id, filename):
        self._start_ui_video(device_id, filename)

    def _stop_emulator_screen_recording(self, device_id):
        self._stop_ui_video(device_id)

    def _guest_shell_arg(self, value: str) -> str:
        return shlex.quote(shlex.quote(str(value)))

    def _apply_emulator_wifi_routes(self, container_id, ip_addr, interface='wlan0'):
        route_rules = '; '.join([
            f'ip route replace 10.0.1.0/24 dev {interface} src {ip_addr} 2>/dev/null || true',
            f'ip route replace 224.0.0.0/4 dev {interface} 2>/dev/null || true',
            'ip rule add to 10.0.1.0/24 lookup local_network pref 91 2>/dev/null || true',
            f'ip rule add to 10.0.1.0/24 lookup {interface} pref 92 2>/dev/null || true',
            'ip rule add to 224.0.0.0/4 lookup local_network pref 93 2>/dev/null || true',
            f'ip rule add to 224.0.0.0/4 lookup {interface} pref 94 2>/dev/null || true',
            'ip rule add to 10.0.1.0/24 lookup main pref 101 2>/dev/null || true',
            'ip rule add to 224.0.0.0/4 lookup main pref 102 2>/dev/null || true',
            f'ip route replace 10.0.1.0/24 dev {interface} table {interface} 2>/dev/null || true',
            f'ip route replace 224.0.0.0/4 dev {interface} table {interface} 2>/dev/null || true',
            f'ip route replace 10.0.1.0/24 dev {interface} table local_network 2>/dev/null || true',
            f'ip route replace 224.0.0.0/4 dev {interface} table local_network 2>/dev/null || true',
            f'ip route replace 10.0.1.0/24 dev {interface} src {ip_addr} table eth0 2>/dev/null || true',
            f'ip route replace 224.0.0.0/4 dev {interface} table eth0 2>/dev/null || true',
            f'iptables -I INPUT -i {interface} -j ACCEPT 2>/dev/null || true',
            f'iptables -I OUTPUT -o {interface} -j ACCEPT 2>/dev/null || true',
        ])
        self.execute_device_cmd(container_id, f'adb shell "{route_rules}"')

    def _nudge_emulator_wifi(self, container_id: str) -> None:
        self.execute_device_cmd(
            container_id,
            'adb shell "'
            'settings put global captive_portal_mode 0 2>/dev/null || true; '
            'settings put global captive_portal_detection_enabled 0 2>/dev/null || true; '
            'svc wifi enable 2>/dev/null || true; '
            'cmd wifi set-wifi-enabled enabled 2>/dev/null || true; '
            'cmd wifi connect-network AndroidWifi open 2>/dev/null || true; '
            'ip link set dev wlan0 up 2>/dev/null || true"',
        )

    def _ensure_guest_dhcpclient_running(self, container_id: str) -> None:
        self.execute_device_cmd(
            container_id,
            'adb shell "pidof dhcpclient >/dev/null 2>&1 || '
            'nohup /vendor/bin/dhcpclient -i wlan0 >/dev/null 2>&1 &"',
        )

    def _record_active_wifi_ip(self, container_id: str, active_ip: str) -> None:
        if not hasattr(self, '_active_ips'):
            self._active_ips = {}
        self._active_ips[container_id] = active_ip
        self._apply_emulator_wifi_routes(container_id, active_ip)
        self.logger.info('Emulator %s WIFI CONNECTED ready with IP %s', container_id, active_ip)

    def _ensure_emulator_wifi_ready(self, container_id, expected_ip=None, timeout_sec=30.0):
        self.execute_device_cmd(
            container_id,
            'adb shell "settings put global captive_portal_mode 0 2>/dev/null || true; '
            'settings put global captive_portal_detection_enabled 0 2>/dev/null || true"',
        )
        start_time = time.time()
        while time.time() - start_time < timeout_sec:
            wifi_out = self._cmd_output_str(self.execute_device_cmd(container_id, 'adb shell cmd wifi status'))
            conn_out = self._cmd_output_str(self.execute_device_cmd(container_id, 'adb shell dumpsys connectivity'))
            addr_out = self._cmd_output_str(self.execute_device_cmd(container_id, 'adb shell "ip -4 addr show dev wlan0"'))

            wifi_connected = 'Wifi is connected' in wifi_out
            cs_connected = connectivity_has_wifi_connected(conn_out)
            ip_match = re.search(r'inet\s+(10\.0\.1\.\d+)', addr_out)
            if wifi_connected and cs_connected and ip_match:
                self._record_active_wifi_ip(container_id, ip_match.group(1))
                return True

            self._nudge_emulator_wifi(container_id)
            if wifi_connected and not ip_match:
                self._ensure_guest_dhcpclient_running(container_id)
            time.sleep(2)

        if expected_ip:
            self._apply_emulator_wifi_routes(container_id, expected_ip)
        self.logger.error('Timeout waiting for WIFI CONNECTED state on %s', container_id)
        return False

    def _wait_for_tv_mdns_ready(self, container_id, timeout_sec=45.0, settle_sec=3.0):
        pattern = r'InitCommissioner|initCommissioner|MdnsAdvertiser|MdnsInterfaceSocket|_matterd\._udp'
        logcat_cmd = f'adb logcat -d -e {shlex.quote(pattern)}'
        start_time = time.time()
        prev_adv_events = None
        while time.time() - start_time < timeout_sec:
            log_res = self.execute_device_cmd(container_id, logcat_cmd)
            log_output = str(log_res.get('output', '')) if str(log_res.get('return_code')) == '0' else ''
            has_commissioner = bool(re.search(r'InitCommissioner|initCommissioner', log_output))
            has_matterd = bool(
                re.search(
                    r'Adding service[^\n]*_matterd\._udp|Registering service[^\n]*_matterd\._udp|_matterd\._udp',
                    log_output,
                )
            )
            if not (has_commissioner and has_matterd):
                prev_adv_events = None
                time.sleep(2)
                continue

            wlan0_probing = bool(re.search(r'MdnsAdvertiser\.wlan0|MdnsInterfaceSocket', log_output))
            adv_events = len(re.findall(r'\[MdnsAdvertiser\][^\n]*(?:Adding|Removing) service', log_output))
            if prev_adv_events is None or adv_events != prev_adv_events:
                prev_adv_events = adv_events
                time.sleep(2)
                continue

            if settle_sec > 0:
                time.sleep(settle_sec)
            self.logger.info(
                'TV app mDNS ready on %s (wlan0_probing=%s, adv_events=%d)',
                container_id,
                wlan0_probing,
                adv_events,
            )
            return True

        self.logger.error('Timeout waiting for TV mDNS readiness on %s', container_id)
        return False

    def _retrigger_casting_discovery(self, container_id: str) -> None:
        self.logger.info('Re-triggering CastingPlayer mDNS browse via startDiscoveryButton')
        if not self._click_ui_element(container_id, resource_id='startDiscoveryButton'):
            self._click_ui_element(container_id, text='Start Discovery')

    def _wait_for_casting_discovery(self, container_id, timeout_sec, rediscover_interval_sec=20.0):
        pattern = r'Discovered CastingPlayer deviceId:'
        logcat_cmd = f'adb logcat -d -e {shlex.quote(pattern)}'
        dump_cmd = 'adb shell "uiautomator dump /data/local/tmp/dump.xml && cat /data/local/tmp/dump.xml"'
        start_time = time.time()
        last_discovery_time = start_time
        while True:
            now = time.time()
            if now - start_time >= timeout_sec:
                break
            log_res = self.execute_device_cmd(container_id, logcat_cmd)
            if str(log_res.get('return_code')) == '0' and re.search(pattern, str(log_res.get('output', ''))):
                self.logger.info('CastingPlayer discovery matched via logcat')
                return True

            dump_res = self.execute_device_cmd(container_id, dump_cmd)
            if str(dump_res.get('return_code')) == '0' and casting_ui_shows_discovered(str(dump_res.get('output', ''))):
                self.logger.info('CastingPlayer discovery matched via UI')
                return True

            if now - last_discovery_time >= rediscover_interval_sec:
                self._retrigger_casting_discovery(container_id)
                last_discovery_time = now
            time.sleep(2)

        self.logger.error('Timeout waiting for CastingPlayer discovery')
        return False

    def _wait_for_logcat(self, container_id, pattern, timeout_sec, since_timestamp=None):
        start_time = time.time()
        since_arg = f'--since {since_timestamp}' if since_timestamp else ''
        compiled_pattern = re.compile(pattern)
        quoted_pattern = shlex.quote(pattern)
        while time.time() - start_time < timeout_sec:
            res = self.execute_device_cmd(container_id, f'adb logcat -d -e {quoted_pattern} {since_arg}')
            if str(res['return_code']) != '0':
                time.sleep(2)
                continue
            matched = next((line for line in res['output'].splitlines() if compiled_pattern.search(line)), None)
            if matched is not None:
                self.logger.info('Found logcat pattern %s: %s', pattern, matched)
                return True, matched
            time.sleep(2)
        self.logger.error('Timeout waiting for logcat pattern %s', pattern)
        return False, ''

    def _ui_shows_casting_connected(self, container_id: str, dump_cmd: str) -> bool:
        dump_res = self.execute_device_cmd(container_id, dump_cmd)
        return str(dump_res['return_code']) == '0' and casting_ui_shows_connected(dump_res['output'])

    def _wait_for_casting_connection(self, container_id, timeout_sec):
        pattern = r'Successfully connected to CastingPlayer'
        logcat_cmd = f'adb logcat -d -e {shlex.quote(pattern)}'
        dump_cmd = 'adb shell "uiautomator dump /data/local/tmp/dump.xml && cat /data/local/tmp/dump.xml"'
        start_time = time.time()
        poll = 0
        while time.time() - start_time < timeout_sec:
            res = self.execute_device_cmd(container_id, logcat_cmd)
            if str(res['return_code']) == '0' and re.search(pattern, res['output']):
                self.logger.info('CastingPlayer connection success matched via logcat')
                return True
            poll += 1
            if poll % 3 == 0 and self._ui_shows_casting_connected(container_id, dump_cmd):
                self.logger.info('CastingPlayer connection success matched via ui')
                return True
            time.sleep(2)
        self.logger.error('Timeout waiting for CastingPlayer connection success')
        return False

    def _click_ui_element(self, container_id, resource_id=None, text=None):
        dump_res = self.execute_device_cmd(
            container_id, 'adb shell "uiautomator dump /data/local/tmp/dump.xml && cat /data/local/tmp/dump.xml"'
        )
        if str(dump_res['return_code']) != '0':
            return False
        xml_content = dump_res['output']
        idx = xml_content.find('<?xml')
        if idx >= 0:
            xml_content = xml_content[idx:]
        center = UiAutomatorHelper.get_element_center(xml_content, resource_id=resource_id, text=text)
        if not center:
            self.logger.error('Could not find element resource_id=%s, text=%s', resource_id, text)
            return False
        x, y = center
        self.logger.info('Clicking element resource_id=%s, text=%s at (%d, %d)', resource_id, text, x, y)
        tap_res = self.execute_device_cmd(container_id, f'adb shell input tap {x} {y}')
        return str(tap_res.get('return_code')) == '0'

    def _accept_tv_commissioning_dialog_if_shown(self, tv_id: str) -> None:
        self.logger.info('Waiting for TV App dialog to inflate')
        time.sleep(5)
        tv_dump = self.execute_device_cmd(
            tv_id, 'adb shell "uiautomator dump /data/local/tmp/dump.xml && cat /data/local/tmp/dump.xml"'
        )
        dump_out = tv_dump['output']
        if 'dialog_title' not in dump_out and 'ALLOW' not in dump_out.upper() and 'OK' not in dump_out.upper():
            return
        self.logger.info('TV App dialog found, trying to accept')
        for label in ('Allow', 'ALLOW', 'OK'):
            if self._click_ui_element(tv_id, text=label):
                return

    def _exercise_launch_url_and_read_vendor_id(self, tv_id: str, casting_id: str) -> None:
        time.sleep(2)
        self.assertTrue(
            self._click_ui_element(casting_id, resource_id='connectionFragmentNextButton'),
            'Failed to click next button',
        )
        time.sleep(2)
        self.assertTrue(
            self._click_ui_element(casting_id, resource_id='selectContentLauncherLaunchURLButton'),
            'Failed to click launch URL button',
        )
        time.sleep(2)
        self.assertTrue(
            self._click_ui_element(casting_id, resource_id='launchUrlButton'),
            'Failed to click launch URL button',
        )

        success, _ = self._wait_for_logcat(
            tv_id,
            r'ContentLauncherManager::HandleLaunchUrl|custom response from content app for content launcher',
            ANDROID_COMMISSION_TIMEOUT_SEC,
        )
        self.assertTrue(success, 'Failed to handle LaunchUrl on TV app')

        success, _ = self._wait_for_logcat(casting_id, r'LaunchURL success', ANDROID_COMMISSION_TIMEOUT_SEC)
        self.assertTrue(success, 'Failed to get LaunchURL success')

        self.logger.info('Testing ReadVendorID')
        self.execute_device_cmd(casting_id, 'adb shell input keyevent 4')
        time.sleep(2)
        self.assertTrue(
            self._click_ui_element(casting_id, resource_id='selectApplicationBasicReadVendorIDButton'),
            'Failed to click selectApplicationBasicReadVendorIDButton',
        )
        time.sleep(2)
        self.assertTrue(
            self._click_ui_element(casting_id, resource_id='readVendorIdButton'),
            'Failed to click readVendorIdButton',
        )
        success, _ = self._wait_for_logcat(casting_id, r'ReadVendorID success', ANDROID_COMMISSION_TIMEOUT_SEC)
        self.assertTrue(success, 'Failed to ReadVendorID')

    def _run_tv_and_casting_ui_flow(self, tv_server, casting_client):
        tv_id = tv_server['id']
        casting_id = casting_client['id']
        disable_bt_cmd = 'adb shell "pm disable-user com.google.android.bluetooth || pm disable-user com.android.bluetooth"'

        self.logger.info('Starting TV App')
        self.execute_device_cmd(tv_id, 'adb shell logcat -G 16M')
        self.execute_device_cmd(tv_id, disable_bt_cmd)
        self.execute_device_cmd(tv_id, 'adb shell am start -n com.matter.tv.server/.MainActivity')
        self.assertTrue(self._wait_for_tv_mdns_ready(tv_id), 'TV app mDNS advertisement did not become ready')

        casting_wifi_ready = self._ensure_emulator_wifi_ready(
            casting_id, expected_ip=casting_client.get('description', {}).get('ipv4_addr')
        )
        self.assertTrue(casting_wifi_ready, f'Casting client Wi-Fi not ready before launch on {casting_id}')

        self.logger.info('Starting Casting App')
        self.execute_device_cmd(casting_id, 'adb shell logcat -G 16M')
        self.execute_device_cmd(casting_id, disable_bt_cmd)
        self.execute_device_cmd(casting_id, 'adb shell am start -n com.chip.casting/com.matter.casting.MainActivity')

        discovered = self._wait_for_casting_discovery(casting_id, ANDROID_COMMISSION_TIMEOUT_SEC)
        self.assertTrue(discovered, 'Failed to discover CastingPlayer')

        time.sleep(2)
        self.assertTrue(
            self._click_ui_element(casting_id, resource_id='commissionable_player_description'),
            'Failed to click player description',
        )
        success, _ = self._wait_for_logcat(tv_id, r'UDC instance=', ANDROID_COMMISSION_TIMEOUT_SEC)
        self.assertTrue(success, 'Failed to receive UDC on TV app')

        self._accept_tv_commissioning_dialog_if_shown(tv_id)
        self.assertTrue(
            self._wait_for_casting_connection(casting_id, ANDROID_COMMISSION_TIMEOUT_SEC),
            'Failed to connect to CastingPlayer',
        )
        self._exercise_launch_url_and_read_vendor_id(tv_id, casting_id)
        self.logger.info('Test passed')

    def _init_and_connect_casting_emulators(self, devices) -> None:
        for dev in devices:
            self.logger.info('Initializing Android emulator environment on %s', dev['id'])
            try:
                init_res = self.query_api(
                    'init_android_emulator',
                    [self.home_id, dev['id']],
                    params={'timeout': ANDROID_BOOT_TIMEOUT_SEC},
                )
            except Exception as exc:
                self.assertTrue(False, f'REST init_android_emulator failed: {exc}')
                return
            if isinstance(init_res, dict) and init_res.get('status') == 'success':
                self.logger.info('Android emulator initialized via REST API: %s', init_res)
            else:
                self.assertTrue(False, f'REST init_android_emulator returned: {init_res}')

        for dev in devices:
            expected_ip = dev.get('description', {}).get('ipv4_addr')
            wifi_ready = self._ensure_emulator_wifi_ready(dev['id'], expected_ip=expected_ip)
            self.assertTrue(wifi_ready, f"Failed to verify WIFI CONNECTED state on {dev['id']}")

    def _verify_casting_pcap_oracles(self, tv_server, casting_client) -> None:
        self.logger.info('Checking pcap for oracles')
        time.sleep(2)
        pcap_dir = os.environ.get('CIRQUE_PCAP_DIR') or os.environ.get('DEVICE_LOG_DIR')
        self.assertTrue(bool(pcap_dir and os.path.isdir(pcap_dir)), f'pcap_dir not found: {pcap_dir}')
        pcap_path = os.path.join(pcap_dir, 'wifi_medium.pcap')
        self.assertTrue(os.path.isfile(pcap_path), f'wifi_medium.pcap not found in {pcap_dir}')

        active_ips = self._active_ips if hasattr(self, '_active_ips') else {}
        tv_desc = tv_server.get('description', {})
        casting_desc = casting_client.get('description', {})
        oracles = _scan_wifi_pcap_oracles(
            pcap_path,
            tv_ip=active_ips.get(tv_server['id']) or tv_desc.get('ipv4_addr', '10.0.1.11'),
            casting_ip=active_ips.get(casting_client['id']) or casting_desc.get('ipv4_addr', '10.0.1.12'),
            tv_mac=tv_desc.get('mac_address', '02:15:b2:00:00:01'),
            casting_mac=casting_desc.get('mac_address', '02:15:b2:00:00:02'),
        )
        self.logger.info(
            'wifi_medium.pcap oracles: _matterd._udp mDNS PTR answers=%d,'
            ' UDC datagrams=%d, operational Matter UDP datagrams=%d',
            oracles.mdns_answers,
            oracles.udc_datagrams,
            oracles.operational_datagrams,
        )
        self.assertTrue(oracles.mdns_answers > 0, 'No _matterd._udp mDNS answer found from TV in pcap')
        self.assertTrue(oracles.udc_datagrams > 0, 'No UDC datagram found from Casting App to TV')

    def test_routine(self):
        tv_server = next(
            d for d in self.non_ap_devices if 'tv-server' in d.get('description', {}).get('chiptool_apk', '')
        )
        casting_client = next(
            d for d in self.non_ap_devices if 'casting-app' in d.get('description', {}).get('chiptool_apk', '')
        )
        self._init_and_connect_casting_emulators((tv_server, casting_client))

        self._start_emulator_screen_recording(tv_server['id'], '1_tv_server_casting.mp4')
        self._start_emulator_screen_recording(casting_client['id'], '2_casting_client_casting.mp4')
        try:
            self._run_tv_and_casting_ui_flow(tv_server, casting_client)
        finally:
            self._stop_emulator_screen_recording(casting_client['id'])
            self._stop_emulator_screen_recording(tv_server['id'])

        self._verify_casting_pcap_oracles(tv_server, casting_client)
        return TestResult.OK


if __name__ == '__main__':
    sys.exit(AndroidTvCastingTest(DEVICE_CONFIG).run_test())
