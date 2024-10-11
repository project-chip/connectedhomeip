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
import android.util.Log;
import androidx.annotation.Nullable;
import chip.devicecontroller.ChipClusters;
import chip.devicecontroller.ChipStructs;
import com.matter.casting.core.CastingApp;
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.core.CastingPlayerDiscovery;
import com.matter.casting.core.Endpoint;
import com.matter.casting.core.MatterCastingPlayerDiscovery;
import com.matter.casting.support.AppParameters;
import com.matter.casting.support.CommissionableData;
import com.matter.casting.support.ConnectionCallbacks;
import com.matter.casting.support.DACProvider;
import com.matter.casting.support.IdentificationDeclarationOptions;
import com.matter.casting.support.MatterCallback;
import com.matter.casting.support.MatterError;
import com.matter.casting.support.TargetAppInfo;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;

/** @deprecated Use the APIs described in /examples/tv-casting-app/APIs.md instead. */
@Deprecated
public class TvCastingApp {
  private static final String TAG = TvCastingApp.class.getSimpleName();

  private com.chip.casting.AppParameters appParametersCompat;
  private int commissioningWindowTimeout;
  private CommissioningCallbacks commissioningCallbacks;
  private SuccessCallback<VideoPlayer> onConnectionSuccess;
  private FailureCallback onConnectionFailure;
  private SuccessCallback<ContentApp> onNewOrUpdatedEndpointCallback;
  private CastingPlayer targetCastingPlayer;

  private abstract class CastingPlayerChangeListenerAdapter
      extends CastingPlayerDiscovery.CastingPlayerChangeListener {
    public abstract void initialize(SuccessCallback<DiscoveredNodeData> discoverySuccessCallback);
  }

  private CastingPlayerChangeListenerAdapter castingPlayerDiscoveryChangeListener =
      new CastingPlayerChangeListenerAdapter() {
        private SuccessCallback<DiscoveredNodeData> discoverySuccessCallbackCompat;

        @Override
        public void initialize(SuccessCallback<DiscoveredNodeData> discoverySuccessCallback) {
          this.discoverySuccessCallbackCompat = discoverySuccessCallback;
        }

        @Override
        public void onAdded(CastingPlayer castingPlayer) {
          if (discoverySuccessCallbackCompat == null) {
            Log.e(TAG, "discoverySuccessCallbackCompat not set");
            return;
          }
          discoverySuccessCallbackCompat.handleInternal(new DiscoveredNodeData((castingPlayer)));
        }

        @Override
        public void onChanged(CastingPlayer castingPlayer) {
          if (discoverySuccessCallbackCompat == null) {
            Log.e(TAG, "discoverySuccessCallbackCompat not set");
            return;
          }
          discoverySuccessCallbackCompat.handleInternal(new DiscoveredNodeData((castingPlayer)));
        }

        @Override
        public void onRemoved(CastingPlayer castingPlayer) {
          Log.e(TAG, "CastingPlayerChangeListener.onRemoved() not implemented");
        }
      };

  private interface DACProviderAdapter extends DACProvider {
    void initialize(com.chip.casting.DACProvider dacProviderCompat);
  }

  private DACProviderAdapter dacProvider =
      new DACProviderAdapter() {
        private com.chip.casting.DACProvider dacProviderCompat;

        public void initialize(com.chip.casting.DACProvider dacProviderCompat) {
          this.dacProviderCompat = dacProviderCompat;
        }

        @Override
        public byte[] GetCertificationDeclaration() {
          if (dacProviderCompat == null) {
            Log.e(TAG, "dacProviderCompat not set");
            return null;
          }
          return dacProviderCompat.GetCertificationDeclaration();
        }

        @Override
        public byte[] GetFirmwareInformation() {
          if (dacProviderCompat == null) {
            Log.e(TAG, "dacProviderCompat not set");
            return null;
          }
          return dacProviderCompat.GetFirmwareInformation();
        }

        @Override
        public byte[] GetDeviceAttestationCert() {
          if (dacProviderCompat == null) {
            Log.e(TAG, "dacProviderCompat not set");
            return null;
          }
          return dacProviderCompat.GetDeviceAttestationCert();
        }

        @Override
        public byte[] GetProductAttestationIntermediateCert() {
          if (dacProviderCompat == null) {
            Log.e(TAG, "dacProviderCompat not set");
            return null;
          }
          return dacProviderCompat.GetProductAttestationIntermediateCert();
        }

        @Override
        public byte[] SignWithDeviceAttestationKey(byte[] message) {
          if (dacProviderCompat == null) {
            Log.e(TAG, "dacProviderCompat not set");
            return null;
          }
          return dacProviderCompat.SignWithDeviceAttestationKey(message);
        }
      };

  private static final List<Long> DISCOVERY_TARGET_DEVICE_TYPE_FILTER =
      Arrays.asList(35L); // Matter Casting Video player = 35;

  private static TvCastingApp sInstance;

  private TvCastingApp() {}

  public static TvCastingApp getInstance() {
    if (sInstance == null) {
      sInstance = new TvCastingApp();
    }
    return sInstance;
  }

  public boolean initApp(
      Context applicationContext, com.chip.casting.AppParameters appParametersCompat) {
    if (applicationContext == null
        || appParametersCompat == null
        || appParametersCompat.getConfigurationManager() == null) {
      return false;
    }

    this.appParametersCompat = appParametersCompat;

    AppParameters appParameters =
        new AppParameters(
            applicationContext,
            () -> appParametersCompat.getConfigurationManager(),
            () -> appParametersCompat.getRotatingDeviceIdUniqueId(),
            () -> {
              CommissionableData commissionableData =
                  new CommissionableData(
                      appParametersCompat.getSetupPasscode(),
                      appParametersCompat.getDiscriminator());
              commissionableData.setSpake2pIterationCount(
                  appParametersCompat.getSpake2pIterationCount());
              commissionableData.setSpake2pSaltBase64(appParametersCompat.getSpake2pSaltBase64());
              commissionableData.setSpake2pVerifierBase64(
                  appParametersCompat.getSpake2pVerifierBase64());
              return commissionableData;
            },
            this.dacProvider);

    // Initialize the SDK using the appParameters and check if it returns successfully
    MatterError err = CastingApp.getInstance().initialize(appParameters);
    if (err.hasError()) {
      Log.e(TAG, "Failed to initialize Matter CastingApp. Err: " + err);
      return false;
    }

    err = CastingApp.getInstance().start();
    if (err.hasError()) {
      Log.e(TAG, "Failed to start Matter CastingApp. Err: " + err);
      return false;
    }
    return true;
  }

  public void setDACProvider(com.chip.casting.DACProvider dacProviderCompat) {
    this.dacProvider.initialize(dacProviderCompat);
  }

  public void discoverVideoPlayerCommissioners(
      SuccessCallback<DiscoveredNodeData> discoverySuccessCallback,
      FailureCallback discoveryFailureCallback) {

    // stop before starting another discovery session
    stopVideoPlayerDiscovery();

    castingPlayerDiscoveryChangeListener.initialize(discoverySuccessCallback);
    MatterError err =
        MatterCastingPlayerDiscovery.getInstance()
            .addCastingPlayerChangeListener(castingPlayerDiscoveryChangeListener);
    if (err.hasError()) {
      Log.e(TAG, "addCastingPlayerChangeListener before starting discovery failed. Err: " + err);
      discoveryFailureCallback.handleInternal(new com.chip.casting.MatterError(err));
      return;
    }

    err =
        MatterCastingPlayerDiscovery.getInstance()
            .startDiscovery(DISCOVERY_TARGET_DEVICE_TYPE_FILTER.get(0));
    if (err.hasError()) {
      Log.e(TAG, "startDiscovery failed. Err: " + err);
      discoveryFailureCallback.handleInternal(new com.chip.casting.MatterError(err));
    }
  }

  public void stopVideoPlayerDiscovery() {
    MatterError err = MatterCastingPlayerDiscovery.getInstance().stopDiscovery();
    if (err.hasError()) {
      Log.e(TAG, "stopDiscovery failed. Err: " + err);
      return;
    }

    err =
        MatterCastingPlayerDiscovery.getInstance()
            .removeCastingPlayerChangeListener(castingPlayerDiscoveryChangeListener);
    if (err.hasError()) {
      Log.e(TAG, "removeCastingPlayerChangeListener failed. Err: " + err);
    }
  }

  public boolean openBasicCommissioningWindow(
      int commissioningWindowTimeout,
      CommissioningCallbacks commissioningCallbacks,
      SuccessCallback<VideoPlayer> onConnectionSuccess,
      FailureCallback onConnectionFailure,
      SuccessCallback<ContentApp> onNewOrUpdatedEndpointCallback) {
    this.commissioningWindowTimeout = commissioningWindowTimeout;
    this.commissioningCallbacks = commissioningCallbacks;
    this.onConnectionSuccess = onConnectionSuccess;
    this.onConnectionFailure = onConnectionFailure;
    this.onNewOrUpdatedEndpointCallback = onNewOrUpdatedEndpointCallback;
    return true;
  }

  public boolean sendCommissioningRequest(DiscoveredNodeData commissioner, Integer vendorId) {
    if (commissioningCallbacks == null
        || onConnectionSuccess == null
        || onConnectionFailure == null
        || onNewOrUpdatedEndpointCallback == null) {
      Log.e(TAG, "Commissioning/connection callbacks not set");
      return false;
    }

    this.targetCastingPlayer = commissioner.getCastingPlayer();

    IdentificationDeclarationOptions idOptions = new IdentificationDeclarationOptions();
    idOptions.addTargetAppInfo(new TargetAppInfo(vendorId, null));

    MatterError err =
        targetCastingPlayer.verifyOrEstablishConnection(
            new ConnectionCallbacks(
                new MatterCallback<Void>() {
                  @Override
                  public void handle(Void response) {
                    ((MatterCallbackHandler) commissioningCallbacks.getCommissioningComplete())
                        .handleInternal(com.chip.casting.MatterError.NO_ERROR);
                    onConnectionSuccess.handleInternal(new VideoPlayer(targetCastingPlayer));

                    List<Endpoint> endpoints = targetCastingPlayer.getEndpoints();
                    if (endpoints != null) {
                      for (Endpoint endpoint : endpoints) {
                        onNewOrUpdatedEndpointCallback.handleInternal(new ContentApp(endpoint));
                      }
                    }
                  }
                },
                new MatterCallback<MatterError>() {
                  @Override
                  public void handle(MatterError err) {
                    onConnectionFailure.handleInternal(new com.chip.casting.MatterError(err));
                  }
                },
                null),
            (short) this.commissioningWindowTimeout,
            idOptions);

    if (err.hasError()) {
      Log.e(
          TAG, "sendCommissioningRequest failed to call verifyOrEstablishConnection. Err: " + err);
      return false;
    }

    return true;
  }

  public boolean verifyOrEstablishConnection(
      VideoPlayer targetVideoPlayer,
      SuccessCallback<VideoPlayer> onConnectionSuccess,
      FailureCallback onConnectionFailure,
      SuccessCallback<ContentApp> onNewOrUpdatedEndpointCallback) {
    Log.e(TAG, "verifyOrEstablishConnection is unsupported");
    return false;
  }

  public void shutdownAllSubscriptions() {
    MatterError err = CastingApp.getInstance().shutdownAllSubscriptions();
    if (err.hasError()) {
      Log.e(TAG, "shutdownAllSubscriptions failed. Err: " + err);
    }
  }

  public void disconnect() {
    if (targetCastingPlayer != null) {
      targetCastingPlayer.disconnect();
    }
  }

  public boolean purgeCache() {
    MatterError err = CastingApp.getInstance().clearCache();
    if (err.hasError()) {
      Log.e(TAG, "purgeCache failed. Err: " + err);
      return false;
    }
    return true;
  }

  private Endpoint getTargetEndpoint(ContentApp contentApp) {
    if (targetCastingPlayer == null || targetCastingPlayer.getEndpoints() == null) {
      return null;
    }

    for (Endpoint endpoint : targetCastingPlayer.getEndpoints()) {
      if (endpoint.getId() == contentApp.getEndpointId()) {
        return endpoint;
      }
    }

    return null;
  }

  public boolean contentLauncherLaunchURL(
      ContentApp contentApp, String contentUrl, String contentDisplayStr, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ContentLauncherCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.ContentLauncherCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.launchURL(
        new ChipClusters.ContentLauncherCluster.LauncherResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "LauncherResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        contentUrl,
        Optional.of(contentDisplayStr),
        Optional.empty());

    return true;
  }

  public boolean contentLauncher_launchContent(
      ContentApp contentApp,
      ContentLauncherTypes.ContentSearch searchCompat,
      boolean autoPlay,
      String data,
      Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ContentLauncherCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.ContentLauncherCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    // translate searchCompat
    ArrayList<ChipStructs.ContentLauncherClusterParameterStruct> parameterList = null;
    if (searchCompat.parameterList != null) {
      parameterList = new ArrayList<>();
      for (ContentLauncherTypes.Parameter parameterCompat : searchCompat.parameterList) {
        ArrayList<ChipStructs.ContentLauncherClusterAdditionalInfoStruct> externalIDList = null;
        if (parameterCompat.externalIDList != null && parameterCompat.externalIDList.isPresent()) {
          externalIDList = new ArrayList<>();
          for (ContentLauncherTypes.AdditionalInfo additionalInfoCompat :
              parameterCompat.externalIDList.get()) {
            externalIDList.add(
                new ChipStructs.ContentLauncherClusterAdditionalInfoStruct(
                    additionalInfoCompat.name, additionalInfoCompat.value));
          }
        }
        parameterList.add(
            new ChipStructs.ContentLauncherClusterParameterStruct(
                parameterCompat.type,
                parameterCompat.value,
                externalIDList != null ? Optional.of(externalIDList) : Optional.empty()));
      }
    }
    ChipStructs.ContentLauncherClusterContentSearchStruct search =
        new ChipStructs.ContentLauncherClusterContentSearchStruct(parameterList);

    cluster.launchContent(
        new ChipClusters.ContentLauncherCluster.LauncherResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "LauncherResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        search,
        autoPlay,
        data != null ? Optional.of(data) : Optional.empty(),
        Optional.empty(),
        Optional.empty());
    return true;
  }

  public boolean mediaPlayback_play(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.play(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean mediaPlayback_pause(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.pause(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean mediaPlayback_stopPlayback(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.stop(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean mediaPlayback_next(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.next(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean mediaPlayback_previous(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;

    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.previous(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean mediaPlayback_rewind(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.rewind(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        Optional.empty());
    return true;
  }

  public boolean mediaPlayback_fastForward(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.fastForward(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        Optional.empty());
    return true;
  }

  public boolean mediaPlayback_startOver(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.startOver(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean mediaPlayback_seek(ContentApp contentApp, long position, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.seek(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        position);
    return true;
  }

  public boolean mediaPlayback_skipForward(
      ContentApp contentApp, long deltaPositionMilliseconds, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.skipForward(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        deltaPositionMilliseconds);
    return true;
  }

  public boolean mediaPlayback_skipBackward(
      ContentApp contentApp, long deltaPositionMilliseconds, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.skipBackward(
        new ChipClusters.MediaPlaybackCluster.PlaybackResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "PlaybackResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        deltaPositionMilliseconds);
    return true;
  }

  public boolean mediaPlayback_subscribeToCurrentState(
      ContentApp contentApp,
      SuccessCallback<MediaPlaybackTypes.PlaybackStateEnum> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MediaPlaybackCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.subscribeCurrentStateAttribute(
        new ChipClusters.IntegerAttributeCallback() {
          @Override
          public void onSuccess(int value) {
            readSuccessHandler.handleInternal(MediaPlaybackTypes.PlaybackStateEnum.values()[value]);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "IntegerAttributeCallback.onError: " + error);
            readFailureHandler.handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }

          @Override
          public void onSubscriptionEstablished(long subscriptionId) {
            subscriptionEstablishedHandler.handleInternal();
          }
        },
        minInterval,
        maxInterval);
    return true;
  }

  public boolean applicationLauncher_launchApp(
      ContentApp contentApp,
      short catalogVendorId,
      String applicationId,
      byte[] data,
      Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ApplicationLauncherCluster cluster =
        endpoint != null
            ? endpoint.getCluster(ChipClusters.ApplicationLauncherCluster.class)
            : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    // translate to application
    Optional<ChipStructs.ApplicationLauncherClusterApplicationStruct> application =
        Optional.empty();
    if (applicationId != null) {
      application =
          Optional.of(
              new ChipStructs.ApplicationLauncherClusterApplicationStruct(
                  (int) catalogVendorId, applicationId));
    }

    cluster.launchApp(
        new ChipClusters.ApplicationLauncherCluster.LauncherResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<byte[]> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "LauncherResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        application,
        data != null ? Optional.of(data) : Optional.empty());
    return true;
  }

  public boolean applicationLauncher_stopApp(
      ContentApp contentApp, short catalogVendorId, String applicationId, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ApplicationLauncherCluster cluster =
        endpoint != null
            ? endpoint.getCluster(ChipClusters.ApplicationLauncherCluster.class)
            : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    // translate to application
    Optional<ChipStructs.ApplicationLauncherClusterApplicationStruct> application =
        Optional.empty();
    if (applicationId != null) {
      application =
          Optional.of(
              new ChipStructs.ApplicationLauncherClusterApplicationStruct(
                  (int) catalogVendorId, applicationId));
    }

    cluster.stopApp(
        new ChipClusters.ApplicationLauncherCluster.LauncherResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<byte[]> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "LauncherResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        application);
    return true;
  }

  public boolean applicationLauncher_hideApp(
      ContentApp contentApp, short catalogVendorId, String applicationId, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ApplicationLauncherCluster cluster =
        endpoint != null
            ? endpoint.getCluster(ChipClusters.ApplicationLauncherCluster.class)
            : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    // translate to application
    Optional<ChipStructs.ApplicationLauncherClusterApplicationStruct> application =
        Optional.empty();
    if (applicationId != null) {
      application =
          Optional.of(
              new ChipStructs.ApplicationLauncherClusterApplicationStruct(
                  (int) catalogVendorId, applicationId));
    }

    cluster.hideApp(
        new ChipClusters.ApplicationLauncherCluster.LauncherResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<byte[]> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "LauncherResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        application);
    return true;
  }

  public boolean targetNavigator_navigateTarget(
      ContentApp contentApp, byte target, String data, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.TargetNavigatorCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.TargetNavigatorCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.navigateTarget(
        new ChipClusters.TargetNavigatorCluster.NavigateTargetResponseCallback() {
          @Override
          public void onSuccess(Integer status, Optional<String> data) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "NavigateTargetResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        (int) target,
        data != null ? Optional.of(data) : Optional.empty());
    return true;
  }

  public boolean targetNavigator_subscribeToTargetList(
      ContentApp contentApp,
      SuccessCallback<Object> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.TargetNavigatorCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.TargetNavigatorCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.subscribeTargetListAttribute(
        new ChipClusters.TargetNavigatorCluster.TargetListAttributeCallback() {
          @Override
          public void onSuccess(
              List<ChipStructs.TargetNavigatorClusterTargetInfoStruct> targetInfoList) {
            List<TargetNavigatorTypes.TargetInfo> targetInfoListCompat = null;
            if (targetInfoList != null) {
              targetInfoListCompat = new ArrayList<>();
              for (ChipStructs.TargetNavigatorClusterTargetInfoStruct targetInfo : targetInfoList) {
                targetInfoListCompat.add(
                    new TargetNavigatorTypes.TargetInfo(targetInfo.identifier, targetInfo.name));
              }
            }
            readSuccessHandler.handleInternal(targetInfoListCompat);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "TargetListAttributeCallback.onError: " + error);
            readFailureHandler.handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }

          @Override
          public void onSubscriptionEstablished(long subscriptionId) {
            subscriptionEstablishedHandler.handleInternal();
          }
        },
        minInterval,
        maxInterval);
    return true;
  }

  public boolean keypadInput_sendKey(ContentApp contentApp, byte keyCode, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.KeypadInputCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.KeypadInputCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.sendKey(
        new ChipClusters.KeypadInputCluster.SendKeyResponseCallback() {
          @Override
          public void onSuccess(Integer status) {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "LauncherResponseCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        (int) keyCode);
    return true;
  }

  public boolean applicationBasic_readVendorName(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ApplicationBasicCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.ApplicationBasicCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.readVendorNameAttribute(
        new ChipClusters.CharStringAttributeCallback() {
          @Override
          public void onSuccess(String value) {
            readSuccessHandler.handleInternal(value);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "CharStringAttributeCallback.onError: " + error);
            readFailureHandler.handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean applicationBasic_readVendorID(
      ContentApp contentApp,
      SuccessCallback<Integer> readSuccessHandler,
      FailureCallback readFailureHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ApplicationBasicCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.ApplicationBasicCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.readVendorIDAttribute(
        new ChipClusters.IntegerAttributeCallback() {
          @Override
          public void onSuccess(int value) {
            readSuccessHandler.handleInternal(value);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "IntegerAttributeCallback.onError: " + error);
            readFailureHandler.handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean applicationBasic_readApplicationName(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ApplicationBasicCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.ApplicationBasicCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.readApplicationNameAttribute(
        new ChipClusters.CharStringAttributeCallback() {
          @Override
          public void onSuccess(String value) {
            readSuccessHandler.handleInternal(value);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "CharStringAttributeCallback.onError: " + error);
            readFailureHandler.handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean applicationBasic_readProductID(
      ContentApp contentApp,
      SuccessCallback<Integer> readSuccessHandler,
      FailureCallback readFailureHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ApplicationBasicCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.ApplicationBasicCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.readProductIDAttribute(
        new ChipClusters.IntegerAttributeCallback() {
          @Override
          public void onSuccess(int value) {
            readSuccessHandler.handleInternal(value);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "IntegerAttributeCallback.onError: " + error);
            readFailureHandler.handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean applicationBasic_readApplicationVersion(
      ContentApp contentApp,
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.ApplicationBasicCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.ApplicationBasicCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.readApplicationVersionAttribute(
        new ChipClusters.CharStringAttributeCallback() {
          @Override
          public void onSuccess(String value) {
            readSuccessHandler.handleInternal(value);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "CharStringAttributeCallback.onError: " + error);
            readFailureHandler.handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean onOff_on(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.OnOffCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.OnOffCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.on(
        new ChipClusters.DefaultClusterCallback() {
          @Override
          public void onSuccess() {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "DefaultClusterCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean onOff_off(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.OnOffCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.OnOffCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.off(
        new ChipClusters.DefaultClusterCallback() {
          @Override
          public void onSuccess() {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "DefaultClusterCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean onOff_toggle(ContentApp contentApp, Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.OnOffCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.OnOffCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.toggle(
        new ChipClusters.DefaultClusterCallback() {
          @Override
          public void onSuccess() {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "DefaultClusterCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        });
    return true;
  }

  public boolean messages_presentMessages(
      ContentApp contentApp,
      byte[] messageID,
      Integer priority,
      Integer messageControl,
      @Nullable Long startTime,
      @Nullable Long duration,
      String messageText,
      Optional<ArrayList<ChipStructs.MessagesClusterMessageResponseOptionStruct>> responses,
      Object responseHandler) {

    Endpoint endpoint = getTargetEndpoint(contentApp);
    ChipClusters.MessagesCluster cluster =
        endpoint != null ? endpoint.getCluster(ChipClusters.MessagesCluster.class) : null;
    if (cluster == null) {
      Log.e(TAG, "Cluster not found");
      return false;
    }

    cluster.presentMessagesRequest(
        new ChipClusters.DefaultClusterCallback() {
          @Override
          public void onSuccess() {
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.NO_ERROR);
          }

          @Override
          public void onError(Exception error) {
            Log.e(TAG, "DefaultClusterCallback.onError: " + error);
            ((MatterCallbackHandler) responseHandler)
                .handleInternal(com.chip.casting.MatterError.MATTER_INTERNAL_ERROR);
          }
        },
        messageID,
        priority,
        messageControl,
        startTime,
        duration,
        messageText,
        responses);
    return true;
  }
}
