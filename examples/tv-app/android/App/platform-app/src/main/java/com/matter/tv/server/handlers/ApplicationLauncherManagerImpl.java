package com.matter.tv.server.handlers;

import android.content.Context;
import android.content.pm.PackageManager;
import android.util.Log;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.Observer;
import com.matter.tv.server.tvapp.Application;
import com.matter.tv.server.tvapp.ApplicationLauncherManager;
import com.matter.tv.server.tvapp.LauncherResponse;
import com.matter.tv.server.utils.EndpointsDataStore;
import com.matter.tv.server.utils.InstallationObserver;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class ApplicationLauncherManagerImpl implements ApplicationLauncherManager {

  private static final String TAG = "ApplicationLauncherService";

  private volatile boolean registered = false;
  private PackageManager packageManager;
  private EndpointsDataStore endpointsDataStore;

  /** Hash Map of packageName & Install Status */
  private Map<String, InstallationObserver.InstallStatus> lastReceivedInstallationStatus =
      new HashMap<>();

  private LiveData<InstallationObserver.InstallState> installStateLiveData;

  public ApplicationLauncherManagerImpl(Context context) {
    packageManager = context.getPackageManager();
    endpointsDataStore = EndpointsDataStore.getInstance(context);
    registerSelf(context);
  }

  private final Observer<InstallationObserver.InstallState> installStateObserver =
      state -> {
        lastReceivedInstallationStatus.put(state.getAppPackageName(), state.getStatus());
        switch (state.getStatus()) {
          case IN_PROGRESS:
            // Installation is in progress
            Log.d(TAG, "Installation of " + state.getAppPackageName() + " in progress");
            break;
          case SUCCEEDED:
            // Installation succeeded
            Log.d(TAG, "Installation of " + state.getAppPackageName() + " succeeded");
            break;
          case FAILED:
            // Installation failed
            Log.d(TAG, "Installation of " + state.getAppPackageName() + " failed");
            break;
        }
      };

  private void stopObservingInstallations() {
    if (installStateLiveData != null) {
      Log.d("InstallationObserver", "Stopped Observing");
      installStateLiveData.removeObserver(installStateObserver);
    }
  }

  public void unregister() {
    stopObservingInstallations();
  }

  private void registerSelf(Context context) {
    if (registered) {
      Log.i(TAG, "Package update receiver for matter already registered");
      return;
    } else {
      registered = true;
    }
    Log.i(TAG, "Registered the matter package updates receiver");

    installStateLiveData = InstallationObserver.installationStates(context);
    installStateLiveData.observeForever(installStateObserver);
    Log.d(TAG, "Started Observing package installations");
  }

  @Override
  public int[] getCatalogList() {
    Log.i(TAG, "Get Catalog List");
    return new int[] {123, 456, 89010};
  }

  @Override
  public LauncherResponse launchApp(Application app, String data) {
    Log.i(
        TAG,
        "Launch app id:" + app.applicationId + " cid:" + app.catalogVendorId + " data:" + data);

    int status = 0;
    String responseData = "";

    // Installed Apps that have declared CSA product id & vendor id in their manifes
    boolean matterEnabledAppdIsInstalled =
        endpointsDataStore.getAllPersistedContentApps().containsKey(app.applicationId);
    // Installed App
    boolean appIsInstalled =
        InstallationObserver.getInstalledPackages(packageManager).contains(app.applicationId);
    boolean isAppInstalling =
        Objects.equals(
            lastReceivedInstallationStatus.get(app.applicationId),
            InstallationObserver.InstallStatus.IN_PROGRESS);
    boolean appInstallFailed =
        Objects.equals(
            lastReceivedInstallationStatus.get(app.applicationId),
            InstallationObserver.InstallStatus.FAILED);

    // This use-case can happen if app is installed
    // but it does not support Matter
    if (!matterEnabledAppdIsInstalled && appIsInstalled) {
      Log.i(
          TAG,
          "Matter enabled app is not installed, but app is installed. Launching app's install page");
      status = LauncherResponse.STATUS_PENDING_USER_APPROVAL;
      responseData = "App is installed, try updating";

      //
      // Add code to launch App Install Page
      //

    } else if (!matterEnabledAppdIsInstalled && !appIsInstalled) {
      Log.i(
          TAG,
          "Matter enabled app is not installed and app is not installed. Launching app's install page");
      if (isAppInstalling) {
        Log.i(TAG, "App is installing");
        status = LauncherResponse.STATUS_INSTALLING;
      } else {
        status = LauncherResponse.STATUS_PENDING_USER_APPROVAL;
        if (appInstallFailed) {
          responseData = "App install failed. Try again";
        }
      }

      //
      // Add code to launch App Install Page
      //

    } else if (matterEnabledAppdIsInstalled && appIsInstalled) {
      Log.i(TAG, "Launching the app");
      status = LauncherResponse.STATUS_SUCCESS;

      //
      // Add code to launch an app
      //
    }

    return new LauncherResponse(status, responseData);
  }

  @Override
  public LauncherResponse stopApp(Application app) {
    Log.i(TAG, "Stop app id:" + app.applicationId + " cid:" + app.catalogVendorId);
    return new LauncherResponse(LauncherResponse.STATUS_SUCCESS, "");
  }

  @Override
  public LauncherResponse hideApp(Application app) {
    Log.i(TAG, "Hide app id:" + app.applicationId + " cid:" + app.catalogVendorId);
    return new LauncherResponse(LauncherResponse.STATUS_SUCCESS, "");
  }
}
