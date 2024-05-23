package com.google.chip.chiptool.attestation

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.util.Log
import androidx.activity.result.ActivityResultCaller
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts

/** Class for finding and launching external attestation apps */
object AttestationAppLauncher {
  /** Registers and returns an [ActivityResultLauncher] for attestation */
  fun getLauncher(
    caller: ActivityResultCaller,
    block: (String?) -> Unit
  ): ActivityResultLauncher<Intent> {
    return caller.registerForActivityResult(ActivityResultContracts.StartActivityForResult()) {
      result ->
      val data = result.data
      if (result.resultCode == Activity.RESULT_OK && data != null) {
        val chipResult = data.getStringExtra(CHIP_RESULT_KEY)
        Log.i(TAG, "Attestation result: $chipResult")
        block(chipResult)
      }
    }
  }

  /** Queries apps which support attestation and returns the first [Intent] */
  fun getAttestationIntent(context: Context): Intent? {
    val packageManager = context.packageManager as PackageManager
    val attestationActivityIntent = Intent(CHIP_ACTION)
    val attestationAppInfo =
      packageManager.queryIntentActivities(attestationActivityIntent, 0).firstOrNull()

    return if (attestationAppInfo != null) {
      attestationActivityIntent.setClassName(
        attestationAppInfo.activityInfo.packageName,
        attestationAppInfo.activityInfo.name
      )

      attestationActivityIntent
    } else {
      Log.e(TAG, "No attestation app found")
      null
    }
  }

  private const val TAG = "AttestationAppLauncher"
  private const val CHIP_ACTION = "chip.intent.action.ATTESTATION"
  private const val CHIP_RESULT_KEY = "chip_result_key"
}
