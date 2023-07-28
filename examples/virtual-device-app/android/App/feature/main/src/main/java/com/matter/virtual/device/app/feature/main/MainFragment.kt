package com.matter.virtual.device.app.feature.main

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.Fragment
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import com.matter.virtual.device.app.core.common.DeepLink
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.feature.main.databinding.FragmentMainBinding
import com.matter.virtual.device.app.feature.main.model.Menu
import dagger.hilt.android.AndroidEntryPoint
import kotlin.math.abs
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class MainFragment : Fragment() {

  private lateinit var binding: FragmentMainBinding
  private lateinit var onBackPressedCallback: OnBackPressedCallback

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    Timber.d("Hit")
    binding = DataBindingUtil.inflate(inflater, R.layout.fragment_main, container, false)
    binding.lifecycleOwner = viewLifecycleOwner

    return binding.root
  }

  @OptIn(ExperimentalSerializationApi::class)
  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    Timber.d("Hit")
    super.onViewCreated(view, savedInstanceState)

    (activity as AppCompatActivity).setSupportActionBar(binding.toolbar)
    (activity as AppCompatActivity).supportActionBar?.setDisplayHomeAsUpEnabled(true)

    binding.appBarLayout.addOnOffsetChangedListener { appBarLayout, verticalOffset ->
      var ratio = 0F
      if (abs(verticalOffset) != 0) {
        ratio = abs(verticalOffset).toFloat() / appBarLayout.totalScrollRange.toFloat()
      }

      binding.collapseTitle.alpha = 1f - ratio * 2f + 0.1f
      binding.toolbarTitle.alpha = (ratio - 0.5f) * 2f + 0.1f
    }

    val itemList = arrayListOf(Menu.ON_OFF_SWITCH)

    val menuAdapter =
      MenuAdapter(
          object : MenuAdapter.ItemHandler {
            override fun onClick(item: Menu) {
              val matterSettings = MatterSettings(device = item.device)
              val jsonSettings = Json.encodeToString(matterSettings)
              try {
                findNavController()
                  .navigate(DeepLink.getDeepLinkRequestForSetupFragment(jsonSettings))
              } catch (e: Exception) {
                Timber.e(e, "navigate failure")
              }
            }
          }
        )
        .apply { submitList(itemList) }

    val sideSpace = resources.getDimension(R.dimen.menu_item_side_space).toInt()
    val bottomSpace = resources.getDimension(R.dimen.menu_item_bottom_space).toInt()

    binding.recyclerView.apply {
      layoutManager = LinearLayoutManager(requireContext())
      if (itemDecorationCount == 0) {
        addItemDecoration(VerticalSpaceItemDecoration(sideSpace, bottomSpace))
      }
      adapter = menuAdapter
    }
  }

  override fun onResume() {
    super.onResume()
    Timber.d("onResume()")
  }

  override fun onAttach(context: Context) {
    Timber.d("Hit")
    super.onAttach(context)

    onBackPressedCallback =
      object : OnBackPressedCallback(true) {
        override fun handleOnBackPressed() {
          Timber.d("handleOnBackPressed()")
          requireActivity().finishAffinity()
        }
      }

    requireActivity().onBackPressedDispatcher.addCallback(this, onBackPressedCallback)
  }

  override fun onDetach() {
    super.onDetach()
    Timber.d("onDetach()")
    onBackPressedCallback.remove()
  }

  override fun onDestroy() {
    super.onDestroy()
    Timber.d("onDestroy()")
  }
}
