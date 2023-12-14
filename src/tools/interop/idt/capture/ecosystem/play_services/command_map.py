#
#    Copyright (c) 2023 Project CHIP Authors
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

getprop = {
    'ro.product.model':               'android_model',
    'ro.build.version.release':       'android_version',
    'ro.build.version.sdk':           'android_api',
    'ro.build.fingerprint':           'build_fingerprint',
    'ro.odm.build.fingerprint':       'odm_build_fingerprint',
    'ro.product.build.fingerprint':   'product_build_fingerprint',
    'ro.ecosystem.build.fingerprint': 'vendor_build_fingerprint',
}

_ap = 'activity provider com.google.android.gms.chimera.container.GmsModuleProvider'
dumpsys = {
    'display_width': 'display | grep StableDisplayWidth | awk -F\'=\' \'{print $2}\'',
    'display_height': 'display | grep StableDisplayHeight | awk -F\'=\' \'{print $2}\'',
    'gha_info': ' package com.google.android.apps.chromecast.app | grep versionName',
    'container_info': 'package com.google.android.gms | grep "versionName"',
    'home_module_info': f'{_ap} | grep "com.google.android.gms.home" | grep -v graph',
    'optional_home_module_info': f'{_ap} | grep "com.google.android.gms.optional_home" | grep -v graph',
    'policy_home_module_info': f'{_ap} | grep "com.google.android.gms.policy_home" | grep -v graph',
    'thread_info': f'{_ap} | grep "com.google.android.gms.threadnetwork"',
    'mdns_info': f'{_ap} | grep -i com.google.android.gms.mdns',
}
