package com.chip.casting.dnssd;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Button;
import androidx.annotation.VisibleForTesting;
import com.chip.casting.app.CastingContext;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;

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
    DiscoveredNodeData commissioner = buildCommissioner(serviceInfo);
    commissioners.add(commissioner);
    Log.d(TAG, "Commissioner resolved: " + commissioner);

    String buttonText = getCommissionerButtonText(commissioner);
    if (!buttonText.isEmpty()) {
      Button commissionerButton = new Button(castingContext.getApplicationContext());
      commissionerButton.setText(buttonText);
      new Handler(Looper.getMainLooper())
          .post(() -> castingContext.getCommissionersLayout().addView(commissionerButton));
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
  public DiscoveredNodeData buildCommissioner(NsdServiceInfo serviceInfo) {
    DiscoveredNodeData commissioner = new DiscoveredNodeData();
    serviceInfo.getAttributes();
    Map<String, byte[]> attributes = serviceInfo.getAttributes();
    commissioner.setDeviceName(new String(attributes.get("DN"), StandardCharsets.UTF_8));
    short deviceType = Short.parseShort(new String(attributes.get("DT"), StandardCharsets.UTF_8));
    commissioner.setDeviceType(deviceType);

    String vp = new String(attributes.get("VP"), StandardCharsets.UTF_8);
    if (vp != null) {
      String[] vpArray = vp.split("\\+");
      if (vpArray.length > 0) {
        commissioner.setVendorId(Short.parseShort(vpArray[0]));
        if (vpArray.length == 2) {
          commissioner.setProductId(Short.parseShort(vpArray[1]));
        }
      }
    }
    return commissioner;
  }

  @VisibleForTesting
  public String getCommissionerButtonText(DiscoveredNodeData commmissioner) {
    String main = commmissioner.getDeviceName() != null ? commmissioner.getDeviceName() : "";
    String aux =
        "" + (commmissioner.getProductId() > 0 ? "Product ID: " + commmissioner.getProductId() : "");
    aux +=
        commmissioner.getDeviceType() > 0
            ? (aux.isEmpty() ? "" : " ") + "Device Type: " + commmissioner.getDeviceType()
            : "";
    aux +=
        commmissioner.getVendorId() > 0
            ? (aux.isEmpty() ? "" : " from ") + "Vendor ID: " + commmissioner.getVendorId()
            : "";
    aux = aux.isEmpty() ? aux : "\n[" + aux + "]";
    return main + aux;
  }
}
