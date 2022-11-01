package com.matter.controller.commands.common;

import java.net.InetAddress;

public final class IPAddress {
  private InetAddress address;

  public void setAddress(InetAddress address) {
    this.address = address;
  }

  @Override
  public String toString() {
    return address.toString();
  }
}
