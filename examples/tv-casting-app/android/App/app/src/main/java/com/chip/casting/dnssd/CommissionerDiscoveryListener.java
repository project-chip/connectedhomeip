package com.chip.casting.dnssd;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;
import com.chip.casting.app.CastingContext;
import com.chip.casting.util.GlobalCastingConstants;
import java.util.ArrayList;
import java.util.List;

public class CommissionerDiscoveryListener implements NsdManager.DiscoveryListener {

  private static final String TAG = CommissionerDiscoveryListener.class.getSimpleName();

  private final CastingContext castingContext;
  private final List<DiscoveredNodeData> commissioners = new ArrayList<>();

  public CommissionerDiscoveryListener(CastingContext castingContext) {
    this.castingContext = castingContext;
  }

  @Override
  public void onDiscoveryStarted(String regType) {
    Log.d(TAG, "Service discovery started. regType: " + regType);
  }

  @Override
  public void onServiceFound(NsdServiceInfo service) {
    Log.d(TAG, "Service discovery success. " + service);
    if (service.getServiceType().equals(GlobalCastingConstants.CommissionerServiceType)) {
      castingContext
          .getNsdManager()
          .resolveService(service, new CommissionerResolveListener(castingContext, commissioners));
    } else {
      Log.d(TAG, "Ignoring discovered service: " + service.toString());
    }
  }

  @Override
  public void onServiceLost(NsdServiceInfo service) {
    // When the network service is no longer available.
    // Internal bookkeeping code goes here.
    Log.e(TAG, "Service lost: " + service);
  }

  @Override
  public void onDiscoveryStopped(String serviceType) {
    Log.i(TAG, "Discovery stopped: " + serviceType);
  }

  @Override
  public void onStartDiscoveryFailed(String serviceType, int errorCode) {
    Log.e(TAG, "Discovery failed to start: Error code:" + errorCode);
    castingContext.getNsdManager().stopServiceDiscovery(this);
  }

  @Override
  public void onStopDiscoveryFailed(String serviceType, int errorCode) {
    Log.e(TAG, "Discovery failed to stop: Error code:" + errorCode);
    castingContext.getNsdManager().stopServiceDiscovery(this);
  }
}
