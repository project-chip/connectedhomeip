package com.matter.tv.server.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import com.matter.tv.app.api.MatterIntentConstants;
import com.matter.tv.app.api.SupportedCluster;
import com.matter.tv.server.model.ContentApp;
import com.matter.tv.server.utils.ResourceUtils;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class ContentAppDiscoveryService extends BroadcastReceiver {
  private static final String TAG = "ContentAppDiscoveryService";
  static final String CLUSTERS_RESOURCE_METADATA_KEY = "com.matter.tv.app.api.clusters";
  static final String MATTER_VENDOR_NAME_METADATA_KEY = "com.matter.tv.app.api.vendor_name";
  static final String MATTER_VENDOR_ID_METADATA_KEY = "com.matter.tv.app.api.vendor_id";
  static final String MATTER_PRODUCT_ID_METADATA_KEY = "com.matter.tv.app.api.product_id";

  private static final String ANDROID_PACKAGE_REMOVED_ACTION =
      "android.intent.action.PACKAGE_REMOVED";
  private static final String ANDROID_PACKAGE_ADDED_ACTION = "android.intent.action.PACKAGE_ADDED";
  private static final String ANDROID_PACKAGE_REPLACED_ACTION =
      "android.intent.action.PACKAGE_REPLACED";
  public static final String DISCOVERY_APPAGENT_ACTION_ADD = "com.matter.tv.server.appagent.add";
  public static final String DISCOVERY_APPAGENT_ACTION_REMOVE =
      "com.matter.tv.server.appagent.remove";
  public static final String DISCOVERY_APPAGENT_EXTRA_PACKAGENAME =
      "com.matter.tv.server.appagent.pkg";
  public static final String DISCOVERY_APPAGENT_EXTRA_ENDPOINTID =
      "com.matter.tv.server.appagent.endpointId";

  private static ResourceUtils resourceUtils = ResourceUtils.getInstance();

  private static final ContentAppDiscoveryService instance = new ContentAppDiscoveryService();

  private ContentAppDiscoveryService() {}

  private volatile boolean registered = false;

  private Map<String, ContentApp> applications = new HashMap<>();

  @Override
  public void onReceive(Context context, Intent intent) {
    final String intentAction = intent.getAction();
    Log.i(TAG, "Some Intent received by the matter server " + intentAction);
    if (intentAction == null || intentAction.isEmpty()) {
      Log.i(TAG, "empty action.");
      return;
    }

    switch (intentAction) {
      case ANDROID_PACKAGE_ADDED_ACTION:
      case ANDROID_PACKAGE_REPLACED_ACTION:
        handlePackageAdded(intent, context);
        break;
      case ANDROID_PACKAGE_REMOVED_ACTION:
        handlePackageRemoved(intent, context);
        break;
      default:
        Log.e(
            TAG,
            new StringBuilder()
                .append("Received unknown Intent: ")
                .append(intent.getAction())
                .toString());
    }
  }

  private void handlePackageAdded(final Intent intent, final Context context) {
    String pkg = intent.getData().getSchemeSpecificPart();
    handlePackageAdded(context, pkg);
  }

  private void handlePackageAdded(Context context, String pkg) {
    PackageManager pm = context.getPackageManager();
    try {
      ApplicationInfo appInfo = pm.getApplicationInfo(pkg, PackageManager.GET_META_DATA);
      if (appInfo.metaData == null) {
        return;
      }
      PackageInfo packageInfo = pm.getPackageInfo(pkg, PackageManager.GET_META_DATA);

      int resId = appInfo.metaData.getInt(CLUSTERS_RESOURCE_METADATA_KEY, 0);
      int vendorId = appInfo.metaData.getInt(MATTER_VENDOR_ID_METADATA_KEY, -1);
      int productId = appInfo.metaData.getInt(MATTER_PRODUCT_ID_METADATA_KEY, -1);
      String vendorName = appInfo.metaData.getString(MATTER_VENDOR_NAME_METADATA_KEY, "");
      String version;
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
        version = String.valueOf(packageInfo.getLongVersionCode());
      } else {
        version = String.valueOf(packageInfo.versionName);
      }

      if (vendorId == -1 || productId == -1) {
        return;
      }

      Set<SupportedCluster> supportedClusters;
      Log.d(TAG, "got static capability for package " + pkg + ", resourceId: " + resId);
      if (resId != 0) {
        Resources res = pm.getResourcesForApplication(appInfo);
        supportedClusters = resourceUtils.getSupportedClusters(res, resId);
      } else {
        supportedClusters = new HashSet<>();
      }

      ContentApp app =
          new ContentApp(pkg, vendorName, vendorId, productId, version, supportedClusters);
      applications.put(pkg, app);

      Intent in = new Intent(DISCOVERY_APPAGENT_ACTION_ADD);
      Bundle extras = new Bundle();
      extras.putString(DISCOVERY_APPAGENT_EXTRA_PACKAGENAME, pkg);
      in.putExtras(extras);
      context.sendBroadcast(in);
    } catch (PackageManager.NameNotFoundException e) {
      Log.e(TAG, "Could not find package " + pkg, e);
    }
  }

  private void handlePackageRemoved(final Intent intent, final Context context) {
    String pkg = intent.getData().getSchemeSpecificPart();
    ContentApp contentApp = applications.get(pkg);
    if (contentApp != null) {
      applications.remove(pkg);
      Intent in = new Intent(DISCOVERY_APPAGENT_ACTION_REMOVE);
      Bundle extras = new Bundle();
      extras.putString(DISCOVERY_APPAGENT_EXTRA_PACKAGENAME, pkg);
      extras.putInt(DISCOVERY_APPAGENT_EXTRA_ENDPOINTID, contentApp.getEndpointId());
      in.putExtras(extras);
      context.sendBroadcast(in);
      Log.i(TAG, "Removing Matter content app " + pkg);
    } else {
      Log.i(TAG, "App not found in set of Matter content apps. Doing nothing for app " + pkg);
    }
  }

  public void registerSelf(Context context) {
    if (registered) {
      Log.i(TAG, "Package update receiver for matter already registered");
      return;
    } else {
      registered = true;
    }
    registerPackageAction(context, ANDROID_PACKAGE_ADDED_ACTION);
    registerPackageAction(context, ANDROID_PACKAGE_REMOVED_ACTION);
    registerPackageAction(context, ANDROID_PACKAGE_REPLACED_ACTION);
    initializeMatterApps(context);
    Log.i(TAG, "Registered the matter package updates receiver");
  }

  private void registerPackageAction(final Context context, final String action) {
    IntentFilter intent = new IntentFilter(action);
    intent.addDataScheme("package");
    context.registerReceiver(this, intent);
  }

  private void initializeMatterApps(Context context) {
    Set<String> matterApps = getMatterApps(context);
    for (String matterApp : matterApps) {
      handlePackageAdded(context, matterApp);
    }
  }

  private Set<String> getMatterApps(Context context) {
    PackageManager pm = context.getPackageManager();
    List<ResolveInfo> receivers =
        pm.queryBroadcastReceivers(
            new Intent(MatterIntentConstants.ACTION_MATTER_COMMAND), PackageManager.MATCH_ALL);

    Set<String> matterApps = new HashSet<>();
    if (receivers.isEmpty()) {
      Log.i(TAG, "No receivers for Matter apps found.");
      return matterApps;
    }

    for (ResolveInfo receiver : receivers) {
      if (receiver != null && receiver.activityInfo != null) {
        Log.i("Activity Info found. Package Name is %s", receiver.activityInfo.packageName);
        matterApps.add(receiver.activityInfo.packageName);
      }
    }
    return matterApps;
  }

  // TODO : Introduce dependency injection
  public static ContentAppDiscoveryService getReceiverInstance() {
    return instance;
  }

  public Map<String, ContentApp> getDiscoveredContentApps() {
    return Collections.unmodifiableMap(applications);
  }

  public ContentApp getDiscoveredContentApp(String appName) {
    return applications.get(appName);
  }
}
