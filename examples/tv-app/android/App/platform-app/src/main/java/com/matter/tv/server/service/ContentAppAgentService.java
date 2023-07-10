package com.matter.tv.server.service;

import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.matter.tv.app.api.IMatterAppAgent;
import com.matter.tv.app.api.MatterIntentConstants;
import com.matter.tv.server.model.ContentApp;
import com.matter.tv.server.receivers.ContentAppDiscoveryService;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class ContentAppAgentService extends Service {

  private static final String TAG = "ContentAppAgentService";
  public static final String ACTION_MATTER_RESPONSE =
      "com.matter.tv.app.api.action.MATTER_COMMAND_RESPONSE";
  public static final String EXTRA_RESPONSE_RECEIVING_PACKAGE = "EXTRA_RESPONSE_RECEIVING_PACKAGE";
  public static final String EXTRA_RESPONSE_ID = "EXTRA_RESPONSE_ID";

  public static final String FAILURE_KEY = "PlatformError";
  public static final String FAILURE_STATUS_KEY = "Status";
  public static final int FAILED_UNSUPPORTED_ENDPOINT = 0x7f;
  public static final int FAILED_UNSUPPORTED_CLUSTER = 0xc3;
  public static final int FAILED_UNSUPPORTED_COMMAND = 0x81;
  public static final int FAILED_UNSUPPORTED_ATTRIBUTE = 0x86;
  public static final int FAILED_UNKNOWN = 0x01;
  public static final int FAILED_TIMEOUT = 0x94;

  private static final int COMMAND_TIMEOUT = 8; // seconds
  private static final int ATTRIBUTE_TIMEOUT = 2; // seconds

  private static ResponseRegistry responseRegistry = new ResponseRegistry();
  private static ExecutorService executorService =
      Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());

  private final IBinder appAgentBinder =
      new IMatterAppAgent.Stub() {
        @Override
        public boolean setSupportedClusters(
            com.matter.tv.app.api.SetSupportedClustersRequest request) throws RemoteException {
          final int callingUID = Binder.getCallingUid();
          final String pkg = getApplicationContext().getPackageManager().getNameForUid(callingUID);
          Log.d(
              TAG,
              "Received request to add the following supported clusters "
                  + request.supportedClusters.toString()
                  + " for app "
                  + pkg);
          ContentApp contentApp =
              ContentAppDiscoveryService.getReceiverInstance().getDiscoveredContentApp(pkg);
          if (contentApp != null) {
            contentApp.setSupportedClusters(request.supportedClusters);
            return true;
          }
          Log.e(TAG, "No matter content app found for package " + pkg);
          return false;
        }

        @Override
        public boolean reportAttributeChange(int clusterId, int attributeId)
            throws RemoteException {
          final int callingUID = Binder.getCallingUid();
          final String pkg = getApplicationContext().getPackageManager().getNameForUid(callingUID);
          Log.d(
              TAG,
              "Received request to report attribute change for cluster "
                  + clusterId
                  + " attribute "
                  + attributeId);
          ContentApp contentApp =
              ContentAppDiscoveryService.getReceiverInstance().getDiscoveredContentApp(pkg);
          if (contentApp != null && contentApp.getEndpointId() != ContentApp.INVALID_ENDPOINTID) {
            // Make this call async so that even if the content apps make this call during command
            // processing and synchronously, the command processing thread will not block for the
            // chip stack lock.
            executorService.execute(
                () -> {
                  AppPlatformService.get()
                      .reportAttributeChange(contentApp.getEndpointId(), clusterId, attributeId);
                });
            return true;
          }
          Log.e(TAG, "No matter content app found for package " + pkg);
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
      Context context, String packageName, long clusterId, long commandId, String payload) {
    Intent in = new Intent(MatterIntentConstants.ACTION_MATTER_COMMAND);
    Bundle extras = new Bundle();
    extras.putByteArray(MatterIntentConstants.EXTRA_COMMAND_PAYLOAD, payload.getBytes());
    extras.putLong(MatterIntentConstants.EXTRA_COMMAND_ID, commandId);
    extras.putLong(MatterIntentConstants.EXTRA_CLUSTER_ID, clusterId);
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
    return getResponse(messageId, COMMAND_TIMEOUT);
  }

  public static String sendAttributeReadRequest(
      Context context, String packageName, long clusterId, long attributeId) {
    Intent in = new Intent(MatterIntentConstants.ACTION_MATTER_COMMAND);
    Bundle extras = new Bundle();
    extras.putString(
        MatterIntentConstants.EXTRA_ATTRIBUTE_ACTION, MatterIntentConstants.ATTRIBUTE_ACTION_READ);
    extras.putLong(MatterIntentConstants.EXTRA_ATTRIBUTE_ID, attributeId);
    extras.putLong(MatterIntentConstants.EXTRA_CLUSTER_ID, clusterId);
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
    return getResponse(messageId, ATTRIBUTE_TIMEOUT);
  }

  @NonNull
  private static String getResponse(int messageId, int timeout) {
    ResponseRegistry.WaitState status =
        responseRegistry.waitForMessage(messageId, timeout, TimeUnit.SECONDS);
    String response = "";
    switch (status) {
      case SUCCESS:
      case INVALID_COUNTER:
        response = responseRegistry.readAndRemoveResponse(messageId);
        if (response == null) {
          response =
              "{\""
                  + FAILURE_KEY
                  + "\":{\""
                  + ContentAppAgentService.FAILURE_STATUS_KEY
                  + "\":"
                  + FAILED_UNKNOWN
                  + "}}";
        }
        break;
      case TIMED_OUT:
        response =
            "{\""
                + FAILURE_KEY
                + "\":{\""
                + ContentAppAgentService.FAILURE_STATUS_KEY
                + "\":"
                + FAILED_TIMEOUT
                + "}}";
        break;
      case INTERRUPTED:
        response = responseRegistry.readAndRemoveResponse(messageId);
        if (response == null) {
          response =
              "{\""
                  + FAILURE_KEY
                  + "\":{\""
                  + ContentAppAgentService.FAILURE_STATUS_KEY
                  + "\":"
                  + FAILED_TIMEOUT
                  + "}}";
        }
        break;
      default:
        response =
            "{\""
                + FAILURE_KEY
                + "\":{\""
                + ContentAppAgentService.FAILURE_STATUS_KEY
                + "\":"
                + FAILED_UNKNOWN
                + "}}";
    }
    Log.d(TAG, "Response " + response + " being returned for message " + messageId);
    return response;
  }

  private static PendingIntent getPendingIntentForResponse(
      Context context, final String targetPackage, final int responseId) {
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
