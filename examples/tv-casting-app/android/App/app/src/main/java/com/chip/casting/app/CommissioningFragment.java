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
import com.chip.casting.dnssd.CommissionerDiscoveryListener;
import com.chip.casting.util.GlobalCastingConstants;

/** A {@link Fragment} to get the TV Casting App commissioned. */
public class CommissioningFragment extends Fragment {
  private static final String TAG = CommissionerDiscoveryListener.class.getSimpleName();

  private final TvCastingApp tvCastingApp;

  public CommissioningFragment(TvCastingApp tvCastingApp) {
    this.tvCastingApp = tvCastingApp;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param tvCastingApp TV Casting App (JNI)
   * @return A new instance of fragment CommissioningFragment.
   */
  // TODO: Rename and change types and number of parameters
  public static CommissioningFragment newInstance(TvCastingApp tvCastingApp) {
    return new CommissioningFragment(tvCastingApp);
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
    boolean status =
        tvCastingApp.openBasicCommissioningWindow(
            GlobalCastingConstants.CommissioningWindowDurationSecs);
    Log.d(
        TAG,
        "CommissioningFragment.onViewCreated tvCastingApp.openBasicCommissioningWindow returned "
            + status);
    TextView commissioningWindowStatusView = getView().findViewById(R.id.commissioningWindowStatus);
    TextView onboardingPayloadView = getView().findViewById(R.id.onboardingPayload);
    if (status == true) {
      commissioningWindowStatusView.setText("Commissioning window opened!");
      onboardingPayloadView.setText("Onboarding Pin: " + GlobalCastingConstants.SetupPasscode);
    } else {
      commissioningWindowStatusView.setText("Commissioning window could not be opened!");
    }
  }
}
