package com.example.contentapp.matter;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import com.matter.tv.app.api.IMatterAppAgent;
import com.matter.tv.app.api.MatterIntentConstants;
import com.matter.tv.app.api.SetSupportedClustersRequest;
import com.matter.tv.app.api.SupportedCluster;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;

public class MatterAgentClient {

  private static final String TAG = "MatterAgentClient";
  private static MatterAgentClient instance;
  private IMatterAppAgent service;
  private boolean bound = false;
  private CountDownLatch latch = new CountDownLatch(1);

  // TODO : Introduce dependency injection
  private MatterAgentClient() {};

  public static synchronized MatterAgentClient getInstance(Context context) {
    if (instance == null || (instance.service == null && !instance.bound)) {
      instance = new MatterAgentClient();
      if (!instance.bindService(context)) {
        Log.e(TAG, "Matter agent binding request unsuccessful.");
        instance = null;
      } else {
        Log.d(TAG, "Matter agent binding request successful.");
      }
    }
    return instance;
  }

  public void reportClusters() {
    IMatterAppAgent matterAgent = instance.getMatterAgent();
    if (matterAgent == null) {
      Log.e(TAG, "Matter agent not retrieved.");
      return;
    }
    SetSupportedClustersRequest supportedClustersRequest = new SetSupportedClustersRequest();
    supportedClustersRequest.supportedClusters = new ArrayList<SupportedCluster>();
    SupportedCluster supportedCluster = new SupportedCluster();
    supportedCluster.clusterIdentifier = 1;

    supportedClustersRequest.supportedClusters.add(supportedCluster);
    try {
      boolean success = matterAgent.setSupportedClusters(supportedClustersRequest);
      Log.d(TAG, "Setting supported clusters returned " + (success ? "True" : "False"));
    } catch (RemoteException e) {
      e.printStackTrace();
    }
  }

  private IMatterAppAgent getMatterAgent() {
    try {
      latch.await();
      return service;
    } catch (InterruptedException e) {
      Log.e(TAG, "Interrupted while waiting for service connection.", e);
    }
    return null;
  }

  private synchronized boolean bindService(Context context) {

    ServiceConnection serviceConnection = new MyServiceConnection();
    final Intent intent = new Intent(MatterIntentConstants.ACTION_MATTER_AGENT);
    if (intent.getComponent() == null) {
      final ResolveInfo resolveInfo =
          resolveBindIntent(
              context,
              intent,
              MatterIntentConstants.PERMISSION_MATTER_AGENT_BIND,
              MatterIntentConstants.PERMISSION_MATTER_AGENT);
      if (resolveInfo == null) {
        Log.e(TAG, "No Service available on device to bind for intent " + intent);
        return false;
      }
      final ComponentName component =
          new ComponentName(resolveInfo.serviceInfo.packageName, resolveInfo.serviceInfo.name);
      intent.setComponent(component);
    }

    try {
      Log.e(TAG, "Binding to service");
      bound = context.bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
      return bound;
    } catch (final Throwable e) {
      Log.e(TAG, "Exception binding to service", e);
    }
    return false;
  }

  /**
   * Returns a {@link ResolveInfo} for a service bindable with the provided intent and permission.
   *
   * @param context Android Context.
   * @param bindIntent The Intent used to bind to the Service. Implicit or Explicit.
   * @param bindPermission The permission that the resolved Service must enforce.
   * @param permissionHeldByService A permission that the resolved Service must hold.
   * @return A {@link ResolveInfo} with ServiceInfo for the service, or null.
   */
  private ResolveInfo resolveBindIntent(
      final Context context,
      final Intent bindIntent,
      final String bindPermission,
      final String permissionHeldByService) {
    if (bindPermission == null || permissionHeldByService == null) {
      Log.w(
          TAG,
          "Must specify the permission protecting the service, as well as "
              + "a permission held by the service's package.");
      return null;
    }
    final PackageManager pm = context.getPackageManager();
    if (pm == null) {
      Log.w(TAG, "Package manager is not available.");
      return null;
    }
    // Check for Services able to handle this intent.
    final List<ResolveInfo> infos = pm.queryIntentServices(bindIntent, 0);
    if (infos == null || infos.isEmpty()) {
      return null;
    }

    // For all the services returned, remove those that don't have the specified permissions.
    int size = infos.size();
    for (int i = size - 1; i >= 0; --i) {
      final ResolveInfo resolveInfo = infos.get(i);
      // The service must be protected by the bindPermission
      if (!bindPermission.equals(resolveInfo.serviceInfo.permission)) {
        Log.w(
            TAG,
            String.format(
                "Service (%s) does not enforce the required permission (%s)",
                resolveInfo.serviceInfo.name, bindPermission));
        infos.remove(i);
        continue;
      }
      // And the service's package must hold the permissionHeldByService permission
      final String pkgName = resolveInfo.serviceInfo.packageName;
      final int state = pm.checkPermission(permissionHeldByService, pkgName);
      if (state != PackageManager.PERMISSION_GRANTED) {
        Log.w(
            TAG,
            String.format(
                "Package (%s) does not hold the required permission (%s)",
                pkgName, bindPermission));
        infos.remove(i);
      }
    }
    size = infos.size();

    if (size > 1) {
      // This is suspicious. This means we've got at least 2 services both claiming to handle
      // this intent, and they both have declared this permission. In this case, filter those
      // that aren't on the system image.
      for (int i = size - 1; i >= 0; --i) {
        final ResolveInfo resolveInfo = infos.get(i);
        try {
          final ApplicationInfo appInfo =
              pm.getApplicationInfo(resolveInfo.serviceInfo.packageName, 0);
          if ((appInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0
              && (appInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) == 0) {
            // Not a system app or an updated system app. Remove this sketchy service.
            infos.remove(i);
          }
        } catch (final PackageManager.NameNotFoundException e) {
          infos.remove(i);
        }
      }
    }

    if (infos.size() > 1) {
      Log.w(
          TAG,
          "More than one permission-enforced system"
              + " service can handle intent "
              + bindIntent
              + " and permission "
              + bindPermission);
    }

    return (infos.isEmpty() ? null : infos.get(0));
  }

  private class MyServiceConnection implements ServiceConnection {

    @Override
    public void onServiceConnected(ComponentName name, IBinder binder) {
      Log.d(
          TAG,
          String.format(
              "onServiceConnected for API with intent action %s",
              MatterIntentConstants.ACTION_MATTER_AGENT));
      service = IMatterAppAgent.Stub.asInterface(binder);
      latch.countDown();
    }

    @Override
    public void onServiceDisconnected(ComponentName name) {
      service = null;
    }
  }
}
