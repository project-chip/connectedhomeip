package com.matter.tv.server.service;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;
import com.matter.tv.server.MainActivity;
import com.matter.tv.server.R;

public class MatterServantService extends Service {
  private static final String CHANNEL_ID = "Matter";

  @Override
  public void onCreate() {
    super.onCreate();
    // Start Matter Server
    MatterServant.get().init(this.getApplicationContext());
    MatterServant.get().initCommissioner();

    AppPlatformService.get().init(this.getApplicationContext());
    AppPlatformService.get().addSelfVendorAsAdmin();
  }

  @Nullable
  @Override
  public IBinder onBind(Intent intent) {
    return null;
  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    createNotificationChannel();
    Intent notificationIntent = new Intent(this, MainActivity.class);
    PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
    Notification notification =
        new NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("MatterServant Service")
            .setContentText("MatterServant is running")
            .setSmallIcon(R.mipmap.ic_launcher)
            .setContentIntent(pendingIntent)
            .build();
    startForeground(1, notification);
    return super.onStartCommand(intent, flags, startId);
  }

  private void createNotificationChannel() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      NotificationChannel serviceChannel =
          new NotificationChannel(
              CHANNEL_ID, "Matter Servant", NotificationManager.IMPORTANCE_DEFAULT);
      NotificationManager manager = getSystemService(NotificationManager.class);
      manager.createNotificationChannel(serviceChannel);
    }
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
  }
}
