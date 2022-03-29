package com.matter.tv.server.service;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import androidx.annotation.Nullable;
import com.matter.tv.app.api.MatterIntentConstants;

public class ContentAppAgentService extends Service {
  @Nullable
  @Override
  public IBinder onBind(Intent intent) {
    return null;
  }

  public static void sendCommand(Context context, String packageName) {
    Intent in = new Intent(MatterIntentConstants.ACTION_MATTER_COMMAND);
    Bundle extras = new Bundle();
    extras.putByteArray(MatterIntentConstants.EXTRA_COMMAND_PAYLOAD, "test payload".getBytes());
    in.putExtras(extras);
    in.setPackage(packageName);
    context.sendBroadcast(in);
  }
}
