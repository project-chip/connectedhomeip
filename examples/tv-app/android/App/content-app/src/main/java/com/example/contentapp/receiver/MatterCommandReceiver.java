package com.example.contentapp.receiver;

import android.app.PendingIntent;
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
        Log.d(
            TAG,
            new StringBuilder()
                .append("Received matter command: ")
                .append(intent.getAction())
                .append(". Payload : ")
                .append(new String(commandPayload))
                .toString());

        PendingIntent pendingIntent =
            intent.getParcelableExtra(
                MatterIntentConstants.EXTRA_DIRECTIVE_RESPONSE_PENDING_INTENT);
        if (pendingIntent != null) {
          final Intent responseIntent =
              new Intent()
                  .putExtra(MatterIntentConstants.EXTRA_RESPONSE_PAYLOAD, "Success".getBytes());
          try {
            pendingIntent.send(context, 0, responseIntent);
          } catch (final PendingIntent.CanceledException ex) {
            Log.e(TAG, "Error sending pending intent to the Matter agent", ex);
          }
        }
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
}
