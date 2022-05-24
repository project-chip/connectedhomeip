package com.chip.casting.app;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.chip.casting.MatterCallbackHandler;
import com.chip.casting.TvCastingApp;
import com.chip.casting.dnssd.DiscoveredNodeData;
import com.chip.casting.util.GlobalCastingConstants;

/** A {@link Fragment} to get the TV Casting App commissioned. */
public class CommissioningFragment extends Fragment {
  private static final String TAG = CommissioningFragment.class.getSimpleName();

  private final TvCastingApp tvCastingApp;
  private final DiscoveredNodeData selectedCommissioner;

  private boolean openCommissioningWindowSuccess;
  private boolean sendUdcSuccess;

  public CommissioningFragment(TvCastingApp tvCastingApp, DiscoveredNodeData selectedCommissioner) {
    this.tvCastingApp = tvCastingApp;
    this.selectedCommissioner = selectedCommissioner;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment CommissioningFragment.
   */
  public static CommissioningFragment newInstance(
      TvCastingApp tvCastingApp, DiscoveredNodeData selectedCommissioner) {
    return new CommissioningFragment(tvCastingApp, selectedCommissioner);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    Callback callback = (CommissioningFragment.Callback) this.getActivity();
    this.openCommissioningWindowSuccess =
        tvCastingApp.openBasicCommissioningWindow(
            GlobalCastingConstants.CommissioningWindowDurationSecs,
            new MatterCallbackHandler() {
              @Override
              public void handle(Status status) {
                Log.d(TAG, "handle() called on CommissioningComplete event with " + status);
                if (status.isSuccess()) {
                  callback.handleCommissioningComplete();
                }
              }
            });
    if (this.openCommissioningWindowSuccess) {
      if (selectedCommissioner != null && selectedCommissioner.getNumIPs() > 0) {
        String ipAddress = selectedCommissioner.getIpAddresses().get(0).getHostAddress();
        Log.d(
            TAG,
            "CommissioningFragment calling tvCastingApp.sendUserDirectedCommissioningRequest with IP: "
                + ipAddress
                + " port: "
                + selectedCommissioner.getPort());

        this.sendUdcSuccess =
            tvCastingApp.sendUserDirectedCommissioningRequest(
                ipAddress, selectedCommissioner.getPort());
      }
    }

    return inflater.inflate(R.layout.fragment_commissioning, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    String commissioningWindowStatus = "Failed to open commissioning window";
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

    TextView commissioningWindowStatusView = getView().findViewById(R.id.commissioningWindowStatus);
    commissioningWindowStatusView.setText(commissioningWindowStatus);
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on completion of commissioning */
    void handleCommissioningComplete();
  }
}
