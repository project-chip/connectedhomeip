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
import android.util.Log;
import androidx.annotation.NonNull;
import com.matter.tv.app.api.SupportedCluster;
import com.matter.tv.server.handlers.ContentAppEndpointManagerImpl;
import com.matter.tv.server.model.ContentApp;
import com.matter.tv.server.receivers.ContentAppDiscoveryService;
import com.matter.tv.server.tvapp.AppPlatform;
import com.matter.tv.server.tvapp.ContentAppSupportedCluster;
import com.matter.tv.server.utils.EndpointsDataStore;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * This class facilitates the communication with the ContentAppPlatform. It uses the JNI interface
 * provided via the {@link AppPlatform} class to communicate with the linux layer of the
 * platform-app. This class also manages the content app endpoints that get added via the
 * ContentAppPlatform. It manages the persistence of the endpoints that were discovered to be able
 * to reuse endpointIds after restart of the platform-app.
 */
public class AppPlatformService {

  private static final String TAG = "AppPlatformService";
  private AppPlatform mAppPlatform;
  private BroadcastReceiver mBroadcastReceiver;
  private EndpointsDataStore endpointsDataStore;

  private AppPlatformService() {}

  public void reportAttributeChange(int endpointId, int clusterId, int attributeId) {
    mAppPlatform.reportAttributeChange(endpointId, clusterId, attributeId);
  }

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
    ContentAppDiscoveryService.getReceiverInstance().registerSelf(context.getApplicationContext());
    mAppPlatform = new AppPlatform(new ContentAppEndpointManagerImpl(context));
    endpointsDataStore = EndpointsDataStore.getInstance(context);
    initializeContentAppEndpoints();
    registerContentAppUpdatesReceiver();
  }

  public void addSelfVendorAsAdmin() {
    mAppPlatform.addSelfVendorAsAdmin();
  }

  private void initializeContentAppEndpoints() {

    // Read the metadada of previously discovered endpoints.
    Map<String, ContentApp> previouslyPersistedEndpoints =
        new HashMap((Map<String, ContentApp>) endpointsDataStore.getAllPersistedContentApps());

    // Get the list of currently discovered content apps.
    Map<String, ContentApp> discoveredContentApps =
        new HashMap<>(ContentAppDiscoveryService.getReceiverInstance().getDiscoveredContentApps());

    // Iterate through the previously discovered endpoints
    for (ContentApp persistedContentApp : previouslyPersistedEndpoints.values()) {
      ContentApp discoveredContentApp =
          discoveredContentApps.remove(persistedContentApp.getAppName());
      if (discoveredContentApp != null) {
        // If the content app for the persisted endpoint is present in currently discovered list,
        // register endpoint with updated metadata and update the metadata in the persisted
        // endpoints.
        discoveredContentApp.setEndpointId(persistedContentApp.getEndpointId());
        addContentApp(discoveredContentApp);
      } else {
        // If the content app for the persisted endpoint is not present register the endpoint with
        // previously persisted data.
        addContentApp(persistedContentApp);
      }
    }

    // For newly discovered content apps register new endpoints and persist the metadata for future
    // use
    for (ContentApp discoveredContentApp : discoveredContentApps.values()) {
      addContentApp(discoveredContentApp);
    }
  }

  private void registerContentAppUpdatesReceiver() {
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
              // if this app was already added as endpoint remove and add so that the app metadata
              // stored by the content app platform is also updated
              ContentApp persistedContentApp =
                  endpointsDataStore.getAllPersistedContentApps().get(app.getAppName());
              if (persistedContentApp != null) {
                mAppPlatform.removeContentApp(persistedContentApp.getEndpointId());
                app.setEndpointId(persistedContentApp.getEndpointId());
              }
              addContentApp(app);
              //            } else if
              // (action.equals(ContentAppDiscoveryService.DISCOVERY_APPAGENT_ACTION_REMOVE)) {
              //              int endpointId =
              //                  intent.getIntExtra(
              //
              // ContentAppDiscoveryService.DISCOVERY_APPAGENT_EXTRA_ENDPOINTID, -1);
              //              if (endpointId != -1) {
              //                removeContentApp(endpointId, packageName);
              //              }
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
    int desiredEndpointId = app.getEndpointId();
    if (desiredEndpointId > 0) {
      retEndpointId =
          mAppPlatform.addContentAppAtEndpoint(
              app.getVendorName(),
              app.getVendorId(),
              app.getAppName(),
              app.getProductId(),
              app.getVersion(),
              mapSupportedClusters(app.getSupportedClusters()),
              desiredEndpointId,
              new ContentAppEndpointManagerImpl(context));
    } else {
      retEndpointId =
          mAppPlatform.addContentApp(
              app.getVendorName(),
              app.getVendorId(),
              app.getAppName(),
              app.getProductId(),
              app.getVersion(),
              mapSupportedClusters(app.getSupportedClusters()),
              new ContentAppEndpointManagerImpl(context));
    }
    if (retEndpointId > 0) {
      app.setEndpointId(retEndpointId);
      endpointsDataStore.persistContentAppEndpoint(app);
    } else {
      Log.e(TAG, "Could not add content app as endpoint. App Name " + app.getAppName());
    }
  }

  private Collection<ContentAppSupportedCluster> mapSupportedClusters(
      Collection<SupportedCluster> supportedClusters) {
    return supportedClusters
        .stream()
        .filter(Objects::nonNull)
        .map(AppPlatformService::mapSupportedCluster)
        .collect(Collectors.toList());
  }

  private static ContentAppSupportedCluster mapSupportedCluster(SupportedCluster cluster) {
    return new ContentAppSupportedCluster(
        cluster.clusterIdentifier,
        cluster.features,
        cluster.optionalCommandIdentifiers,
        cluster.optionalAttributesIdentifiers);
  }
}
