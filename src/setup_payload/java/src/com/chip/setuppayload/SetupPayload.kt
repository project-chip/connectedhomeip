package com.chip.setuppayload

object SetupPayload(
    version: Int,
    vendorId: Int,
    productId: Int,
    requiresCustomFlow: Boolean,
    rendezvousInformation: Int,
    discriminator: Int,
    setUpPinCode: Long,
    serialNumber: String
)
