package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.multi_admin_client_fragment.multiAdminClustCommandStatus
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.multiAdminClusterUpdateAddressBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.basicCommissioningMethodBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.enhancedCommissioningMethodBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.revokeBtn
import kotlinx.coroutines.*

class MultiAdminClientFragment : Fragment() {
  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.multi_admin_client_fragment, container, false).apply {

      multiAdminClusterUpdateAddressBtn.setOnClickListener { updateAddressClick() }
      basicCommissioningMethodBtn.setOnClickListener { scope.launch { sendBasicCommissioningCommandClick() } }
      enhancedCommissioningMethodBtn.setOnClickListener { scope.launch { sendEnhancedCommissioningCommandClick() } }
      revokeBtn.setOnClickListener { scope.launch { sendRevokeCommandClick() } }
    }
  }

  override fun onStop() {
    super.onStop()
    scope.cancel()
  }

  private fun updateAddressClick() {
  }

  private suspend fun sendBasicCommissioningCommandClick() {
  }

  private suspend fun sendEnhancedCommissioningCommandClick() {
  }

  private suspend fun sendRevokeCommandClick() {
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      multiAdminClustCommandStatus.text = msg
    }
  }

  companion object {
    private const val TAG = "MultiAdminClientFragment"
    fun newInstance(): MultiAdminClientFragment = MultiAdminClientFragment()
  }
}
