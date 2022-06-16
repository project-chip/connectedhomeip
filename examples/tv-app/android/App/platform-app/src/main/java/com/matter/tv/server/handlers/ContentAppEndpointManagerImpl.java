package com.matter.tv.server.handlers;

import android.content.Context;
import android.util.Log;
import com.matter.tv.server.model.ContentApp;
import com.matter.tv.server.receivers.ContentAppDiscoveryService;
import com.matter.tv.server.service.ContentAppAgentService;
import com.matter.tv.server.tvapp.ContentAppEndpointManager;

public class ContentAppEndpointManagerImpl implements ContentAppEndpointManager {

  private static final String TAG = "MatterMainActivity";
  private final Context context;

  public ContentAppEndpointManagerImpl(Context context) {
    this.context = context;
  }

  public String sendCommand(int endpointId, int clusterId, int commandId, String commandPayload) {
    Log.d(TAG, "Received a command for endpointId " + endpointId + ". Message " + commandPayload);
    for (ContentApp app :
        ContentAppDiscoveryService.getReceiverInstance().getDiscoveredContentApps().values()) {
      if (app.getEndpointId() == endpointId) {
        Log.d(
            TAG, "Sending a command for endpointId " + endpointId + ". Message " + commandPayload);
        return ContentAppAgentService.sendCommand(
            context, app.getAppName(), clusterId, commandId, commandPayload);
      }
    }
    return "Success";
  }
}
