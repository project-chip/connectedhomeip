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
import android.content.Intent
import android.os.Build
import android.net.wifi.WifiManager
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.*
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
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.foundation.BorderStroke
import java.io.BufferedReader
import java.io.InputStreamReader
import kotlin.concurrent.thread

data class EndpointConfig(
    val endpointId: Int,
    var deviceType: String,
    var parentId: Int = 0,
    var nodeLabel: String = "",
    var bridged: Boolean = false
)

class MainActivity : ComponentActivity() {

    private var androidChipPlatform: AndroidChipPlatform? = null
    private var logcatProcess: Process? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            AppUI()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
    }

    private fun startServer(
        discriminator: Int,
        configurationJson: String,
        onLogsUpdated: (String) -> Unit
    ): Pair<String, String>? {
        try {
            val intent = Intent(this, MatterServerService::class.java).apply {
                putExtra("discriminator", discriminator)
                putExtra("configurationJson", configurationJson)
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                startForegroundService(intent)
            } else {
                startService(intent)
            }

            // Generate Manual Setup Code and QR Code payloads via JNI
            val codes = App.getInstance().getOnboardingCodes(discriminator)
            val manualCode = codes[0]
            val qrCode = codes[1]

            // Start streaming logs in real time
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
        val intent = Intent(this, MatterServerService::class.java)
        stopService(intent)
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
            var process: Process? = null
            try {
                // Read from local logcat, filter logs matching our PID
                process = Runtime.getRuntime().exec(arrayOf("logcat", "-v", "time", "--pid", pid.toString()))
                logcatProcess = process
                process.inputStream.bufferedReader().use { reader ->
                    var line: String?
                    while (reader.readLine().also { line = it } != null) {
                        val logLine = line ?: break
                        if (logLine.contains("SVR") ||
                            logLine.contains("AllDevicesApp") ||
                            logLine.contains("ChimeDevice") ||
                            logLine.contains("OccupancySensor") ||
                            logLine.contains("TogglingOccupancy")) {
                            onLineReceived(logLine)
                        }
                    }
                }
            } catch (e: Exception) {
                onLineReceived("Logs closed: ${e.message}")
            } finally {
                process?.destroy()
            }
        }
    }

    private fun generateQRCode(content: String): Bitmap? {
        return try {
            val writer = QRCodeWriter()
            val bitMatrix = writer.encode(content, BarcodeFormat.QR_CODE, 400, 400)
            val width = bitMatrix.width
            val height = bitMatrix.height
            val pixels = IntArray(width * height)
            for (y in 0 until height) {
                val offset = y * width
                for (x in 0 until width) {
                    pixels[offset + x] = if (bitMatrix.get(x, y)) android.graphics.Color.BLACK else android.graphics.Color.WHITE
                }
            }
            val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565)
            bitmap.setPixels(pixels, 0, width, 0, 0, width, height)
            bitmap
        } catch (e: Exception) {
            null
        }
    }

    @OptIn(ExperimentalMaterial3Api::class)
    @Composable
    fun AppUI() {
        val supportedDevices = remember { App.getInstance().supportedDeviceTypes.toList() }
        val basicSupportedDevices = remember {
            App.getInstance().supportedDeviceTypes.filter { it != "aggregator" && it != "bridged-node" }
        }
        val configuredEndpoints = remember { mutableStateListOf<EndpointConfig>() }
        var selectedTab by remember { mutableStateOf(0) }
        val basicSelectedDevices = remember { mutableStateListOf<String>() }
        var isBridgeMode by remember { mutableStateOf(false) }

        var discriminatorText by remember { mutableStateOf("3840") }
        var isRunning by remember { mutableStateOf(false) }

        var manualCode by remember { mutableStateOf("") }
        var qrCode by remember { mutableStateOf("") }

        val logLines = remember { mutableStateListOf<String>() }

        var showConfigTopology by remember { mutableStateOf(true) }
        var runningSelectedTab by remember { mutableStateOf(0) }
        val lazyListState = rememberLazyListState()

        LaunchedEffect(logLines.size) {
            if (logLines.isNotEmpty()) {
                lazyListState.animateScrollToItem(logLines.size - 1)
            }
        }

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
                        val currentConfigs = if (selectedTab == 0) {
                            val basicConfigs = mutableListOf<EndpointConfig>()
                            if (isBridgeMode) {
                                basicConfigs.add(EndpointConfig(1, "aggregator", parentId = 0, nodeLabel = "Aggregator", bridged = false))
                                var nextEp = 2
                                basicSelectedDevices.forEach { type ->
                                    basicConfigs.add(EndpointConfig(nextEp, type, parentId = 1, nodeLabel = "", bridged = true))
                                    nextEp += 2
                                }
                            } else {
                                basicSelectedDevices.forEachIndexed { i, type ->
                                    basicConfigs.add(EndpointConfig(i + 1, type, parentId = 0, nodeLabel = "", bridged = false))
                                }
                            }
                            basicConfigs
                        } else {
                            configuredEndpoints
                        }

                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text("Data Model Topology", style = MaterialTheme.typography.titleMedium)
                            TextButton(onClick = { showConfigTopology = !showConfigTopology }) {
                                Text(if (showConfigTopology) "Hide Preview" else "Show Preview")
                            }
                        }

                        if (showConfigTopology) {
                            DataModelTopologyPreview(configs = currentConfigs)
                            Spacer(modifier = Modifier.height(16.dp))
                        }

                        TabRow(selectedTabIndex = selectedTab) {
                            Tab(selected = selectedTab == 0, onClick = { selectedTab = 0 }) {
                                Text("Basic", modifier = Modifier.padding(vertical = 12.dp))
                            }
                            Tab(selected = selectedTab == 1, onClick = { selectedTab = 1 }) {
                                Text("Advanced", modifier = Modifier.padding(vertical = 12.dp))
                            }
                        }
                        Spacer(modifier = Modifier.height(16.dp))

                        if (selectedTab == 0) {
                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                horizontalArrangement = Arrangement.SpaceBetween,
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                Text("Bridge Mode", style = MaterialTheme.typography.bodyLarge)
                                Switch(
                                    checked = isBridgeMode,
                                    onCheckedChange = { isBridgeMode = it }
                                )
                            }

                            Spacer(modifier = Modifier.height(16.dp))

                            Text("1. Select Devices to Simulate:", style = MaterialTheme.typography.titleMedium)
                            Spacer(modifier = Modifier.height(8.dp))

                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                val allSelected = basicSelectedDevices.size == basicSupportedDevices.size
                                Checkbox(
                                    checked = allSelected,
                                    onCheckedChange = { checked ->
                                        basicSelectedDevices.clear()
                                        if (checked) {
                                            basicSelectedDevices.addAll(basicSupportedDevices)
                                        }
                                    }
                                )
                                Spacer(modifier = Modifier.width(8.dp))
                                Text("ALL", style = MaterialTheme.typography.bodyLarge, fontWeight = FontWeight.Bold)
                            }

                            Spacer(modifier = Modifier.height(8.dp))

                            basicSupportedDevices.forEach { deviceType ->
                                Row(
                                    modifier = Modifier.fillMaxWidth(),
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Checkbox(
                                        checked = basicSelectedDevices.contains(deviceType),
                                        onCheckedChange = { checked ->
                                            if (checked) basicSelectedDevices.add(deviceType)
                                            else basicSelectedDevices.remove(deviceType)
                                        }
                                    )
                                    Text(deviceType, style = MaterialTheme.typography.bodyLarge)
                                }
                            }
                        } else {
                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                horizontalArrangement = Arrangement.SpaceBetween,
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                Text("1. Configure Simulator Devices:", style = MaterialTheme.typography.titleMedium)
                                Button(
                                    onClick = {
                                        val nextId = if (configuredEndpoints.isEmpty()) 1 else configuredEndpoints.maxOf { it.endpointId } + 1
                                        configuredEndpoints.add(EndpointConfig(nextId, supportedDevices.firstOrNull() ?: "chime"))
                                    }
                                ) {
                                    Text("Add Endpoint")
                                }
                            }
                            Spacer(modifier = Modifier.height(8.dp))

                            configuredEndpoints.forEachIndexed { index, config ->
                                Card(
                                    modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp),
                                    shape = RoundedCornerShape(8.dp),
                                    colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceVariant)
                                ) {
                                    Column(modifier = Modifier.padding(12.dp)) {
                                        Row(
                                            modifier = Modifier.fillMaxWidth(),
                                            horizontalArrangement = Arrangement.SpaceBetween,
                                            verticalAlignment = Alignment.CenterVertically
                                        ) {
                                            Text("Endpoint ID: ${config.endpointId}", style = MaterialTheme.typography.titleSmall)
                                            TextButton(onClick = { configuredEndpoints.removeAt(index) }) {
                                                Text("Remove", color = Color.Red, fontSize = 12.sp)
                                            }
                                        }
                                        Spacer(modifier = Modifier.height(4.dp))
                                        
                                        // Device Type Selection
                                        var deviceExpanded by remember { mutableStateOf(false) }
                                        Box {
                                            OutlinedButton(onClick = { deviceExpanded = true }, modifier = Modifier.fillMaxWidth()) {
                                                Text("Device Type: ${config.deviceType}")
                                            }
                                            DropdownMenu(expanded = deviceExpanded, onDismissRequest = { deviceExpanded = false }) {
                                                supportedDevices.forEach { type ->
                                                    DropdownMenuItem(
                                                        text = { Text(type) },
                                                        onClick = {
                                                            configuredEndpoints[index] = config.copy(deviceType = type)
                                                            deviceExpanded = false
                                                        }
                                                    )
                                                }
                                            }
                                        }
                                        Spacer(modifier = Modifier.height(4.dp))

                                        // Parent ID Selection
                                        var parentExpanded by remember { mutableStateOf(false) }
                                        val parentOptions = listOf(0) + configuredEndpoints.map { it.endpointId }.filter { it != config.endpointId }
                                        Box {
                                            OutlinedButton(onClick = { parentExpanded = true }, modifier = Modifier.fillMaxWidth()) {
                                                val parentLabel = if (config.parentId == 0) "None" else "Endpoint ${config.parentId}"
                                                Text("Parent Endpoint: $parentLabel")
                                            }
                                            DropdownMenu(expanded = parentExpanded, onDismissRequest = { parentExpanded = false }) {
                                                parentOptions.forEach { opt ->
                                                    val label = if (opt == 0) "None" else "Endpoint $opt"
                                                    DropdownMenuItem(
                                                        text = { Text(label) },
                                                        onClick = {
                                                            configuredEndpoints[index] = config.copy(parentId = opt)
                                                            parentExpanded = false
                                                        }
                                                    )
                                                }
                                            }
                                        }
                                        Spacer(modifier = Modifier.height(4.dp))

                                        // Node Label TextField
                                        var tempLabel by remember(config.nodeLabel) { mutableStateOf(config.nodeLabel) }
                                        OutlinedTextField(
                                            value = tempLabel,
                                            onValueChange = {
                                                tempLabel = it
                                                configuredEndpoints[index] = config.copy(nodeLabel = it)
                                            },
                                            label = { Text("Node Label (Optional)") },
                                            modifier = Modifier.fillMaxWidth(),
                                            singleLine = true
                                        )
                                        Spacer(modifier = Modifier.height(8.dp))

                                        Row(
                                            modifier = Modifier.fillMaxWidth(),
                                            verticalAlignment = Alignment.CenterVertically
                                        ) {
                                            Checkbox(
                                                checked = config.bridged,
                                                onCheckedChange = { checked ->
                                                    configuredEndpoints[index] = config.copy(bridged = checked)
                                                }
                                            )
                                            Spacer(modifier = Modifier.width(8.dp))
                                            Text("Bridged (Auto-inject Bridged Node parent)", style = MaterialTheme.typography.bodyMedium)
                                        }
                                    }
                                }
                            }
                        }

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
                                val serializedJson = if (selectedTab == 0) {
                                    val basicConfigs = mutableListOf<EndpointConfig>()
                                    if (isBridgeMode) {
                                        basicConfigs.add(EndpointConfig(1, "aggregator", parentId = 0, nodeLabel = "Aggregator", bridged = false))
                                        var nextEp = 2
                                        basicSelectedDevices.forEach { type ->
                                            basicConfigs.add(EndpointConfig(nextEp, type, parentId = 1, nodeLabel = "", bridged = true))
                                            nextEp += 2
                                        }
                                    } else {
                                        basicSelectedDevices.forEachIndexed { i, type ->
                                            basicConfigs.add(EndpointConfig(i + 1, type, parentId = 0, nodeLabel = "", bridged = false))
                                        }
                                    }
                                    serializeConfigs(basicConfigs)
                                } else {
                                    serializeConfigs(configuredEndpoints)
                                }
                                thread {
                                    val codes = startServer(disc, serializedJson) { line ->
                                        runOnUiThread {
                                            if (logLines.size > 150) {
                                                logLines.removeAt(0)
                                            }
                                            logLines.add(line)
                                        }
                                    }
                                    if (codes != null) {
                                        runOnUiThread {
                                            manualCode = codes.first
                                            qrCode = codes.second
                                            isRunning = true
                                        }
                                    }
                                }
                            },
                            modifier = Modifier.weight(1f),
                            enabled = if (selectedTab == 0) basicSelectedDevices.isNotEmpty() else configuredEndpoints.isNotEmpty()
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

                        TabRow(selectedTabIndex = runningSelectedTab) {
                            Tab(selected = runningSelectedTab == 0, onClick = { runningSelectedTab = 0 }) {
                                Text("Onboarding", modifier = Modifier.padding(vertical = 12.dp))
                            }
                            Tab(selected = runningSelectedTab == 1, onClick = { runningSelectedTab = 1 }) {
                                Text("Topology", modifier = Modifier.padding(vertical = 12.dp))
                            }
                            Tab(selected = runningSelectedTab == 2, onClick = { runningSelectedTab = 2 }) {
                                Text("Logs", modifier = Modifier.padding(vertical = 12.dp))
                            }
                        }
                        Spacer(modifier = Modifier.height(16.dp))

                        if (runningSelectedTab == 0) {
                            // Onboarding Tab
                            Spacer(modifier = Modifier.height(8.dp))
                            Card(
                                modifier = Modifier.fillMaxWidth(),
                                shape = RoundedCornerShape(12.dp)
                            ) {
                                Column(
                                    modifier = Modifier.padding(16.dp).fillMaxWidth(),
                                    horizontalAlignment = Alignment.CenterHorizontally
                                ) {
                                    val qrBitmap = generateQRCode(qrCode)
                                    if (qrBitmap != null) {
                                        Image(
                                            bitmap = qrBitmap.asImageBitmap(),
                                            contentDescription = "Matter QR Code",
                                            modifier = Modifier.size(200.dp)
                                        )
                                        Spacer(modifier = Modifier.height(12.dp))
                                    }
                                    Text("Passcode: 20202021", fontSize = 18.sp, fontWeight = FontWeight.Medium)
                                    Spacer(modifier = Modifier.height(4.dp))
                                    Text("Discriminator: $discriminatorText", fontSize = 18.sp)
                                    Spacer(modifier = Modifier.height(4.dp))
                                    Text("Manual Pairing Code: $manualCode", fontSize = 16.sp, fontFamily = FontFamily.Monospace)
                                }
                            }
                        }

                        if (runningSelectedTab == 1) {
                            // Topology Tab
                            val currentConfigs = if (selectedTab == 0) {
                                val basicConfigs = mutableListOf<EndpointConfig>()
                                if (isBridgeMode) {
                                    basicConfigs.add(EndpointConfig(1, "aggregator", parentId = 0, nodeLabel = "Aggregator", bridged = false))
                                    var nextEp = 2
                                    basicSelectedDevices.forEach { type ->
                                        basicConfigs.add(EndpointConfig(nextEp, type, parentId = 1, nodeLabel = "", bridged = true))
                                        nextEp += 2
                                    }
                                } else {
                                    basicSelectedDevices.forEachIndexed { i, type ->
                                        basicConfigs.add(EndpointConfig(i + 1, type, parentId = 0, nodeLabel = "", bridged = false))
                                    }
                                }
                                basicConfigs
                            } else {
                                configuredEndpoints
                            }

                            Column(modifier = Modifier.fillMaxWidth().verticalScroll(rememberScrollState())) {
                                DataModelTopologyPreview(configs = currentConfigs)
                            }
                        }

                        if (runningSelectedTab == 2) {
                            // Logs Tab
                            Box(
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .weight(1f)
                                    .background(Color.Black, shape = RoundedCornerShape(8.dp))
                                    .border(1.dp, Color.DarkGray, shape = RoundedCornerShape(8.dp))
                                    .padding(8.dp)
                            ) {
                                LazyColumn(
                                    state = lazyListState,
                                    modifier = Modifier.fillMaxSize()
                                ) {
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

    private fun serializeConfigs(configs: List<EndpointConfig>): String {
        val jsonArray = org.json.JSONArray()
        for (c in configs) {
            val jsonObject = org.json.JSONObject().apply {
                put("endpointId", c.endpointId)
                put("deviceType", c.deviceType)
                put("parentId", c.parentId)
                put("bridged", c.bridged)
                put("nodeLabel", c.nodeLabel)
            }
            jsonArray.put(jsonObject)
        }
        return jsonArray.toString()
    }
}

data class TreeNode(
    val label: String,
    val endpointId: Int,
    val type: String,
    val children: MutableList<TreeNode> = mutableListOf()
)

private fun buildTopologyTree(configs: List<EndpointConfig>): TreeNode {
    val root = TreeNode("Root Node", 0, "root")
    
    // Expand bridged configurations for the preview tree
    val expanded = mutableListOf<TreeNode>()
    configs.forEach { c ->
        if (c.bridged && c.deviceType != "bridged-node") {
            val bridgeNode = TreeNode(
                label = if (c.nodeLabel.isNotEmpty()) c.nodeLabel else "Bridged Node",
                endpointId = c.endpointId,
                type = "bridged-node"
            )
            bridgeNode.children.add(TreeNode(
                label = c.deviceType,
                endpointId = c.endpointId + 1,
                type = c.deviceType
            ))
            expanded.add(bridgeNode)
        } else {
            expanded.add(TreeNode(
                label = if (c.nodeLabel.isNotEmpty()) c.nodeLabel else c.deviceType,
                endpointId = c.endpointId,
                type = c.deviceType
            ))
        }
    }

    val nodeMap = mutableMapOf<Int, TreeNode>()
    nodeMap[0] = root
    expanded.forEach { nodeMap[it.endpointId] = it }

    expanded.forEach { node ->
        val origConfig = configs.find { it.endpointId == node.endpointId }
        val parentId = origConfig?.parentId ?: 0
        val parentNode = nodeMap[parentId] ?: root
        if (parentNode != node) {
            parentNode.children.add(node)
        }
    }
    
    return root
}

@Composable
fun DataModelTopologyPreview(configs: List<EndpointConfig>) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp),
        shape = RoundedCornerShape(12.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)),
        border = BorderStroke(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.2f))
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            val tree = buildTopologyTree(configs)
            RenderTreeNode(node = tree, level = 0)
        }
    }
}

@Composable
fun RenderTreeNode(node: TreeNode, level: Int) {
    Column(modifier = Modifier.padding(start = if (level > 0) 16.dp else 0.dp)) {
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.padding(vertical = 4.dp)
        ) {
            if (level > 0) {
                Text("└── ", style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.onSurfaceVariant)
            }
            
            val containerColor = when (node.type) {
                "root" -> MaterialTheme.colorScheme.primaryContainer
                "aggregator" -> MaterialTheme.colorScheme.secondaryContainer
                "bridged-node" -> MaterialTheme.colorScheme.tertiaryContainer
                else -> MaterialTheme.colorScheme.surface
            }
            val textColor = when (node.type) {
                "root" -> MaterialTheme.colorScheme.onPrimaryContainer
                "aggregator" -> MaterialTheme.colorScheme.onSecondaryContainer
                "bridged-node" -> MaterialTheme.colorScheme.onTertiaryContainer
                else -> MaterialTheme.colorScheme.onSurface
            }

            Surface(
                color = containerColor,
                contentColor = textColor,
                shape = RoundedCornerShape(4.dp),
                border = BorderStroke(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.12f)),
                modifier = Modifier.padding(end = 4.dp)
            ) {
                Text(
                    text = " EP ${node.endpointId} ",
                    style = MaterialTheme.typography.labelSmall,
                    modifier = Modifier.padding(horizontal = 4.dp, vertical = 2.dp)
                )
            }

            Text(
                text = node.label,
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = if (node.type == "root" || node.type == "aggregator") FontWeight.Bold else FontWeight.Normal
            )
        }
        
        node.children.forEach { child ->
            RenderTreeNode(node = child, level = level + 1)
        }
    }
}

