package com.chip.casting.app;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;
import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.DiagnosticDataProviderImpl;
import chip.platform.NsdManagerServiceBrowser;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesConfigurationManager;
import chip.platform.PreferencesKeyValueStoreManager;
import com.chip.casting.DACProviderStub;
import com.chip.casting.TvCastingApp;
import com.chip.casting.dnssd.DiscoveredNodeData;
import com.chip.casting.util.GlobalCastingConstants;

public class MainActivity extends AppCompatActivity
    implements CommissionerDiscoveryFragment.Callback, CommissioningFragment.Callback {

  private static final String TAG = MainActivity.class.getSimpleName();

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
  public void handleCommissioningButtonClicked(DiscoveredNodeData commissioner) {
    showFragment(CommissioningFragment.newInstance(tvCastingApp, commissioner));
  }

  @Override
  public void handleCommissioningComplete() {
    showFragment(ContentLauncherFragment.newInstance(tvCastingApp));
  }

  /**
   * The order is important, must first new TvCastingApp to load dynamic library, then
   * AndroidChipPlatform to prepare platform, then start ChipAppServer, then call init on
   * TvCastingApp
   */
  private void initJni() {
    tvCastingApp = new TvCastingApp();

    tvCastingApp.setDACProvider(new DACProviderStub());
    Context applicationContext = this.getApplicationContext();
    AndroidChipPlatform chipPlatform =
        new AndroidChipPlatform(
            new AndroidBleManager(),
            new PreferencesKeyValueStoreManager(applicationContext),
            new PreferencesConfigurationManager(applicationContext),
            new NsdManagerServiceResolver(applicationContext),
            new NsdManagerServiceBrowser(applicationContext),
            new ChipMdnsCallbackImpl(),
            new DiagnosticDataProviderImpl(applicationContext));

    chipPlatform.updateCommissionableDataProviderData(
        null, null, 0, GlobalCastingConstants.SetupPasscode, GlobalCastingConstants.Discriminator);

    chipAppServer = new ChipAppServer();
    chipAppServer.startApp();

    tvCastingApp.init();
  }

  private void showFragment(Fragment fragment, boolean showOnBack) {
    Log.d(
        TAG,
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
