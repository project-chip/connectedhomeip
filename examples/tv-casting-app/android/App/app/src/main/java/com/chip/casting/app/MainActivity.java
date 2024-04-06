package com.chip.casting.app;

import android.os.Bundle;
import android.util.Log;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;
import com.R;
import com.chip.casting.AppParameters;
import com.chip.casting.DiscoveredNodeData;
import com.chip.casting.TvCastingApp;
import com.chip.casting.util.GlobalCastingConstants;
import com.matter.casting.ActionSelectorFragment;
import com.matter.casting.ConnectionExampleFragment;
import com.matter.casting.ContentLauncherLaunchURLExampleFragment;
import com.matter.casting.DiscoveryExampleFragment;
import com.matter.casting.InitializationExample;
import com.matter.casting.PreferencesConfigurationManager;
import com.matter.casting.core.CastingPlayer;
import java.util.Random;

public class MainActivity extends AppCompatActivity
    implements CommissionerDiscoveryFragment.Callback,
        ConnectionFragment.Callback,
        SelectClusterFragment.Callback,
        DiscoveryExampleFragment.Callback,
        ConnectionExampleFragment.Callback,
        ActionSelectorFragment.Callback {

  private static final String TAG = MainActivity.class.getSimpleName();

  private TvCastingApp tvCastingApp;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    Log.i(TAG, "ChipCastingSimplified = " + GlobalCastingConstants.ChipCastingSimplified);
    boolean ret =
        GlobalCastingConstants.ChipCastingSimplified
            ? InitializationExample.initAndStart(this.getApplicationContext()).hasNoError()
            : initJni();
    if (!ret) {
      Log.e(TAG, "Failed to initialize Matter TV casting library");
      return;
    }

    Fragment fragment = null;
    if (GlobalCastingConstants.ChipCastingSimplified) {
      fragment = DiscoveryExampleFragment.newInstance();
    } else {
      fragment = CommissionerDiscoveryFragment.newInstance(tvCastingApp);
    }
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
  public void handleConnectionButtonClicked(CastingPlayer castingPlayer) {
    Log.i(TAG, "MainActivity.handleConnectionButtonClicked() called");
    showFragment(ConnectionExampleFragment.newInstance(castingPlayer));
  }

  @Override
  public void handleCommissioningComplete() {
    showFragment(SelectClusterFragment.newInstance(tvCastingApp));
  }

  @Override
  public void handleConnectionComplete(CastingPlayer castingPlayer) {
    Log.i(TAG, "MainActivity.handleConnectionComplete() called ");
    showFragment(ActionSelectorFragment.newInstance(castingPlayer));
  }

  @Override
  public void handleContentLauncherLaunchURLSelected(CastingPlayer selectedCastingPlayer) {
    showFragment(ContentLauncherLaunchURLExampleFragment.newInstance(selectedCastingPlayer));
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
    showFragment(
        GlobalCastingConstants.ChipCastingSimplified
            ? DiscoveryExampleFragment.newInstance()
            : CommissionerDiscoveryFragment.newInstance(tvCastingApp));
  }

  /**
   * The order is important, must first new TvCastingApp to load dynamic library, then
   * AndroidChipPlatform to prepare platform, then start ChipAppServer, then call init on
   * TvCastingApp
   */
  private boolean initJni() {
    tvCastingApp = TvCastingApp.getInstance();

    tvCastingApp.setDACProvider(new com.chip.casting.util.DACProviderStub());

    AppParameters appParameters = new AppParameters();
    appParameters.setConfigurationManager(
        new PreferencesConfigurationManager(
            this.getApplicationContext(), "chip.platform.ConfigurationManager"));
    byte[] rotatingDeviceIdUniqueId =
        new byte[AppParameters.MIN_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH];
    new Random().nextBytes(rotatingDeviceIdUniqueId);
    appParameters.setRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueId);
    appParameters.setSetupPasscode(GlobalCastingConstants.SetupPasscode);
    appParameters.setDiscriminator(GlobalCastingConstants.Discriminator);
    return tvCastingApp.initApp(this.getApplicationContext(), appParameters);
  }

  private void showFragment(Fragment fragment, boolean showOnBack) {
    Log.d(
        TAG,
        "showFragment() called with " + fragment.getClass().getSimpleName() + " and " + showOnBack);
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
