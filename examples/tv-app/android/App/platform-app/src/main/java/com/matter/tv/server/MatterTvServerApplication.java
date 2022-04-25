package com.matter.tv.server;

import android.app.Application;
import android.content.Intent;
import android.os.Build;
import com.matter.tv.server.service.MatterServantService;

public class MatterTvServerApplication extends Application {
  @Override
  public void onCreate() {
    super.onCreate();
    startMatterServantService();
  }

  private void startMatterServantService() {
    Intent intent = new Intent(this, MatterServantService.class);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      getApplicationContext().startForegroundService(intent);
    } else {
      startService(intent);
    }
  }
}
