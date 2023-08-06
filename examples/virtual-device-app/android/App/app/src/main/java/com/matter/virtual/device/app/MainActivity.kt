package com.matter.virtual.device.app

import android.Manifest
import android.annotation.SuppressLint
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Bundle
import android.os.PowerManager
import android.provider.Settings
import android.view.Gravity
import android.view.MenuItem
import android.view.View
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.matter.virtual.device.app.core.common.EventObserver
import com.matter.virtual.device.app.core.ui.SharedViewModel
import com.matter.virtual.device.app.core.ui.UiState
import com.matter.virtual.device.app.databinding.ActivityMainBinding
import dagger.hilt.android.AndroidEntryPoint
import timber.log.Timber

@AndroidEntryPoint
class MainActivity : AppCompatActivity() {

  private lateinit var binding: ActivityMainBinding
  private val viewModel by viewModels<SharedViewModel>()

  private val permissions =
    arrayOf(
      Manifest.permission.BLUETOOTH_SCAN,
      Manifest.permission.BLUETOOTH_ADVERTISE,
      Manifest.permission.BLUETOOTH_CONNECT,
      Manifest.permission.ACCESS_FINE_LOCATION
    )

  private val requestMultiplePermissions =
    registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { permissions ->
      permissions.entries.forEach { Timber.d("${it.key}:${it.value}") }
    }

  @SuppressLint("BatteryLife")
  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    Timber.d("onCreate()")

    val isPermissionsGranted =
      permissions.all {
        ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
      }

    if (!isPermissionsGranted) {
      requestMultiplePermissions.launch(permissions)
    }

    val powerManager = applicationContext.getSystemService(POWER_SERVICE) as PowerManager?
    powerManager?.let { manager ->
      if (!manager.isIgnoringBatteryOptimizations(packageName)) {
        Timber.d("not in battery optimization whitelist")
        val intent =
          Intent().apply {
            action = Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS
            data = Uri.parse("package:$packageName")
          }
        startActivity(intent)
      }
    }

    binding = ActivityMainBinding.inflate(layoutInflater)
    setContentView(binding.root)

    viewModel.uiState.observe(
      this,
      EventObserver { uiState ->
        when (uiState) {
          UiState.Waiting -> {
            binding.progress.visibility = View.VISIBLE
          }
          UiState.Exit -> {
            binding.progress.visibility = View.GONE
            finishAffinity()
          }
          is UiState.Reset -> {
            showFactoryResetPopup(getString(uiState.messageResId), uiState.isCancelable)
          }
          else -> {}
        }
      }
    )
  }

  override fun onDestroy() {
    super.onDestroy()
    Timber.d("onDestroy()")
  }

  override fun onOptionsItemSelected(item: MenuItem): Boolean {
    when (item.itemId) {
      android.R.id.home -> {
        onBackPressed()
        return true
      }
    }

    return super.onOptionsItemSelected(item)
  }

  override fun onRequestPermissionsResult(
    requestCode: Int,
    permissions: Array<out String>,
    grantResults: IntArray
  ) {
    Timber.d("RequestCode:$requestCode")
    super.onRequestPermissionsResult(requestCode, permissions, grantResults)
  }

  private fun showFactoryResetPopup(message: String, isCancelable: Boolean) {
    val builder =
      AlertDialog.Builder(this)
        .setTitle("Factory Reset")
        .setMessage(message)
        .setPositiveButton("Ok") { dialog, _ ->
          Timber.d("Ok")
          dialog.dismiss()
          viewModel.resetMatterAppServer()
        }
        .setCancelable(false)

    if (isCancelable) {
      builder.setNegativeButton("Cancel") { dialog, _ ->
        Timber.d("Cancel")
        dialog.dismiss()
      }
    }

    val dialog = builder.create()
    dialog.window?.setGravity(Gravity.BOTTOM)
    dialog.show()
  }
}
