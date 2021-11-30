package com.tcl.chip.chiptvserver.service;

import android.content.Context;
import androidx.annotation.NonNull;
import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesConfigurationManager;
import chip.platform.PreferencesKeyValueStoreManager;
import com.tcl.chip.tvapp.ContentLaunchManagerStub;
import com.tcl.chip.tvapp.KeypadInputManagerStub;
import com.tcl.chip.tvapp.LowPowerManagerStub;
import com.tcl.chip.tvapp.MediaInputManagerStub;
import com.tcl.chip.tvapp.MediaPlaybackManagerStub;
import com.tcl.chip.tvapp.TvApp;
import com.tcl.chip.tvapp.TvChannelManagerStub;
import com.tcl.chip.tvapp.WakeOnLanManagerStub;

public class MatterServant {

  private MatterServant() {}

  private static class SingletonHolder {
    static MatterServant instance = new MatterServant();
  }

  public static MatterServant get() {
    return SingletonHolder.instance;
  }

  public void init(@NonNull Context context) {
    TvApp tvApp = new TvApp();
    tvApp.setKeypadInputManager(new KeypadInputManagerStub());
    tvApp.setWakeOnLanManager(new WakeOnLanManagerStub());
    tvApp.setMediaInputManager(new MediaInputManagerStub());
    tvApp.setContentLaunchManager(new ContentLaunchManagerStub());
    tvApp.setLowPowerManager(new LowPowerManagerStub());
    tvApp.setMediaPlaybackManager(new MediaPlaybackManagerStub());
    tvApp.setTvChannelManager(new TvChannelManagerStub());

    Context applicationContext = context.getApplicationContext();
    AndroidChipPlatform chipPlatform =
        new AndroidChipPlatform(
            new AndroidBleManager(),
            new PreferencesKeyValueStoreManager(applicationContext),
            new PreferencesConfigurationManager(applicationContext),
            new NsdManagerServiceResolver(applicationContext),
            new ChipMdnsCallbackImpl());

    ChipAppServer chipAppServer = new ChipAppServer();
    chipAppServer.startApp();
  }
}
