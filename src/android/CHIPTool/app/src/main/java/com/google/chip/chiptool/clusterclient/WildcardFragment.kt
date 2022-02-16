package com.google.chip.chiptool.clusterclient

import android.app.AlertDialog
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import androidx.appcompat.widget.MenuItemHoverListener
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipPathId
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.wildcard_fragment.attributeIdEd
import kotlinx.android.synthetic.main.wildcard_fragment.clusterIdEd
import kotlinx.android.synthetic.main.wildcard_fragment.endpointIdEd
import kotlinx.android.synthetic.main.wildcard_fragment.outputTv
import kotlinx.android.synthetic.main.wildcard_fragment.view.readBtn
import kotlinx.android.synthetic.main.wildcard_fragment.view.subscribeBtn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class WildcardFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private val reportCallback = object : ReportCallback {
    override fun onError(attributePath: ChipAttributePath, ex: Exception) {
      Log.i(TAG, "Report error for $attributePath: $ex")
    }

    override fun onReport(values: Map<ChipAttributePath, Any?>) {
      Log.i(TAG, "Received report with ${values.size} values")
      val builder = StringBuilder()
      values.forEach { builder.append("${it.key}: ${it.value}\n") }
      val output = builder.toString()

      Log.i(TAG, output)
      requireActivity().runOnUiThread { outputTv.text = output }
    }
  }

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?,
  ): View {
    scope = viewLifecycleOwner.lifecycleScope
    return inflater.inflate(R.layout.wildcard_fragment, container, false).apply {
      subscribeBtn.setOnClickListener { scope.launch { showSubscribeDialog() } }
      readBtn.setOnClickListener { scope.launch { read() } }

      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment
    }
  }

  private suspend fun subscribe(minInterval: Int, maxInterval: Int) {
    val subscriptionEstablishedCallback =
      SubscriptionEstablishedCallback { Log.i(TAG, "Subscription to device established") }

    val endpointId = getChipPathIdForText(endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(attributeIdEd.text.toString())
    val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)

    deviceController.subscribeToPath(subscriptionEstablishedCallback,
                                     reportCallback,
                                     ChipClient.getConnectedDevicePointer(requireContext(),
                                                                          addressUpdateFragment.deviceId),
                                     attributePath,
                                     minInterval,
                                     maxInterval)
  }

  private suspend fun read() {
    val endpointId = getChipPathIdForText(endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(attributeIdEd.text.toString())
    val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)

    deviceController.readPath(reportCallback,
                              ChipClient.getConnectedDevicePointer(requireContext(),
                                                                   addressUpdateFragment.deviceId),
                              attributePath)
  }

  private fun showSubscribeDialog() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.subscribe_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    val minIntervalEd = dialogView.findViewById<EditText>(R.id.minIntervalEd)
    val maxIntervalEd = dialogView.findViewById<EditText>(R.id.maxIntervalEd)
    dialogView.findViewById<Button>(R.id.subscribeBtn).setOnClickListener {
      scope.launch {
        subscribe(minIntervalEd.text.toString().toInt(), maxIntervalEd.text.toString().toInt())
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private fun getChipPathIdForText(text: String): ChipPathId {
    return if (text.isEmpty()) ChipPathId.forWildcard() else ChipPathId.forId(text.toLong())
  }

  companion object {
    private const val TAG = "WildcardFragment"

    fun newInstance(): WildcardFragment = WildcardFragment()
  }
}