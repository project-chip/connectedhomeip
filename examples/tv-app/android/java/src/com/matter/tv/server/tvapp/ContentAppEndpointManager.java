package com.matter.tv.server.tvapp;

public interface ContentAppEndpointManager {

  public String sendCommand(int endpointId, int clusterId, int commandId, String commandPayload);

  public String readAttribute(int endpointId, int clusterId, int attributeId);
}
