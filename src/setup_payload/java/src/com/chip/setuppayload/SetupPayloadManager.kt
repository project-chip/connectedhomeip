package com.chip.setuppayload

import java.lang.System

class SetupPayloadManager(qrCodeString: String) {

    private val qrPayloadParserPtr: SetupPayloadManager

    init {
        qrPayloadParserPtr = newQrCodeSetupPayloadParser(qrCodeString)
    }

    fun fetchPayload(): SetupPayload = populatePayload(qrPayloadParserPtr)

    external fun newQrCodeSetupPayloadParser(qrCodeString: String): Long
    external fun populatePayload(qrCodePayloadParser: Long): SetupPayload

    companion object {
        init {
            System.loadLibrary("SetupPayload")
        }
    }
}