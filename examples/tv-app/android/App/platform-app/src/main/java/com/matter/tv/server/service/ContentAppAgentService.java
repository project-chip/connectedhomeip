package com.matter.tv.server.service;

import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import androidx.annotation.Nullable;
import com.matter.tv.app.api.IMatterAppAgent;
import com.matter.tv.app.api.MatterIntentConstants;
import java.util.concurrent.TimeUnit;

public class ContentAppAgentService extends Service {

  private static final String TAG = "ContentAppAgentService";
  public static final String ACTION_MATTER_RESPONSE =
      "com.matter.tv.app.api.action.MATTER_COMMAND_RESPONSE";
  public static final String EXTRA_RESPONSE_RECEIVING_PACKAGE = "EXTRA_RESPONSE_RECEIVING_PACKAGE";
  public static final String EXTRA_RESPONSE_ID = "EXTRA_RESPONSE_ID";

  private static ResponseRegistry responseRegistry = new ResponseRegistry();

  private final IBinder appAgentBinder =
      new IMatterAppAgent.Stub() {
        @Override
        public boolean setSupportedClusters(
            com.matter.tv.app.api.SetSupportedClustersRequest request) throws RemoteException {
          Log.d(
              TAG,
              "Received request to add the following supported clusters "
                  + request.supportedClusters.toString());
          // TODO : need to (re)discover the app with the new clusters.
          return true;
        }

        @Override
        public boolean reportAttributeChange(
            com.matter.tv.app.api.ReportAttributeChangeRequest request) throws RemoteException {
          return false;
        }
      };

  @Nullable
  @Override
  public IBinder onBind(final Intent intent) {
    Log.d(TAG, "Received binding request.");
    if (MatterIntentConstants.ACTION_MATTER_AGENT.equals(intent.getAction())) {
      Log.d(TAG, "Returning MatterAppAgent");
      return appAgentBinder;
    }
    return null;
  }

  public static String sendCommand(
      Context context, String packageName, int clusterId, int commandId, String payload) {
    Intent in = new Intent(MatterIntentConstants.ACTION_MATTER_COMMAND);
    Bundle extras = new Bundle();
    extras.putByteArray(MatterIntentConstants.EXTRA_COMMAND_PAYLOAD, payload.getBytes());
    extras.putInt(MatterIntentConstants.EXTRA_COMMAND_ID, commandId);
    extras.putInt(MatterIntentConstants.EXTRA_CLUSTER_ID, clusterId);
    in.putExtras(extras);
    in.setPackage(packageName);
    int flags = Intent.FLAG_INCLUDE_STOPPED_PACKAGES;
    flags |= Intent.FLAG_RECEIVER_FOREGROUND;
    in.setFlags(flags);
    int messageId = responseRegistry.getNextMessageCounter();
    in.putExtra(
        MatterIntentConstants.EXTRA_DIRECTIVE_RESPONSE_PENDING_INTENT,
        getPendingIntentForResponse(context, packageName, messageId));
    context.sendBroadcast(in);
    responseRegistry.waitForMessage(messageId, 10, TimeUnit.SECONDS);
    String response = responseRegistry.readAndRemoveResponse(messageId);
    if (response == null) {
      response = "";
    }
    Log.d(TAG, "Response " + response + " being returned for message " + messageId);
    return response;
  }

  public static String sendAttributeReadRequest(
      Context context, String packageName, int clusterId, int attributeId) {
    Intent in = new Intent(MatterIntentConstants.ACTION_MATTER_COMMAND);
    Bundle extras = new Bundle();
    extras.putString(
        MatterIntentConstants.EXTRA_ATTRIBUTE_ACTION, MatterIntentConstants.ATTRIBUTE_ACTION_READ);
    extras.putInt(MatterIntentConstants.EXTRA_ATTRIBUTE_ID, attributeId);
    extras.putInt(MatterIntentConstants.EXTRA_CLUSTER_ID, clusterId);
    in.putExtras(extras);
    in.setPackage(packageName);
    int flags = Intent.FLAG_INCLUDE_STOPPED_PACKAGES;
    flags |= Intent.FLAG_RECEIVER_FOREGROUND;
    in.setFlags(flags);
    int messageId = responseRegistry.getNextMessageCounter();
    in.putExtra(
        MatterIntentConstants.EXTRA_DIRECTIVE_RESPONSE_PENDING_INTENT,
        getPendingIntentForResponse(context, packageName, messageId));
    context.sendBroadcast(in);
    responseRegistry.waitForMessage(messageId, 10, TimeUnit.SECONDS);
    String response = responseRegistry.readAndRemoveResponse(messageId);
    if (response == null) {
      response = "";
    }
    Log.d(TAG, "Response " + response + " being returned for message " + messageId);
    return response;
  }

  private static PendingIntent getPendingIntentForResponse(
      final Context context, final String targetPackage, final int responseId) {
    Intent ackBackIntent = new Intent(ACTION_MATTER_RESPONSE);
    ackBackIntent.setClass(context, ContentAppAgentService.class);
    ackBackIntent.putExtra(EXTRA_RESPONSE_RECEIVING_PACKAGE, targetPackage);
    ackBackIntent.putExtra(EXTRA_RESPONSE_ID, responseId);

    return PendingIntent.getService(context, 0, ackBackIntent, PendingIntent.FLAG_ONE_SHOT);
  }

  @Override
  public int onStartCommand(final Intent intent, final int flags, final int startId) {
    if (intent != null && ACTION_MATTER_RESPONSE.equals(intent.getAction())) {
      String response =
          new String(intent.getByteArrayExtra(MatterIntentConstants.EXTRA_RESPONSE_PAYLOAD));
      int messageId = intent.getIntExtra(EXTRA_RESPONSE_ID, Integer.MAX_VALUE);
      Log.d(TAG, "Response " + response + " received for message " + messageId);
      responseRegistry.receivedMessageResponse(messageId, response);
    }
    return START_NOT_STICKY;
  }
}
