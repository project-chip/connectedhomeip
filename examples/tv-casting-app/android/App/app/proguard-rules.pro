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

# ============================================================================
# 3. chip.devicecontroller — CHIPInteractionModel.jar
#
# Kept at the individual-class level rather than chip.devicecontroller.**
# because CHIPInteractionModel.jar contains ~6,700 classes — the majority
# being ChipClusters, ChipStructs, and ChipEventStructs inner classes
# generated for all ~200 Matter clusters.
#
# The optimized build compiles slim TLV decoder overrides covering only the
# 19 casting clusters (CHIPAttributeTLVValueDecoder-override.cpp and
# CHIPEventTLVValueDecoder-override.cpp in tv-casting-common/). Only those
# 19 clusters' switch-case branches exist in libTvCastingApp.so, so only
# their Java counterparts need to be kept. R8 prunes all remaining cluster
# classes, including ClusterInfoMapping/ReadMapping/WriteMapping (~1,047
# generated classes) which are not referenced by the casting app.
# ============================================================================

# Interaction model callbacks and exceptions — called from JNI
-keep class chip.devicecontroller.ChipDeviceControllerException { *; }
-keep class chip.devicecontroller.ChipClusterException { *; }
-keep class chip.devicecontroller.StatusException { *; }
-keep class chip.devicecontroller.ChipInteractionClient { *; }
-keep class chip.devicecontroller.ChipICDClient { *; }
-keep class chip.devicecontroller.ICDClientInfo { *; }
-keep class chip.devicecontroller.ChipTLVType { *; }
-keep class chip.devicecontroller.ChipTLVType$* { *; }
-keep class chip.devicecontroller.ChipTLVValueDecoder { *; }
-keep class chip.devicecontroller.ReportCallback { *; }
-keep class chip.devicecontroller.ReportCallbackJni { *; }
-keep class chip.devicecontroller.InvokeCallback { *; }
-keep class chip.devicecontroller.InvokeCallbackJni { *; }
-keep class chip.devicecontroller.WriteAttributesCallback { *; }
-keep class chip.devicecontroller.WriteAttributesCallbackJni { *; }
-keep class chip.devicecontroller.SubscriptionEstablishedCallback { *; }
-keep class chip.devicecontroller.ResubscriptionAttemptCallback { *; }
-keep class chip.devicecontroller.ExtendableInvokeCallback { *; }
-keep class chip.devicecontroller.ExtendableInvokeCallbackJni { *; }
-keep class chip.devicecontroller.GetConnectedDeviceCallbackJni { *; }

# model.* — parameter types in ChipInteractionClient native method signatures
-keep class chip.devicecontroller.model.** { *; }

# Base cluster classes — superclasses of all ChipClusters inner classes
-keep class chip.devicecontroller.ChipClusters { *; }
-keep class chip.devicecontroller.ChipClusters$BaseChipCluster { *; }
-keep class chip.devicecontroller.ChipClusters$BaseAttributeCallback { *; }
-keep class chip.devicecontroller.ChipClusters$BaseClusterCallback { *; }

# ChipClusters inner classes — exactly the 19 casting clusters present in the
# slim TLV decoder overrides compiled into libTvCastingApp.so.
# Source of truth: CHIPAttributeTLVValueDecoder-override.cpp and
# CHIPEventTLVValueDecoder-override.cpp in tv-casting-common/.
-keep class chip.devicecontroller.ChipClusters$AccountLoginCluster { *; }
-keep class chip.devicecontroller.ChipClusters$AccountLoginCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$ApplicationBasicCluster { *; }
-keep class chip.devicecontroller.ChipClusters$ApplicationBasicCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$ApplicationLauncherCluster { *; }
-keep class chip.devicecontroller.ChipClusters$ApplicationLauncherCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$AudioOutputCluster { *; }
-keep class chip.devicecontroller.ChipClusters$AudioOutputCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$BindingCluster { *; }
-keep class chip.devicecontroller.ChipClusters$BindingCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$ChannelCluster { *; }
-keep class chip.devicecontroller.ChipClusters$ChannelCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$ContentAppObserverCluster { *; }
-keep class chip.devicecontroller.ChipClusters$ContentAppObserverCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$ContentControlCluster { *; }
-keep class chip.devicecontroller.ChipClusters$ContentControlCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$ContentLauncherCluster { *; }
-keep class chip.devicecontroller.ChipClusters$ContentLauncherCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$DescriptorCluster { *; }
-keep class chip.devicecontroller.ChipClusters$DescriptorCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$KeypadInputCluster { *; }
-keep class chip.devicecontroller.ChipClusters$KeypadInputCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$LevelControlCluster { *; }
-keep class chip.devicecontroller.ChipClusters$LevelControlCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$LowPowerCluster { *; }
-keep class chip.devicecontroller.ChipClusters$LowPowerCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$MediaInputCluster { *; }
-keep class chip.devicecontroller.ChipClusters$MediaInputCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$MediaPlaybackCluster { *; }
-keep class chip.devicecontroller.ChipClusters$MediaPlaybackCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$MessagesCluster { *; }
-keep class chip.devicecontroller.ChipClusters$MessagesCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$OnOffCluster { *; }
-keep class chip.devicecontroller.ChipClusters$OnOffCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$TargetNavigatorCluster { *; }
-keep class chip.devicecontroller.ChipClusters$TargetNavigatorCluster$* { *; }
-keep class chip.devicecontroller.ChipClusters$WakeOnLanCluster { *; }
-keep class chip.devicecontroller.ChipClusters$WakeOnLanCluster$* { *; }

# ChipStructs — only the 19 clusters in the slim attribute TLV decoder.
# These FindClass calls are inside switch-case branches and only execute when
# that cluster's attribute response arrives; all other cluster structs are pruned.
-keep class chip.devicecontroller.ChipStructs$AccountLoginCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$ApplicationBasicCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$ApplicationLauncherCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$AudioOutputCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$BindingCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$ChannelCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$ContentAppObserverCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$ContentControlCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$ContentLauncherCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$DescriptorCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$KeypadInputCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$LevelControlCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$LowPowerCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$MediaInputCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$MediaPlaybackCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$MessagesCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$OnOffCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$TargetNavigatorCluster* { *; }
-keep class chip.devicecontroller.ChipStructs$WakeOnLanCluster* { *; }

# ChipEventStructs — only the 19 clusters in the slim event TLV decoder.
-keep class chip.devicecontroller.ChipEventStructs$AccountLoginCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$ApplicationBasicCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$ApplicationLauncherCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$AudioOutputCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$BindingCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$ChannelCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$ContentAppObserverCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$ContentControlCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$ContentLauncherCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$DescriptorCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$KeypadInputCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$LevelControlCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$LowPowerCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$MediaInputCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$MediaPlaybackCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$MessagesCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$OnOffCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$TargetNavigatorCluster* { *; }
-keep class chip.devicecontroller.ChipEventStructs$WakeOnLanCluster* { *; }

# ============================================================================
# 4. Keep native method declarations so the JNI linkage works
# ============================================================================
-keepclasseswithmembernames class * {
    native <methods>;
}

# ============================================================================
# 5. Keep the Android Application subclass (entry point)
# ============================================================================
-keep class com.matter.casting.ChipTvCastingApplication { *; }

# ============================================================================
# 6. Standard Android keep rules
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
# 7. Suppress warnings for annotations not present at runtime
# ============================================================================

# javax.annotation.* annotations (Nonnull, Nullable) are compile-time only
# and not included in the Android runtime. They are safe to ignore.
-dontwarn javax.annotation.**
