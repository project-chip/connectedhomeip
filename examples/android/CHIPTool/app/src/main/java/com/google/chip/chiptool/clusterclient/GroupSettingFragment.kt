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
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ChipStructs
import chip.devicecontroller.ChipStructs.AccessControlClusterAccessControlEntryStruct
import chip.devicecontroller.ChipStructs.GroupKeyManagementClusterGroupKeySetStruct
import chip.devicecontroller.GroupKeySecurityPolicy
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.GroupSettingFragmentBinding
import com.google.chip.chiptool.util.DeviceIdUtil
import java.lang.Exception
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

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
            keySetIdEd.text.toString().toUInt().toInt(),
            GroupKeySecurityPolicy.valueOf(keySecurityPolicySp.selectedItem.toString())
              .id
              .toUInt()
              .toInt(),
            hexStringToByteArray(epochKey0Ed.text.toString()),
            epochStartTime0Ed.text.toString().toULong().toLong(),
            hexStringToByteArray(epochKey1Ed.text.toString()),
            epochStartTime1Ed.text.toString().toULong().toLong(),
            hexStringToByteArray(epochKey2Ed.text.toString()),
            epochStartTime2Ed.text.toString().toULong().toLong()
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
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    val cluster = ChipClusters.GroupKeyManagementCluster(devicePtr, 0)

    cluster.keySetWrite(
      object : ChipClusters.DefaultClusterCallback {
        override fun onError(e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("onError : ${e.toString()}")
        }

        override fun onSuccess() {
          Log.d(TAG, "onResponse")
          showMessage("onResponse")
        }
      },
      groupKeySetStruct
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
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    val cluster = ChipClusters.GroupKeyManagementCluster(devicePtr, 0)
    cluster.writeGroupKeyMapAttribute(
      object : ChipClusters.DefaultClusterCallback {
        override fun onError(e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : ${e.toString()}")
        }

        override fun onSuccess() {
          Log.d(TAG, "onResponse")
          showMessage("write Success")
        }
      },
      arrayListOf(
        ChipStructs.GroupKeyManagementClusterGroupKeyMapStruct(
          groupId.toInt(),
          groupKeySetId.toInt(),
          deviceController.fabricIndex
        )
      )
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
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    val cluster = ChipClusters.GroupsCluster(devicePtr, 0)

    cluster.addGroup(
      object : ChipClusters.GroupsCluster.AddGroupResponseCallback {
        override fun onError(e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : ${e.toString()}")
        }

        override fun onSuccess(status: Int?, groupID: Int?) {
          Log.d(TAG, "onResponse")
          showMessage("onResponse : $status, $groupID")
        }
      },
      groupId.toInt(),
      groupName
    )
  }

  private suspend fun readAccessControl() {
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    val cluster = ChipClusters.AccessControlCluster(devicePtr, 0)
    cluster.readAclAttribute(
      object : ChipClusters.AccessControlCluster.AclAttributeCallback {
        override fun onError(e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : $e")
        }

        override fun onSuccess(value: MutableList<AccessControlClusterAccessControlEntryStruct>?) {
          requireActivity().runOnUiThread { showAddAccessControlDialog(value) }
        }
      }
    )
  }

  private fun showAddAccessControlDialog(
    value: List<AccessControlClusterAccessControlEntryStruct>?
  ) {
    if (value == null) {
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
          value,
          groupIdEd.text.toString().toUInt(),
          AccessControlEntry.valueOf(accessControlEntrySp.selectedItem.toString()).id
        )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun sendAccessControl(
    value: List<AccessControlClusterAccessControlEntryStruct>,
    groupId: UInt,
    privilege: UInt
  ) {
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    // If GroupID is already added to AccessControl, do not add it.
    val cluster = ChipClusters.AccessControlCluster(devicePtr, 0)
    val sendEntry = ArrayList<AccessControlClusterAccessControlEntryStruct>()
    for (entry in value) {
      if (
        entry.authMode == AccessControlEntry.Operate.id.toInt() /* Group */ &&
          entry.subjects != null &&
          entry.subjects!!.contains(groupId.toULong().toLong())
      ) {
        continue
      }
      sendEntry.add(entry)
    }

    val newEntry =
      AccessControlClusterAccessControlEntryStruct(
        privilege.toInt(),
        AccessControlEntry.Operate.id.toInt() /* Group */,
        arrayListOf(groupId.toULong().toLong()),
        null,
        deviceController.fabricIndex.toUInt().toInt()
      )
    sendEntry.add(newEntry)

    cluster.writeAclAttribute(
      object : ChipClusters.DefaultClusterCallback {
        override fun onError(e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : ${e.toString()}")
        }

        override fun onSuccess() {
          Log.d(TAG, "onResponse")
          showMessage("write Success")
        }
      },
      sendEntry
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
