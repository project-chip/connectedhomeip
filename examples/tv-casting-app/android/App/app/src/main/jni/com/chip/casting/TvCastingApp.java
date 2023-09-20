/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.chip.casting;

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.wifi.WifiManager;
import android.util.Log;
import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.DiagnosticDataProviderImpl;
import chip.platform.NsdManagerServiceBrowser;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesKeyValueStoreManager;
import java.util.Arrays;
import java.util.List;

public class TvCastingApp {
  private static final String TAG = TvCastingApp.class.getSimpleName();
  private static final String DISCOVERY_TARGET_SERVICE_TYPE = "_matterd._udp.";
  private static final List<Long> DISCOVERY_TARGET_DEVICE_TYPE_FILTER =
      Arrays.asList(35L); // Video player = 35;

  private static TvCastingApp sInstance;
  private Context applicationContext;
  private ChipAppServer chipAppServer;
  private NsdManagerServiceResolver.NsdManagerResolverAvailState nsdManagerResolverAvailState;
  private boolean discoveryStarted = false;
  private Object discoveryLock = new Object();

  private WifiManager.MulticastLock multicastLock;
  private NsdManager nsdManager;
  private NsdDiscoveryListener nsdDiscoveryListener;

  private TvCastingApp() {}

  public static TvCastingApp getInstance() {
    if (sInstance == null) {
      sInstance = new TvCastingApp();
    }
    return sInstance;
  }

  public boolean initApp(Context applicationContext, AppParameters appParameters) {
    if (applicationContext == null
        || appParameters == null
        || appParameters.getConfigurationManager() == null) {
      return false;
    }

    this.applicationContext = applicationContext;
    nsdManagerResolverAvailState = new NsdManagerServiceResolver.NsdManagerResolverAvailState();
    NsdManagerServiceResolver nsdManagerServiceResolver =
        new NsdManagerServiceResolver(applicationContext, nsdManagerResolverAvailState);

    AndroidChipPlatform chipPlatform =
        new AndroidChipPlatform(
            new AndroidBleManager(),
            new PreferencesKeyValueStoreManager(applicationContext),
            appParameters.getConfigurationManager(),
            nsdManagerServiceResolver,
            new NsdManagerServiceBrowser(applicationContext),
            new ChipMdnsCallbackImpl(),
            new DiagnosticDataProviderImpl(applicationContext));

    boolean ret =
        chipPlatform.updateCommissionableDataProviderData(
            appParameters.getSpake2pVerifierBase64(),
            appParameters.getSpake2pSaltBase64(),
            appParameters.getSpake2pIterationCount(),
            appParameters.getSetupPasscode(),
            appParameters.getDiscriminator());
    if (!ret) {
      Log.e(
          TAG,
          "TvCastingApp.initApp failed to updateCommissionableDataProviderData on chipPlatform");
      return ret;
    }

    ret = preInitJni(appParameters);
    if (!ret) {
      Log.e(TAG, "TvCastingApp.initApp failed in preInitJni");
      return ret;
    }

    chipAppServer = new ChipAppServer();
    ret = chipAppServer.startApp();
    if (!ret) {
      Log.e(TAG, "TvCastingApp.initApp failed in start chipAppServer");
      return ret;
    }

    return initJni(appParameters);
  }

  public native void setDACProvider(DACProvider provider);

  private native boolean preInitJni(AppParameters appParameters);

  private native boolean initJni(AppParameters appParameters);

  public void discoverVideoPlayerCommissioners(
      SuccessCallback<DiscoveredNodeData> discoverySuccessCallback,
      FailureCallback discoveryFailureCallback) {
    synchronized (discoveryLock) {
      Log.d(TAG, "TvCastingApp.discoverVideoPlayerCommissioners called");

      if (this.discoveryStarted) {
        Log.d(TAG, "Discovery already started, stopping before starting again");
        stopVideoPlayerDiscovery();
      }

      List<VideoPlayer> preCommissionedVideoPlayers = readCachedVideoPlayers();

      WifiManager wifiManager =
          (WifiManager) applicationContext.getSystemService(Context.WIFI_SERVICE);
      multicastLock = wifiManager.createMulticastLock("multicastLock");
      multicastLock.setReferenceCounted(true);
      multicastLock.acquire();

      nsdManager = (NsdManager) applicationContext.getSystemService(Context.NSD_SERVICE);
      nsdDiscoveryListener =
          new NsdDiscoveryListener(
              nsdManager,
              DISCOVERY_TARGET_SERVICE_TYPE,
              DISCOVERY_TARGET_DEVICE_TYPE_FILTER,
              preCommissionedVideoPlayers,
              discoverySuccessCallback,
              discoveryFailureCallback,
              nsdManagerResolverAvailState);

      nsdManager.discoverServices(
          DISCOVERY_TARGET_SERVICE_TYPE, NsdManager.PROTOCOL_DNS_SD, nsdDiscoveryListener);
      Log.d(TAG, "TvCastingApp.discoverVideoPlayerCommissioners started");
      this.discoveryStarted = true;
    }
  }

  public void stopVideoPlayerDiscovery() {
    synchronized (discoveryLock) {
      Log.d(TAG, "TvCastingApp trying to stop video player discovery");
      if (this.discoveryStarted
          && nsdManager != null
          && multicastLock != null
          && nsdDiscoveryListener != null) {
        Log.d(TAG, "TvCastingApp stopping Video Player commissioner discovery");
        try {
          nsdManager.stopServiceDiscovery(nsdDiscoveryListener);
        } catch (IllegalArgumentException e) {
          Log.w(
              TAG,
              "TvCastingApp received exception on calling nsdManager.stopServiceDiscovery() "
                  + e.getMessage());
        }

        if (multicastLock.isHeld()) {
          multicastLock.release();
        }
        this.discoveryStarted = false;
      }
    }
  }

  public native boolean openBasicCommissioningWindow(
      int duration,
      CommissioningCallbacks commissioningCallbacks,
      SuccessCallback<VideoPlayer> onConnectionSuccess,
      FailureCallback onConnectionFailure,
      SuccessCallback<ContentApp> onNewOrUpdatedEndpointCallback);

  public native boolean sendCommissioningRequest(DiscoveredNodeData commissioner);

  /** @Deprecated Use sendCommissioningRequest(DiscoveredNodeData) instead */
  private native boolean sendUserDirectedCommissioningRequest(String address, int port);

  public native List<VideoPlayer> readCachedVideoPlayers();

  public native boolean verifyOrEstablishConnection(
      VideoPlayer targetVideoPlayer,
      SuccessCallback<VideoPlayer> onConnectionSuccess,
      FailureCallback onConnectionFailure,
      SuccessCallback<ContentApp> onNewOrUpdatedEndpointCallback);

  public native void shutdownAllSubscriptions();

  public native void disconnect();

  public native List<VideoPlayer> getActiveTargetVideoPlayers();

  public native boolean purgeCache();

  /*
   * CONTENT LAUNCHER CLUSTER
   *
   * TODO: Add API to subscribe to AcceptHeader
   */
  public native boolean contentLauncherLaunchURL(
      ContentApp contentApp, String contentUrl, String contentDisplayStr, Object launchURLHandler);

  public native boolean contentLauncher_launchContent(
      ContentApp contentApp,
      ContentLauncherTypes.ContentSearch search,
      boolean autoPlay,
      String data,
      Object responseHandler);

  public native boolean contentLauncher_subscribeToSupportedStreamingProtocols(
      ContentApp contentApp,
      SuccessCallback<Integer> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  /*
   * LEVEL CONTROL CLUSTER
   */
  public native boolean levelControl_step(
      ContentApp contentApp,
      byte stepMode,
      byte stepSize,
      short transitionTime,
      byte optionMask,
      byte optionOverridem,
      Object responseHandler);

  public native boolean levelControl_moveToLevel(
      ContentApp contentApp,
      byte level,
      short transitionTime,
      byte optionMask,
      byte optionOverridem,
      Object responseHandler);

  public native boolean levelControl_subscribeToCurrentLevel(
      ContentApp contentApp,
      SuccessCallback<Byte> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean levelControl_subscribeToMinLevel(
      ContentApp contentApp,
      SuccessCallback<Byte> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean levelControl_subscribeToMaxLevel(
      ContentApp contentApp,
      SuccessCallback<Byte> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  /*
   * MEDIA PLAYBACK CLUSTER
   */
  public native boolean mediaPlayback_play(ContentApp contentApp, Object responseHandler);

  public native boolean mediaPlayback_pause(ContentApp contentApp, Object responseHandler);

  public native boolean mediaPlayback_stopPlayback(ContentApp contentApp, Object responseHandler);

  public native boolean mediaPlayback_next(ContentApp contentApp, Object responseHandler);

  public native boolean mediaPlayback_previous(ContentApp contentApp, Object responseHandler);

  public native boolean mediaPlayback_rewind(ContentApp contentApp, Object responseHandler);

  public native boolean mediaPlayback_fastForward(ContentApp contentApp, Object responseHandler);

  public native boolean mediaPlayback_startOver(ContentApp contentApp, Object responseHandler);

  public native boolean mediaPlayback_seek(
      ContentApp contentApp, long position, Object responseHandler);

  public native boolean mediaPlayback_skipForward(
      ContentApp contentApp, long deltaPositionMilliseconds, Object responseHandler);

  public native boolean mediaPlayback_skipBackward(
      ContentApp contentApp, long deltaPositionMilliseconds, Object responseHandler);

  public native boolean mediaPlayback_subscribeToCurrentState(
      ContentApp contentApp,
      SuccessCallback<MediaPlaybackTypes.PlaybackStateEnum> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToDuration(
      ContentApp contentApp,
      SuccessCallback<Long> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToSampledPosition(
      ContentApp contentApp,
      SuccessCallback<MediaPlaybackTypes.PlaybackPosition> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToPlaybackSpeed(
      ContentApp contentApp,
      SuccessCallback<Float> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToSeekRangeEnd(
      ContentApp contentApp,
      SuccessCallback<Long> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToSeekRangeStart(
      ContentApp contentApp,
      SuccessCallback<Long> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  /*
   * APPLICATION LAUNCHER CLUSTER
   */
  public native boolean applicationLauncher_launchApp(
      ContentApp contentApp,
      short catalogVendorId,
      String applicationId,
      byte[] data,
      Object responseHandler);

  public native boolean applicationLauncher_stopApp(
      ContentApp contentApp, short catalogVendorId, String applicationId, Object responseHandler);

  public native boolean applicationLauncher_hideApp(
      ContentApp contentApp, short catalogVendorId, String applicationId, Object responseHandler);

  /*
   * TARGET NAVIGATOR CLUSTER
   */
  public native boolean targetNavigator_navigateTarget(
      ContentApp contentApp, byte target, String data, Object responseHandler);

  public native boolean targetNavigator_subscribeToCurrentTarget(
      ContentApp contentApp,
      SuccessCallback<Byte> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean targetNavigator_subscribeToTargetList(
      ContentApp contentApp,
      SuccessCallback<Object> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  /*
   * KEYPAD INPUT CLUSTER
   */
  public native boolean keypadInput_sendKey(
      ContentApp contentApp, byte keyCode, Object responseHandler);

  /**
   * APPLICATION BASIC
   *
   * <p>TODO: Add APIs to subscribe to & read Application, Status and AllowedVendorList
   */
  public native boolean applicationBasic_subscribeToVendorName(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_subscribeToVendorID(
      ContentApp contentApp,
      SuccessCallback<Integer> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_subscribeToApplicationName(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_subscribeToProductID(
      ContentApp contentApp,
      SuccessCallback<Integer> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_subscribeToApplicationVersion(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_readVendorName(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler);

  public native boolean applicationBasic_readVendorID(
      ContentApp contentApp,
      SuccessCallback<Integer> readSuccessHandler,
      FailureCallback readFailureHandler);

  public native boolean applicationBasic_readApplicationName(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler);

  public native boolean applicationBasic_readProductID(
      ContentApp contentApp,
      SuccessCallback<Integer> readSuccessHandler,
      FailureCallback readFailureHandler);

  public native boolean applicationBasic_readApplicationVersion(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler);

  public native boolean onOff_on(ContentApp contentApp, Object responseHandler);

  public native boolean onOff_off(ContentApp contentApp, Object responseHandler);

  public native boolean onOff_toggle(ContentApp contentApp, Object responseHandler);

  static {
    System.loadLibrary("TvCastingApp");
  }
}
