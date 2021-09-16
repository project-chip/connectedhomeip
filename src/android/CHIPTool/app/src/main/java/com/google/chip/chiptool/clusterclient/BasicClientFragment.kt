package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipClusters.BasicCluster
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlinx.android.synthetic.main.basic_client_fragment.basicClustCommandStatus
import kotlinx.android.synthetic.main.basic_client_fragment.basicClusterDeviceIdEd
import kotlinx.android.synthetic.main.basic_client_fragment.basicClusterFabricIdEd
import kotlinx.android.synthetic.main.basic_client_fragment.view.basicClusterUpdateAddressBtn
import kotlinx.android.synthetic.main.basic_client_fragment.view.readProducNameBtn
import kotlinx.android.synthetic.main.basic_client_fragment.view.readUserLabelBtn
import kotlinx.android.synthetic.main.basic_client_fragment.view.writeUserLabelBtn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class BasicClientFragment : Fragment() {
  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.basic_client_fragment, container, false).apply {

      basicClusterUpdateAddressBtn.setOnClickListener { scope.launch{ } }
      readUserLabelBtn.setOnClickListener { scope.launch{ } }
      writeUserLabelBtn.setOnClickListener { scope.launch{ } }
      readProducNameBtn.setOnClickListener { scope.launch{ } }
    }
  }

  override fun onStop() {
    super.onStop()
    scope.cancel()
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      basicClustCommandStatus.text = msg
    }
  }

  companion object {
    private const val TAG = "BasicClientFragment"
    fun newInstance(): BasicClientFragment = BasicClientFragment()
  }
}
