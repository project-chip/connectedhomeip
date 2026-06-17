/*
 *   Copyright (c) 2026 Project CHIP Authors
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
package com.google.matter.alldevices;

import android.content.Context
import android.net.wifi.WifiManager
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import chip.platform.*
import com.google.zxing.BarcodeFormat
import com.google.zxing.qrcode.QRCodeWriter
import android.graphics.Bitmap
import java.io.BufferedReader
import java.io.InputStreamReader
import kotlin.concurrent.thread

class MainActivity : ComponentActivity() {

    private var multicastLock: WifiManager.MulticastLock? = null
    private var androidChipPlatform: AndroidChipPlatform? = null
    private var logcatProcess: Process? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Acquire multicast lock for DNS-SD discovery to function properly on Android
        val wifi = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        multicastLock = wifi.createMulticastLock("AllDevicesAppMulticastLock").apply {
            setReferenceCounted(true)
            acquire()
        }

        setContent {
            AppUI()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        stopServer()
        multicastLock?.let {
            if (it.isHeld) {
                it.release()
            }
        }
    }

    private fun startServer(
        discriminator: Int,
        selectedDevices: List<String>,
        onLogsUpdated: (String) -> Unit
    ): Pair<String, String>? {
        try {
            // 1. Initialize Android platform managers
            androidChipPlatform = AndroidChipPlatform(
                null, // BLE manager disabled
                null, // NFC manager disabled
                PreferencesKeyValueStoreManager(applicationContext),
                PreferencesConfigurationManager(applicationContext),
                NsdManagerServiceResolver(applicationContext),
                NsdManagerServiceBrowser(applicationContext),
                ChipMdnsCallbackImpl(),
                DiagnosticDataProviderImpl(applicationContext)
            )

            // 2. Set passcode (20202021) and custom discriminator
            androidChipPlatform?.updateCommissionableDataProviderData(
                null, // Use default Spake2p verifier
                null, // Use default Spake2p salt
                0,    // Use default Spake2p iteration count
                20202021L,
                discriminator
            )

            // 3. Start C++ Matter stack and register chosen code-driven devices
            val success = App.getInstance().startApp(selectedDevices.toTypedArray())
            if (!success) {
                Toast.makeText(this, "Failed to start Matter app", Toast.LENGTH_LONG).show()
                return null
            }

            // 4. Generate Manual Setup Code and QR Code payloads via JNI
            val codes = App.getInstance().getOnboardingCodes(discriminator)
            val manualCode = codes[0]
            val qrCode = codes[1]

            // 5. Start streaming logs in real time
            startLogging(onLogsUpdated)

            return Pair(manualCode, qrCode)
        } catch (e: Exception) {
            Toast.makeText(this, "Error: ${e.message}", Toast.LENGTH_LONG).show()
            return null
        }
    }

    private fun stopServer() {
        logcatProcess?.destroy()
        logcatProcess = null
        App.getInstance().stopApp()
    }

    private fun factoryReset() {
        stopServer()
        // Wipes all shared preferences credentials stored by Matter
        val sharedPrefs = getSharedPreferences("chip.platform.KeyValueStore", Context.MODE_PRIVATE)
        sharedPrefs.edit().clear().commit()
        
        val configPrefs = getSharedPreferences("chip.platform.ConfigurationManager", Context.MODE_PRIVATE)
        configPrefs.edit().clear().commit()

        Toast.makeText(this, "KVS Storage Wiped. App needs restart.", Toast.LENGTH_LONG).show()
    }

    private fun startLogging(onLineReceived: (String) -> Unit) {
        logcatProcess?.destroy()
        val pid = android.os.Process.myPid()
        thread {
            try {
                // Read from local logcat, filter logs matching our PID
                val process = Runtime.getRuntime().exec(arrayOf("logcat", "-v", "time", "--pid", pid.toString()))
                logcatProcess = process
                val reader = BufferedReader(InputStreamReader(process.inputStream))
                var line: String?
                while (reader.readLine().also { line = it } != null) {
                    onLineReceived(line!!)
                }
            } catch (e: Exception) {
                onLineReceived("Logs closed: ${e.message}")
            }
        }
    }

    private fun generateQRCode(content: String): Bitmap? {
        return try {
            val writer = QRCodeWriter()
            val bitMatrix = writer.encode(content, BarcodeFormat.QR_CODE, 400, 400)
            val width = bitMatrix.width
            val height = bitMatrix.height
            val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565)
            for (x in 0 until width) {
                for (y in 0 until height) {
                    bitmap.setPixel(x, y, if (bitMatrix.get(x, y)) android.graphics.Color.BLACK else android.graphics.Color.WHITE)
                }
            }
            bitmap
        } catch (e: Exception) {
            null
        }
    }

    @OptIn(ExperimentalMaterial3Api::class)
    @Composable
    fun AppUI() {
        val supportedDevices = remember { App.getInstance().supportedDeviceTypes.toList() }
        val selectedDevices = remember { mutableStateListOf<String>() }

        var discriminatorText by remember { mutableStateOf("3840") }
        var isRunning by remember { mutableStateOf(false) }

        var manualCode by remember { mutableStateOf("") }
        var qrCode by remember { mutableStateOf("") }

        val logLines = remember { mutableStateListOf<String>() }

        Scaffold(
            topBar = {
                TopAppBar(
                    title = { Text("Matter All-Devices Simulator") },
                    colors = TopAppBarDefaults.topAppBarColors(containerColor = MaterialTheme.colorScheme.primaryContainer)
                )
            }
        ) { padding ->
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding)
                    .padding(16.dp)
            ) {
                if (!isRunning) {
                    // Configuration Form
                    Column(
                        modifier = Modifier
                            .weight(1f)
                            .verticalScroll(rememberScrollState())
                    ) {
                        Text("1. Select Devices to Simulate:", style = MaterialTheme.typography.titleMedium)
                        Spacer(modifier = Modifier.height(8.dp))

                        supportedDevices.forEach { deviceType ->
                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                Checkbox(
                                    checked = selectedDevices.contains(deviceType),
                                    onCheckedChange = { checked ->
                                        if (checked) selectedDevices.add(deviceType)
                                        else selectedDevices.remove(deviceType)
                                    }
                                )
                                Text(deviceType, style = MaterialTheme.typography.bodyLarge)
                            }
                        }

                        Spacer(modifier = Modifier.height(16.dp))

                        OutlinedTextField(
                            value = discriminatorText,
                            onValueChange = { discriminatorText = it },
                            label = { Text("2. Setup Discriminator") },
                            modifier = Modifier.fillMaxWidth()
                        )
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(16.dp)
                    ) {
                        Button(
                            onClick = {
                                val disc = discriminatorText.toIntOrNull() ?: 3840
                                logLines.clear()
                                val codes = startServer(disc, selectedDevices) { line ->
                                    // Keep logs to latest 150 lines
                                    if (logLines.size > 150) {
                                        logLines.removeAt(0)
                                    }
                                    logLines.add(line)
                                }
                                if (codes != null) {
                                    manualCode = codes.first
                                    qrCode = codes.second
                                    isRunning = true
                                }
                            },
                            modifier = Modifier.weight(1f),
                            enabled = selectedDevices.isNotEmpty()
                        ) {
                            Text("Start Server")
                        }

                        Button(
                            onClick = { factoryReset() },
                            colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.error),
                            modifier = Modifier.weight(1f)
                        ) {
                            Text("Factory Reset")
                        }
                    }
                } else {
                    // Running Server State
                    Column(
                        modifier = Modifier.weight(1f),
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        Text("Server is Running", style = MaterialTheme.typography.headlineMedium, color = Color(0xFF4CAF50))
                        Spacer(modifier = Modifier.height(16.dp))

                        // QR Code and Code Details Card
                        Card(
                            modifier = Modifier.fillMaxWidth(),
                            shape = RoundedCornerShape(12.dp)
                        ) {
                            Column(
                                modifier = Modifier.padding(16.dp),
                                horizontalAlignment = Alignment.CenterHorizontally
                            ) {
                                val qrBitmap = generateQRCode(qrCode)
                                if (qrBitmap != null) {
                                    Image(
                                        bitmap = qrBitmap.asImageBitmap(),
                                        contentDescription = "Matter QR Code",
                                        modifier = Modifier.size(180.dp)
                                    )
                                    Spacer(modifier = Modifier.height(8.dp))
                                }
                                Text("Passcode: 20202021", fontSize = 18.sp)
                                Text("Discriminator: $discriminatorText", fontSize = 18.sp)
                                Text("Manual Pairing Code: $manualCode", fontSize = 16.sp)
                            }
                        }

                        Spacer(modifier = Modifier.height(16.dp))

                        // Logs Console Card
                        Text("App Log Viewer:", style = MaterialTheme.typography.titleMedium, modifier = Modifier.align(Alignment.Start))
                        Spacer(modifier = Modifier.height(4.dp))
                        Box(
                            modifier = Modifier
                                .fillMaxWidth()
                                .weight(1f)
                                .background(Color.Black, shape = RoundedCornerShape(8.dp))
                                .border(1.dp, Color.DarkGray, shape = RoundedCornerShape(8.dp))
                                .padding(8.dp)
                        ) {
                            LazyColumn(modifier = Modifier.fillMaxSize()) {
                                items(logLines) { line ->
                                    Text(
                                        text = line,
                                        color = Color.Green,
                                        fontFamily = FontFamily.Monospace,
                                        fontSize = 11.sp
                                    )
                                }
                            }
                        }
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    Button(
                        onClick = {
                            stopServer()
                            isRunning = false
                        },
                        colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.error),
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text("Stop Server")
                    }
                }
            }
        }
    }
}
