package com.google.chip.chiptool.provisioning

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.enter_wifi_network_fragment.view.*
import kotlinx.android.synthetic.main.enter_thread_network_fragment.*

class EnterThreadNetworkFragment : Fragment() {

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?): View {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.enter_thread_network_fragment, container, false).apply {
      saveNetworkBtn.setOnClickListener { onSaveNetworkClicked() }
    }
  }

  private fun onSaveNetworkClicked() {
    val channelStr = channelEd.text.toString()
    val panIdStr = panidEd.text.toString()
    val xpanIdStr = xpanidEd.text.toString().filterNot { c -> c == ':' }
    val masterKeyStr = masterKeyEd.text.toString().filterNot { c -> c == ':' }

    if (channelStr.isEmpty()) {
      Toast.makeText(requireContext(), "Channel is empty", Toast.LENGTH_SHORT).show()
      return
    }

    if (panIdStr.isEmpty()) {
      Toast.makeText(requireContext(), "PAN ID is empty", Toast.LENGTH_SHORT).show()
      return
    }

    if (xpanIdStr.length != NUM_XPANID_BYTES * 2) {
      Toast.makeText(requireContext(), "Extended PAN ID is invalid", Toast.LENGTH_SHORT).show()
      return
    }

    if (masterKeyStr.length != NUM_MASTER_KEY_BYTES * 2) {
      Toast.makeText(requireContext(), "Master key is invalid", Toast.LENGTH_SHORT).show()
      return
    }

    ChipClient.getDeviceController().sendThreadCredentials(
        channelStr.toInt(),
        panIdStr.toInt(16),
        xpanIdStr.hexToByteArray(),
        masterKeyStr.hexToByteArray())

    closeNetworkProvisioningFragment()
  }

  private fun closeNetworkProvisioningFragment() {
    requireActivity().supportFragmentManager.popBackStack()
  }

  companion object {
    private const val NUM_XPANID_BYTES = 8
    private const val NUM_MASTER_KEY_BYTES = 16

    private fun String.hexToByteArray(): ByteArray {
      return chunked(2).map{ byteStr -> byteStr.toUByte(16).toByte()}.toByteArray()
    }

    fun newInstance() = EnterThreadNetworkFragment()
  }
}