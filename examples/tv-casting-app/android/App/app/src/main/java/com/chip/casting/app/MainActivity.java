package com.chip.casting.app;

import android.net.nsd.NsdManager;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import com.chip.casting.dnssd.CommissionerDiscoveryListener;
import com.chip.casting.util.GlobalCastingConstants;

public class MainActivity extends AppCompatActivity {

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    startCommissionerDiscovery();
  }

  private void startCommissionerDiscovery() {
    CastingContext castingContext = new CastingContext(this);
    NsdManager.DiscoveryListener commissionerDiscoveryListener =
        new CommissionerDiscoveryListener(castingContext);
    castingContext
        .getNsdManager()
        .discoverServices(
            GlobalCastingConstants.CommissionerServiceType,
            NsdManager.PROTOCOL_DNS_SD,
            commissionerDiscoveryListener);
  }
}
