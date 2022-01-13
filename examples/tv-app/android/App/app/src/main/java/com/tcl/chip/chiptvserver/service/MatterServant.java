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
import com.tcl.chip.tvapp.ChannelManagerStub;
import com.tcl.chip.tvapp.Clusters;
import com.tcl.chip.tvapp.ContentLaunchManagerStub;
import com.tcl.chip.tvapp.KeypadInputManagerStub;
import com.tcl.chip.tvapp.LowPowerManagerStub;
import com.tcl.chip.tvapp.MediaInputManagerStub;
import com.tcl.chip.tvapp.MediaPlaybackManagerStub;
import com.tcl.chip.tvapp.TvApp;
import com.tcl.chip.tvapp.WakeOnLanManagerStub;

public class MatterServant {

  private ChipAppServer chipAppServer;

  private MatterServant() {}

  private static class SingletonHolder {
    static MatterServant instance = new MatterServant();
  }

  public static MatterServant get() {
    return SingletonHolder.instance;
  }

  public void init(@NonNull Context context) {
    TvApp tvApp =
        new TvApp(
            (app, clusterId, endpoint) -> {
              switch (clusterId) {
                case Clusters.ClusterId_KeypadInput:
                  app.setKeypadInputManager(endpoint, new KeypadInputManagerStub(endpoint));
                  break;
                case Clusters.ClusterId_WakeOnLan:
                  app.setWakeOnLanManager(endpoint, new WakeOnLanManagerStub(endpoint));
                  break;
                case Clusters.ClusterId_MediaInput:
                  app.setMediaInputManager(endpoint, new MediaInputManagerStub(endpoint));
                  break;
                case Clusters.ClusterId_ContentLauncher:
                  app.setContentLaunchManager(endpoint, new ContentLaunchManagerStub(endpoint));
                  break;
                case Clusters.ClusterId_LowPower:
                  app.setLowPowerManager(endpoint, new LowPowerManagerStub(endpoint));
                  break;
                case Clusters.ClusterId_MediaPlayback:
                  app.setMediaPlaybackManager(endpoint, new MediaPlaybackManagerStub(endpoint));
                  break;
                case Clusters.ClusterId_Channel:
                  app.setChannelManager(endpoint, new ChannelManagerStub(endpoint));
                  break;
              }
            });

    Context applicationContext = context.getApplicationContext();
    AndroidChipPlatform chipPlatform =
        new AndroidChipPlatform(
            new AndroidBleManager(),
            new PreferencesKeyValueStoreManager(applicationContext),
            new PreferencesConfigurationManager(applicationContext),
            new NsdManagerServiceResolver(applicationContext),
            new ChipMdnsCallbackImpl());

    chipAppServer = new ChipAppServer();
    chipAppServer.startApp();
  }

  public void restart() {
    chipAppServer.stopApp();
    chipAppServer.startApp();
  }
}
