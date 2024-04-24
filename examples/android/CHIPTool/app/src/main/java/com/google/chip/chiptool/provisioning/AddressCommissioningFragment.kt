package com.google.chip.chiptool.provisioning

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.wifi.ScanResult
import android.net.wifi.WifiConfiguration
import android.net.wifi.WifiManager
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.databinding.AddressCommissioningFragmentBinding
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.util.FragmentUtil
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
  private var _binding: AddressCommissioningFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = AddressCommissioningFragmentBinding.inflate(inflater, container, false)
    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    binding.commissionBtn.setOnClickListener {
      val address = binding.addressEditText.text.toString()
      val discriminator = binding.discriminatorEditText.text.toString()
      val pincode = binding.pincodeEditText.text.toString()
      val port = binding.portEditText.text.toString()

      if (address.isEmpty() || discriminator.isEmpty() || pincode.isEmpty()) {
        Log.e(TAG, "Address, discriminator, or pincode was empty: $address $discriminator $pincode")
        return@setOnClickListener
      }

      FragmentUtil.getHost(this, BarcodeFragment.Callback::class.java)
        ?.onCHIPDeviceInfoReceived(
          CHIPDeviceInfo(
            discriminator = discriminator.toInt(),
            setupPinCode = pincode.toLong(),
            ipAddress = address,
            port = port.toInt()
          )
        )
    }

    binding.discoverBtn.setOnClickListener { _ ->
      binding.discoverBtn.isEnabled = false
      val deviceController = ChipClient.getDeviceController(requireContext())
      deviceController.discoverCommissionableNodes()
      scope.launch {
        delay(7000)
        updateSpinner()
        binding.discoverBtn.isEnabled = true
      }
    }

    binding.wifiConnectBtn.setOnClickListener { _ ->
      binding.wifiConnectBtn.isEnabled = false
      val context = getActivity()
      val wifiManager = context?.getSystemService(Context.WIFI_SERVICE) as WifiManager

      // TODO : filter SSID with Information Element, OPEN authentication
      var config: WifiConfiguration = WifiConfiguration()
      config.SSID = "\"${wifiApSsid}\""
      config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE)

      Log.d(TAG, "Disconnect existing connection")
      wifiManager.disconnect()
      Log.d(TAG, "Add network ${config.SSID}")
      var res = wifiManager.addNetwork(config)
      if (res == -1) {
        Log.d(TAG, "Add network failed")
      } else {
        var success = wifiManager.enableNetwork(res, true)
        if (success) {
          Log.d(TAG, "Enable network ${config.SSID} succeeded")
        } else {
          Log.d(TAG, "Enable network ${config.SSID} failed")
          binding.wifiConnectBtn.isEnabled = true
        }
      }
    }

    binding.wifiScanBtn.setOnClickListener { _ ->
      binding.wifiScanBtn.isEnabled = false
      val context = getActivity()
      val wifiManager = context?.getSystemService(Context.WIFI_SERVICE) as WifiManager
      val wifiScanReceiver =
        object : BroadcastReceiver() {
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
            binding.wifiScanBtn.isEnabled = true
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

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private fun updateWifiScanListSpinner(scanResults: MutableList<ScanResult>) {
    wifiApList.clear()
    for (result in scanResults) {
      if (result.SSID.toString().isNotEmpty())
        wifiApList.add("${result.SSID}, ${result.BSSID}, ${result.level}")
    }
    requireActivity().runOnUiThread {
      binding.wifiScanListSpinner.adapter =
        ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, wifiApList)
      binding.wifiScanListSpinner.onItemSelectedListener =
        object : AdapterView.OnItemSelectedListener {
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
    for (i in 0..10) {
      val device = deviceController.getDiscoveredDevice(i) ?: break
      ipAddressList.add("${device.ipAddress}, ${device.discriminator}, ${device.port}")
    }
    requireActivity().runOnUiThread {
      binding.discoverListSpinner.adapter =
        ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, ipAddressList)
      binding.discoverListSpinner.onItemSelectedListener =
        object : AdapterView.OnItemSelectedListener {
          override fun onItemSelected(parent: AdapterView<*>, view: View, position: Int, id: Long) {
            val address = ipAddressList[position].split(",")[0].trim()
            val discriminator = ipAddressList[position].split(",")[1].trim()
            val port = ipAddressList[position].split(",")[2].trim()

            binding.addressEditText.setText(address)
            binding.discriminatorEditText.setText(discriminator)
            binding.portEditText.setText(port)
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
