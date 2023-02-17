package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.EditorInfo
import android.widget.ArrayAdapter
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipClusters.BasicInformationCluster
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.BasicClientFragmentBinding
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class BasicClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: BasicClientFragmentBinding? = null
  private val binding get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = BasicClientFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    deviceController.setCompletionListener(ChipControllerCallback())

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.writeNodeLabelBtn.setOnClickListener { scope.launch {
      sendWriteNodeLabelAttribute()
      binding.nodeLabelEd.onEditorAction(EditorInfo.IME_ACTION_DONE)
    }}
    binding.writeLocationBtn.setOnClickListener { scope.launch {
      sendWriteLocationAttribute()
      binding.locationEd.onEditorAction(EditorInfo.IME_ACTION_DONE)
    }}
    binding.writeLocalConfigDisabledSwitch.setOnCheckedChangeListener { _, isChecked ->
      scope.launch { sendWriteLocalConfigDisabledAttribute(isChecked) }
    }
    makeAttributeList()
    binding.attributeNameSpinner.adapter = makeAttributeNamesAdapter()
    binding.readAttributeBtn.setOnClickListener { scope.launch { readAttributeButtonClick() }}

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {}

    override fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
      Log.d(TAG, "onCommissioningComplete for nodeId $nodeId: $errorCode")
    }

    override fun onNotifyChipConnectionClosed() {
      Log.d(TAG, "onNotifyChipConnectionClosed")
    }

    override fun onCloseBleComplete() {
      Log.d(TAG, "onCloseBleComplete")
    }

    override fun onError(error: Throwable?) {
      Log.d(TAG, "onError: $error")
    }
  }


  private fun makeAttributeNamesAdapter(): ArrayAdapter<String> {
    return ArrayAdapter(
      requireContext(),
      android.R.layout.simple_spinner_dropdown_item,
      ATTRIBUTES.toList()
    ).apply {
      setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
    }
  }

  private suspend fun readAttributeButtonClick() {
    try {
      readBasicClusters(binding.attributeNameSpinner.selectedItemPosition)
    } catch (ex: Exception) {
      showMessage("readBasicCluster failed: $ex")
    }
  }

  private suspend fun readBasicClusters(itemIndex: Int) {
    when(ATTRIBUTES[itemIndex]) {
      getString(R.string.basic_cluster_data_model_revision_text) -> sendReadDataModelRevisionAttribute()
      getString(R.string.basic_cluster_vendor_name_text) -> sendReadVendorNameAttribute()
      getString(R.string.basic_cluster_vendor_id_text) -> sendReadVendorIDAttribute()
      getString(R.string.basic_cluster_product_name_text) -> sendReadProductNameAttribute()
      getString(R.string.basic_cluster_product_id_text) -> sendReadProductIDAttribute()
      getString(R.string.basic_cluster_node_label_text) -> sendReadNodeLabelAttribute()
      getString(R.string.basic_cluster_location_text) -> sendReadLocationAttribute()
      getString(R.string.basic_cluster_hardware_version_text) -> sendReadHardwareVersionAttribute()
      getString(R.string.basic_cluster_hardware_version_string_text) -> sendReadHardwareVersionStringAttribute()
      getString(R.string.basic_cluster_software_version_text) -> sendReadSoftwareVersionAttribute()
      getString(R.string.basic_cluster_software_version_string_text) -> sendReadSoftwareVersionStringAttribute()
      getString(R.string.basic_cluster_manufacturing_date_text) -> sendReadManufacturingDateAttribute()
      getString(R.string.basic_cluster_part_number_text) -> sendReadPartNumberAttribute()
      getString(R.string.basic_cluster_product_url_text) -> sendReadProductURLAttribute()
      getString(R.string.basic_cluster_product_label_text) -> sendReadProductLabelAttribute()
      getString(R.string.basic_cluster_serial_number_text) -> sendReadSerialNumberAttribute()
      getString(R.string.basic_cluster_local_config_disabled_text) -> sendReadLocalConfigDisabledAttribute()
      getString(R.string.basic_cluster_reachable_text) -> sendReadReachableAttribute()
      getString(R.string.basic_cluster_cluster_revision_text) -> sendReadClusterRevisionAttribute()
    }
  }

  private fun makeAttributeList() {
    ATTRIBUTES.add(getString(R.string.basic_cluster_data_model_revision_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_vendor_name_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_vendor_id_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_product_name_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_product_id_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_node_label_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_location_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_hardware_version_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_hardware_version_string_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_software_version_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_software_version_string_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_manufacturing_date_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_part_number_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_product_url_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_product_label_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_serial_number_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_local_config_disabled_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_reachable_text))
    ATTRIBUTES.add(getString(R.string.basic_cluster_cluster_revision_text))
  }

  private suspend fun sendReadDataModelRevisionAttribute() {
    getBasicClusterForDevice().readDataModelRevisionAttribute(object : ChipClusters.IntegerAttributeCallback {
      override fun onSuccess(value: Int) {
        Log.i(TAG,"[Read Success] DataModelRevision: $value")
        showMessage("[Read Success] DataModelRevision: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read DataModelRevision failure $ex")
        Log.e(TAG, "Read DataModelRevision failure", ex)
      }
    })
  }

  private suspend fun sendReadVendorNameAttribute() {
    getBasicClusterForDevice().readVendorNameAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] VendorName: $value")
        showMessage("[Read Success] VendorName: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read VendorName failure $ex")
        Log.e(TAG, "Read VendorName failure", ex)
      }
    })
  }

  private suspend fun sendReadVendorIDAttribute() {
    getBasicClusterForDevice().readVendorIDAttribute(object : ChipClusters.BasicInformationCluster.VendorIDAttributeCallback {
      override fun onSuccess(value: Int) {
        Log.i(TAG,"[Read Success] VendorID: $value")
        showMessage("[Read Success] VendorID: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read VendorID failure $ex")
        Log.e(TAG, "Read VendorID failure", ex)
      }
    })
  }

  private suspend fun sendReadProductNameAttribute() {
    getBasicClusterForDevice().readProductNameAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] ProductName: $value")
        showMessage("[Read Success] ProductName: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read ProductName failure $ex")
        Log.e(TAG, "Read ProductName failure", ex)
      }
    })
  }

  private suspend fun sendReadProductIDAttribute() {
    getBasicClusterForDevice().readProductIDAttribute(object : ChipClusters.IntegerAttributeCallback {
      override fun onSuccess(value: Int) {
        Log.i(TAG,"[Read Success] ProductID: $value")
        showMessage("[Read Success] ProductID: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read ProductID failure $ex")
        Log.e(TAG, "Read ProductID failure", ex)
      }
    })
  }

  private suspend fun sendReadNodeLabelAttribute() {
    getBasicClusterForDevice().readNodeLabelAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] NodeLabel: $value")
        showMessage("[Read Success] NodeLabel: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read NodeLabel failure $ex")
        Log.e(TAG, "Read NodeLabel failure", ex)
      }
    })
  }

  private suspend fun sendWriteNodeLabelAttribute() {
    getBasicClusterForDevice().writeNodeLabelAttribute(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("Write NodeLabel success")
      }

      override fun onError(ex: Exception) {
        showMessage("Write NodeLabel failure $ex")
        Log.e(TAG, "Write NodeLabel failure", ex)
      }
    }, binding.nodeLabelEd.text.toString())
  }

  private suspend fun sendReadLocationAttribute() {
    getBasicClusterForDevice().readLocationAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] Location: $value")
        showMessage("[Read Success] Location: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read Location failure $ex")
        Log.e(TAG, "Read Location failure", ex)
      }
    })
  }

  private suspend fun sendWriteLocationAttribute() {
    getBasicClusterForDevice().writeLocationAttribute(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("Write Location success")
      }

      override fun onError(ex: Exception) {
        showMessage("Write Location failure $ex")
        Log.e(TAG, "Write Location failure", ex)
      }
    }, binding.locationEd.text.toString())
  }

  private suspend fun sendReadHardwareVersionAttribute() {
    getBasicClusterForDevice().readHardwareVersionAttribute(object : ChipClusters.IntegerAttributeCallback {
      override fun onSuccess(value: Int) {
        Log.i(TAG,"[Read Success] HardwareVersion: $value")
        showMessage("[Read Success] HardwareVersion: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read HardwareVersion failure $ex")
        Log.e(TAG, "Read HardwareVersion failure", ex)
      }
    })
  }

  private suspend fun sendReadHardwareVersionStringAttribute() {
    getBasicClusterForDevice().readHardwareVersionStringAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] HardwareVersionString: $value")
        showMessage("[Read Success] HardwareVersionString: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read HardwareVersionString failure $ex")
        Log.e(TAG, "Read HardwareVersionString failure", ex)
      }
    })
  }

  private suspend fun sendReadSoftwareVersionAttribute() {
    getBasicClusterForDevice().readSoftwareVersionAttribute(object : ChipClusters.LongAttributeCallback {
      override fun onSuccess(value: Long) {
        Log.i(TAG,"[Read Success] SoftwareVersion: $value")
        showMessage("[Read Success] SoftwareVersion: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read SoftwareVersion failure $ex")
        Log.e(TAG, "Read SoftwareVersion failure", ex)
      }
    })
  }

  private suspend fun sendReadSoftwareVersionStringAttribute() {
    getBasicClusterForDevice().readSoftwareVersionStringAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] SoftwareVersionString $value")
        showMessage("[Read Success] SoftwareVersionString: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read SoftwareVersionString failure $ex")
        Log.e(TAG, "Read SoftwareVersionString failure", ex)
      }
    })
  }

  private suspend fun sendReadManufacturingDateAttribute() {
    getBasicClusterForDevice().readManufacturingDateAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] ManufacturingDate $value")
        showMessage("[Read Success] ManufacturingDate: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read ManufacturingDate failure $ex")
        Log.e(TAG, "Read ManufacturingDate failure", ex)
      }
    })
  }

  private suspend fun sendReadPartNumberAttribute() {
    getBasicClusterForDevice().readPartNumberAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] PartNumber $value")
        showMessage("[Read Success] PartNumber: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read PartNumber failure $ex")
        Log.e(TAG, "Read PartNumber failure", ex)
      }
    })
  }

  private suspend fun sendReadProductURLAttribute() {
    getBasicClusterForDevice().readProductURLAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] ProductURL $value")
        showMessage("[Read Success] ProductURL: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read ProductURL failure $ex")
        Log.e(TAG, "Read ProductURL failure", ex)
      }
    })
  }

  private suspend fun sendReadProductLabelAttribute() {
    getBasicClusterForDevice().readProductLabelAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] ProductLabel $value")
        showMessage("[Read Success] ProductLabel: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read ProductLabel failure $ex")
        Log.e(TAG, "Read ProductLabel failure", ex)
      }
    })
  }

  private suspend fun sendReadSerialNumberAttribute() {
    getBasicClusterForDevice().readSerialNumberAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        Log.i(TAG,"[Read Success] SerialNumber $value")
        showMessage("[Read Success] SerialNumber: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read SerialNumber failure $ex")
        Log.e(TAG, "Read SerialNumber failure", ex)
      }
    })
  }

  private suspend fun sendReadLocalConfigDisabledAttribute() {
    getBasicClusterForDevice().readLocalConfigDisabledAttribute(object : ChipClusters.BooleanAttributeCallback {
      override fun onSuccess(value: Boolean) {
        Log.i(TAG,"[Read Success] LocalConfigDisabled $value")
        showMessage("[Read Success] LocalConfigDisabled: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read LocalConfigDisabled failure $ex")
        Log.e(TAG, "Read LocalConfigDisabled failure", ex)
      }
    })
  }

  private suspend fun sendWriteLocalConfigDisabledAttribute(localConfigDisabled: Boolean) {
    getBasicClusterForDevice().writeLocalConfigDisabledAttribute(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("Write LocalConfigDisabled success")
      }

      override fun onError(ex: Exception) {
        showMessage("Write LocalConfigDisabled failure $ex")
        Log.e(TAG, "Write LocalConfigDisabled failure", ex)
      }
    }, localConfigDisabled)
  }

  private suspend fun sendReadReachableAttribute() {
    getBasicClusterForDevice().readReachableAttribute(object : ChipClusters.BooleanAttributeCallback {
      override fun onSuccess(value: Boolean) {
        Log.i(TAG,"[Read Success] Reachable $value")
        showMessage("[Read Success] Reachable: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read Reachable failure $ex")
        Log.e(TAG, "Read Reachable failure", ex)
      }
    })
  }

  private suspend fun sendReadClusterRevisionAttribute() {
    getBasicClusterForDevice().readClusterRevisionAttribute(object : ChipClusters.IntegerAttributeCallback {
      override fun onSuccess(value: Int) {
        Log.i(TAG,"[Read Success] ClusterRevision $value")
        showMessage("[Read Success] ClusterRevision: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read ClusterRevision failure $ex")
        Log.e(TAG, "Read ClusterRevision failure", ex)
      }
    })
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      binding.basicClusterCommandStatus.text = msg
    }
  }

  private suspend fun getBasicClusterForDevice(): BasicInformationCluster {
    return BasicInformationCluster(
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId), ENDPOINT
    )
  }

  companion object {
    private const val TAG = "BasicClientFragment"
    private const val ENDPOINT = 0
    private val ATTRIBUTES: MutableList<String> = mutableListOf()

    fun newInstance(): BasicClientFragment = BasicClientFragment()
  }
}
