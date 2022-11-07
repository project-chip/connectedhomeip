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

import android.Manifest
import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.DisplayMetrics
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.content.ContextCompat
import androidx.core.content.ContextCompat.checkSelfPermission
import androidx.fragment.app.Fragment
import chip.setuppayload.SetupPayload
import chip.setuppayload.SetupPayloadParser
import chip.setuppayload.SetupPayloadParser.UnrecognizedQrCodeException
import com.google.chip.chiptool.R
import com.google.chip.chiptool.SelectActionFragment
import com.google.chip.chiptool.util.FragmentUtil
import com.google.mlkit.vision.barcode.BarcodeScanner
import com.google.mlkit.vision.barcode.BarcodeScanning
import com.google.mlkit.vision.barcode.common.Barcode
import com.google.mlkit.vision.common.InputImage
import kotlinx.android.synthetic.main.barcode_fragment.view.inputAddressBtn
import java.util.concurrent.Executors
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min

/** Launches the camera to scan for QR code. */
class BarcodeFragment : Fragment() {

    private lateinit var previewView: PreviewView
    private var manualCodeEditText: EditText? = null
    private var manualCodeBtn: Button? = null

    private fun aspectRatio(width: Int, height: Int): Int {
        val previewRatio = max(width, height).toDouble() / min(width, height)
        if (abs(previewRatio - RATIO_4_3_VALUE) <= abs(previewRatio - RATIO_16_9_VALUE)) {
            return AspectRatio.RATIO_4_3
        }
        return AspectRatio.RATIO_16_9
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (!hasCameraPermission()) {
            requestCameraPermission()
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return inflater.inflate(R.layout.barcode_fragment, container, false).apply {
            previewView = findViewById(R.id.camera_view)
            manualCodeEditText = findViewById(R.id.manualCodeEditText)
            manualCodeBtn = findViewById(R.id.manualCodeBtn)
            startCamera()
            inputAddressBtn.setOnClickListener {
                FragmentUtil.getHost(
                    this@BarcodeFragment,
                    SelectActionFragment.Callback::class.java
                )?.onShowDeviceAddressInput()
            }
        }
    }

    @SuppressLint("UnsafeOptInUsageError")
    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(requireActivity())
        cameraProviderFuture.addListener({
            val cameraProvider: ProcessCameraProvider = cameraProviderFuture.get()
            val metrics = DisplayMetrics().also { previewView.display?.getRealMetrics(it) }
            val screenAspectRatio = aspectRatio(metrics.widthPixels, metrics.heightPixels)
            // Preview
            val preview: Preview = Preview.Builder()
                .setTargetAspectRatio(screenAspectRatio)
                .setTargetRotation(previewView.display.rotation)
                .build()
            preview.setSurfaceProvider(previewView.surfaceProvider)

            // Setup barcode scanner
            val imageAnalysis = ImageAnalysis.Builder()
                .setTargetAspectRatio(screenAspectRatio)
                .setTargetRotation(previewView.display.rotation)
                .build()
            val cameraExecutor = Executors.newSingleThreadExecutor()
            val barcodeScanner: BarcodeScanner = BarcodeScanning.getClient()
            imageAnalysis.setAnalyzer(cameraExecutor) { imageProxy ->
                processImageProxy(barcodeScanner, imageProxy)
            }
            // Select back camera as a default
            val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA
            try {
                // Unbind use cases before rebinding
                cameraProvider.unbindAll()

                // Bind use cases to camera
                cameraProvider.bindToLifecycle(this, cameraSelector, preview, imageAnalysis)

            } catch (exc: Exception) {
                Log.e(TAG, "Use case binding failed", exc)
            }
        }, ContextCompat.getMainExecutor(requireActivity()))

        //workaround: can not use gms to scan the code in China, added a EditText to debug
        manualCodeBtn?.setOnClickListener {
            val qrCode = manualCodeEditText?.text.toString()
            Log.d(TAG, "Submit Code:$qrCode")
            handleInputQrCode(qrCode)
        }
    }

    @ExperimentalGetImage
    private fun processImageProxy(
        barcodeScanner: BarcodeScanner,
        imageProxy: ImageProxy
    ) {
        val inputImage =
            InputImage.fromMediaImage(imageProxy.image!!, imageProxy.imageInfo.rotationDegrees)

        barcodeScanner.process(inputImage)
            .addOnSuccessListener { barcodes ->
                barcodes.forEach {
                    handleScannedQrCode(it)
                }
            }
            .addOnFailureListener {
                Log.e(TAG, it.message ?: it.toString())
            }.addOnCompleteListener {
                // When the image is from CameraX analysis use case, must call image.close() on received
                // images when finished using them. Otherwise, new images may not be received or the camera
                // may stall.
                imageProxy.close()
            }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>,
        grantResults: IntArray
    ) {
        if (requestCode == REQUEST_CODE_CAMERA_PERMISSION) {
            if (grantResults.size == 1 && grantResults[0] == PackageManager.PERMISSION_DENIED) {
                showCameraPermissionAlert()
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        }
    }

    private fun handleInputQrCode(qrCode: String) {
        lateinit var payload: SetupPayload
        try {
            payload = SetupPayloadParser().parseQrCode(qrCode)
        } catch (ex: UnrecognizedQrCodeException) {
            Log.e(TAG, "Unrecognized QR Code", ex)
            Toast.makeText(requireContext(), "Unrecognized QR Code", Toast.LENGTH_SHORT).show()
        }
        FragmentUtil.getHost(this, Callback::class.java)
            ?.onCHIPDeviceInfoReceived(CHIPDeviceInfo.fromSetupPayload(payload))
    }

    private fun handleScannedQrCode(barcode: Barcode) {
        Handler(Looper.getMainLooper()).post {
            lateinit var payload: SetupPayload
            try {
                payload = SetupPayloadParser().parseQrCode(barcode.displayValue)
            } catch (ex: UnrecognizedQrCodeException) {
                Log.e(TAG, "Unrecognized QR Code", ex)
                Toast.makeText(requireContext(), "Unrecognized QR Code", Toast.LENGTH_SHORT).show()
                return@post
            }
            FragmentUtil.getHost(this, Callback::class.java)
                ?.onCHIPDeviceInfoReceived(CHIPDeviceInfo.fromSetupPayload(payload))
        }
    }

    private fun showCameraPermissionAlert() {
        AlertDialog.Builder(requireContext())
            .setTitle(R.string.camera_permission_missing_alert_title)
            .setMessage(R.string.camera_permission_missing_alert_subtitle)
            .setPositiveButton(R.string.camera_permission_missing_alert_try_again) { _, _ ->
                requestCameraPermission()
            }
            .setCancelable(false)
            .create()
            .show()
    }

    private fun showCameraUnavailableAlert() {
        AlertDialog.Builder(requireContext())
            .setTitle(R.string.camera_unavailable_alert_title)
            .setMessage(R.string.camera_unavailable_alert_subtitle)
            .setPositiveButton(R.string.camera_unavailable_alert_exit) { _, _ ->
                requireActivity().finish()
            }
            .setCancelable(false)
            .create()
            .show()
    }

    private fun hasCameraPermission(): Boolean {
        return (PackageManager.PERMISSION_GRANTED
                == checkSelfPermission(requireContext(), Manifest.permission.CAMERA))
    }

    private fun requestCameraPermission() {
        val permissions = arrayOf(Manifest.permission.CAMERA)
        requestPermissions(permissions, REQUEST_CODE_CAMERA_PERMISSION)
    }

    /** Interface for notifying the host. */
    interface Callback {
        /** Notifies host of the [CHIPDeviceInfo] from the scanned QR code. */
        fun onCHIPDeviceInfoReceived(deviceInfo: CHIPDeviceInfo)
    }

    companion object {
        private const val TAG = "BarcodeFragment"
        private const val REQUEST_CODE_CAMERA_PERMISSION = 100;

        @JvmStatic
        fun newInstance() = BarcodeFragment()

        private const val RATIO_4_3_VALUE = 4.0 / 3.0
        private const val RATIO_16_9_VALUE = 16.0 / 9.0
    }
}