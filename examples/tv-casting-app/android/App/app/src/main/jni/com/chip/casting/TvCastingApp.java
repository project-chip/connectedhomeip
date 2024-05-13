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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.function.Predicate;

public class TvCastingApp {
  private static final String TAG = TvCastingApp.class.getSimpleName();
  private static final String DISCOVERY_TARGET_SERVICE_TYPE = "_matterd._udp.";
  private static final List<Long> DISCOVERY_TARGET_DEVICE_TYPE_FILTER =
      Arrays.asList(35L); // Video player = 35;

  // delay before which we assume undiscovered cached players may be in STR mode
  private static final long CHIP_DEVICE_CONFIG_STR_DISCOVERY_DELAY_SEC = 5;

  private static TvCastingApp sInstance;
  private Context applicationContext;
  private ChipAppServer chipAppServer;
  private NsdManagerServiceResolver.NsdManagerResolverAvailState nsdManagerResolverAvailState;
  private boolean discoveryStarted = false;
  private Object discoveryLock = new Object();

  private List<DiscoveredNodeData> discoveredPlayers;
  private ScheduledFuture<?> reportSleepingVideoPlayerCommissionersFuture;

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
      discoveredPlayers = new ArrayList<>();
      nsdDiscoveryListener =
          new NsdDiscoveryListener(
              nsdManager,
              DISCOVERY_TARGET_SERVICE_TYPE,
              DISCOVERY_TARGET_DEVICE_TYPE_FILTER,
              preCommissionedVideoPlayers,
              new SuccessCallback<DiscoveredNodeData>() {
                @Override
                public void handle(DiscoveredNodeData commissioner) {
                  Log.d(TAG, "Discovered commissioner added " + commissioner);
                  discoveredPlayers.add(commissioner);
                  discoverySuccessCallback.handle(commissioner);
                }
              },
              discoveryFailureCallback,
              nsdManagerResolverAvailState);

      nsdManager.discoverServices(
          DISCOVERY_TARGET_SERVICE_TYPE, NsdManager.PROTOCOL_DNS_SD, nsdDiscoveryListener);
      Log.d(TAG, "TvCastingApp.discoverVideoPlayerCommissioners started");

      /**
       * Surface players (as DiscoveredNodeData objects on discoverySuccessCallback) that we
       * previously connected to and received their WakeOnLAN MACAddress, but could not discover
       * over DNS-SD this time in CHIP_DEVICE_CONFIG_STR_DISCOVERY_DELAY_SEC. This API will also
       * ensure that the reported players were previously discoverable within
       * CHIP_DEVICE_CONFIG_STR_CACHE_LAST_DISCOVERED_HOURS.
       *
       * <p>The DiscoveredNodeData object for such players will have the IsAsleep attribute set to
       * true, which can optionally be used for any special UX treatment when displaying them.
       *
       * <p>Surfacing such players as discovered will allow displaying them to the user, who may
       * want to cast to them. In such a case, the VerifyOrEstablishConnection API will turn them on
       * over WakeOnLan.
       */
      this.reportSleepingVideoPlayerCommissionersFuture =
          Executors.newScheduledThreadPool(1)
              .schedule(
                  () -> {
                    Log.d(
                        TAG,
                        "Scheduling reportSleepingCommissioners with commissioner count "
                            + (preCommissionedVideoPlayers != null
                                ? preCommissionedVideoPlayers.size()
                                : 0));
                    reportSleepingVideoPlayerCommissioners(
                        preCommissionedVideoPlayers, discoverySuccessCallback);
                  },
                  CHIP_DEVICE_CONFIG_STR_DISCOVERY_DELAY_SEC * 1000,
                  TimeUnit.MILLISECONDS);

      this.discoveryStarted = true;
    }
  }

  private void reportSleepingVideoPlayerCommissioners(
      List<VideoPlayer> cachedVideoPlayers,
      SuccessCallback<DiscoveredNodeData> discoverySuccessCallback) {
    Log.d(
        TAG,
        "TvCastingApp.reportSleepingVideoPlayerCommissioners called with commissioner count "
            + (cachedVideoPlayers != null ? cachedVideoPlayers.size() : 0));
    if (cachedVideoPlayers == null) {
      Log.d(TAG, "No cached video players available.");
      return;
    }

    for (VideoPlayer player : cachedVideoPlayers) {
      Log.d(TAG, "Cached Video Player: " + player);
      // do NOT surface this cached Player if we don't have its MACAddress
      if (player.getMACAddress() == null) {
        Log.d(
            TAG,
            "TvCastingApp.reportSleepingVideoPlayerCommissioners Skipping Player with hostName"
                + player.getHostName()
                + " but no MACAddress");
        continue;
      }

      // do NOT surface this cached Player if it has not been discoverable recently (in
      // CHIP_DEVICE_CONFIG_STR_CACHE_LAST_DISCOVERED_HOURS)
      if (!WasRecentlyDiscoverable(player)) {
        Log.d(
            TAG,
            "TvCastingApp.reportSleepingVideoPlayerCommissioners Skipping Player with hostName"
                + player.getHostName()
                + " that has not been discovered recently");
        continue;
      }

      // do NOT surface this cached Player if it was just discovered right now (in this discovery
      // call)
      boolean justDiscovered =
          discoveredPlayers
              .stream()
              .anyMatch(
                  new Predicate<DiscoveredNodeData>() {
                    @Override
                    public boolean test(DiscoveredNodeData discoveredNodeData) {
                      return player.getHostName().equals(discoveredNodeData.getHostName());
                    }
                  });
      if (justDiscovered) {
        Log.d(
            TAG,
            "TvCastingApp.reportSleepingVideoPlayerCommissioners Skipping Player with hostName"
                + player.getHostName()
                + " that was just discovered");
        continue;
      }

      // DO surface this cached Player (as asleep)
      Log.d(TAG, "Reporting sleeping player with hostName " + player.getHostName());
      player.setIsAsleep(true);
      discoverySuccessCallback.handle(new DiscoveredNodeData(player));
    }
  }

  private native boolean WasRecentlyDiscoverable(VideoPlayer player);

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

        if (reportSleepingVideoPlayerCommissionersFuture != null) {
          reportSleepingVideoPlayerCommissionersFuture.cancel(false);
        }
        this.discoveryStarted = false;
      }
    }
  }

  void resetDiscoveryState() {
    synchronized (discoveryLock) {
      Log.d(TAG, "TvCastingApp resetting discovery state");
      this.discoveryStarted = false;
      this.nsdDiscoveryListener = null;
      if (multicastLock != null && multicastLock.isHeld()) {
        multicastLock.release();
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

  public native boolean messages_presentMessages(
      ContentApp contentApp, String messageText, Object responseHandler);

  static {
    System.loadLibrary("TvCastingApp");
  }
}
