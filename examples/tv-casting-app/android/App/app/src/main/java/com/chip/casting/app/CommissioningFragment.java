package com.chip.casting.app;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.chip.casting.TvCastingApp;
import com.chip.casting.dnssd.DiscoveredNodeData;
import com.chip.casting.util.GlobalCastingConstants;

/** A {@link Fragment} to get the TV Casting App commissioned. */
public class CommissioningFragment extends Fragment {
  private static final String TAG = CommissioningFragment.class.getSimpleName();

  private final TvCastingApp tvCastingApp;
  private final DiscoveredNodeData selectedCommissioner;

  public CommissioningFragment(TvCastingApp tvCastingApp, DiscoveredNodeData selectedCommissioner) {
    this.tvCastingApp = tvCastingApp;
    this.selectedCommissioner = selectedCommissioner;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param tvCastingApp TV Casting App (JNI)
   * @return A new instance of fragment CommissioningFragment.
   */
  // TODO: Rename and change types and number of parameters
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
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_commissioning, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    String commissioningWindowStatus = "Failed to open commissioning window";
    if (tvCastingApp.openBasicCommissioningWindow(
        GlobalCastingConstants.CommissioningWindowDurationSecs)) {
      commissioningWindowStatus = "Commissioning window has been opened. Commission manually.";
      if (selectedCommissioner != null && selectedCommissioner.getNumIPs() > 0) {
        String ipAddress = selectedCommissioner.getIpAddresses().get(0).getHostAddress();
        Log.d(
            TAG,
            "CommissioningFragment calling tvCastingApp.sendUserDirectedCommissioningRequest with IP: "
                + ipAddress
                + " port: "
                + selectedCommissioner.getPort());
        if (tvCastingApp.sendUserDirectedCommissioningRequest(
            ipAddress, selectedCommissioner.getPort())) {
          commissioningWindowStatus =
              "Commissioning window has been opened. Commissioning requested from "
                  + selectedCommissioner.getDeviceName();
        }
      }
      TextView onboardingPayloadView = getView().findViewById(R.id.onboardingPayload);
      onboardingPayloadView.setText("Onboarding PIN: " + GlobalCastingConstants.SetupPasscode);
    }
    TextView commissioningWindowStatusView = getView().findViewById(R.id.commissioningWindowStatus);
    commissioningWindowStatusView.setText(commissioningWindowStatus);
  }
}
