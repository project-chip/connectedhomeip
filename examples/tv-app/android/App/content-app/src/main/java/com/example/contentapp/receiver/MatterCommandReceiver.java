package com.example.contentapp.receiver;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.example.contentapp.AttributeHolder;
import com.example.contentapp.CommandResponseHolder;
import com.example.contentapp.MainActivity;
import com.example.contentapp.matter.MatterAgentClient;
import com.matter.tv.app.api.MatterIntentConstants;

public class MatterCommandReceiver extends BroadcastReceiver {
  private static final String TAG = "MatterCommandReceiver";

  @Override
  public void onReceive(Context context, Intent intent) {

    MatterAgentClient.initialize(context.getApplicationContext());

    final String intentAction = intent.getAction();
    Log.i(TAG, "Some Intent received from the matter server " + intentAction);
    if (intentAction == null || intentAction.isEmpty()) {
      Log.i(TAG, "empty action.");
      return;
    }

    switch (intentAction) {
      case MatterIntentConstants.ACTION_MATTER_COMMAND:
        long commandId = intent.getLongExtra(MatterIntentConstants.EXTRA_COMMAND_ID, -1);
        long clusterId = intent.getLongExtra(MatterIntentConstants.EXTRA_CLUSTER_ID, -1);
        if (commandId != -1) {
          byte[] commandPayload =
              intent.getByteArrayExtra(MatterIntentConstants.EXTRA_COMMAND_PAYLOAD);
          String command = new String(commandPayload);
          String message =
              new StringBuilder()
                  .append("Received matter command ")
                  .append(commandId)
                  .append(" on cluster ")
                  .append(clusterId)
                  .append(" with payload : ")
                  .append(command)
                  .toString();
          Log.d(TAG, message);
          String response =
              CommandResponseHolder.getInstance().getCommandResponse(clusterId, commandId);

          Intent in = new Intent(context, MainActivity.class);
          in.putExtra(MatterIntentConstants.EXTRA_COMMAND_PAYLOAD, command);
          in.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
          context.startActivity(in);

          Log.d(TAG, "Started activity. Now sending response");

          sendResponseViaPendingIntent(context, intent, response);
        } else {
          long attributeId = intent.getLongExtra(MatterIntentConstants.EXTRA_ATTRIBUTE_ID, -1);
          String attributeAction =
              intent.getStringExtra(MatterIntentConstants.EXTRA_ATTRIBUTE_ACTION);
          if (attributeAction.equals(MatterIntentConstants.ATTRIBUTE_ACTION_READ)) {
            String response =
                "{\""
                    + attributeId
                    + "\":"
                    + AttributeHolder.getInstance().getAttributeValue(clusterId, attributeId)
                    + "}";
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
