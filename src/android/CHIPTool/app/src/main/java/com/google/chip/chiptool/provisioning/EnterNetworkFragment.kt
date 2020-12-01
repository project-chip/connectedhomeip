package com.google.chip.chiptool.provisioning

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.google.android.material.tabs.TabLayout
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.enter_network_fragment.view.*

class EnterNetworkFragment : Fragment() {

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.enter_network_fragment, container, false).apply {
      tabs.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {
        override fun onTabReselected(tab: TabLayout.Tab) = Unit
        override fun onTabUnselected(tab: TabLayout.Tab) = Unit
        override fun onTabSelected(tab: TabLayout.Tab) {
          when (tab.text.toString()) {
            TAB_CAPTION_WIFI -> selectPage(EnterWifiNetworkFragment.newInstance())
            TAB_CAPTION_THREAD -> selectPage(EnterThreadNetworkFragment.newInstance())
          }
        }
      })

      selectPage(EnterWifiNetworkFragment.newInstance())
    }
  }

  private fun selectPage(fragment: Fragment) {
    childFragmentManager.beginTransaction().replace(R.id.page, fragment).commit()
  }

  companion object {
    private const val TAB_CAPTION_WIFI = "Wi-Fi"
    private const val TAB_CAPTION_THREAD = "Thread"

    fun newInstance() = EnterNetworkFragment()
  }

}