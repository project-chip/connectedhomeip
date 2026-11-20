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
package com.google.chip.chiptool.provisioning

import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.NetworkCredentialsParcelable
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.FragmentUtil
import java.lang.reflect.InvocationHandler
import java.lang.reflect.InvocationTargetException
import java.lang.reflect.Proxy
import java.util.concurrent.Executor

/**
 * Fragment to collect Wi-Fi network information from user and send it to device being provisioned.
 */
class EnterNetworkFragment : Fragment() {
  /** Unregisters the pending ThreadNetworkController dataset callback, if any. */
  private var unregisterDatasetCallback: (() -> Unit)? = null

  private val networkType: ProvisionNetworkType
    get() =
      requireNotNull(
        ProvisionNetworkType.fromName(arguments?.getString(ARG_PROVISION_NETWORK_TYPE))
      )

  interface Callback {
    fun onNetworkCredentialsEntered(networkCredentials: NetworkCredentialsParcelable)
  }

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?,
  ): View? {
    val layoutRes =
      when (networkType) {
        ProvisionNetworkType.WIFI -> R.layout.enter_wifi_network_fragment
        ProvisionNetworkType.THREAD -> R.layout.enter_thread_network_fragment
      }

    return inflater.inflate(layoutRes, container, false).apply {
      val fragmentView = this
      val saveNetworkBtn: Button = findViewById(R.id.saveNetworkBtn)
      saveNetworkBtn.setOnClickListener { onSaveNetworkClicked(fragmentView) }
      if (networkType == ProvisionNetworkType.THREAD) {
        findViewById<Button>(R.id.fetchThreadDatasetBtn)?.apply {
          if (!GmsThreadDatasetHelper.IS_AVAILABLE && Build.VERSION.SDK_INT < 34) {
            visibility = View.GONE
          } else {
            setOnClickListener { fetchThreadDatasetFromSystem() }
          }
        }
      }
    }
  }

  private fun onSaveNetworkClicked(view: View) {
    if (networkType == ProvisionNetworkType.WIFI) {
      saveWiFiNetwork(view)
    } else {
      saveThreadNetwork(view)
    }
  }

  private fun saveWiFiNetwork(view: View) {
    val ssidEd: EditText = view.findViewById(R.id.ssidEd)
    val pwdEd: EditText = view.findViewById(R.id.pwdEd)
    val ssid = ssidEd?.text
    val pwd = pwdEd?.text

    if (ssid.isNullOrBlank() || pwd.isNullOrBlank()) {
      Toast.makeText(requireContext(), "Ssid and password required.", Toast.LENGTH_SHORT).show()
      return
    }

    val networkCredentials =
      NetworkCredentialsParcelable.forWiFi(
        NetworkCredentialsParcelable.WiFiCredentials(ssid.toString(), pwd.toString())
      )
    FragmentUtil.getHost(this, Callback::class.java)
      ?.onNetworkCredentialsEntered(networkCredentials)
  }

  private fun saveThreadNetwork(view: View) {
    val channelEd: EditText = view.findViewById(R.id.channelEd)
    val panIdEd: EditText = view.findViewById(R.id.panIdEd)
    val xpanIdEd: EditText = view.findViewById(R.id.xpanIdEd)
    val masterKeyEd: EditText = view.findViewById(R.id.masterKeyEd)
    val channelStr = channelEd.text
    val panIdStr = panIdEd.text

    if (channelStr.isNullOrBlank()) {
      Toast.makeText(requireContext(), "Channel is empty", Toast.LENGTH_SHORT).show()
      return
    }

    if (panIdStr.isNullOrBlank()) {
      Toast.makeText(requireContext(), "PAN ID is empty", Toast.LENGTH_SHORT).show()
      return
    }

    if (xpanIdEd.text.isNullOrBlank()) {
      Toast.makeText(requireContext(), "XPAN ID is empty", Toast.LENGTH_SHORT).show()
      return
    }

    val xpanIdStr = xpanIdEd.text.toString().filterNot { c -> c == ':' }
    if (xpanIdStr.length != NUM_XPANID_BYTES * 2) {
      Toast.makeText(requireContext(), "Extended PAN ID is invalid", Toast.LENGTH_SHORT).show()
      return
    }

    if (masterKeyEd.text.isNullOrBlank()) {
      Toast.makeText(requireContext(), "Master Key is empty", Toast.LENGTH_SHORT).show()
      return
    }

    val masterKeyStr = masterKeyEd.text.toString().filterNot { c -> c == ':' }
    if (masterKeyStr.length != NUM_MASTER_KEY_BYTES * 2) {
      Toast.makeText(requireContext(), "Master key is invalid", Toast.LENGTH_SHORT).show()
      return
    }

    val operationalDataset =
      makeThreadOperationalDataset(
        channelStr.toString().toInt(),
        panIdStr.toString().toInt(16),
        xpanIdStr.hexToByteArray(),
        masterKeyStr.hexToByteArray(),
      )

    val networkCredentials =
      NetworkCredentialsParcelable.forThread(
        NetworkCredentialsParcelable.ThreadCredentials(operationalDataset)
      )
    FragmentUtil.getHost(this, Callback::class.java)
      ?.onNetworkCredentialsEntered(networkCredentials)
  }

  private fun makeThreadOperationalDataset(
    channel: Int,
    panId: Int,
    xpanId: ByteArray,
    masterKey: ByteArray,
  ): ByteArray {
    // channel
    var dataset = byteArrayOf(TYPE_CHANNEL.toByte(), NUM_CHANNEL_BYTES.toByte())
    dataset += 0x00.toByte() // Channel Page 0.
    dataset += (channel.shr(8) and 0xFF).toByte()
    dataset += (channel and 0xFF).toByte()

    // PAN ID
    dataset += TYPE_PANID.toByte()
    dataset += NUM_PANID_BYTES.toByte()
    dataset += (panId.shr(8) and 0xFF).toByte()
    dataset += (panId and 0xFF).toByte()

    // Extended PAN ID
    dataset += TYPE_XPANID.toByte()
    dataset += NUM_XPANID_BYTES.toByte()
    dataset += xpanId

    // Network Master Key
    dataset += TYPE_MASTER_KEY.toByte()
    dataset += NUM_MASTER_KEY_BYTES.toByte()
    dataset += masterKey

    return dataset
  }

  private fun String.hexToByteArray(): ByteArray {
    return chunked(2).map { byteStr -> byteStr.toUByte(16).toByte() }.toByteArray()
  }

  override fun onDestroyView() {
    unregisterDatasetCallback?.invoke()
    unregisterDatasetCallback = null
    super.onDestroyView()
  }

  private fun fetchThreadDatasetFromSystem() {
    if (!GmsThreadDatasetHelper.IS_AVAILABLE) {
      fetchThreadDatasetFromSystemService()
      return
    }

    GmsThreadDatasetHelper.fetchFromGms(
      fragment = this,
      onFallback = { if (view != null) fetchThreadDatasetFromSystemService() },
    )
  }

  @Suppress("DEPRECATION")
  override fun onActivityResult(requestCode: Int, resultCode: Int, data: android.content.Intent?) {
    super.onActivityResult(requestCode, resultCode, data)
    val candidates =
      GmsThreadDatasetHelper.parseConsentResult(requestCode, resultCode, data) ?: return
    if (view == null) {
      return
    }
    when (candidates.size) {
      0 ->
        Toast.makeText(
            requireContext(),
            R.string.enter_thread_no_network_shared,
            Toast.LENGTH_SHORT,
          )
          .show()
      1 -> onThreadDatasetFetched(candidates[0].activeDatasetTlvs)
      else -> showThreadNetworkPicker(candidates)
    }
  }

  private fun showThreadNetworkPicker(candidates: List<ThreadDatasetCandidate>) {
    val labels =
      candidates
        .map {
          getString(
            R.string.enter_thread_network_choice,
            it.networkName,
            it.extendedPanId.toFormattedHexString(),
            it.channel,
          )
        }
        .toTypedArray()
    AlertDialog.Builder(requireContext())
      .setTitle(R.string.enter_thread_pick_network_title)
      .setItems(labels) { _, which -> onThreadDatasetFetched(candidates[which].activeDatasetTlvs) }
      .setNegativeButton(android.R.string.cancel, null)
      .show()
  }

  /** Populates the Thread fields, ignoring results that arrive after the view is destroyed. */
  private fun onThreadDatasetFetched(tlvs: ByteArray) {
    val fragmentView = view ?: return
    populateThreadFieldsFromTlvs(fragmentView, tlvs)
    Toast.makeText(requireContext(), R.string.enter_thread_fetch_success, Toast.LENGTH_SHORT).show()
  }

  /**
   * Reads the active dataset from the Android 14+ ThreadNetworkManager system service.
   *
   * The ThreadNetworkController APIs are system APIs guarded by THREAD_NETWORK_PRIVILEGED, so this
   * path only works when CHIPTool is installed as a privileged or platform-signed app.
   */
  private fun fetchThreadDatasetFromSystemService() {
    val context = requireContext()
    if (Build.VERSION.SDK_INT < 34) {
      Toast.makeText(context, R.string.enter_thread_requires_api_34, Toast.LENGTH_SHORT).show()
      return
    }

    if (
      ContextCompat.checkSelfPermission(context, THREAD_NETWORK_PRIVILEGED_PERMISSION) !=
        PackageManager.PERMISSION_GRANTED
    ) {
      Toast.makeText(
          context,
          R.string.enter_thread_privileged_permission_required,
          Toast.LENGTH_LONG,
        )
        .show()
      return
    }

    val threadNetworkManager = context.getSystemService(THREAD_NETWORK_SERVICE)
    if (threadNetworkManager == null) {
      Toast.makeText(context, R.string.enter_thread_manager_unavailable, Toast.LENGTH_SHORT).show()
      return
    }

    try {
      val controllers =
        threadNetworkManager.javaClass
          .getMethod("getAllThreadNetworkControllers")
          .invoke(threadNetworkManager) as? List<*>
      val controller = controllers?.firstOrNull()
      if (controller == null) {
        Toast.makeText(context, R.string.enter_thread_no_controller, Toast.LENGTH_SHORT).show()
        return
      }

      val callbackClass =
        Class.forName("android.net.thread.ThreadNetworkController\$OperationalDatasetCallback")
      val unregisterMethod =
        controller.javaClass.getMethod("unregisterOperationalDatasetCallback", callbackClass)
      lateinit var proxyCallback: Any
      val invocationHandler = InvocationHandler { proxy, method, args ->
        when (method.name) {
          "hashCode" -> System.identityHashCode(proxy)
          "equals" -> proxy === args?.getOrNull(0)
          "toString" ->
            "OperationalDatasetCallbackProxy@${Integer.toHexString(System.identityHashCode(proxy))}"
          "onActiveOperationalDatasetChanged" -> {
            onSystemActiveDatasetChanged(args?.getOrNull(0))
            null
          }
          else -> null
        }
      }
      proxyCallback =
        Proxy.newProxyInstance(callbackClass.classLoader, arrayOf(callbackClass), invocationHandler)

      // Drop a registration still pending from a previous tap before registering a new one.
      unregisterDatasetCallback?.invoke()
      controller.javaClass
        .getMethod("registerOperationalDatasetCallback", Executor::class.java, callbackClass)
        .invoke(controller, ContextCompat.getMainExecutor(context), proxyCallback)
      unregisterDatasetCallback = {
        try {
          unregisterMethod.invoke(controller, proxyCallback)
        } catch (e: Exception) {
          Log.e(TAG, "Failed to unregister OperationalDatasetCallback", e)
        }
      }
    } catch (e: Exception) {
      showFetchError(e)
    }
  }

  /** Runs on the main executor; the callback is one-shot and is unregistered on first delivery. */
  private fun onSystemActiveDatasetChanged(activeDataset: Any?) {
    unregisterDatasetCallback?.invoke()
    unregisterDatasetCallback = null
    if (view == null) {
      return
    }

    if (activeDataset == null) {
      Toast.makeText(requireContext(), R.string.enter_thread_no_active_dataset, Toast.LENGTH_SHORT)
        .show()
      return
    }

    try {
      val tlvs =
        activeDataset.javaClass.getMethod("toThreadTlvs").invoke(activeDataset) as ByteArray
      onThreadDatasetFetched(tlvs)
    } catch (e: Exception) {
      showFetchError(e)
    }
  }

  private fun showFetchError(e: Exception) {
    // Reflection wraps the real failure (e.g. SecurityException) in InvocationTargetException.
    val cause = (e as? InvocationTargetException)?.targetException ?: e
    Log.e(TAG, "Failed to fetch Thread dataset from ThreadNetworkManager", cause)
    Toast.makeText(
        requireContext(),
        getString(R.string.enter_thread_fetch_error, cause.message ?: cause.javaClass.simpleName),
        Toast.LENGTH_SHORT,
      )
      .show()
  }

  private fun populateThreadFieldsFromTlvs(view: View, tlvs: ByteArray) {
    var channel: Int? = null
    var panId: Int? = null
    var xpanId: ByteArray? = null
    var masterKey: ByteArray? = null

    var i = 0
    while (i + 1 < tlvs.size) {
      val type = tlvs[i].toInt() and 0xFF
      val length = tlvs[i + 1].toInt() and 0xFF
      if (i + 2 + length > tlvs.size) {
        break
      }
      val value = tlvs.copyOfRange(i + 2, i + 2 + length)
      when (type) {
        TYPE_CHANNEL -> {
          if (value.size >= 3) {
            channel = ((value[1].toInt() and 0xFF) shl 8) or (value[2].toInt() and 0xFF)
          }
        }
        TYPE_PANID -> {
          if (value.size >= 2) {
            panId = ((value[0].toInt() and 0xFF) shl 8) or (value[1].toInt() and 0xFF)
          }
        }
        TYPE_XPANID -> {
          if (value.size == NUM_XPANID_BYTES) {
            xpanId = value
          }
        }
        TYPE_MASTER_KEY -> {
          if (value.size == NUM_MASTER_KEY_BYTES) {
            masterKey = value
          }
        }
      }
      i += 2 + length
    }

    channel?.let { view.findViewById<EditText>(R.id.channelEd).setText(it.toString()) }
    panId?.let {
      val hexPanId = String.format("%04X", it)
      view.findViewById<EditText>(R.id.panIdEd).setText(hexPanId)
    }
    xpanId?.let {
      val hexXpanId = it.toFormattedHexString()
      view.findViewById<EditText>(R.id.xpanIdEd).setText(hexXpanId)
    }
    masterKey?.let {
      val hexMasterKey = it.toFormattedHexString()
      view.findViewById<EditText>(R.id.masterKeyEd).setText(hexMasterKey)
    }
  }

  private fun ByteArray.toFormattedHexString(): String {
    return joinToString(":") { "%02X".format(it) }
  }

  companion object {
    private const val TAG = "EnterNetworkFragment"
    private const val THREAD_NETWORK_SERVICE = "thread_network"
    private const val THREAD_NETWORK_PRIVILEGED_PERMISSION =
      "android.permission.THREAD_NETWORK_PRIVILEGED"
    private const val ARG_PROVISION_NETWORK_TYPE = "provision_network_type"
    private const val NETWORK_COMMISSIONING_CLUSTER_ENDPOINT = 0

    private const val NUM_CHANNEL_BYTES = 3
    private const val NUM_PANID_BYTES = 2
    private const val NUM_XPANID_BYTES = 8
    private const val NUM_MASTER_KEY_BYTES = 16
    private const val TYPE_CHANNEL = 0 // Type of Thread Channel TLV.
    private const val TYPE_PANID = 1 // Type of Thread PAN ID TLV.
    private const val TYPE_XPANID = 2 // Type of Thread Extended PAN ID TLV.
    private const val TYPE_MASTER_KEY = 5 // Type of Thread Network Master Key TLV.

    fun newInstance(provisionNetworkType: ProvisionNetworkType): EnterNetworkFragment {
      return EnterNetworkFragment().apply {
        arguments =
          Bundle(1).apply { putString(ARG_PROVISION_NETWORK_TYPE, provisionNetworkType.name) }
      }
    }
  }
}
