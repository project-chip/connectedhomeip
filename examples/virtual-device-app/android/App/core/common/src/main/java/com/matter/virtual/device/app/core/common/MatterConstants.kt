package com.matter.virtual.device.app.core.common

object MatterConstants {
  const val DEFAULT_VERSION = 0
  const val DEFAULT_VENDOR_ID = 0xFFF1
  const val DEFAULT_PRODUCT_ID = 0x8003
  const val DEFAULT_COMMISSIONING_FLOW = 0
  const val DEFAULT_SETUP_PINCODE = 20202021L
  const val DEFAULT_DISCRIMINATOR = 3840
  const val DEFAULT_DEVICE_NAME = "Matter Device"
  const val DEFAULT_ENDPOINT = 1

  const val TEST_SPAKE2P_VERIFIER =
    "uWFwqugDNGiEck/po7KHwwMwwqZgN10XuyBajPGuyzUEV/iree4lOrao5GuwnlQ65CJzbeUB49s31EH+NEkg0JVI5MGCQGMMT/SRPFNRODm3wH/MBiehuFc6FJ/NH6Rmzw=="
  const val TEST_SPAKE2P_SALT = "U1BBS0UyUCBLZXkgU2FsdA=="
  const val TEST_SPAKE2P_ITERATION_COUNT = 1000
  const val TEST_SETUP_PASSCODE: Long = 20202021
}
