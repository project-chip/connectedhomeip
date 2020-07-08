/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
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
