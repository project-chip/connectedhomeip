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
    '/mock/',
    '/python/',
    '/Test',
    '/testing/',
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
    '/platform/ASR/',
    '/platform/NuttX/',
    r'POSIX\.h$',

    # These run on large systems, so no restrictions
    #   - bridges
    #   - cameras
    #   - commissioners
    #   - commodity tariff
    #   - JF devices
    #   - media devices
    # keep-sorted: start
    'src/app/app-platform/',
    'src/app/clusters/application-basic-server/',
    'src/app/clusters/application-launcher-server/',
    'src/app/clusters/audio-output-server/',
    'src/app/clusters/bridged-device-basic-information-server/',
    'src/app/clusters/camera-av-settings-user-level-management-server/',
    'src/app/clusters/camera-av-stream-management-server/',
    'src/app/clusters/channel-server/',
    'src/app/clusters/commodity-tariff-server/',
    'src/app/clusters/content-launch-server/',
    'src/app/clusters/ecosystem-information-server/',
    'src/app/clusters/media-input-server/',
    'src/app/clusters/media-playback-server/',
    'src/app/clusters/push-av-stream-transport-server/',
    'src/app/clusters/target-navigator-server/',
    'src/app/clusters/webrtc-transport-provider-server/',
    'src/app/clusters/webrtc-transport-requestor-server/',
    'src/app/clusters/zone-management-server/',
    'src/controller/',
    'src/credentials/jcm/',
    'src/lib/support/jsontlv/',
    'src/setup_payload/',
    'src/tracing/esp32_diagnostics/',
    'src/tracing/esp32_trace/',
    'src/tracing/json/',
    # keep-sorted: end
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
    'app/data-model/ListLargeSystemExtensions.h',      # uses std::set
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
    'src/app/data-model/ListLargeSystemExtensions.h': {'set'},
    'src/platform/DeviceSafeQueue.h': {'queue'},

    # Only uses <chrono> for zero-cost types.
    'src/system/SystemClock.h': {'chrono'},
    'src/lib/core/StringBuilderAdapters.h': {'chrono'},

    'src/credentials/attestation_verifier/FileAttestationTrustStore.h': {'vector'},
    'src/credentials/attestation_verifier/FileAttestationTrustStore.cpp': {'string'},
    'src/credentials/attestation_verifier/TestDACRevocationDelegateImpl.cpp': {'fstream'},

    # Uses platform-define to switch between list and array
    'src/lib/dnssd/minimal_mdns/ResponseSender.h': {'list'},

    # Not intended for embedded clients
    'src/app/PendingResponseTrackerImpl.h': {'unordered_set'},

    # Not intended for embedded clients
    'src/lib/core/TLVVectorWriter.cpp': {'vector'},
    'src/lib/core/TLVVectorWriter.h': {'vector'},
    'src/lib/support/TemporaryFileStream.h': {'ostream', 'streambuf', 'string'},
    'src/app/icd/client/DefaultICDClientStorage.cpp': {'vector'},
    'src/app/icd/client/DefaultICDClientStorage.h': {'vector'},
    'src/app/icd/client/DefaultICDStorageKey.h': {'vector'},
    'src/qrcodetool/setup_payload_commands.cpp': {'string'},
    'src/access/AccessRestrictionProvider.h': {'vector', 'map'},
    # nrfconnect test runner
    'src/test_driver/nrfconnect/main/runner.cpp': {'vector'},

    # Not intended for embedded clients
    'src/app/server/JointFabricDatastore.cpp': {'vector', 'unordered_set'},
    'src/app/server/JointFabricDatastore.h': {'vector', 'unordered_set'},
}
