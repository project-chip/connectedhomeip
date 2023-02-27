package com.matter.controller.commands.common

import java.net.InetAddress

class IPAddress(private var address: InetAddress) {
  fun setAddress(address: InetAddress) {
    this.address = address
  }

  fun getHostAddress(): String {
    return address.hostAddress
  }

  override fun toString(): String {
    return address.toString()
  }
}