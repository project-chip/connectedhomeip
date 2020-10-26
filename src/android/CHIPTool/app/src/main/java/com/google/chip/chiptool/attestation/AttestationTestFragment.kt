package com.google.chip.chiptool.attestation

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.attestation_test_fragment.view.*

/** Fragment for launching external attestation apps */
class AttestationTestFragment : Fragment() {
  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View? {
    return inflater.inflate(R.layout.attestation_test_fragment, container, false).apply {
      attestationText.text = context.getString(R.string.attestation_fetching_status)
      val appIntent = AttestationAppLauncher.getAttestationIntent(requireContext())

      if (appIntent != null) {
        AttestationAppLauncher
          .getLauncher(this@AttestationTestFragment) { result ->
            attestationText.text = result
          }
          .launch(appIntent)
      } else {
        attestationText.text = context.getString(R.string.attestation_app_not_found)
      }
    }
  }

  companion object {
    @JvmStatic
    fun newInstance(): AttestationTestFragment = AttestationTestFragment()
  }
}
