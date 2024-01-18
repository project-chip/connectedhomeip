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
import androidx.fragment.app.activityViewModels
import androidx.fragment.app.viewModels
import androidx.navigation.NavOptions
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import com.matter.virtual.device.app.core.common.DeepLink
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.ui.SharedViewModel
import com.matter.virtual.device.app.feature.main.databinding.FragmentMainBinding
import com.matter.virtual.device.app.feature.main.model.Menu
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class MainFragment : Fragment() {

  private lateinit var binding: FragmentMainBinding
  private val viewModel by viewModels<MainViewModel>()
  private val sharedViewModel by activityViewModels<SharedViewModel>()

  private lateinit var onBackPressedCallback: OnBackPressedCallback

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    Timber.d("onCreateView()")
    binding = DataBindingUtil.inflate(inflater, R.layout.fragment_main, container, false)
    binding.lifecycleOwner = viewLifecycleOwner

    return binding.root
  }

  @OptIn(ExperimentalSerializationApi::class)
  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    Timber.d("onViewCreated()")
    super.onViewCreated(view, savedInstanceState)

    (activity as AppCompatActivity).setSupportActionBar(binding.toolbar)
    (activity as AppCompatActivity).supportActionBar?.setDisplayHomeAsUpEnabled(true)

    viewModel.uiState.observe(viewLifecycleOwner) { uiState ->
      Timber.d("uiState:$uiState")
      when (uiState) {
        MainUiState.Loading -> {}
        is MainUiState.CommissioningCompleted -> {
          try {
            val navOptions =
              NavOptions.Builder()
                .setPopUpTo(destinationId = R.id.mainFragment, inclusive = true)
                .build()

            findNavController()
              .navigate(
                DeepLink.getDeepLinkRequestFromDevice(
                  uiState.device,
                  Json.encodeToString(MatterSettings(device = uiState.device))
                ),
                navOptions
              )
          } catch (e: Exception) {
            Timber.e("navigate failure")
          }
        }
        is MainUiState.Reset -> {
          sharedViewModel.requestFactoryReset(
            messageResId = uiState.messageResId,
            isCancelable = uiState.isCancelable
          )
        }
        MainUiState.Start -> {
          val itemList = arrayListOf(Menu.ON_OFF_SWITCH)

          val menuAdapter =
            MenuAdapter(
                object : MenuAdapter.ItemHandler {
                  override fun onClick(item: Menu) {
                    viewModel.consumeUiState()

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
      }
    }
  }

  override fun onResume() {
    super.onResume()
    Timber.d("onResume()")
  }

  override fun onAttach(context: Context) {
    Timber.d("onAttach()")
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
