package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import chip.devicecontroller.ChipDeviceController
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.clusterclient.AddressUpdateFragment
import com.google.chip.chiptool.databinding.ClusterInteractionFragmentBinding
import kotlin.properties.Delegates
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class ClusterInteractionFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope
  private lateinit var addressUpdateFragment: AddressUpdateFragment
  private var devicePtr by Delegates.notNull<Long>()

  private var _binding: ClusterInteractionFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = ClusterInteractionFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    deviceController.setCompletionListener(GenericChipDeviceListener())
    binding.endpointList.visibility = View.GONE
    binding.getEndpointListBtn.setOnClickListener {
      scope.launch {
        devicePtr =
          try {
            ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
          } catch (e: IllegalStateException) {
            Log.d(TAG, "getConnectedDevicePointer exception", e)
            showMessage("getConnectedDevicePointer fail!")
            return@launch
          }
        showMessage("Retrieving endpoints")
        binding.endpointList.visibility = View.VISIBLE
      }
    }

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment
    var dataList: List<EndpointItem> = ArrayList()
    // TODO: Dynamically retrieve endpoint information using descriptor cluster
    // hardcode the endpoint for now
    for (i in 0 until 2) {
      dataList += EndpointItem(i)
    }

    binding.endpointList.adapter = EndpointAdapter(dataList, EndpointListener())
    binding.endpointList.layoutManager = LinearLayoutManager(requireContext())
    binding.bottomNavigationBar.setOnNavigationItemSelectedListener(bottomNavigationListener)

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      Toast.makeText(requireContext(), msg, Toast.LENGTH_SHORT).show()
    }
  }

  override fun onStop() {
    super.onStop()
    scope.cancel()
  }

  companion object {
    private const val TAG = "ClusterInteractionFragment"

    fun newInstance(): ClusterInteractionFragment = ClusterInteractionFragment()
  }

  private fun showFragment(fragment: Fragment) {
    val fragmentTransaction =
      requireActivity()
        .supportFragmentManager
        .beginTransaction()
        .replace(R.id.nav_host_fragment, fragment, fragment.javaClass.simpleName)
    fragmentTransaction.addToBackStack(null)
    fragmentTransaction.commit()
  }

  private val bottomNavigationListener =
    BottomNavigationView.OnNavigationItemSelectedListener { menuItem ->
      when (menuItem.itemId) {
        R.id.clusterInteractionHistory -> {
          val fragment = ClusterInteractionHistoryFragment.newInstance()
          showFragment(fragment)
          return@OnNavigationItemSelectedListener true
        }
        R.id.clusterInteractionSettings -> {
          val fragment = ClusterInteractionSettingsFragment()
          showFragment(fragment)
          return@OnNavigationItemSelectedListener true
        }
      }
      false
    }

  inner class EndpointListener : EndpointAdapter.OnItemClickListener {
    override fun onItemClick(position: Int) {
      showFragment(
        ClusterDetailFragment.newInstance(addressUpdateFragment.deviceId, position, null)
      )
    }
  }
}
