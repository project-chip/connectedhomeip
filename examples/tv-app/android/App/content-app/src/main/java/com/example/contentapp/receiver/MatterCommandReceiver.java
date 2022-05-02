package com.example.contentapp.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.matter.tv.app.api.MatterIntentConstants;

public class MatterCommandReceiver extends BroadcastReceiver {
  private static final String TAG = "MatterCommandReceiver";

  @Override
  public void onReceive(Context context, Intent intent) {
    final String intentAction = intent.getAction();
    Log.i(TAG, "Some Intent received from the matter server " + intentAction);
    if (intentAction == null || intentAction.isEmpty()) {
      Log.i(TAG, "empty action.");
      return;
    }

    switch (intentAction) {
      case MatterIntentConstants.ACTION_MATTER_COMMAND:
        byte[] commandPayload =
            intent.getByteArrayExtra(MatterIntentConstants.EXTRA_COMMAND_PAYLOAD);
        Log.e(
            TAG,
            new StringBuilder()
                .append("Received matter command: ")
                .append(intent.getAction())
                .toString());
      default:
        Log.e(
            TAG,
            new StringBuilder()
                .append("Received unknown Intent: ")
                .append(intent.getAction())
                .toString());
    }
  }
}
