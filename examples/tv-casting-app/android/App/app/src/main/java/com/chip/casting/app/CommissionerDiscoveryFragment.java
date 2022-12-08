package com.chip.casting.app;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import com.chip.casting.DiscoveredNodeData;
import com.chip.casting.FailureCallback;
import com.chip.casting.MatterError;
import com.chip.casting.SuccessCallback;
import com.chip.casting.TvCastingApp;

/** A {@link Fragment} to discover commissioners on the network */
public class CommissionerDiscoveryFragment extends Fragment {
  private static final String TAG = CommissionerDiscoveryFragment.class.getSimpleName();
  private static final long DISCOVERY_DURATION_SECS = 10;
  private final TvCastingApp tvCastingApp;

  public CommissionerDiscoveryFragment(TvCastingApp tvCastingApp) {
    this.tvCastingApp = tvCastingApp;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment CommissionerDiscoveryFragment.
   */
  public static CommissionerDiscoveryFragment newInstance(TvCastingApp tvCastingApp) {
    return new CommissionerDiscoveryFragment(tvCastingApp);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_commissioner_discovery, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);

    Button manualCommissioningButton = getView().findViewById(R.id.manualCommissioningButton);
    Callback callback = (Callback) this.getActivity();
    View.OnClickListener manualCommissioningButtonOnClickListener =
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            callback.handleCommissioningButtonClicked(null);
          }
        };
    manualCommissioningButton.setOnClickListener(manualCommissioningButtonOnClickListener);

    Context context = this.getContext();
    SuccessCallback<DiscoveredNodeData> successCallback =
        new SuccessCallback<DiscoveredNodeData>() {
          @Override
          public void handle(DiscoveredNodeData discoveredNodeData) {
            Log.d(TAG, "Discovered a Video Player Commissioner: " + discoveredNodeData);
            String buttonText = getCommissionerButtonText(discoveredNodeData);

            if (!buttonText.isEmpty()) {
              Button commissionerButton = new Button(context);
              commissionerButton.setText(buttonText);
              CommissionerDiscoveryFragment.Callback callback =
                  (CommissionerDiscoveryFragment.Callback) getActivity();
              commissionerButton.setOnClickListener(
                  new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                      Log.d(
                          TAG,
                          "CommissionerResolveListener.onServiceResolved.OnClickListener.onClick called for "
                              + discoveredNodeData);
                      callback.handleCommissioningButtonClicked(discoveredNodeData);
                    }
                  });
              new Handler(Looper.getMainLooper())
                  .post(
                      () ->
                          ((LinearLayout) getActivity().findViewById(R.id.castingCommissioners))
                              .addView(commissionerButton));
            }
          }
        };

    FailureCallback failureCallback =
        new FailureCallback() {
          @Override
          public void handle(MatterError matterError) {
            Log.e(TAG, "Error occurred during video player commissioner discovery: " + matterError);
          }
        };

    Button discoverButton = getView().findViewById(R.id.discoverButton);
    discoverButton.setOnClickListener(
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            Log.d(TAG, "Discovering on button click");
            tvCastingApp.discoverVideoPlayerCommissioners(
                DISCOVERY_DURATION_SECS, successCallback, failureCallback);
          }
        });

    Log.d(TAG, "Auto discovering");
    tvCastingApp.discoverVideoPlayerCommissioners(
        DISCOVERY_DURATION_SECS, successCallback, failureCallback);
  }

  @VisibleForTesting
  public String getCommissionerButtonText(DiscoveredNodeData commissioner) {
    String main = commissioner.getDeviceName() != null ? commissioner.getDeviceName() : "";
    String aux =
        "" + (commissioner.getProductId() > 0 ? "Product ID: " + commissioner.getProductId() : "");
    aux +=
        commissioner.getDeviceType() > 0
            ? (aux.isEmpty() ? "" : " ") + "Device Type: " + commissioner.getDeviceType()
            : "";
    aux +=
        commissioner.getVendorId() > 0
            ? (aux.isEmpty() ? "" : " from ") + "Vendor ID: " + commissioner.getVendorId()
            : "";
    aux = aux.isEmpty() ? aux : "\n[" + aux + "]";

    String preCommissioned = commissioner.isPreCommissioned() ? " (Pre-commissioned)" : "";
    return main + aux + preCommissioned;
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener of Commissioning Button click. */
    void handleCommissioningButtonClicked(DiscoveredNodeData selectedCommissioner);
  }
}
