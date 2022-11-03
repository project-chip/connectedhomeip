/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.google.chip.chiptool.setuppayloadscanner

import com.google.android.gms.vision.Detector
import com.google.android.gms.vision.barcode.Barcode

/** Processes the detected [Barcode]s from a Camera scan. */
class CHIPBarcodeProcessor(
    private val listener: BarcodeDetectionListener
) : Detector.Processor<Barcode> {

    override fun receiveDetections(detections: Detector.Detections<Barcode>) {
        val detectedItems = detections.detectedItems
        if (detectedItems.size() <= 0) {
            return
        }
        val barcode = detectedItems.valueAt(0)
        listener.handleScannedQrCode(barcode)
    }

    override fun release() {

    }

    /** Interface for receiving the detected QR Code from a Camera scan. */
    interface BarcodeDetectionListener {

        /** Provides the [Barcode] detected in camera scan to the registered listener. */
        fun handleScannedQrCode(barcode: Barcode)
    }
}
