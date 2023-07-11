package com.google.chip.chiptool.attestation

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.AttestationTestFragmentBinding

/** Fragment for launching external attestation apps */
class AttestationTestFragment : Fragment() {
  private var _binding: AttestationTestFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = AttestationTestFragmentBinding.inflate(inflater, container, false)

    binding.attestationText.text = context!!.getString(R.string.attestation_fetching_status)
    val appIntent = AttestationAppLauncher.getAttestationIntent(requireContext())
    if (appIntent != null) {
      AttestationAppLauncher.getLauncher(this@AttestationTestFragment) { result ->
          binding.attestationText.text = result
        }
        .launch(appIntent)
    } else {
      binding.attestationText.text = context!!.getString(R.string.attestation_app_not_found)
    }

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  companion object {
    @JvmStatic fun newInstance(): AttestationTestFragment = AttestationTestFragment()
  }
}
