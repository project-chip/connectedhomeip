package com.example.contentapp;

import android.util.Log;
import com.matter.tv.app.api.Clusters;
import java.util.HashMap;
import java.util.Map;

/** Class to hold attribute values to help test attribute read and subscribe use cases. */
public class CommandResponseHolder {
  private Map<Long, Map<Long, String>> responseValues = new HashMap<>();
  private static final String TAG = "CommandResponseHolder";
  private static final Long DEFAULT_COMMAND = -1L;

  private static CommandResponseHolder instance = new CommandResponseHolder();

  private CommandResponseHolder() {
    // Setting up responses
    setResponseValue(
        Clusters.ContentLauncher.Id,
        DEFAULT_COMMAND,
        "{\"0\":0, \"1\":\"custom response from content app for content launcher\"}");
    setResponseValue(
        Clusters.TargetNavigator.Id,
        DEFAULT_COMMAND,
        "{\"0\":0, \"1\":\"custom response from content app for target navigator\"}");
    setResponseValue(
        Clusters.MediaPlayback.Id,
        DEFAULT_COMMAND,
        "{\"0\":0, \"1\":\"custom response from content app for media playback\"}");
    setResponseValue(
        Clusters.AccountLogin.Id,
        Clusters.AccountLogin.Commands.GetSetupPIN.ID,
        "{\"0\":\"20202021\"}");
  };

  public static CommandResponseHolder getInstance() {
    return instance;
  }

  public void setResponseValue(long clusterId, long commandId, String value) {
    if (value == null) {
      Log.d(TAG, "Setting null for cluster " + clusterId + " command " + commandId);
    }
    Map<Long, String> responses = responseValues.get(clusterId);
    if (responses == null) {
      responses = new HashMap<>();
      responseValues.put(clusterId, responses);
    }
    responses.put(commandId, value);
  }

  public String getCommandResponse(long clusterId, long commandId) {
    Map<Long, String> responses = responseValues.get(clusterId);
    String response = responses.get(commandId);
    if (response == null) {
      response = responses.get(DEFAULT_COMMAND);
    }
    return response;
  }
}
