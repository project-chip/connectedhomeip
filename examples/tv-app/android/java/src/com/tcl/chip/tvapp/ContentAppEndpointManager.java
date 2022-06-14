package com.tcl.chip.tvapp;

public interface ContentAppEndpointManager {

  public String sendCommand(int endpointId, int clusterId, int commandId, String commandPayload);
}
