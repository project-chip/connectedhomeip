package com.google.chip.chiptool.provisioning

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.enter_wifi_network_fragment.*
import kotlinx.android.synthetic.main.enter_wifi_network_fragment.view.*

class EnterWifiNetworkFragment : Fragment() {

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.enter_wifi_network_fragment, container, false).apply {
      saveNetworkBtn.setOnClickListener { onSaveNetworkClicked() }
    }
  }

  private fun onSaveNetworkClicked() {
    val ssid = ssidEd.text
    val pwd = pwdEd.text

    if (ssid.isNullOrBlank() || pwd.isNullOrBlank()) {
      Toast.makeText(requireContext(), "Ssid and password required.", Toast.LENGTH_SHORT).show()
      return
    }

    ChipClient.getDeviceController().apply {
      sendWiFiCredentials(ssid.toString(), pwd.toString())
    }
  }

  companion object {
    fun newInstance() = EnterWifiNetworkFragment()
  }

}