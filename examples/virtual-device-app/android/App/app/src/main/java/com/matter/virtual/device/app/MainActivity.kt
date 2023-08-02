package com.matter.virtual.device.app

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.view.MenuItem
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.matter.virtual.device.app.databinding.ActivityMainBinding
import dagger.hilt.android.AndroidEntryPoint
import timber.log.Timber

@AndroidEntryPoint
class MainActivity : AppCompatActivity() {

  private lateinit var binding: ActivityMainBinding

  private val permissions = arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)

  private val requestMultiplePermissions =
    registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { permissions ->
      permissions.entries.forEach { Timber.d("${it.key}:${it.value}") }
    }

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

    binding = ActivityMainBinding.inflate(layoutInflater)
    setContentView(binding.root)
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
}
