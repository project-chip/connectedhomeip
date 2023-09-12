properties_and_commands = {
    'android_model': 'shell getprop ro.product.model',
    'android_version': 'shell getprop ro.build.version.release',
    'android_api': 'shell getprop ro.build.version.sdk',

    'build_fingerprint': 'shell getprop ro.build.fingerprint',
    'odm_build_fingerprint': 'shell getprop ro.odm.build.fingerprint',
    'product_build_fingerprint': 'shell getprop ro.product.build.fingerprint',
    'vendor_build_fingerprint': 'shell getprop ro.ecosystem.build.fingerprint',

    'display_width': 'shell dumpsys display | grep StableDisplayWidth | awk -F\'=\' \'{print $2}\'',
    'display_height': 'shell dumpsys display | grep StableDisplayHeight | awk -F\'=\' \'{print $2}\'',

    'container_info': 'shell dumpsys package com.google.android.gms | grep "versionName"',
    'home_module_info': 'shell dumpsys activity provider com.google.android.gms.chimera.container.GmsModuleProvider | grep "com.google.android.gms.home"',
    'optional_home_module_info': 'shell dumpsys activity provider com.google.android.gms.chimera.container.GmsModuleProvider | grep "com.google.android.gms.optional_home"',
    'policy_home_module_info': 'shell dumpsys activity provider com.google.android.gms.chimera.container.GmsModuleProvider | grep "com.google.android.gms.policy_home"',
    'thread_info': 'shell dumpsys activity provider com.google.android.gms.chimera.container.GmsModuleProvider | grep "com.google.android.gms.threadnetwork"',
    'mdns_info': 'shell dumpsys activity provider com.google.android.gms.chimera.container.GmsModuleProvider | grep -i com.google.android.gms.mdns',
    'gha_info': 'shell dumpsys package com.google.android.apps.chromecast.app | grep versionName',
}
