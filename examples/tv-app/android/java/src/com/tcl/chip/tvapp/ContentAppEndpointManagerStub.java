package com.tcl.chip.tvapp;

public class ContentAppEndpointManagerStub implements ContentAppEndpointManager {

  ContentAppEndpointManager delegate;

  public ContentAppEndpointManagerStub(ContentAppEndpointManager delegate) {
    this.delegate = delegate;
  }

  @Override
  public String sendCommand(int endpointId, String commandPayload) {
    return delegate.sendCommand(endpointId, commandPayload);
  }
}
