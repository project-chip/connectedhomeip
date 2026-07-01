package com.google.matter.alldevices;

import android.content.Context;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.DiagnosticDataProviderImpl;
import chip.platform.NsdManagerServiceBrowser;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesConfigurationManager;
import chip.platform.PreferencesKeyValueStoreManager;

public class App {
  private static final App INSTANCE = new App();
  private static AndroidChipPlatform platform = null;

  public static App getInstance() {
    return INSTANCE;
  }

  private App() {}

  public synchronized void initializePlatform(Context context, int discriminator) {
    if (platform == null) {
      platform =
          new AndroidChipPlatform(
              null,
              null,
              new PreferencesKeyValueStoreManager(context),
              new PreferencesConfigurationManager(context),
              new NsdManagerServiceResolver(context),
              new NsdManagerServiceBrowser(context),
              new ChipMdnsCallbackImpl(),
              new DiagnosticDataProviderImpl(context));
    }
    platform.updateCommissionableDataProviderData(null, null, 0, 20202021L, discriminator);
  }

  public native String[] getSupportedDeviceTypes();

  public native boolean startApp(String configurationJson);

  public native String[] getOnboardingCodes(int discriminator);

  public native boolean stopApp();

  public native void resetApp();

  static {
    System.loadLibrary("AllDevicesApp");
  }
}
