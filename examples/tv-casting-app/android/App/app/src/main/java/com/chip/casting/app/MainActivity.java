package com.chip.casting.app;

import android.content.Context;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;
import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.DiagnosticDataProviderImpl;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesConfigurationManager;
import chip.platform.PreferencesKeyValueStoreManager;
import com.chip.casting.DACProviderStub;
import com.chip.casting.TvCastingApp;
import com.chip.casting.util.GlobalCastingConstants;

public class MainActivity extends AppCompatActivity
    implements CommissionerDiscoveryFragment.Callback {

  private ChipAppServer chipAppServer;
  private TvCastingApp tvCastingApp;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    initJni();

    Fragment fragment = CommissionerDiscoveryFragment.newInstance();
    getSupportFragmentManager()
        .beginTransaction()
        .add(R.id.main_fragment_container, fragment, fragment.getClass().getSimpleName())
        .commit();
  }

  @Override
  public void handleManualCommissioningButtonClicked() {
    showFragment(CommissioningFragment.newInstance(tvCastingApp));
  }

  private void initJni() {
    tvCastingApp =
        new TvCastingApp((app, clusterId, duration) -> app.openBasicCommissioningWindow(duration));

    tvCastingApp.setDACProvider(new DACProviderStub());
    Context applicationContext = this.getApplicationContext();
    AndroidChipPlatform chipPlatform =
        new AndroidChipPlatform(
            new AndroidBleManager(),
            new PreferencesKeyValueStoreManager(applicationContext),
            new PreferencesConfigurationManager(applicationContext),
            new NsdManagerServiceResolver(applicationContext),
            new ChipMdnsCallbackImpl(),
            new DiagnosticDataProviderImpl(applicationContext));

    chipPlatform.updateCommissionableDataProviderData(
        null, null, 0, GlobalCastingConstants.SetupPasscode, GlobalCastingConstants.Discriminator);

    chipAppServer = new ChipAppServer();
    chipAppServer.startApp();
  }

  private void showFragment(Fragment fragment, boolean showOnBack) {
    System.out.println(
        "showFragment called with " + fragment.getClass().getSimpleName() + " and " + showOnBack);
    FragmentTransaction fragmentTransaction =
        getSupportFragmentManager()
            .beginTransaction()
            .replace(R.id.main_fragment_container, fragment, fragment.getClass().getSimpleName());
    if (showOnBack) {
      fragmentTransaction.addToBackStack(null);
    }

    fragmentTransaction.commit();
  }

  private void showFragment(Fragment fragment) {
    showFragment(fragment, true);
  }
}
