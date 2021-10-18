package com.google.chip.chiptool.provisioning

import android.os.Bundle
import android.os.Handler
import android.os.Looper
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

import chip.platform.NsdManagerServiceDiscover;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.ServiceResolver;
import chip.platform.ChipMdnsCallback;

class AddressCommissioningFragment : Fragment() {
  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View? {
    return inflater.inflate(R.layout.address_commissioning_fragment, container, false)
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    serviceDiscover = NsdManagerServiceDiscover(requireContext())
    serviceResolver = NsdManagerServiceResolver(requireContext())

    updateSpinnerHandler = Handler(Looper.getMainLooper()) {
      updateSpinner()
      true
    }

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

    discoverBtn.setOnCheckedChangeListener { _, isChecked ->
      if (!isChecked) {
        serviceDiscover.stopDiscover(0, 0)
        commissionBtn.isEnabled = true
        return@setOnCheckedChangeListener
      }
      commissionBtn.isEnabled = false
      ipAddressList.clear()
      updateSpinnerHandler.sendEmptyMessage(0)
      serviceDiscover.startDiscover(SERVICE_TYPE, 0, 0, discoverCallback)
    }
  }

  private fun updateSpinner() {
    requireActivity().runOnUiThread {
      discoverListSpinner.adapter =
        ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, ipAddressList)
      discoverListSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
        override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
          val address = ipAddressList[position].split(",")[0].trim()
          val discriminator = ipAddressList[position].split(",")[1].trim()
          addressEditText.setText(address)
          discriminatorEditText.setText(discriminator)
        }

        override fun onNothingSelected(parent: AdapterView<*>?) {}
      }
    }
  }

  companion object {
    private const val TAG = "AddressCommissioningFragment"
    private const val SERVICE_TYPE = "_matterc._udp"
    private lateinit var serviceDiscover : NsdManagerServiceDiscover
    private lateinit var serviceResolver : ServiceResolver
    private val ipAddressList = ArrayList<String>()
    private lateinit var updateSpinnerHandler: Handler

    private val mdnsCallback = object: ChipMdnsCallback {
      override fun handleServiceResolve(
              instanceName: String?,
              serviceType: String?,
              address: String?,
              port: Int,
              attributes: MutableMap<String, ByteArray>?,
              callbackHandle: Long,
              contextHandle: Long,
              errorCode: Int
      ) {
        if (errorCode != 0) {
          return
        }

        if (attributes != null) {
          val value = String(attributes["CM"] ?: return)
          val discriminator = String(attributes["D"] ?: return)
          if (value == "1") {
            ipAddressList.add("${address}, $discriminator")
            updateSpinnerHandler.sendEmptyMessage(0)
          }
        }
      }
    }

    private val discoverCallback = object: NsdManagerServiceDiscover.Callback {
      override fun handleServiceDiscover(
        instanceName: String?,
        serviceType: String?,
        callbackHandle: Long,
        contextHandle: Long,
        errorCode: Int
      ) {
       if (errorCode != 0) {
         return
       }
        serviceResolver.resolve(instanceName, serviceType, 0, 0, mdnsCallback)
      }
    }
    fun newInstance(): AddressCommissioningFragment = AddressCommissioningFragment()
  }
}