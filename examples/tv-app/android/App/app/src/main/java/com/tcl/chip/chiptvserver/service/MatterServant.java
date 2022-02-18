/*
 *   Copyright (c) 2021-2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.tcl.chip.chiptvserver.service;

import android.content.Context;
import androidx.annotation.NonNull;
import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.DiagnosticDataProviderImpl;
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
            new ChipMdnsCallbackImpl(),
            new DiagnosticDataProviderImpl(applicationContext));

    chipAppServer = new ChipAppServer();
    chipAppServer.startApp();
  }

  public void restart() {
    chipAppServer.stopApp();
    chipAppServer.startApp();
  }
}
