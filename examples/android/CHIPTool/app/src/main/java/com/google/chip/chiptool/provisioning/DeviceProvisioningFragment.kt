/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package com.google.chip.chiptool.provisioning

import android.bluetooth.BluetoothGatt
import android.content.DialogInterface
import android.graphics.Typeface
import android.nfc.Tag
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.animation.Animation
import android.view.animation.LinearInterpolator
import android.view.animation.RotateAnimation
import android.widget.ImageView
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.AttestationInfo
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.CommissionParameters
import chip.devicecontroller.DeviceAttestationDelegate
import chip.devicecontroller.ICDDeviceInfo
import chip.devicecontroller.ICDRegistrationInfo
import chip.devicecontroller.NetworkCredentials
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.NetworkCredentialsParcelable
import com.google.chip.chiptool.R
import com.google.chip.chiptool.bluetooth.BluetoothManager
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.util.DeviceIdUtil
import com.google.chip.chiptool.util.FragmentUtil
import com.google.chip.chiptool.CHIPToolActivity
import chip.platform.AndroidChipPlatform.ServiceResolveListener
import matter.onboardingpayload.DiscoveryCapability
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.Runnable
import kotlinx.coroutines.launch

@ExperimentalCoroutinesApi
class DeviceProvisioningFragment : Fragment(), ServiceResolveListener {

  private lateinit var deviceInfo: CHIPDeviceInfo

  private var gatt: BluetoothGatt? = null

  private val networkCredentialsParcelable: NetworkCredentialsParcelable?
    get() = arguments?.getParcelable(ARG_NETWORK_CREDENTIALS)

  private lateinit var deviceController: ChipDeviceController

  private lateinit var scope: CoroutineScope

  private var dialog: AlertDialog? = null
  private var currentNfcCommissioningStage : String = commissioningStage_Cleanup

  // NFC commissioning variables
  private var deviceId : Long = 0
  // Animated ImageView used to show a rotating icon for the on-going stage
  private var animatedImageView: ImageView? = null
  private var nfcCommissioningAlertDialogView: View? = null
  private var operationalDiscoveryDone = false
  private var nfcUnpoweredPhaseCompleted = false
  private var nfcCommissioningCompleted = false


  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    deviceController = ChipClient.getDeviceController(requireContext())
  }

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    scope = viewLifecycleOwner.lifecycleScope
    deviceInfo = checkNotNull(requireArguments().getParcelable(ARG_DEVICE_INFO))

    return inflater.inflate(R.layout.barcode_fragment, container, false).apply {
      if (savedInstanceState == null) {
        if (deviceInfo.ipAddress != null) {
          pairDeviceWithAddress()
        } else {
          val isNFCCommissioningSupported = deviceInfo.discoveryCapabilities.contains(DiscoveryCapability.NFC)
          var androidNfcTag = CHIPToolActivity.getAndroidNfcTag()

          if ((androidNfcTag != null) && (isNFCCommissioningSupported)) {
            startConnectingToDeviceThroughNFC(androidNfcTag)
          } else {
            startConnectingToDeviceThroughBLE()
          }
        }
      }
    }
  }

  override fun onStop() {
    super.onStop()
    gatt = null
    dialog = null
  }

  override fun onDestroy() {
    super.onDestroy()
    deviceController.close()
    deviceController.setDeviceAttestationDelegate(0, EmptyAttestationDelegate())
  }

  private class EmptyAttestationDelegate : DeviceAttestationDelegate {
    override fun onDeviceAttestationCompleted(
      devicePtr: Long,
      attestationInfo: AttestationInfo,
      errorCode: Long
    ) {}
  }

  private fun setAttestationDelegate() {
    deviceController.setDeviceAttestationDelegate(DEVICE_ATTESTATION_FAILED_TIMEOUT) {
      devicePtr,
      _,
      errorCode ->
      Log.i(
        TAG,
        "Device attestation errorCode: $errorCode, " +
          "Look at 'src/credentials/attestation_verifier/DeviceAttestationVerifier.h' " +
          "AttestationVerificationResult enum to understand the errors"
      )

      val activity = requireActivity()

      if (errorCode == STATUS_PAIRING_SUCCESS) {
        activity.runOnUiThread(Runnable { deviceController.continueCommissioning(devicePtr, true) })

        return@setDeviceAttestationDelegate
      }

      activity.runOnUiThread(
        Runnable {
          if (dialog != null && dialog?.isShowing == true) {
            Log.d(TAG, "dialog is already showing")
            return@Runnable
          }
          dialog =
            AlertDialog.Builder(activity)
              .setPositiveButton(
                "Continue",
                DialogInterface.OnClickListener { dialog, id ->
                  deviceController.continueCommissioning(devicePtr, true)
                }
              )
              .setNegativeButton(
                "No",
                DialogInterface.OnClickListener { dialog, id ->
                  deviceController.continueCommissioning(devicePtr, false)
                }
              )
              .setTitle("Device Attestation")
              .setMessage(
                "Device Attestation failed for device under commissioning. Do you wish to continue pairing?"
              )
              .show()
        }
      )
    }
  }

  private fun pairDeviceWithAddress() {
    // IANA CHIP port
    val id = DeviceIdUtil.getNextAvailableId(requireContext())

    DeviceIdUtil.setNextAvailableId(requireContext(), id + 1)
    deviceController.setCompletionListener(ConnectionCallback())

    setAttestationDelegate()

    deviceController.pairDeviceWithAddress(
      id,
      deviceInfo.ipAddress,
      deviceInfo.port,
      deviceInfo.discriminator,
      deviceInfo.setupPinCode,
      null
    )
  }

  private fun startConnectingToDeviceThroughBLE() {

    if (gatt != null) {
      return
    }
    scope.launch {
      val bluetoothManager = BluetoothManager()

      showMessage(R.string.rendezvous_over_ble_scanning_text, deviceInfo.discriminator.toString())
      val device =
        bluetoothManager.getBluetoothDevice(
          requireContext(),
          deviceInfo.discriminator,
          deviceInfo.isShortDiscriminator
        )
          ?: run {
            showMessage(R.string.rendezvous_over_ble_scanning_failed_text)
            return@launch
          }

      showMessage(
        R.string.rendezvous_over_ble_connecting_text,
        device.name ?: device.address.toString()
      )

      gatt = bluetoothManager.connect(requireContext(), device)

      showMessage(R.string.rendezvous_over_ble_pairing_text)
      deviceController.setCompletionListener(ConnectionCallback())

      val deviceId = DeviceIdUtil.getNextAvailableId(requireContext())
      val connId = bluetoothManager.connectionId
      var network: NetworkCredentials? = null
      var networkParcelable = checkNotNull(networkCredentialsParcelable)

      val wifi = networkParcelable.wiFiCredentials
      if (wifi != null) {
        network =
          NetworkCredentials.forWiFi(NetworkCredentials.WiFiCredentials(wifi.ssid, wifi.password))
      }

      val thread = networkParcelable.threadCredentials
      if (thread != null) {
        network =
          NetworkCredentials.forThread(
            NetworkCredentials.ThreadCredentials(thread.operationalDataset)
          )
      }

      setAttestationDelegate()

      val params =
        CommissionParameters.Builder()
          .setCsrNonce(null)
          .setNetworkCredentials(network)
          .setICDRegistrationInfo(null)
          .build()

      deviceController.pairDeviceThroughBLE(gatt, connId, deviceId, deviceInfo.setupPinCode, params)
      DeviceIdUtil.setNextAvailableId(requireContext(), deviceId + 1)
    }
  }

  private fun startConnectingToDeviceThroughNFC(nfcTag : Tag) {

    ChipClient.getAndroidNfcCommissioningManager().setNFCTag(nfcTag)

    scope.launch {

      displayNfcCommissioningPopup();

      deviceController.setCompletionListener(ConnectionCallback())

      deviceId = DeviceIdUtil.getNextAvailableId(requireContext())
      var network: NetworkCredentials? = null
      var networkParcelable = checkNotNull(networkCredentialsParcelable)

      val wifi = networkParcelable.wiFiCredentials
      if (wifi != null) {
        network = NetworkCredentials.forWiFi(NetworkCredentials.WiFiCredentials(wifi.ssid, wifi.password))
      }

      val thread = networkParcelable.threadCredentials
      if (thread != null) {
        network = NetworkCredentials.forThread(NetworkCredentials.ThreadCredentials(thread.operationalDataset))
      }

      setAttestationDelegate()

      val params = CommissionParameters.Builder()
          .setCsrNonce(null)
          .setNetworkCredentials(network)
          .setICDRegistrationInfo(null)
          .build()

      deviceController.pairDeviceThroughNfc(deviceId, deviceInfo.setupPinCode, params)
      DeviceIdUtil.setNextAvailableId(requireContext(), deviceId + 1)
    }
  }

  private fun showMessage(msgResId: Int, stringArgs: String? = null) {
    requireActivity().runOnUiThread {
      val context = requireContext()
      val msg = context.getString(msgResId, stringArgs)
      Log.i(TAG, "showMessage:$msg")
      Toast.makeText(context, msg, Toast.LENGTH_SHORT).show()
    }
  }

  inner class ConnectionCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {
      Log.d(TAG, "onConnectDeviceComplete")
    }

    override fun onStatusUpdate(status: Int) {
      Log.d(TAG, "Pairing status update: $status")
    }

    // This notification happens when a stage is finished (successfully or not)
    override fun onCommissioningStatusUpdate(nodeId: Long, stage: String, errorCode: Long) {
    }

    // This notification happens when a stage is starting
    override fun onCommissioningStageStart(nodeId: Long, stage: String) {
      displayNfcCommissioningProgress(stage)
    }

    override fun onCommissioningComplete(nodeId: Long, errorCode: Long) {
      if (errorCode == STATUS_PAIRING_SUCCESS) {
        // When NFC-based commissioning without power is used, an "onCommissioningComplete" notification
        // is received at end of 1st commissioning phase (over NFC).
        // This notification is not transmitted to other classes since the commissioning is not
        // fully completed. The 2nd commissioning phase should still take place.
        if (nfcUnpoweredPhaseCompleted) {
          return
        }

        FragmentUtil.getHost(this@DeviceProvisioningFragment, Callback::class.java)
          ?.onCommissioningComplete(0L, nodeId)
      } else {
        showMessage(R.string.rendezvous_over_ble_pairing_failure_text)
        FragmentUtil.getHost(this@DeviceProvisioningFragment, Callback::class.java)
          ?.onCommissioningComplete(errorCode)
      }
    }

    override fun onPairingComplete(code: Long) {
      Log.d(TAG, "onPairingComplete: $code")

      if (code != STATUS_PAIRING_SUCCESS) {
        showMessage(R.string.rendezvous_over_ble_pairing_failure_text)
        FragmentUtil.getHost(this@DeviceProvisioningFragment, Callback::class.java)
          ?.onCommissioningComplete(code)
      }
    }

    override fun onOpCSRGenerationComplete(csr: ByteArray) {
      Log.d(TAG, String(csr))
    }

    override fun onPairingDeleted(code: Long) {
      Log.d(TAG, "onPairingDeleted: $code")
    }

    override fun onCloseBleComplete() {
      Log.d(TAG, "onCloseBleComplete")
    }

    override fun onError(error: Throwable?) {
      Log.d(TAG, "onError: $error")
    }

    override fun onICDRegistrationInfoRequired() {
      Log.d(TAG, "onICDRegistrationInfoRequired")
      deviceController.updateCommissioningICDRegistrationInfo(
        ICDRegistrationInfo.newBuilder().setICDStayActiveDurationMsec(30000L).build()
      )
    }

    override fun onICDRegistrationComplete(errorCode: Long, icdDeviceInfo: ICDDeviceInfo) {
      Log.d(
        TAG,
        "onICDRegistrationComplete - errorCode: $errorCode, symmetricKey : ${icdDeviceInfo.symmetricKey.toHex()}, icdDeviceInfo : $icdDeviceInfo"
      )
      requireActivity().runOnUiThread {
        Toast.makeText(
            requireActivity(),
            getString(
              R.string.icd_registration_completed,
              icdDeviceInfo.userActiveModeTriggerHint.toString(),
              icdDeviceInfo.userActiveModeTriggerInstruction,
              icdDeviceInfo.idleModeDuration.toString(),
              icdDeviceInfo.activeModeDuration.toString(),
              icdDeviceInfo.activeModeThreshold.toString()
            ),
            Toast.LENGTH_LONG
          )
          .show()
      }
    }
  }

  private fun ByteArray.toHex(): String =
    joinToString(separator = "") { eachByte -> "%02x".format(eachByte) }

  /** Callback from [DeviceProvisioningFragment] notifying any registered listeners. */
  interface Callback {
    /** Notifies that commissioning has been completed. */
    fun onCommissioningComplete(code: Long, nodeId: Long = 0L)
  }


  fun displayNfcCommissioningPopup() {

    requireActivity().runOnUiThread(java.lang.Runnable {
      val alertDialogBuilder = AlertDialog.Builder(requireActivity())

      // inflate XML content
      nfcCommissioningAlertDialogView =
        layoutInflater.inflate(R.layout.fragment_commissioning_progress, null)
      alertDialogBuilder
        .setTitle(getString(R.string.matter_commissioning_in_progress))
        .setCancelable(false)
        .setNegativeButton(
          "Close"
        ) { dialog, id ->
          if (deviceController != null) {
            try {
              if (!nfcCommissioningCompleted) {
                deviceController.stopDevicePairing(deviceId)
              }
            } catch (e: Error) {
              e.printStackTrace()
            } catch (e: java.lang.Exception) {
              e.printStackTrace()
            }
          }
          nfcCommissioningAlertDialogView = null
          dialog.cancel()
        }
      alertDialogBuilder.setView(nfcCommissioningAlertDialogView)

      // create alert dialog
      val alertDialog = alertDialogBuilder.create()

      nfcCommissioningCompleted = false
      nfcUnpoweredPhaseCompleted = false
      displayInitialStage()

      ChipClient.setServiceResolveListener(this)

      // show it
      alertDialog.show()
      alertDialog.getButton(AlertDialog.BUTTON_NEGATIVE).setTextColor(
        resources.getColor(R.color.light_blue)
      )
    })
  }

  private fun setAnimatedImageView(imageView: ImageView?) {

    // If an animation is currently running, stop it
    if (animatedImageView != null) {
      animatedImageView!!.clearAnimation()
      animatedImageView!!.setImageResource(R.drawable.ic_check_green_24dp)
      animatedImageView = null
    }

    if (imageView == null) {
      return
    }

    imageView!!.setImageResource(R.drawable.circular_arrow_green)
    imageView!!.visibility = View.VISIBLE

    val anim = RotateAnimation(
      0.0f,
      360.0f,
      Animation.RELATIVE_TO_SELF,
      .5f,
      Animation.RELATIVE_TO_SELF,
      .5f
    )
    anim.interpolator = LinearInterpolator()
    anim.repeatCount = Animation.INFINITE
    anim.duration = 3000
    imageView.animation = anim
    imageView.startAnimation(anim)

    animatedImageView = imageView
  }

  // Update the UI showing the progression of NFC-based commissioning
  private fun displayNfcCommissioningProgress(stage: String) {
    if (nfcCommissioningAlertDialogView == null) {
      // nfcCommissioningAlertDialog is not displayed. This happens when doing other kind of
      // commissioning (BLE, Wifi PAF...) or when the popup has been closed in the meantime.
      return
    }

    currentNfcCommissioningStage = stage;

    when (currentNfcCommissioningStage) {
      commissioningStage_SendDACCertificateRequest -> {
        displayCheckDeviceAttestationStage()
      }
      commissioningStage_SendNOC -> {
        displaySetNOCStage()
      }
      commissioningStage_ThreadNetworkSetup -> {
        displaySetOperationalNetworkStage()
      }
      commissioningStage_UnpoweredPhaseComplete -> {
        displayUnpoweredPhaseComplete()
      }
      commissioningStage_FindOperationalForStayActive -> {
        displayFindOperationalForStayActiveStage()
      }
      commissioningStage_SendComplete -> {
        displayCommissioningCompleteStage()
      }
      else -> Log.d(TAG,"No action for stage: ${stage}")
    }
  }

  private fun displayInitialStage() {
    requireActivity().runOnUiThread(java.lang.Runnable {
      if (nfcCommissioningAlertDialogView != null) {
        // Get all the widgets Ids
        val nfcImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.nfcImageView)
        val threadImageView =
          nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.threadImageView)
        val paseImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.paseImageView)
        val paseTextView = nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.paseTextView)
        val deviceAttestationImageView =
          nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.deviceAttestationImageView)
        val deviceAttestationTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.deviceAttestationTextView)
        val nocImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.nocImageView)
        val nocTextView = nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.nocTextView)
        val operationalNetworkImageView =
          nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.operationalNetworkImageView)
        val operationalNetworkTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.operationalNetworkTextView)
        val pleaseSwitchOnTheDeviceTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.pleaseSwitchOnTheDeviceTextView)
        val clickHereWhenDoneTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.clickHereWhenDoneTextView)
        val performServiceDiscoveryImageView =
          nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.performServiceDiscoveryImageView)
        val performServiceDiscoveryTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.performServiceDiscoveryTextView)
        val caseImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.caseImageView)
        val caseTextView = nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.caseTextView)
        val commissioningDoneTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.commissioningDoneTextView)

        // Set the widgets
        setAnimatedImageView(paseImageView);
        nfcImageView.setImageResource(R.drawable.new_nfc_logo_black)
        threadImageView.setImageResource(R.drawable.thread_logo_grey)
        deviceAttestationImageView.visibility = View.INVISIBLE
        nocImageView.visibility = View.INVISIBLE
        operationalNetworkImageView.visibility = View.INVISIBLE
        performServiceDiscoveryImageView.visibility = View.INVISIBLE
        caseImageView.visibility = View.INVISIBLE
        paseTextView.setTextColor(resources.getColor(R.color.dark_blue))
        deviceAttestationTextView.setTextColor(resources.getColor(R.color.light_grey))
        nocTextView.setTextColor(resources.getColor(R.color.light_grey))
        operationalNetworkTextView.setTextColor(resources.getColor(R.color.light_grey))
        pleaseSwitchOnTheDeviceTextView.setTextColor(resources.getColor(R.color.light_grey))
        clickHereWhenDoneTextView.setTextColor(resources.getColor(R.color.light_grey))
        clickHereWhenDoneTextView.setClickable(false);
        performServiceDiscoveryTextView.setTextColor(resources.getColor(R.color.light_grey))
        caseTextView.setTextColor(resources.getColor(R.color.light_grey))
        commissioningDoneTextView.setTextColor(resources.getColor(R.color.light_grey))

        pleaseSwitchOnTheDeviceTextView.setTypeface(Typeface.DEFAULT)
        clickHereWhenDoneTextView.setTypeface(Typeface.DEFAULT)
        commissioningDoneTextView.setTypeface(Typeface.DEFAULT)
      }
     })
  }

  private fun displayCheckDeviceAttestationStage() {
    requireActivity().runOnUiThread(java.lang.Runnable {
      if (nfcCommissioningAlertDialogView != null) {
        val deviceAttestationImageView =
          nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.deviceAttestationImageView)
        val deviceAttestationTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.deviceAttestationTextView)

        setAnimatedImageView(deviceAttestationImageView);
        deviceAttestationTextView.setTextColor(resources.getColor(R.color.dark_blue))
      }
    })
  }

  private fun displaySetNOCStage() {
    requireActivity().runOnUiThread(java.lang.Runnable {
      if (nfcCommissioningAlertDialogView != null) {
        val nocImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.nocImageView)
        val nocTextView = nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.nocTextView)

        setAnimatedImageView(nocImageView);
        nocTextView.setTextColor(resources.getColor(R.color.dark_blue))
      }
    })
  }

  private fun displaySetOperationalNetworkStage() {
    requireActivity().runOnUiThread(java.lang.Runnable {
      if (nfcCommissioningAlertDialogView != null) {
        val operationalNetworkImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.operationalNetworkImageView)
        val operationalNetworkTextView = nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.operationalNetworkTextView)

        setAnimatedImageView(operationalNetworkImageView);
        operationalNetworkTextView.setTextColor(resources.getColor(R.color.dark_blue))
      }
    })
  }

  private fun displayUnpoweredPhaseComplete() {
    requireActivity().runOnUiThread(java.lang.Runnable {
      if (nfcCommissioningAlertDialogView != null) {
        val pleaseSwitchOnTheDeviceTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.pleaseSwitchOnTheDeviceTextView)
        val clickHereWhenDoneTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.clickHereWhenDoneTextView)

        setAnimatedImageView(null);
        pleaseSwitchOnTheDeviceTextView.setTextColor(resources.getColor(R.color.dark_blue))
        pleaseSwitchOnTheDeviceTextView.setTypeface(Typeface.DEFAULT_BOLD)
        clickHereWhenDoneTextView.setTextColor(resources.getColor(R.color.light_blue))
        clickHereWhenDoneTextView.setTypeface(Typeface.DEFAULT_BOLD)
        clickHereWhenDoneTextView.setClickable(true)
        clickHereWhenDoneTextView.setOnClickListener(View.OnClickListener{
          continueCommissioningAfterConnectNetworkRequest()
        })

      }
    })
  }

  private fun displayFindOperationalForStayActiveStage() {
    requireActivity().runOnUiThread(java.lang.Runnable {
      if (nfcCommissioningAlertDialogView != null) {
        val pleaseSwitchOnTheDeviceTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.pleaseSwitchOnTheDeviceTextView)
        val clickHereWhenDoneTextView =
          nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.clickHereWhenDoneTextView)
        val nfcImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.nfcImageView)
        val caseImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.caseImageView)
        val caseTextView = nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.caseTextView)
        val performServiceDiscoveryImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.performServiceDiscoveryImageView)
        val threadImageView = nfcCommissioningAlertDialogView!!.findViewById<ImageView>(R.id.threadImageView)
        val performServiceDiscoveryTextView = nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.performServiceDiscoveryTextView)

        if (operationalDiscoveryDone) {
          // Operational Discovery done so CASE is now starting
          setAnimatedImageView(caseImageView);
          caseTextView.setTextColor(resources.getColor(R.color.dark_blue))
        } else {
          // Operational Discovery still on-going
          setAnimatedImageView(performServiceDiscoveryImageView);
          nfcImageView.setImageResource(R.drawable.new_nfc_logo_grey)

          pleaseSwitchOnTheDeviceTextView.setTextColor(resources.getColor(R.color.light_grey))
          pleaseSwitchOnTheDeviceTextView.setTypeface(Typeface.DEFAULT)
          clickHereWhenDoneTextView.setTextColor(resources.getColor(R.color.light_grey))
          clickHereWhenDoneTextView.setTypeface(Typeface.DEFAULT)
          clickHereWhenDoneTextView.setClickable(false)

          threadImageView.setImageResource(R.drawable.thread_logo_black)
          performServiceDiscoveryTextView.setTextColor(resources.getColor(R.color.dark_blue))
        }
      }
    })
  }

  private fun displayCommissioningCompleteStage() {
    requireActivity().runOnUiThread(java.lang.Runnable {
      if (nfcCommissioningAlertDialogView != null) {
        val commissioningDoneTextView = nfcCommissioningAlertDialogView!!.findViewById<TextView>(R.id.commissioningDoneTextView)
        setAnimatedImageView(null);
        commissioningDoneTextView.setTextColor(resources.getColor(R.color.dark_blue))
        commissioningDoneTextView.setTypeface(Typeface.DEFAULT_BOLD)
        nfcCommissioningCompleted = true
      }
    })
  }

  private fun continueCommissioningAfterConnectNetworkRequest() {
    operationalDiscoveryDone = false

    val chipDeviceController = ChipClient.getDeviceController(requireActivity())

    chipDeviceController.continueCommissioningAfterConnectNetworkRequest(deviceId)
  }


  companion object {
    private const val TAG = "DeviceProvisioningFragment"
    private const val ARG_DEVICE_INFO = "device_info"
    private const val ARG_NETWORK_CREDENTIALS = "network_credentials"
    private const val STATUS_PAIRING_SUCCESS = 0L

    // Commissioning stages
    const val commissioningStage_ReadCommissioningInfo = "ReadCommissioningInfo"
    const val commissioningStage_SendDACCertificateRequest = "SendDACCertificateRequest"
    const val commissioningStage_SendNOC = "SendNOC"
    const val commissioningStage_ThreadNetworkSetup = "ThreadNetworkSetup"
    const val commissioningStage_UnpoweredPhaseComplete = "UnpoweredPhaseComplete"
    const val commissioningStage_FindOperationalForStayActive = "FindOperationalForStayActive"
    const val commissioningStage_SendComplete = "SendComplete"
    const val commissioningStage_Cleanup = "Cleanup"

    /**
     * Set for the fail-safe timer before onDeviceAttestationFailed is invoked.
     *
     * This time depends on the Commissioning timeout of your app.
     */
    private const val DEVICE_ATTESTATION_FAILED_TIMEOUT = 600

    /**
     * Return a new instance of [DeviceProvisioningFragment]. [networkCredentialsParcelable] can be
     * null for IP commissioning.
     */
    fun newInstance(
      deviceInfo: CHIPDeviceInfo,
      networkCredentialsParcelable: NetworkCredentialsParcelable?,
    ): DeviceProvisioningFragment {
      return DeviceProvisioningFragment().apply {
        arguments =
          Bundle(2).apply {
            putParcelable(ARG_DEVICE_INFO, deviceInfo)
            putParcelable(ARG_NETWORK_CREDENTIALS, networkCredentialsParcelable)
          }
      }
    }
  }

  override fun onServiceResolve(instanceName : String, serviceType : String) {
    Log.d(TAG, "DeviceProvisioning: onServiceResolve: " + instanceName + "." + serviceType)
    if (currentNfcCommissioningStage == commissioningStage_FindOperationalForStayActive) {
      operationalDiscoveryDone = true
      displayNfcCommissioningProgress(currentNfcCommissioningStage);
    }
  }

}
