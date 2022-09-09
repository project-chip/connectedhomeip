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
import android.content.SharedPreferences;
import android.util.Log;
import androidx.annotation.NonNull;
import com.matter.tv.server.handlers.ContentAppEndpointManagerImpl;
import com.matter.tv.server.model.ContentApp;
import com.matter.tv.server.receivers.ContentAppDiscoveryService;
import com.matter.tv.server.tvapp.AppPlatform;
import java.util.HashMap;
import java.util.Map;

/**
 * This class facilitates the communication with the ContentAppPlatform. It uses the JNI interface
 * provided via the {@link AppPlatform} class to communicate with the linux layer of the
 * platform-app. This class also manages the content app endpoints that get added via the
 * ContentAppPlatform. It manages the persistence of the endpoints that were discovered to be able
 * to reuse endpointIds after restart of the platform-app.
 */
public class AppPlatformService {

  private static final String TAG = "AppPlatformService";
  private static final String MATTER_APPPLATFORM_ENDPOINTS = "matter.appplatform.endpoints";
  private AppPlatform mAppPlatform;
  private BroadcastReceiver mBroadcastReceiver;
  private SharedPreferences discoveredEndpoints;

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
    discoveredEndpoints =
        context.getSharedPreferences(MATTER_APPPLATFORM_ENDPOINTS, Context.MODE_PRIVATE);
    mAppPlatform = new AppPlatform(new ContentAppEndpointManagerImpl(context));
    ContentAppDiscoveryService.getReceiverInstance().registerSelf(context.getApplicationContext());
    Map<String, Integer> previouslyPersistedEndpoints = new HashMap();
    previouslyPersistedEndpoints.putAll((Map<String, Integer>) discoveredEndpoints.getAll());
    for (ContentApp app :
        ContentAppDiscoveryService.getReceiverInstance().getDiscoveredContentApps().values()) {
      addContentApp(app);
      previouslyPersistedEndpoints.remove(app.getAppName());
    }
    SharedPreferences.Editor editor = discoveredEndpoints.edit();
    for (Map.Entry<String, Integer> appEntry : previouslyPersistedEndpoints.entrySet()) {
      editor.remove(appEntry.getKey());
      // TODO : Figure out how to cleanup ACLs
    }
    editor.apply();
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
                removeContentApp(endpointId, packageName);
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
    int retEndpointId = -1;
    int desiredEndpointId = discoveredEndpoints.getInt(app.getAppName(), -1);
    if (desiredEndpointId > 0) {
      retEndpointId =
          mAppPlatform.addContentAppAtEndpoint(
              app.getVendorName(),
              app.getVendorId(),
              app.getAppName(),
              app.getProductId(),
              "1.0",
              desiredEndpointId,
              new ContentAppEndpointManagerImpl(context));
    } else {
      retEndpointId =
          mAppPlatform.addContentApp(
              app.getVendorName(),
              app.getVendorId(),
              app.getAppName(),
              app.getProductId(),
              "1.0",
              new ContentAppEndpointManagerImpl(context));
    }
    if (retEndpointId > 0) {
      app.setEndpointId(retEndpointId);
      discoveredEndpoints.edit().putInt(app.getAppName(), app.getEndpointId()).apply();
    } else {
      Log.e(TAG, "Could not add content app as endpoint. App Name " + app.getAppName());
    }
  }

  public int removeContentApp(int endpointID, String appName) {
    int retEndpointId = mAppPlatform.removeContentApp(endpointID);
    if (endpointID == retEndpointId) {
      discoveredEndpoints.edit().remove(appName).apply();
    }
    return retEndpointId;
  }
}
