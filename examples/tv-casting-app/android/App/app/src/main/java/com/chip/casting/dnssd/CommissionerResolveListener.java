package com.chip.casting.dnssd;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import androidx.annotation.VisibleForTesting;
import com.chip.casting.app.CastingContext;
import com.chip.casting.app.CommissionerDiscoveryFragment;
import com.chip.casting.util.GlobalCastingConstants;
import java.util.List;

public class CommissionerResolveListener implements NsdManager.ResolveListener {

  private static final String TAG = CommissionerResolveListener.class.getSimpleName();
  private final CastingContext castingContext;
  private final List<DiscoveredNodeData> commissioners;

  public CommissionerResolveListener(
      CastingContext castingContext, List<DiscoveredNodeData> commissioners) {
    this.castingContext = castingContext;
    this.commissioners = commissioners;
  }

  @Override
  public void onServiceResolved(NsdServiceInfo serviceInfo) {
    DiscoveredNodeData commissioner = new DiscoveredNodeData(serviceInfo);
    Log.d(TAG, "Commissioner resolved: " + commissioner);

    if (isPassingDeviceTypeFilter(commissioner)) {
      commissioners.add(commissioner);
      String buttonText = getCommissionerButtonText(commissioner);
      if (!buttonText.isEmpty()) {
        Button commissionerButton = new Button(castingContext.getApplicationContext());
        commissionerButton.setText(buttonText);
        CommissionerDiscoveryFragment.Callback callback =
            (CommissionerDiscoveryFragment.Callback) castingContext.getFragmentActivity();
        commissionerButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                Log.d(
                    TAG,
                    "CommissionerResolveListener.onServiceResolved.OnClickListener.onClick called for "
                        + commissioner);
                callback.handleCommissioningButtonClicked(commissioner);
              }
            });
        new Handler(Looper.getMainLooper())
            .post(() -> castingContext.getCommissionersLayout().addView(commissionerButton));
      }
    } else Log.e(TAG, "Skipped displaying " + commissioner);
  }

  @Override
  public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
    switch (errorCode) {
      case NsdManager.FAILURE_ALREADY_ACTIVE:
        Log.e(TAG, "FAILURE_ALREADY_ACTIVE - Service: " + serviceInfo);
        castingContext
            .getNsdManager()
            .resolveService(
                serviceInfo, new CommissionerResolveListener(castingContext, commissioners));
        break;
      case NsdManager.FAILURE_INTERNAL_ERROR:
        Log.e(TAG, "FAILURE_INTERNAL_ERROR - Service: " + serviceInfo);
        break;
      case NsdManager.FAILURE_MAX_LIMIT:
        Log.e(TAG, "FAILURE_MAX_LIMIT - Service: " + serviceInfo);
        break;
    }
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
    return main + aux;
  }

  private boolean isPassingDeviceTypeFilter(DiscoveredNodeData commissioner) {
    return GlobalCastingConstants.CommissionerDeviceTypeFilter == null
        || GlobalCastingConstants.CommissionerDeviceTypeFilter.isEmpty()
        || GlobalCastingConstants.CommissionerDeviceTypeFilter.contains(
            commissioner.getDeviceType());
  }
}
