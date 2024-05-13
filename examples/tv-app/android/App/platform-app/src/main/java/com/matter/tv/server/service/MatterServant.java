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
package com.matter.tv.server.service;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import androidx.annotation.NonNull;
import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.DiagnosticDataProviderImpl;
import chip.platform.NsdManagerServiceBrowser;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesConfigurationManager;
import chip.platform.PreferencesKeyValueStoreManager;
import com.matter.tv.server.MatterCommissioningPrompter;
import com.matter.tv.server.tvapp.ChannelManagerStub;
import com.matter.tv.server.tvapp.Clusters;
import com.matter.tv.server.tvapp.ContentLaunchManagerStub;
import com.matter.tv.server.tvapp.DACProviderStub;
import com.matter.tv.server.tvapp.DeviceEventProvider;
import com.matter.tv.server.tvapp.KeypadInputManagerStub;
import com.matter.tv.server.tvapp.LevelManagerStub;
import com.matter.tv.server.tvapp.LowPowerManagerStub;
import com.matter.tv.server.tvapp.MediaInputManagerStub;
import com.matter.tv.server.tvapp.MediaPlaybackManagerStub;
import com.matter.tv.server.tvapp.MessagesManagerStub;
import com.matter.tv.server.tvapp.OnOffManagerStub;
import com.matter.tv.server.tvapp.TvApp;
import com.matter.tv.server.tvapp.WakeOnLanManagerStub;

public class MatterServant {

  public int testSetupPasscode = 20202021;
  public int testDiscriminator = 0xF00;

  private ChipAppServer chipAppServer;
  private TvApp mTvApp;
  private boolean mIsOn = true;
  private int mOnOffEndpoint;
  private int mLevelEndpoint;

  private MatterServant() {}

  private static class SingletonHolder {
    static MatterServant instance = new MatterServant();
  }

  public static MatterServant get() {
    return SingletonHolder.instance;
  }

  private Context context;
  private Activity activity;

  public void init(@NonNull Context context) {

    this.context = context;

    // The order is important, must
    // first new TvApp to load dynamic library
    // then chipPlatform to prepare platform
    // then TvApp.preServerInit to initialize any server configuration
    // then start ChipAppServer
    // then TvApp.postServerInit to init app platform
    mTvApp =
        new TvApp(
            (app, clusterId, endpoint) -> {
              if (clusterId == Clusters.ClusterId_KeypadInput) {
                app.setKeypadInputManager(endpoint, new KeypadInputManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_WakeOnLan) {
                app.setWakeOnLanManager(endpoint, new WakeOnLanManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_MediaInput) {
                app.setMediaInputManager(endpoint, new MediaInputManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_ContentLauncher) {
                app.setContentLaunchManager(endpoint, new ContentLaunchManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_LowPower) {
                app.setLowPowerManager(endpoint, new LowPowerManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_MediaPlayback) {
                app.setMediaPlaybackManager(endpoint, new MediaPlaybackManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_Channel) {
                app.setChannelManager(endpoint, new ChannelManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_Messaging) {
                app.setMessagesManager(endpoint, new MessagesManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_OnOff) {
                mOnOffEndpoint = endpoint;
                app.setOnOffManager(endpoint, new OnOffManagerStub(endpoint));
              } else if (clusterId == Clusters.ClusterId_LevelControl) {
                mLevelEndpoint = endpoint;
                app.setLevelManager(endpoint, new LevelManagerStub(endpoint));
              }
            });
    mTvApp.setDACProvider(new DACProviderStub());

    mTvApp.setChipDeviceEventProvider(
        new DeviceEventProvider() {
          @Override
          public void onCommissioningComplete() {
            Log.d("lz", "onCommissioningComplete: ");
          }
        });
    Context applicationContext = context.getApplicationContext();
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
        null, null, 0, testSetupPasscode, testDiscriminator);

    mTvApp.preServerInit();

    chipAppServer = new ChipAppServer();
    chipAppServer.startApp();
  }

  public void initCommissioner() {
    mTvApp.initializeCommissioner(new MatterCommissioningPrompter(context));
  }

  public void restart() {
    chipAppServer.stopApp();
    chipAppServer.startApp();
  }

  public void toggleOnOff() {
    mTvApp.setOnOff(mOnOffEndpoint, mIsOn);
    mIsOn = !mIsOn;
  }

  public void setActivity(Activity activity) {
    this.activity = activity;
  }

  public Activity getActivity() {
    return activity;
  }

  public void sendCustomCommand(String customCommand) {
    Log.i(MatterServant.class.getName(), customCommand);
    // TODO: insert logic ot send custom command here
  }

  public void updateLevel(int value) {
    mTvApp.setCurrentLevel(mLevelEndpoint, value);
  }
}
