package com.chip.casting.app;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;
import com.chip.casting.AppParameters;
import com.chip.casting.DACProviderStub;
import com.chip.casting.DiscoveredNodeData;
import com.chip.casting.TvCastingApp;
import com.chip.casting.util.GlobalCastingConstants;
import java.util.Random;

public class MainActivity extends AppCompatActivity
    implements CommissionerDiscoveryFragment.Callback,
        ConnectionFragment.Callback,
        SelectClusterFragment.Callback {

  private static final String TAG = MainActivity.class.getSimpleName();

  private TvCastingApp tvCastingApp;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    initJni();

    Fragment fragment = CommissionerDiscoveryFragment.newInstance(tvCastingApp);
    getSupportFragmentManager()
        .beginTransaction()
        .add(R.id.main_fragment_container, fragment, fragment.getClass().getSimpleName())
        .commit();
  }

  @Override
  public void handleCommissioningButtonClicked(DiscoveredNodeData commissioner) {
    showFragment(ConnectionFragment.newInstance(tvCastingApp, commissioner));
  }

  @Override
  public void handleCommissioningComplete() {
    showFragment(SelectClusterFragment.newInstance(tvCastingApp));
  }

  @Override
  public void handleContentLauncherSelected() {
    showFragment(ContentLauncherFragment.newInstance(tvCastingApp));
  }

  @Override
  public void handleCertTestLauncherSelected() {
    showFragment(CertTestFragment.newInstance(tvCastingApp));
  }

  @Override
  public void handleMediaPlaybackSelected() {
    showFragment(MediaPlaybackFragment.newInstance(tvCastingApp));
  }

  @Override
  public void handleDisconnect() {
    showFragment(CommissionerDiscoveryFragment.newInstance(tvCastingApp));
  }

  /**
   * The order is important, must first new TvCastingApp to load dynamic library, then
   * AndroidChipPlatform to prepare platform, then start ChipAppServer, then call init on
   * TvCastingApp
   */
  private void initJni() {
    tvCastingApp = new TvCastingApp();

    Context applicationContext = this.getApplicationContext();

    AppParameters appParameters = new AppParameters();
    byte[] rotatingDeviceIdUniqueId =
        new byte[AppParameters.MIN_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH];
    new Random().nextBytes(rotatingDeviceIdUniqueId);
    appParameters.setRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueId);
    appParameters.setDacProvider(new DACProviderStub());
    appParameters.setSetupPasscode(GlobalCastingConstants.SetupPasscode);
    appParameters.setDiscriminator(GlobalCastingConstants.Discriminator);
    tvCastingApp.initApp(applicationContext, appParameters);
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
