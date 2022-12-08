#
# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""Configuration for #include checking."""

from typing import Dict, Set


# IGNORE lists source files that are not checked at all.
#
# Each entry is a string giving a Python regular expression,
# un-anchored and case sensitive.

IGNORE: Set[str] = {

    '/examples/',
    '/java/',
    '/Jni',
    '/pybindings/',
    '/python/',
    '/Test',
    '/tests/',
    '/tools/',

    # Platforms can opt in or out.
    '/darwin/',
    '/platform/Ameba/',
    '/platform/android/',
    '/platform/Beken/',
    '/platform/Infineon/CYW30739/',
    '/platform/Darwin/',
    '/platform/silabs/',
    '/platform/ESP32/',
    '/platform/fake/',
    '/platform/Linux/',
    '/platform/nxp/',
    '/platform/Tizen/',
    '/platform/Infineon/PSOC6/',
    '/platform/bouffalolab/BL602',
    '/platform/webos/',
    '/platform/mt793x/',
    r'POSIX\.h$',
}


# DENY lists disallowed include files.

DENY: Set[str] = {

    # C++ headers often unsuitable for small platforms.
    'chrono',
    'clocale',
    'coroutine',
    'deque',
    'exception',
    'forward_list',
    'fstream',
    'iomanip',
    'ios',
    'iostream',
    'istream',
    'list',
    'locale',
    'locale.h',
    'map',
    'multimap',
    'multiset',
    'ostream',
    'queue',
    'set',
    'sstream',
    'stdexcept',
    'streambuf',
    'string',
    'string_view',
    'syncstream',
    'unordered_map',
    'unordered_set',
    'vector',

    # CHIP headers using STL containers.
    'lib/support/CHIPListUtils.h',      # uses std::set
    'src/platform/DeviceSafeQueue.h',   # uses std::deque
}


# ALLOW describes exceptions to DENY.
#
# The dictionary key is the name of the file performing the #include.
# The value is a set of names allowed to be included from that file
# despite being in DENY.

ALLOW: Dict[str, Set[str]] = {

    # Not intended for embedded clients (#11705).
    'src/app/ClusterStateCache.h': {'list', 'map', 'set', 'vector', 'queue'},
    'src/app/BufferedReadCallback.h': {'vector'},
    'src/lib/support/IniEscaping.cpp': {'string'},
    'src/lib/support/IniEscaping.h': {'string'},

    # Itself in DENY.
    'src/lib/support/CHIPListUtils.h': {'set'},
    'src/platform/DeviceSafeQueue.h': {'queue'},

    # Only uses <chrono> for zero-cost types.
    'src/system/SystemClock.h': {'chrono'},
    'src/platform/mbed/MbedEventTimeout.h': {'chrono'},

    'src/app/app-platform/ContentApp.h': {'list', 'string'},
    'src/app/clusters/application-basic-server/application-basic-delegate.h': {'list'},
    'src/app/clusters/application-basic-server/application-basic-server.cpp': {'list'},
    'src/app/clusters/application-launcher-server/application-launcher-delegate.h': {'list'},
    'src/app/clusters/audio-output-server/audio-output-delegate.h': {'list'},
    'src/app/clusters/channel-server/channel-delegate.h': {'list'},
    'src/app/clusters/content-launch-server/content-launch-delegate.h': {'list'},
    'src/app/clusters/content-launch-server/content-launch-server.cpp': {'list'},
    'src/app/clusters/media-input-server/media-input-delegate.h': {'list'},
    'src/app/clusters/media-playback-server/media-playback-delegate.h': {'list'},
    'src/app/clusters/target-navigator-server/target-navigator-delegate.h': {'list'},

    'src/credentials/attestation_verifier/FileAttestationTrustStore.h': {'vector'},
    'src/credentials/attestation_verifier/FileAttestationTrustStore.cpp': {'string'},

    'src/setup_payload/AdditionalDataPayload.h': {'string'},
    'src/setup_payload/AdditionalDataPayloadParser.cpp': {'vector'},
    'src/setup_payload/Base38Decode.h': {'string', 'vector'},
    'src/setup_payload/ManualSetupPayloadGenerator.h': {'string'},
    'src/setup_payload/ManualSetupPayloadParser.cpp': {'string', 'vector'},
    'src/setup_payload/ManualSetupPayloadParser.h': {'string'},
    'src/setup_payload/QRCodeSetupPayloadParser.cpp': {'vector'},
    'src/setup_payload/QRCodeSetupPayloadParser.h': {'string'},
    'src/setup_payload/SetupPayloadHelper.cpp': {'fstream'},
    'src/setup_payload/SetupPayloadHelper.h': {'string'},
    'src/setup_payload/SetupPayload.h': {'map', 'string', 'vector'},

    # Uses platform-define to switch between list and array
    'src/lib/dnssd/minimal_mdns/ResponseSender.h': {'list'},

    # Not really for embedded consumers; uses std::deque to keep track
    # of a list of discovered things.
    'src/controller/SetUpCodePairer.h': {'deque'},

    'src/controller/ExamplePersistentStorage.cpp': {'fstream'}
}
