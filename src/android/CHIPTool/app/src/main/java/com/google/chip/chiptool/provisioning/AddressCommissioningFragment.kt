package com.google.chip.chiptool.provisioning

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.wifi.WifiManager
import android.net.wifi.ScanResult
import android.net.wifi.WifiConfiguration
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.util.FragmentUtil
import kotlinx.android.synthetic.main.address_commissioning_fragment.addressEditText
import kotlinx.android.synthetic.main.address_commissioning_fragment.commissionBtn
import kotlinx.android.synthetic.main.address_commissioning_fragment.discoverBtn
import kotlinx.android.synthetic.main.address_commissioning_fragment.discoverListSpinner
import kotlinx.android.synthetic.main.address_commissioning_fragment.discriminatorEditText
import kotlinx.android.synthetic.main.address_commissioning_fragment.pincodeEditText
import kotlinx.android.synthetic.main.address_commissioning_fragment.wifiConnectBtn
import kotlinx.android.synthetic.main.address_commissioning_fragment.wifiScanBtn
import kotlinx.android.synthetic.main.address_commissioning_fragment.wifiScanListSpinner
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

class AddressCommissioningFragment : Fragment() {
  private val ipAddressList = ArrayList<String>()
  private val wifiApList = ArrayList<String>()
  private var wifiApSsid = String()
  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View? {
    return inflater.inflate(R.layout.address_commissioning_fragment, container, false)
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    commissionBtn.setOnClickListener {
      val address = addressEditText.text.toString()
      val discriminator = discriminatorEditText.text.toString()
      val pincode = pincodeEditText.text.toString()

      if (address.isEmpty() || discriminator.isEmpty() || pincode.isEmpty()) {
        Log.e(TAG, "Address, discriminator, or pincode was empty: $address $discriminator $pincode")
        return@setOnClickListener
      }

      FragmentUtil.getHost(this, BarcodeFragment.Callback::class.java)?.onCHIPDeviceInfoReceived(
        CHIPDeviceInfo(
          discriminator = discriminator.toInt(),
          setupPinCode = pincode.toLong(),
          ipAddress = address
        )
      )
    }

    discoverBtn.setOnClickListener { _ ->
      discoverBtn.isEnabled = false
      val deviceController = ChipClient.getDeviceController(requireContext())
      deviceController.discoverCommissionableNodes()
      scope.launch {
        delay(7000)
        updateSpinner()
        discoverBtn.isEnabled = true
      }
    }

    wifiConnectBtn.setOnClickListener { _ ->
      wifiConnectBtn.isEnabled = false
      val context = getActivity()
      val wifiManager = context?.getSystemService(Context.WIFI_SERVICE) as WifiManager

      // TODO : filter SSID with Information Element, OPEN authentication
      var config : WifiConfiguration = WifiConfiguration();
      config.SSID = "\"${wifiApSsid}\""
      config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE)

      Log.d(TAG, "Disconnect existing connection")
      wifiManager.disconnect()
      Log.d(TAG, "Add network ${config.SSID}")
      var res = wifiManager.addNetwork(config)
      if (res == -1) {
        Log.d(TAG,"Add network failed")
      } else {
        var success = wifiManager.enableNetwork(res, true)
        if (success) {
          Log.d(TAG, "Enable network ${config.SSID} succeeded")
        } else {
          Log.d(TAG, "Enable network ${config.SSID} failed")
          wifiConnectBtn.isEnabled = true
        }
      }
    }

    wifiScanBtn.setOnClickListener { _ ->
      wifiScanBtn.isEnabled = false
      val context = getActivity()
      val wifiManager = context?.getSystemService(Context.WIFI_SERVICE) as WifiManager
      val wifiScanReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
          Log.d(TAG, "Scan result event received")
          val success = intent.getBooleanExtra(WifiManager.EXTRA_RESULTS_UPDATED, false)
          if (success) {
            Log.d(TAG, "Scan succeeded")
            val results = wifiManager.scanResults
            updateWifiScanListSpinner(results)
          } else {
            Log.d(TAG, "Scan failed")
          }
          wifiScanBtn.isEnabled = true
        }
      }

      val intentFilter = IntentFilter()
      intentFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)

      context.registerReceiver(wifiScanReceiver, intentFilter)
      val success = wifiManager.startScan()
      if (!success) {
        Log.d(TAG, "Scan not started")
       // TODO: scan failure handling
      } else {
        Log.d(TAG, "Scan started")
      }
    }
  }

  private fun updateWifiScanListSpinner(scanResults : MutableList<ScanResult>) {
    wifiApList.clear()
    for (result in scanResults) {
      if (result.SSID.toString().isNotEmpty())
        wifiApList.add("${result.SSID}, ${result.BSSID}, ${result.level}")
    }
    requireActivity().runOnUiThread {
      wifiScanListSpinner.adapter =
        ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, wifiApList)
      wifiScanListSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
        override fun onItemSelected(parent: AdapterView<*>, view: View, position: Int, id: Long) {
          wifiApSsid = wifiApList[position].split(",")[0].trim()
          Log.d(TAG, "ready to connect to $wifiApSsid")
        }
        override fun onNothingSelected(parent: AdapterView<*>) {}
      }
    }
  }

  private fun updateSpinner() {
    val deviceController = ChipClient.getDeviceController(requireContext())
    for(i in 0..10) {
      val device = deviceController.getDiscoveredDevice(i) ?: break
      ipAddressList.add("${device.ipAddress}, ${device.discriminator}")
    }
    requireActivity().runOnUiThread {
      discoverListSpinner.adapter =
        ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, ipAddressList)
      discoverListSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
        override fun onItemSelected(parent: AdapterView<*>, view: View, position: Int, id: Long) {
          val address = ipAddressList[position].split(",")[0].trim()
          val discriminator = ipAddressList[position].split(",")[1].trim()
          addressEditText.setText(address)
          discriminatorEditText.setText(discriminator)
        }

        override fun onNothingSelected(parent: AdapterView<*>) {}
      }
    }
  }

  companion object {
    private const val TAG = "AddressCommissioningFragment"

    fun newInstance(): AddressCommissioningFragment = AddressCommissioningFragment()
  }
}
