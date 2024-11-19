package com.chip.casting.app;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.R;
import com.chip.casting.CommissioningCallbacks;
import com.chip.casting.ContentApp;
import com.chip.casting.DiscoveredNodeData;
import com.chip.casting.FailureCallback;
import com.chip.casting.MatterCallbackHandler;
import com.chip.casting.MatterError;
import com.chip.casting.SuccessCallback;
import com.chip.casting.TvCastingApp;
import com.chip.casting.VideoPlayer;
import com.chip.casting.util.GlobalCastingConstants;
import java.util.concurrent.Executors;

/**
 * @deprecated Refer to com.matter.casting.ConnectionExampleFragment.
 *     <p>A {@link Fragment} to get the TV Casting App commissioned / connected
 */
@Deprecated
public class ConnectionFragment extends Fragment {
  private static final String TAG = ConnectionFragment.class.getSimpleName();
  private static final Integer targetContentAppVendorId = 65521;

  private final TvCastingApp tvCastingApp;
  private final DiscoveredNodeData selectedCommissioner;

  private boolean openCommissioningWindowSuccess;
  private boolean sendUdcSuccess;

  private TextView commissioningWindowStatusView;
  private TextView onboardingPayloadView;

  public ConnectionFragment(TvCastingApp tvCastingApp, DiscoveredNodeData selectedCommissioner) {
    this.tvCastingApp = tvCastingApp;
    this.selectedCommissioner = selectedCommissioner;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment CommissioningFragment.
   */
  public static ConnectionFragment newInstance(
      TvCastingApp tvCastingApp, DiscoveredNodeData selectedCommissioner) {
    return new ConnectionFragment(tvCastingApp, selectedCommissioner);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    return inflater.inflate(R.layout.fragment_connection, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    onboardingPayloadView = getView().findViewById(R.id.onboardingPayload);
    commissioningWindowStatusView = getView().findViewById(R.id.commissioningWindowStatus);

    String commissioningWindowStatus =
        (selectedCommissioner != null && selectedCommissioner.isPreCommissioned())
            ? "Establishing CASE session with video player..."
            : "Requesting to be commissioned by the video player...";
    commissioningWindowStatusView.setText(commissioningWindowStatus);

    Executors.newSingleThreadExecutor()
        .submit(
            () -> {
              Callback callback = (ConnectionFragment.Callback) this.getActivity();

              SuccessCallback<VideoPlayer> onConnectionSuccess =
                  new SuccessCallback<VideoPlayer>() {
                    @Override
                    public void handle(VideoPlayer videoPlayer) {
                      Log.d(TAG, "handle() called on OnConnectionSuccess with " + videoPlayer);
                      callback.handleCommissioningComplete();
                    }
                  };

              FailureCallback onConnectionFailure =
                  new FailureCallback() {
                    @Override
                    public void handle(MatterError matterError) {
                      Log.d(TAG, "handle() called on OnConnectionFailure with " + matterError);
                    }
                  };

              SuccessCallback<ContentApp> onNewOrUpdatedEndpoints =
                  new SuccessCallback<ContentApp>() {
                    @Override
                    public void handle(ContentApp contentApp) {
                      Log.d(TAG, "handle() called on OnNewOrUpdatedEndpoint with " + contentApp);
                    }
                  };

              if (selectedCommissioner != null && selectedCommissioner.isPreCommissioned()) {
                VideoPlayer videoPlayer = selectedCommissioner.toConnectableVideoPlayer();
                Log.d(
                    TAG,
                    "Calling verifyOrEstablishConnectionSuccess with VideoPlayer: " + videoPlayer);
                tvCastingApp.verifyOrEstablishConnection(
                    videoPlayer, onConnectionSuccess, onConnectionFailure, onNewOrUpdatedEndpoints);
              } else {
                beginCommissioning(
                    onConnectionSuccess, onConnectionFailure, onNewOrUpdatedEndpoints);
              }
            });
  }

  private void beginCommissioning(
      SuccessCallback<VideoPlayer> onConnectionSuccess,
      FailureCallback onConnectionFailure,
      SuccessCallback<ContentApp> onNewOrUpdatedEndpoints) {
    Log.d(TAG, "Running commissioning");
    MatterCallbackHandler commissioningCompleteCallback =
        new MatterCallbackHandler() {
          @Override
          public void handle(MatterError error) {
            Log.d(TAG, "handle() called on CommissioningComplete event with " + error);
          }
        };

    SuccessCallback<Void> sessionEstablishmentStartedCallback =
        new SuccessCallback<Void>() {
          @Override
          public void handle(Void response) {
            Log.d(TAG, "handle() called on SessionEstablishmentStartedCallback");
          }
        };

    SuccessCallback<Void> sessionEstablishedCallback =
        new SuccessCallback<Void>() {
          @Override
          public void handle(Void response) {
            Log.d(TAG, "handle() called on SessionEstablishedCallback");
          }
        };

    FailureCallback sessionEstablishmentErrorCallback =
        new FailureCallback() {
          @Override
          public void handle(MatterError error) {
            Log.d(TAG, "handle() called on SessionEstablishmentError event with " + error);
          }
        };

    FailureCallback sessionEstablishmentStoppedCallback =
        new FailureCallback() {
          @Override
          public void handle(MatterError error) {
            Log.d(TAG, "handle() called on SessionEstablishmentStopped event with " + error);
          }
        };

    CommissioningCallbacks commissioningCallbacks =
        new CommissioningCallbacks.Builder()
            .commissioningComplete(commissioningCompleteCallback)
            .sessionEstablishmentStarted(sessionEstablishmentStartedCallback)
            .sessionEstablished(sessionEstablishedCallback)
            .sessionEstablishmentError(sessionEstablishmentErrorCallback)
            .sessionEstablishmentStopped(sessionEstablishmentStoppedCallback)
            .build();

    this.openCommissioningWindowSuccess =
        tvCastingApp.openBasicCommissioningWindow(
            GlobalCastingConstants.CommissioningWindowDurationSecs,
            commissioningCallbacks,
            onConnectionSuccess,
            onConnectionFailure,
            onNewOrUpdatedEndpoints);

    if (this.openCommissioningWindowSuccess) {
      if (selectedCommissioner != null && selectedCommissioner.getNumIPs() > 0) {
        String ipAddress = selectedCommissioner.getIpAddresses().get(0).getHostAddress();
        Log.d(
            TAG,
            "ConnectionFragment calling tvCastingApp.sendUserDirectedCommissioningRequest with IP: "
                + ipAddress
                + " port: "
                + selectedCommissioner.getPort());

        this.sendUdcSuccess =
            tvCastingApp.sendCommissioningRequest(selectedCommissioner, targetContentAppVendorId);
        updateUiOnConnectionSuccess();
      }
    } else {
      getActivity()
          .runOnUiThread(
              () -> {
                commissioningWindowStatusView.setText("Failed to open commissioning window");
              });
    }
  }

  private void updateUiOnConnectionSuccess() {
    getActivity()
        .runOnUiThread(
            () -> {
              String finalCommissioningWindowStatus =
                  "Commissioning window has been opened. Commission manually.";
              if (this.sendUdcSuccess) {
                finalCommissioningWindowStatus =
                    "Commissioning window has been opened. Commissioning requested from "
                        + selectedCommissioner.getDeviceName();
              }
              onboardingPayloadView.setText(
                  "Onboarding PIN: "
                      + GlobalCastingConstants.SetupPasscode
                      + "\nDiscriminator: "
                      + GlobalCastingConstants.Discriminator);
              commissioningWindowStatusView.setText(finalCommissioningWindowStatus);
            });
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on completion of commissioning */
    void handleCommissioningComplete();
  }
}
