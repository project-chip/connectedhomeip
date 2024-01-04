package com.matter.virtual.device.app.core.ui

import android.os.Bundle
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.PopupMenu
import androidx.annotation.LayoutRes
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import com.matter.virtual.device.app.core.common.EventObserver
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.ui.databinding.LayoutAppbarBinding
import timber.log.Timber

abstract class BaseFragment<T : ViewDataBinding, V : BaseViewModel>(
  @LayoutRes val layoutResId: Int
) : Fragment() {

  private var _binding: T? = null
  protected val binding
    get() = _binding!!

  protected abstract val viewModel: V
  private val sharedViewModel by activityViewModels<SharedViewModel>()
  protected lateinit var matterSettings: MatterSettings

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View? {
    _binding = DataBindingUtil.inflate(inflater, layoutResId, container, false)
    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)
    Timber.d("onViewCreated()")

    binding.lifecycleOwner = viewLifecycleOwner

    /** Appbar */
    val layoutAppBarBinding = setupAppbar()
    (activity as AppCompatActivity).setSupportActionBar(layoutAppBarBinding.toolbar)
    (activity as AppCompatActivity).supportActionBar?.setDisplayHomeAsUpEnabled(true)

    layoutAppBarBinding.toolbarMoreMenuButton.setOnClickListener {
      Timber.d("More")
      showMoreMenuPopup(it)
    }

    setupNavArgs()
    setupUi()
    setupObservers()

    viewModel.onFabricRemoved.observe(
      viewLifecycleOwner,
      EventObserver { onFrabricRemoved ->
        if (onFrabricRemoved) {
          sharedViewModel.requestFactoryReset(
            messageResId = R.string.dialog_fabric_removed_factory_reset_message,
            isCancelable = false
          )
        }
      }
    )
  }

  protected abstract fun setupAppbar(): LayoutAppbarBinding

  protected abstract fun setupNavArgs()

  protected abstract fun setupUi()

  protected abstract fun setupObservers()

  override fun onDestroyView() {
    super.onDestroyView()
    Timber.d("onDestroyView()")
    _binding = null
  }

  private fun showMoreMenuPopup(anchor: View) {
    val morePopupMenu =
      PopupMenu(
          requireContext(),
          anchor,
          Gravity.TOP or Gravity.END,
          0,
          R.style.CustomMenuPopupRound
        )
        .apply {
          menuInflater.inflate(R.menu.more, this.menu)

          setOnMenuItemClickListener { menuItem ->
            return@setOnMenuItemClickListener when (menuItem.itemId) {
              R.id.action_reset -> {
                Timber.d("reset")
                sharedViewModel.requestFactoryReset(
                  messageResId = R.string.dialog_factory_reset_message,
                  isCancelable = true
                )
                false
              }
              else -> {
                false
              }
            }
          }
        }

    morePopupMenu.show()
  }
}
