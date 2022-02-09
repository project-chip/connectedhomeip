package com.chip.casting.app;

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import com.chip.casting.dnssd.CommissionerDiscoveryListener;
import com.chip.casting.util.GlobalCastingConstants;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class MainActivity extends AppCompatActivity {

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    startCommissionerDiscovery();
  }

  private void startCommissionerDiscovery() {
    WifiManager wifi = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
    WifiManager.MulticastLock multicastLock = wifi.createMulticastLock("multicastLock");
    multicastLock.setReferenceCounted(true);
    multicastLock.acquire();

    CastingContext castingContext = new CastingContext(this);
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
  }
}
