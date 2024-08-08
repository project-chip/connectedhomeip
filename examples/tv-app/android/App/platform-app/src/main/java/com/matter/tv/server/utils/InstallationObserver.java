package com.matter.tv.server.utils;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageInstaller;
import android.content.pm.PackageManager;
import android.util.Log;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

public class InstallationObserver {
  public enum InstallStatus {
    IN_PROGRESS,
    SUCCEEDED,
    FAILED
  }

  public static class InstallState {
    private final String appPackageName;
    private final InstallStatus status;

    public InstallState(String appPackageName, InstallStatus status) {
      this.appPackageName = appPackageName;
      this.status = status;
    }

    public String getAppPackageName() {
      return appPackageName;
    }

    public InstallStatus getStatus() {
      return status;
    }

    public String getPackageShortName() {
      return appPackageName.substring(appPackageName.lastIndexOf(".") + 1);
    }

    public String getPackageTitle() {
      String shortName = getPackageShortName();
      return shortName.substring(0, 1).toUpperCase(Locale.getDefault()) + shortName.substring(1);
    }
  }

  private static InstallState stateFromId(PackageInstaller installer, int sessionId) {
    PackageInstaller.SessionInfo info = installer.getSessionInfo(sessionId);
    if (info == null || info.getAppPackageName() == null) {
      return null;
    }
    return new InstallState(info.getAppPackageName(), InstallStatus.IN_PROGRESS);
  }

  public static Set<String> getInstalledPackages(PackageManager packageManager) {
    List<PackageInfo> packageInfoList = packageManager.getInstalledPackages(0);
    Set<String> setOfInstalledApps = new HashSet<>();
    for (PackageInfo info : packageInfoList) {
      setOfInstalledApps.add(info.packageName);
    }
    return setOfInstalledApps;
  }

  public static LiveData<InstallState> installationStates(Context context) {
    MutableLiveData<InstallState> liveData = new MutableLiveData<>();
    PackageInstaller packageInstaller = context.getPackageManager().getPackageInstaller();
    Map<Integer, InstallState> inProgressSessions = new HashMap<>();

    PackageInstaller.SessionCallback callback =
        new PackageInstaller.SessionCallback() {
          @Override
          public void onCreated(int sessionId) {
            Log.d(TAG, "onCreated ");
            InstallState state = stateFromId(packageInstaller, sessionId);
            if (state != null) {
              inProgressSessions.put(sessionId, state);
              liveData.postValue(state);
            }
          }

          @Override
          public void onBadgingChanged(int sessionId) {}

          @Override
          public void onActiveChanged(int sessionId, boolean active) {
            Log.d(TAG, "onActiveChanged");
            InstallState state = stateFromId(packageInstaller, sessionId);
            if (state != null) {
              inProgressSessions.put(sessionId, state);
              liveData.postValue(state);
            }
          }

          @Override
          public void onProgressChanged(int sessionId, float progress) {
            Log.d(TAG, "onProgressChanged:" + progress);
            InstallState state = stateFromId(packageInstaller, sessionId);
            if (state != null) {
              inProgressSessions.put(sessionId, state);
              liveData.postValue(state);
            }
          }

          @Override
          public void onFinished(int sessionId, boolean success) {
            Log.d(TAG, "onFinished " + sessionId + " " + success);
            InstallState current = inProgressSessions.get(sessionId);
            if (current != null) {
              InstallState newState =
                  new InstallState(
                      current.getAppPackageName(),
                      success ? InstallStatus.SUCCEEDED : InstallStatus.FAILED);
              liveData.postValue(newState);
              inProgressSessions.remove(sessionId);
            }
          }
        };

    packageInstaller.registerSessionCallback(callback);

    for (PackageInstaller.SessionInfo info : packageInstaller.getMySessions()) {
      if (info.isActive() && info.getAppPackageName() != null) {
        InstallState state = new InstallState(info.getAppPackageName(), InstallStatus.IN_PROGRESS);
        inProgressSessions.put(info.getSessionId(), state);
        liveData.postValue(state);
      }
    }

    return liveData;
  }

  private static final String TAG = "InstallationObserver";
}
