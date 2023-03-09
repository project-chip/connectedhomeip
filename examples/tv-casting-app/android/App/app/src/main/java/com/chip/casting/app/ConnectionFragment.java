package com.chip.casting.app;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.chip.casting.ContentApp;
import com.chip.casting.DiscoveredNodeData;
import com.chip.casting.FailureCallback;
import com.chip.casting.MatterCallbackHandler;
import com.chip.casting.MatterError;
import com.chip.casting.SuccessCallback;
import com.chip.casting.TvCastingApp;
import com.chip.casting.VideoPlayer;
import com.chip.casting.util.GlobalCastingConstants;

/** A {@link Fragment} to get the TV Casting App commissioned / connected. */
public class ConnectionFragment extends Fragment {
  private static final String TAG = ConnectionFragment.class.getSimpleName();

  private final TvCastingApp tvCastingApp;
  private final DiscoveredNodeData selectedCommissioner;

  private boolean openCommissioningWindowSuccess;
  private boolean sendUdcSuccess;

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
      Log.d(TAG, "Calling verifyOrEstablishConnectionSuccess with VideoPlayer: " + videoPlayer);
      tvCastingApp.verifyOrEstablishConnection(
          videoPlayer, onConnectionSuccess, onConnectionFailure, onNewOrUpdatedEndpoints);
    } else {
      Log.d(TAG, "Running commissioning");
      this.openCommissioningWindowSuccess =
          tvCastingApp.openBasicCommissioningWindow(
              GlobalCastingConstants.CommissioningWindowDurationSecs,
              new MatterCallbackHandler() {
                @Override
                public void handle(MatterError error) {
                  Log.d(TAG, "handle() called on CommissioningComplete event with " + error);
                }
              },
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

          this.sendUdcSuccess = tvCastingApp.sendCommissioningRequest(selectedCommissioner);
        }
      }
    }

    return inflater.inflate(R.layout.fragment_connection, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    String commissioningWindowStatus = "";
    if (selectedCommissioner != null && selectedCommissioner.isPreCommissioned()) {
      commissioningWindowStatus = "Establishing connection with selected Video Player";
    } else {
      commissioningWindowStatus = "Failed to open commissioning window";
      if (this.openCommissioningWindowSuccess) {
        commissioningWindowStatus = "Commissioning window has been opened. Commission manually.";
        if (this.sendUdcSuccess) {
          commissioningWindowStatus =
              "Commissioning window has been opened. Commissioning requested from "
                  + selectedCommissioner.getDeviceName();
        }
        TextView onboardingPayloadView = getView().findViewById(R.id.onboardingPayload);
        onboardingPayloadView.setText(
            "Onboarding PIN: "
                + GlobalCastingConstants.SetupPasscode
                + "\nDiscriminator: "
                + GlobalCastingConstants.Discriminator);
      }
    }

    TextView commissioningWindowStatusView = getView().findViewById(R.id.commissioningWindowStatus);
    commissioningWindowStatusView.setText(commissioningWindowStatus);
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on completion of commissioning */
    void handleCommissioningComplete();
  }
}
