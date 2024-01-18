package com.matter.tv.server.tvapp;

public interface ContentAppEndpointManager {

  public String sendCommand(int endpointId, long clusterId, long commandId, String commandPayload);

  public String readAttribute(int endpointId, long clusterId, long attributeId);
}
