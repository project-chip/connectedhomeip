package com.google.chip.chiptool.clusterclient

import android.app.AlertDialog
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.Spinner
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterIDMapping
import chip.devicecontroller.GroupKeySecurityPolicy
import chip.devicecontroller.InvokeCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlEntryStruct
import chip.devicecontroller.cluster.structs.GroupKeyManagementClusterGroupKeySetStruct
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.InvokeElement
import chip.devicecontroller.model.NodeState
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.GroupSettingFragmentBinding
import com.google.chip.chiptool.util.DeviceIdUtil
import java.lang.Exception
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class GroupSettingFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: GroupSettingFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = GroupSettingFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    deviceController.setCompletionListener(ChipControllerCallback())

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.sendKeySetWriteBtn.setOnClickListener { sendKeySetWriteBtnClick() }
    binding.writeGroupKeyMapBtn.setOnClickListener { writeGroupKeyMapBtnClick() }
    binding.sendAddGroupBtn.setOnClickListener { sendAddGroupBtnClick() }
    binding.addAccessControlBtn.setOnClickListener { scope.launch { readAccessControl() } }
    binding.addGroupBtn.setOnClickListener { addGroupBtnClick() }
    binding.removeGroupBtn.setOnClickListener { removeGroupBtnClick() }
    binding.addkeysetBtn.setOnClickListener { addKeySetBtnClick() }
    binding.removekeysetBtn.setOnClickListener { removeKeySetBtnClick() }
    binding.bindkeysetBtn.setOnClickListener { bindKeySetBtnClick(true) }
    binding.unbindkeysetBtn.setOnClickListener { bindKeySetBtnClick(false) }

    return binding.root
  }

  private fun addGroupBtnClick() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.add_group_dialog, null)
    val groupIdEd = dialogView.findViewById<EditText>(R.id.groupIdEd)
    val groupNameEd = dialogView.findViewById<EditText>(R.id.groupNameEd)

    dialogView.findViewById<Button>(R.id.sendGroupDialogBtn).visibility = View.GONE

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.addGroupDialogBtn).setOnClickListener {
      val groupId = groupIdEd.text.toString().toUInt()
      val ret = deviceController.addGroup(groupId.toInt(), groupNameEd.text.toString())
      showMessage("addGroup : $ret")
      updateGroupSettingView()
      DeviceIdUtil.setCommissionedNodeId(
        requireContext(),
        AddressUpdateFragment.getNodeIdFromGroupId(groupId).toLong()
      )
      addressUpdateFragment.updateDeviceIdSpinner()
      requireActivity().runOnUiThread { dialog.dismiss() }
    }
    dialog.show()
  }

  private fun removeGroupBtnClick() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.remove_group_dialog, null)

    val groupList = deviceController.availableGroupIds

    val groupListSp = dialogView.findViewById<Spinner>(R.id.groupListSp)
    groupListSp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        groupList.map { it.toString() }
      )

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.removeGroupDialogBtn).setOnClickListener {
      val selectedGroupId = groupListSp.selectedItem.toString().toUInt()
      val ret = deviceController.removeGroup(selectedGroupId.toInt())
      showMessage("removeGroup : $selectedGroupId - $ret")
      updateGroupSettingView()
      DeviceIdUtil.removeCommissionedNodeId(
        requireContext(),
        AddressUpdateFragment.getNodeIdFromGroupId(selectedGroupId).toLong()
      )
      addressUpdateFragment.updateDeviceIdSpinner()
      requireActivity().runOnUiThread { dialog.dismiss() }
    }
    dialog.show()
  }

  private fun addKeySetBtnClick() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.add_key_set_dialog, null)
    val keySetIdEd = dialogView.findViewById<EditText>(R.id.keySetIdEd)
    val keySecurityPolicySp = dialogView.findViewById<Spinner>(R.id.keySecurityPolicySp)
    keySecurityPolicySp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        GroupKeySecurityPolicy.values()
      )
    val validityTimeEd = dialogView.findViewById<EditText>(R.id.validityTimeEd)
    val epochKeyEd = dialogView.findViewById<EditText>(R.id.epochKeyEd)

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.addKeySetDialogBtn).setOnClickListener {
      val ret =
        deviceController.addKeySet(
          keySetIdEd.text.toString().toUInt().toInt(),
          GroupKeySecurityPolicy.valueOf(keySecurityPolicySp.selectedItem.toString()),
          validityTimeEd.text.toString().toULong().toLong(),
          hexStringToByteArray(epochKeyEd.text.toString())
        )
      showMessage("addKeySet : $ret")
      updateGroupSettingView()
      requireActivity().runOnUiThread { dialog.dismiss() }
    }
    dialog.show()
  }

  private fun removeKeySetBtnClick() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.remove_key_set_dialog, null)

    val keySetList = deviceController.keySetIds

    val keySetListSp = dialogView.findViewById<Spinner>(R.id.keySetListSp)
    keySetListSp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        keySetList.map { it.toString() }
      )

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.removeGroupDialogBtn).setOnClickListener {
      val selectedKeySetId = keySetListSp.selectedItem.toString().toInt()
      val ret = deviceController.removeKeySet(selectedKeySetId)
      showMessage("removeKeySet : $selectedKeySetId - $ret")
      updateGroupSettingView()
      requireActivity().runOnUiThread { dialog.dismiss() }
    }
    dialog.show()
  }

  private fun bindKeySetBtnClick(isBind: Boolean) {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.bind_key_set_dialog, null)

    val groupList = deviceController.availableGroupIds
    val keySetList = deviceController.keySetIds

    val groupSp = dialogView.findViewById<Spinner>(R.id.groupSp)
    groupSp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        groupList.map { it.toString() }
      )

    val keySetSp = dialogView.findViewById<Spinner>(R.id.keySetSp)
    keySetSp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        keySetList.map { it.toString() }
      )

    dialogView.findViewById<Button>(R.id.bindKeySetDialogBtn).visibility =
      if (isBind) {
        View.VISIBLE
      } else {
        View.GONE
      }
    dialogView.findViewById<Button>(R.id.unbindKeySetDialogBtn).visibility =
      if (!isBind) {
        View.VISIBLE
      } else {
        View.GONE
      }

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.bindKeySetDialogBtn).setOnClickListener {
      val groupId = groupSp.selectedItem.toString().toUInt().toInt()
      val keySetId = keySetSp.selectedItem.toString().toUInt().toInt()
      val ret = deviceController.bindKeySet(groupId, keySetId)
      showMessage("bindKeySet : $groupId, $keySetId - $ret")
      updateGroupSettingView()
      requireActivity().runOnUiThread { dialog.dismiss() }
    }

    dialogView.findViewById<Button>(R.id.unbindKeySetDialogBtn).setOnClickListener {
      val groupId = groupSp.selectedItem.toString().toUInt().toInt()
      val keySetId = keySetSp.selectedItem.toString().toUInt().toInt()
      val ret = deviceController.unbindKeySet(groupId, keySetId)
      showMessage("unbindKeySet : $groupId, $keySetId - $ret")
      updateGroupSettingView()
      requireActivity().runOnUiThread { dialog.dismiss() }
    }
    dialog.show()
  }

  private fun sendKeySetWriteBtnClick() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.send_key_set_dialog, null)
    val keySetIdEd = dialogView.findViewById<EditText>(R.id.keySetIdEd)
    val keySecurityPolicySp = dialogView.findViewById<Spinner>(R.id.keySecurityPolicySp)
    keySecurityPolicySp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        GroupKeySecurityPolicy.values()
      )
    val epochKey0Ed = dialogView.findViewById<EditText>(R.id.epochKey0Ed)
    val epochStartTime0Ed = dialogView.findViewById<EditText>(R.id.epochStartTime0Ed)
    val epochKey1Ed = dialogView.findViewById<EditText>(R.id.epochKey1Ed)
    val epochStartTime1Ed = dialogView.findViewById<EditText>(R.id.epochStartTime1Ed)
    val epochKey2Ed = dialogView.findViewById<EditText>(R.id.epochKey2Ed)
    val epochStartTime2Ed = dialogView.findViewById<EditText>(R.id.epochStartTime2Ed)

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.addKeySetDialogBtn).setOnClickListener {
      scope.launch {
        val keySetWritestruct =
          GroupKeyManagementClusterGroupKeySetStruct(
            keySetIdEd.text.toString().toUInt(),
            GroupKeySecurityPolicy.valueOf(keySecurityPolicySp.selectedItem.toString()).id.toUInt(),
            hexStringToByteArray(epochKey0Ed.text.toString()),
            epochStartTime0Ed.text.toString().toULong(),
            hexStringToByteArray(epochKey1Ed.text.toString()),
            epochStartTime1Ed.text.toString().toULong(),
            hexStringToByteArray(epochKey2Ed.text.toString()),
            epochStartTime2Ed.text.toString().toULong()
          )
        sendKeySetWrite(keySetWritestruct)
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun sendKeySetWrite(
    groupKeySetStruct: GroupKeyManagementClusterGroupKeySetStruct
  ) {
    val tlvWriter =
      TlvWriter()
        .startStructure(AnonymousTag)
        .apply {
          groupKeySetStruct.toTlv(
            ContextSpecificTag(
              ClusterIDMapping.GroupKeyManagement.KeySetWriteCommandField.GroupKeySet.id
            ),
            this
          )
        }
        .endStructure()

    deviceController.invoke(
      object : InvokeCallback {
        override fun onError(e: java.lang.Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("onError : ${e.toString()}")
        }

        override fun onResponse(invokeElement: InvokeElement?, successCode: Long) {
          Log.d(TAG, "onResponse")
          showMessage("onResponse : $successCode")
        }
      },
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId),
      InvokeElement.newInstance(
        0,
        ClusterIDMapping.GroupKeyManagement.ID,
        ClusterIDMapping.GroupKeyManagement.Command.KeySetWrite.id,
        tlvWriter.getEncoded(),
        null
      ),
      0,
      0
    )
  }

  private fun writeGroupKeyMapBtnClick() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.write_group_key_dialog, null)
    val groupIdEd = dialogView.findViewById<EditText>(R.id.groupIdEd)
    val groupKeySetIdEd = dialogView.findViewById<EditText>(R.id.groupKeySetIdEd)

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.writeGroupKeyMapBtn).setOnClickListener {
      scope.launch {
        writeGroupKeyMap(
          groupIdEd.text.toString().toUInt(),
          groupKeySetIdEd.text.toString().toUInt()
        )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun writeGroupKeyMap(groupId: UInt, groupKeySetId: UInt) {
    val tlvWriter =
      TlvWriter().apply {
        startArray(AnonymousTag)
        startStructure(AnonymousTag)
        put(ContextSpecificTag(1), groupId)
        put(ContextSpecificTag(2), groupKeySetId)
        endStructure()
        endArray()
      }

    deviceController.write(
      object : WriteAttributesCallback {
        override fun onError(attributePath: ChipAttributePath?, e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : ${e.toString()}")
        }

        override fun onResponse(attributePath: ChipAttributePath?) {
          Log.d(TAG, "onResponse")
          showMessage("write Success")
        }
      },
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId),
      listOf(
        AttributeWriteRequest.newInstance(
          0,
          ClusterIDMapping.GroupKeyManagement.ID,
          ClusterIDMapping.GroupKeyManagement.Attribute.GroupKeyMap.id,
          tlvWriter.getEncoded()
        )
      ),
      0,
      0
    )
  }

  private fun sendAddGroupBtnClick() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.add_group_dialog, null)
    val groupIdEd = dialogView.findViewById<EditText>(R.id.groupIdEd)
    val groupNameEd = dialogView.findViewById<EditText>(R.id.groupNameEd)

    dialogView.findViewById<Button>(R.id.addGroupDialogBtn).visibility = View.GONE

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.sendGroupDialogBtn).setOnClickListener {
      scope.launch {
        sendAddGroup(groupIdEd.text.toString().toUInt(), groupNameEd.text.toString())
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun sendAddGroup(groupId: UInt, groupName: String) {
    val tlvWriter =
      TlvWriter().apply {
        startStructure(AnonymousTag)
        put(ContextSpecificTag(0), groupId)
        put(ContextSpecificTag(1), groupName)
        endStructure()
      }

    deviceController.invoke(
      object : InvokeCallback {
        override fun onError(e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : ${e.toString()}")
        }

        override fun onResponse(invokeElement: InvokeElement?, successCode: Long) {
          Log.d(TAG, "onResponse")
          showMessage("onResponse : $successCode")
        }
      },
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId),
      InvokeElement.newInstance(
        0,
        ClusterIDMapping.Groups.ID,
        ClusterIDMapping.Groups.Command.AddGroup.id,
        tlvWriter.getEncoded(),
        null
      ),
      0,
      0
    )
  }

  private suspend fun readAccessControl() {
    val endpointId = 0
    val clusterId = ClusterIDMapping.AccessControl.ID
    val attributeId = ClusterIDMapping.AccessControl.Attribute.Acl.id

    val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
    deviceController.readAttributePath(
      object : ReportCallback {
        override fun onError(
          attributePath: ChipAttributePath?,
          eventPath: ChipEventPath?,
          e: Exception
        ) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : $e")
        }

        override fun onReport(nodeState: NodeState?) {
          Log.d(TAG, "onResponse")
          val tlv =
            nodeState
              ?.getEndpointState(endpointId)
              ?.getClusterState(clusterId)
              ?.getAttributeState(attributeId)
              ?.tlv
          requireActivity().runOnUiThread { showAddAccessControlDialog(tlv) }
        }
      },
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId),
      listOf(attributePath),
      0
    )
  }

  private fun showAddAccessControlDialog(tlv: ByteArray?) {
    if (tlv == null) {
      Log.d(TAG, "Access Control read fail")
      showMessage("Access Control read fail")
      return
    }

    val dialogView =
      requireActivity().layoutInflater.inflate(R.layout.add_access_control_dialog, null)
    val groupIdEd = dialogView.findViewById<EditText>(R.id.groupIdEd)
    val accessControlEntrySp = dialogView.findViewById<Spinner>(R.id.accessControlEntrySp)
    accessControlEntrySp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        AccessControlEntry.values()
      )

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.addAccessControlBtn).setOnClickListener {
      scope.launch {
        sendAccessControl(
          tlv,
          groupIdEd.text.toString().toUInt(),
          AccessControlEntry.valueOf(accessControlEntrySp.selectedItem.toString()).id
        )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun sendAccessControl(tlv: ByteArray, groupId: UInt, privilege: UInt) {
    val tlvWriter = TlvWriter().startArray(AnonymousTag)
    var entryStructList: List<AccessControlClusterAccessControlEntryStruct>
    TlvReader(tlv).also {
      entryStructList = buildList {
        it.enterArray(AnonymousTag)
        while (!it.isEndOfContainer()) {
          add(AccessControlClusterAccessControlEntryStruct.fromTlv(AnonymousTag, it))
        }
        it.exitContainer()
      }
    }

    // If GroupID is already added to AccessControl, do not add it.
    for (entry in entryStructList) {
      if (
        entry.authMode == 3U /* Group */ &&
          entry.subjects != null &&
          entry.subjects!!.contains(groupId.toULong())
      ) {
        continue
      }

      entry.toTlv(AnonymousTag, tlvWriter)
    }

    val newEntry =
      AccessControlClusterAccessControlEntryStruct(
        privilege,
        3U /* Group */,
        listOf(groupId.toULong()),
        null,
        deviceController.fabricIndex.toUInt()
      )
    newEntry.toTlv(AnonymousTag, tlvWriter)
    tlvWriter.endArray()

    deviceController.write(
      object : WriteAttributesCallback {
        override fun onError(attributePath: ChipAttributePath?, e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : ${e.toString()}")
        }

        override fun onResponse(attributePath: ChipAttributePath?) {
          Log.d(TAG, "onResponse")
          showMessage("write Success")
        }
      },
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId),
      listOf(
        AttributeWriteRequest.newInstance(
          0,
          ClusterIDMapping.AccessControl.ID,
          ClusterIDMapping.AccessControl.Attribute.Acl.id,
          tlvWriter.getEncoded()
        )
      ),
      0,
      0
    )
  }

  override fun onStart() {
    super.onStart()

    updateGroupSettingView()
  }

  private fun updateGroupSettingView() {
    requireActivity().runOnUiThread {
      var viewString = "Group :\n"

      for (groupId in deviceController.availableGroupIds) {
        val keySetIdString =
          if (deviceController.findKeySetId(groupId).isPresent) {
            deviceController.findKeySetId(groupId).get().toString()
          } else {
            "[]"
          }
        viewString += "\t$groupId, ${deviceController.getGroupName(groupId)}, $keySetIdString\n"
      }

      viewString += "\nKeySet :\n"
      for (keySetId in deviceController.keySetIds) {
        viewString +=
          "\t$keySetId, ${deviceController.getKeySecurityPolicy(keySetId).map { it.name }}\n"
      }

      binding.multiAdminClusterCommandStatus.text = viewString
    }
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
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

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread { binding.groupSettingStatusTv.text = msg }
  }

  companion object {
    private const val TAG = "GroupSettingFragment"

    fun newInstance(): GroupSettingFragment = GroupSettingFragment()

    private const val HEX_RADIX = 16
    private const val HEX_BIT_SHIFT = 4

    private fun hexStringToByteArray(hexString: String): ByteArray {
      val len = hexString.length
      val data = ByteArray(len / 2)
      var i = 0
      while (i < len) {
        data[i / 2] =
          ((Character.digit(hexString[i], HEX_RADIX) shl HEX_BIT_SHIFT) +
              Character.digit(hexString[i + 1], HEX_RADIX))
            .toByte()
        i += 2
      }
      return data
    }

    enum class AccessControlEntry(val id: UInt) {
      Manage(4U),
      Operate(3U),
      ProxyView(2U),
      View(1U),
      Administer(5U)
    }
  }
}
