package com.tcl.chip.tvapp;

public interface ContentAppEndpointManager {

  public String sendCommand(int endpointId, String commandPayload);
}
