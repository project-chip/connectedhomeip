package com.tcl.chip.chiptvserver;

import android.app.Application;
import android.content.Intent;
import android.os.Build;
import com.tcl.chip.chiptvserver.service.MatterServantService;

public class ChipTvServerApplication extends Application {
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
