package com.example.contentapp.receiver;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.matter.tv.app.api.MatterIntentConstants;

public class MatterCommandReceiver extends BroadcastReceiver {
  private static final String TAG = "MatterCommandReceiver";
  private static final int ACCEPT_HEADER = 0;
  private static final int SUPPORTED_STREAMING_PROTOCOLS = 1;

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
        int commandId = intent.getIntExtra(MatterIntentConstants.EXTRA_COMMAND_ID, -1);
        if (commandId != -1) {
          int clusterId = intent.getIntExtra(MatterIntentConstants.EXTRA_CLUSTER_ID, -1);
          byte[] commandPayload =
              intent.getByteArrayExtra(MatterIntentConstants.EXTRA_COMMAND_PAYLOAD);
          Log.d(
              TAG,
              new StringBuilder()
                  .append("Received matter command ")
                  .append(commandId)
                  .append(" on cluster ")
                  .append(clusterId)
                  .append(" with payload : ")
                  .append(new String(commandPayload))
                  .toString());
          String response = "{\"0\":1, \"1\":\"custom response from content app\"}";
          sendResponseViaPendingIntent(context, intent, response);
        } else {
          int attributeId = intent.getIntExtra(MatterIntentConstants.EXTRA_ATTRIBUTE_ID, -1);
          String attributeAction =
              intent.getStringExtra(MatterIntentConstants.EXTRA_ATTRIBUTE_ACTION);
          if (attributeAction.equals(MatterIntentConstants.ATTRIBUTE_ACTION_READ)) {
            String response;
            if (attributeId == ACCEPT_HEADER) {
              response =
                  "{\"0\": [\"video/mp4\", \"application/x-mpegURL\", \"application/dash+xml\"] }";
            } else if (attributeId == SUPPORTED_STREAMING_PROTOCOLS) {
              response = "{\"1\":3}";
            } else {
              response = "";
            }
            sendResponseViaPendingIntent(context, intent, response);
          } else {
            Log.e(
                TAG,
                new StringBuilder()
                    .append("Received unknown Attribute Action: ")
                    .append(intent.getAction())
                    .toString());
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

  private void sendResponseViaPendingIntent(Context context, Intent intent, String response) {
    PendingIntent pendingIntent =
        intent.getParcelableExtra(MatterIntentConstants.EXTRA_DIRECTIVE_RESPONSE_PENDING_INTENT);
    if (pendingIntent != null) {
      final Intent responseIntent =
          new Intent().putExtra(MatterIntentConstants.EXTRA_RESPONSE_PAYLOAD, response.getBytes());
      try {
        pendingIntent.send(context, 0, responseIntent);
      } catch (final PendingIntent.CanceledException ex) {
        Log.e(TAG, "Error sending pending intent to the Matter agent", ex);
      }
    }
  }
}
