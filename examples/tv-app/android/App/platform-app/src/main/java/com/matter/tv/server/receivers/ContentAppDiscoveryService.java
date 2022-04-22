package com.matter.tv.server.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.Log;
import com.matter.tv.app.api.MatterIntentConstants;
import com.matter.tv.server.utils.ResourceUtils;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class ContentAppDiscoveryService extends BroadcastReceiver {
  private static final String TAG = "MatterContentAppDiscoveryService";
  static final String CLUSTERS_RESOURCE_METADATA_KEY = "com.matter.app_agent_api.clusters";
  private static final String ANDROID_PACKAGE_REMOVED_ACTION =
      "android.intent.action.PACKAGE_REMOVED";
  private static final String ANDROID_PACKAGE_ADDED_ACTION = "android.intent.action.PACKAGE_ADDED";

  private static ResourceUtils resourceUtils = ResourceUtils.getInstance();

  private static final ContentAppDiscoveryService instance = new ContentAppDiscoveryService();

  public ContentAppDiscoveryService() {}

  private volatile boolean registered = false;

  @Override
  public void onReceive(Context context, Intent intent) {
    final String intentAction = intent.getAction();
    Log.i(TAG, "Some Intent received by the matter server " + intentAction);
    if (intentAction == null || intentAction.isEmpty()) {
      Log.i(TAG, "empty action.");
      return;
    }

    switch (intentAction) {
      case Intent.ACTION_BOOT_COMPLETED:
        //                discoveryAgent.init();
        break;
      case ANDROID_PACKAGE_ADDED_ACTION:
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
    Log.i(TAG, pkg + " Added. MATTERSERVER");

    handlePackageAdded(context, pkg);
  }

  private void handlePackageAdded(Context context, String pkg) {
    PackageManager pm = context.getPackageManager();
    try {
      ApplicationInfo appInfo = pm.getApplicationInfo(pkg, PackageManager.GET_META_DATA);
      if (appInfo.metaData == null) {
        Log.i(TAG, pkg + " has no metadata.");
        return;
      }

      int resId = appInfo.metaData.getInt(CLUSTERS_RESOURCE_METADATA_KEY, 0);
      Log.d(TAG, "got static capability for package " + pkg + ", resourceId: " + resId);
      if (resId != 0) {
        Resources res = pm.getResourcesForApplication(appInfo);
        String rawJson = resourceUtils.getRawTextResource(res, resId);
        Log.d(TAG, "Got capabilities resource:\n" + rawJson);

        Intent in = new Intent("com.matter.tv.server.appagent.add");
        Bundle extras = new Bundle();
        extras.putString("com.matter.tv.server.appagent.add.pkg", pkg);
        extras.putString("com.matter.tv.server.appagent.add.clusters", rawJson);
        in.putExtras(extras);
        context.sendBroadcast(in);
      }
    } catch (PackageManager.NameNotFoundException e) {
      Log.e(TAG, "Could not find package " + pkg, e);
    }
  }

  private void handlePackageRemoved(final Intent intent, final Context context) {
    String pkg = intent.getData().getSchemeSpecificPart();
    Log.i(TAG, pkg + " Removed. MATTERSERVER");
    Intent in = new Intent("com.matter.tv.server.appagent.remove");
    Bundle extras = new Bundle();
    extras.putString("com.matter.tv.server.appagent.add.pkg", pkg);
    in.putExtras(extras);
    context.sendBroadcast(in);
  }

  public void registerSelf(Context context) {
    Log.i(TAG, "Starting the registration of the matter package update receiver");
    if (registered) {
      Log.i(TAG, "Package update receiver for matter already registered");
      return;
    } else {
      registered = true;
    }

    Log.i(TAG, "Trying to register the matter package update receiver");
    IntentFilter pckAdded = new IntentFilter(ANDROID_PACKAGE_ADDED_ACTION);
    pckAdded.hasDataScheme("package");
    context.registerReceiver(this, pckAdded);
    IntentFilter pckRemoved = new IntentFilter(ANDROID_PACKAGE_REMOVED_ACTION);
    pckRemoved.hasDataScheme("package");
    context.registerReceiver(this, pckRemoved);
    Log.i(TAG, "Registered the matter package update receiver");
  }

  public void initializeMatterApps(Context context) {
    Set<String> matterApps = getMatterApps(context);
    for (String matterApp : matterApps) {
      handlePackageAdded(context, matterApp);
    }
  }

  private Set<String> getMatterApps(Context context) {
    PackageManager pm = context.getPackageManager();
    List<ResolveInfo> receivers =
        pm.queryBroadcastReceivers(new Intent(MatterIntentConstants.ACTION_MATTER_COMMAND), 0);

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

  public static ContentAppDiscoveryService getRecieverInstance() {
    return instance;
  }
}
