# ProGuard / R8 rules for the Matter TV Casting App
#
# The native library (libTvCastingApp.so) calls back into Java via JNI.
# R8 cannot trace these references, so we must explicitly keep all classes
# and members that are accessed from native code.

# ============================================================================
# 1. App-level JNI classes (called from libTvCastingApp.so via FindClass/GetMethodID)
# ============================================================================

# Casting core classes — constructed and field-accessed from native code
-keep class com.matter.casting.core.** { *; }

# Casting support classes — fields read/written from native code
-keep class com.matter.casting.support.** { *; }

# Compat JNI layer — legacy API surface also called from native code
-keep class com.chip.casting.** { *; }

# ============================================================================
# 2. CHIP SDK classes (from dependency JARs, called via JNI from libTvCastingApp.so
#    and the CHIP platform native code)
# ============================================================================

# Platform layer — JNI bridge between native CHIP stack and Android
-keep class chip.platform.** { *; }

# App server — CHIP application server Java bindings
-keep class chip.appserver.** { *; }

# Interaction model — cluster/command/attribute Java bindings
-keep class chip.clusterinfo.** { *; }

# Device controller — used by the interaction model layer
-keep class chip.devicecontroller.** { *; }

# Setup payload (QR code / manual pairing code parsing)
-keep class chip.setuppayload.** { *; }

# TLV codec — used by interaction model serialization
-keep class chip.tlv.** { *; }

# ============================================================================
# 3. Keep native method declarations so the JNI linkage works
# ============================================================================
-keepclasseswithmembernames class * {
    native <methods>;
}

# ============================================================================
# 4. Keep the Android Application subclass (entry point)
# ============================================================================
-keep class com.matter.casting.ChipTvCastingApplication { *; }

# ============================================================================
# 5. Standard Android keep rules
# ============================================================================

# Keep Parcelable implementations
-keepclassmembers class * implements android.os.Parcelable {
    public static final ** CREATOR;
}

# Keep enum values (used by reflection in some Android APIs)
-keepclassmembers enum * {
    public static **[] values();
    public static ** valueOf(java.lang.String);
}

# ============================================================================
# 6. Suppress warnings for annotations not present at runtime
# ============================================================================

# javax.annotation.* annotations (Nonnull, Nullable) are compile-time only
# and not included in the Android runtime. They are safe to ignore.
-dontwarn javax.annotation.**
