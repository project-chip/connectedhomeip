package com.tcl.chip.chiptest

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.text.method.ScrollingMovementMethod
import androidx.appcompat.app.AppCompatActivity
import chip.platform.*
import com.tcl.chip.chiptest.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

  private val msgShowLog = 1

  private lateinit var binding: ActivityMainBinding
  private val mainHandler: Handler =
    object : Handler(Looper.getMainLooper()) {
      override fun handleMessage(msg: Message) {
        super.handleMessage(msg)
        when (msg.what) {
          msgShowLog -> {
            binding.testLog.append(msg.obj.toString())
          }
        }
      }
    }

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)

    binding = ActivityMainBinding.inflate(layoutInflater)
    setContentView(binding.root)

    binding.testLog.movementMethod = ScrollingMovementMethod()
    TestEngine.setListener { log ->
      var msg = mainHandler.obtainMessage(msgShowLog, log)
      mainHandler.sendMessage(msg)
    }

    binding.runButton.setOnClickListener {
      binding.testLog.text = ""
      Thread {
          var ret = TestEngine.runTest()

          var log: String =
            if (ret == 0) {
              getString(R.string.test_finished)
            } else {
              getString(R.string.test_failed, ret)
            }
          var msg = mainHandler.obtainMessage(msgShowLog, log)
          mainHandler.sendMessage(msg)
        }
        .start()
    }

    AndroidChipPlatform(
      AndroidBleManager(),
      PreferencesKeyValueStoreManager(this),
      PreferencesConfigurationManager(this),
      NsdManagerServiceResolver(this),
      NsdManagerServiceBrowser(this),
      ChipMdnsCallbackImpl(),
      DiagnosticDataProviderImpl(this)
    )
  }
}
