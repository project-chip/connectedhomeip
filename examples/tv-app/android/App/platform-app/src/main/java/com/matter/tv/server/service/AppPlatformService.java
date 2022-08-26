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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import androidx.annotation.NonNull;
import com.matter.tv.server.MatterCommissioningPrompter;
import com.matter.tv.server.handlers.ContentAppEndpointManagerImpl;
import com.matter.tv.server.model.ContentApp;
import com.matter.tv.server.receivers.ContentAppDiscoveryService;
import com.matter.tv.server.tvapp.AppPlatform;

public class AppPlatformService {

  private AppPlatform mAppPlatform;
  private BroadcastReceiver mBroadcastReceiver;

  private AppPlatformService() {}

  private static class SingletonHolder {
    static AppPlatformService instance = new AppPlatformService();
  }

  public static AppPlatformService get() {
    return SingletonHolder.instance;
  }

  private Context context;
  private Activity activity;

  public void init(@NonNull Context context) {
    this.context = context;
    mAppPlatform =
        new AppPlatform(
            new MatterCommissioningPrompter(activity), new ContentAppEndpointManagerImpl(context));
    ContentAppDiscoveryService.getReceiverInstance().registerSelf(context.getApplicationContext());
    for (ContentApp app :
        ContentAppDiscoveryService.getReceiverInstance().getDiscoveredContentApps().values()) {
      addContentApp(app);
    }
    registerReceiver();
  }

  private void registerReceiver() {
    mBroadcastReceiver =
        new BroadcastReceiver() {
          @Override
          public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            String packageName =
                intent.getStringExtra(
                    ContentAppDiscoveryService.DISCOVERY_APPAGENT_EXTRA_PACKAGENAME);
            if (action.equals(ContentAppDiscoveryService.DISCOVERY_APPAGENT_ACTION_ADD)) {
              ContentApp app =
                  ContentAppDiscoveryService.getReceiverInstance()
                      .getDiscoveredContentApps()
                      .get(packageName);
              addContentApp(app);
            } else if (action.equals(ContentAppDiscoveryService.DISCOVERY_APPAGENT_ACTION_REMOVE)) {
              int endpointId =
                  intent.getIntExtra(
                      ContentAppDiscoveryService.DISCOVERY_APPAGENT_EXTRA_ENDPOINTID, -1);
              if (endpointId != -1) {
                removeContentApp(endpointId);
              }
            }
          }
        };
    context.registerReceiver(
        mBroadcastReceiver,
        new IntentFilter(ContentAppDiscoveryService.DISCOVERY_APPAGENT_ACTION_ADD));
    context.registerReceiver(
        mBroadcastReceiver,
        new IntentFilter(ContentAppDiscoveryService.DISCOVERY_APPAGENT_ACTION_REMOVE));
  }

  public void setActivity(Activity activity) {
    this.activity = activity;
  }

  public void addContentApp(ContentApp app) {
    app.setEndpointId(
        mAppPlatform.addContentApp(
            app.getVendorName(),
            app.getVendorId(),
            app.getAppName(),
            app.getProductId(),
            "1.0",
            new ContentAppEndpointManagerImpl(context)));
  }

  public int removeContentApp(int endpointID) {
    return mAppPlatform.removeContentApp(endpointID);
  }
}
