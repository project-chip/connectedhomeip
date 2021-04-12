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
import android.text.TextUtils
import android.util.DisplayMetrics
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.annotation.RequiresPermission
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
import com.google.android.gms.vision.barcode.BarcodeDetector
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.FragmentUtil
import com.google.mlkit.vision.barcode.BarcodeScanner
import com.google.mlkit.vision.barcode.BarcodeScanning
import com.google.mlkit.vision.common.InputImage
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min

/** Launches the camera to scan for QR code. */
class BarcodeFragment : Fragment() {


    private var cameraPreviewView: PreviewView? = null
    private var barcodeDetector: BarcodeDetector? = null
    private var cameraStarted = false
    private val screenAspectRatio: Int
        get() {
            // Get screen metrics used to setup camera for full screen resolution
            val metrics = DisplayMetrics().also { cameraPreviewView?.display?.getRealMetrics(it) }
            return aspectRatio(metrics.widthPixels, metrics.heightPixels)
        }

    private var cameraExecutor: ExecutorService? = null
    private var cameraProvider: ProcessCameraProvider? = null

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
            cameraPreviewView = findViewById(R.id.camera_view)
        }
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        initializeBarcodeDetectorAndCamera()
    }

    @SuppressLint("MissingPermission")
    override fun onResume() {
        super.onResume()

        if (hasCameraPermission() && !cameraStarted) {
            startCamera()
        }
    }

    private fun initializeBarcodeDetectorAndCamera() {
        barcodeDetector?.let { detector ->
            if (!detector.isOperational) {
                showCameraUnavailableAlert()
            }
            return
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


    @SuppressLint("UnsafeExperimentalUsageError")
    private fun processImageProxy(
            barcodeScanner: BarcodeScanner,
            imageProxy: ImageProxy
    ) {
        val inputImage =
                InputImage.fromMediaImage(imageProxy.image!!, imageProxy.imageInfo.rotationDegrees)

        barcodeScanner.process(inputImage)
                .addOnSuccessListener { barcodes ->
                    barcodes.forEach {
                        val displayValue = it.displayValue
                        if (!TextUtils.isEmpty(displayValue)) {
                            Log.d(TAG, displayValue!!)
                            handleScannedQrCode(displayValue)
                        }
                    }
                }
                .addOnFailureListener {
                    Log.e(TAG, it.toString(), it)
                }.addOnCompleteListener {
                    // When the image is from CameraX analysis use case, must call image.close() on received
                    // images when finished using them. Otherwise, new images may not be received or the camera
                    // may stall.
                    imageProxy.close()
                }
    }

    @SuppressLint("MissingPermission")
    fun handleScannedQrCode(barcodeValue: String) {
        Handler(Looper.getMainLooper()).post {
            stopCamera()

            lateinit var payload: SetupPayload
            try {
                payload = SetupPayloadParser().parseQrCode(barcodeValue)
            } catch (ex: UnrecognizedQrCodeException) {
                Log.e(TAG, "Unrecognized QR Code", ex)
                Toast.makeText(requireContext(), "Unrecognized QR Code", Toast.LENGTH_SHORT).show()

                // Restart camera view.
                if (hasCameraPermission() && !cameraStarted) {
                    startCamera()
                }
                return@post
            }
            val deviceInfo = CHIPDeviceInfo(
                    payload.version,
                    payload.vendorId,
                    payload.productId,
                    payload.discriminator,
                    payload.setupPinCode,
                    payload.optionalQRCodeInfo.mapValues { (_, info) ->
                        QrCodeInfo(info.tag, info.type, info.data, info.int32)
                    }
            )
            FragmentUtil.getHost(this, Callback::class.java)
                    ?.onCHIPDeviceInfoReceived(deviceInfo)
        }
    }

    /**
     *  [androidx.camera.core.ImageAnalysis], [androidx.camera.core.Preview] requires enum value of
     *  [androidx.camera.core.AspectRatio]. Currently it has values of 4:3 & 16:9.
     *
     *  Detecting the most suitable ratio for dimensions provided in @params by counting absolute
     *  of preview ratio to one of the provided values.
     *
     *  @param width - preview width
     *  @param height - preview height
     *  @return suitable aspect ratio
     */
    private fun aspectRatio(width: Int, height: Int): Int {
        val previewRatio = max(width, height).toDouble() / min(width, height)
        if (abs(previewRatio - RATIO_4_3_VALUE) <= abs(previewRatio - RATIO_16_9_VALUE)) {
            return AspectRatio.RATIO_4_3
        }
        return AspectRatio.RATIO_16_9
    }

    override fun onPause() {
        super.onPause()
        stopCamera()
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


    @RequiresPermission(Manifest.permission.CAMERA)
    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(requireContext())

        cameraProviderFuture.addListener(Runnable {
            // Used to bind the lifecycle of cameras to the lifecycle owner
            cameraProvider = cameraProviderFuture.get()
            // Initialize our background executor
            cameraExecutor = Executors.newSingleThreadExecutor()
            // Preview
            val preview = Preview.Builder()
                    .build()
                    .also {
                        it.setSurfaceProvider(cameraPreviewView?.createSurfaceProvider())
                    }

            // Select back camera as a default
            val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA

            val barcodeScanner: BarcodeScanner = BarcodeScanning.getClient()
            val imageAnalysis = ImageAnalysis.Builder()
                    .setTargetAspectRatio(screenAspectRatio)
                    .setTargetRotation(cameraPreviewView!!.display.rotation)
                    .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                    .build()

            imageAnalysis.setAnalyzer(
                    cameraExecutor!!,
                    ImageAnalysis.Analyzer { imageProxy ->
                        processImageProxy(barcodeScanner, imageProxy)
                    }
            )
            try {
                // Unbind use cases before rebinding
                cameraProvider?.unbindAll()

                // Bind use cases to camera
                cameraProvider?.bindToLifecycle(
                        this, cameraSelector, imageAnalysis, preview)
                cameraStarted = true
            } catch (exc: Exception) {
                Log.e(TAG, "Use case binding failed", exc)
            }

        }, ContextCompat.getMainExecutor(requireContext()))
    }

    private fun stopCamera() {
        cameraStarted = false
        // Unbind use cases
        cameraProvider?.unbindAll()
        // Shutdown background executor
        cameraExecutor?.shutdown()
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
        private const val PERMISSION_CAMERA_REQUEST = 1

        private const val RATIO_4_3_VALUE = 4.0 / 3.0
        private const val RATIO_16_9_VALUE = 16.0 / 9.0

        @JvmStatic
        fun newInstance() = BarcodeFragment()
    }
}
