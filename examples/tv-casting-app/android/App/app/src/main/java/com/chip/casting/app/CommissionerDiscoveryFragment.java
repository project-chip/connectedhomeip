package com.chip.casting.app;

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.chip.casting.dnssd.CommissionerDiscoveryListener;
import com.chip.casting.dnssd.DiscoveredNodeData;
import com.chip.casting.util.GlobalCastingConstants;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

/** A {@link Fragment} to discover commissioners on the network */
public class CommissionerDiscoveryFragment extends Fragment {
  private static final String TAG = CommissionerDiscoveryFragment.class.getSimpleName();

  public CommissionerDiscoveryFragment() {}

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment CommissionerDiscoveryFragment.
   */
  public static CommissionerDiscoveryFragment newInstance() {
    return new CommissionerDiscoveryFragment();
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    startCommissionerDiscovery();
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
    manualCommissioningButton.setOnClickListener(
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            callback.handleCommissioningButtonClicked(null);
          }
        });
  }

  private void startCommissionerDiscovery() {
    Log.d(TAG, "CommissionerDiscoveryFragment.startCommissionerDiscovery called");
    WifiManager wifi = (WifiManager) this.getContext().getSystemService(Context.WIFI_SERVICE);
    WifiManager.MulticastLock multicastLock = wifi.createMulticastLock("multicastLock");
    multicastLock.setReferenceCounted(true);
    multicastLock.acquire();

    CastingContext castingContext = new CastingContext(this.getActivity());
    NsdManager.DiscoveryListener commissionerDiscoveryListener =
        new CommissionerDiscoveryListener(castingContext);

    NsdManager nsdManager = castingContext.getNsdManager();
    nsdManager.discoverServices(
        GlobalCastingConstants.CommissionerServiceType,
        NsdManager.PROTOCOL_DNS_SD,
        commissionerDiscoveryListener);

    // Stop discovery after specified timeout
    Executors.newSingleThreadScheduledExecutor()
        .schedule(
            new Runnable() {
              @Override
              public void run() {
                nsdManager.stopServiceDiscovery(commissionerDiscoveryListener);
                multicastLock.release();
              }
            },
            10,
            TimeUnit.SECONDS);
    Log.d(TAG, "CommissionerDiscoveryFragment.startCommissionerDiscovery ended");
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener of Commissioning Button click. */
    void handleCommissioningButtonClicked(DiscoveredNodeData selectedCommissioner);
  }
}
