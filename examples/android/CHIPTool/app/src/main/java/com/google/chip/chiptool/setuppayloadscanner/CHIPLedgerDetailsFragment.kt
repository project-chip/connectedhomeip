/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package com.google.chip.chiptool.setuppayloadscanner

import android.net.Uri
import android.os.Bundle
import android.util.Base64
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.android.volley.Request
import com.android.volley.toolbox.JsonObjectRequest
import com.android.volley.toolbox.Volley
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.ChipLedgerInfoFragmentBinding
import com.google.chip.chiptool.util.FragmentUtil
import com.google.gson.Gson

/** Show the [CHIPDeviceInfo] from Ledger */
class CHIPLedgerDetailsFragment : Fragment() {
  private lateinit var deviceInfo: CHIPDeviceInfo
  private var _binding: ChipLedgerInfoFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = ChipLedgerInfoFragmentBinding.inflate(inflater, container, false)
    deviceInfo = checkNotNull(requireArguments().getParcelable(ARG_DEVICE_INFO))
    val queue = Volley.newRequestQueue(context)
    // VID / PID
    binding.vendorIdTv.text = "${deviceInfo.vendorId}"
    binding.productIdTv.text = "${deviceInfo.productId}"

    // Ledger api url
    val url =
      Uri.parse(context!!.getString(R.string.dcl_api_root_url))
        .buildUpon()
        .appendPath("${deviceInfo.vendorId}")
        .appendPath("${deviceInfo.productId}")
        .build()
        .toString()
    Log.d(TAG, "Dcl request Url: $url")

    // Ledger API call
    val jsonObjectRequest =
      JsonObjectRequest(
        Request.Method.GET,
        url,
        null,
        { response ->
          Log.d(TAG, "Response from dcl $response")

          // parse redirect Url
          val responseJson = response.getJSONObject(context!!.getString(R.string.dcl_response_key))
          val redirectUrl =
            responseJson.getString(context!!.getString(R.string.dcl_custom_flow_url_key))
          Log.d(TAG, "Redirect Url from Ledger: $redirectUrl")
          binding.commissioningFlowUrlTv.text = redirectUrl

          // generate redirect payload
          val gson = Gson()
          val payloadJson = gson.toJson(deviceInfo)
          val payloadBase64 = Base64.encodeToString(payloadJson.toByteArray(), Base64.DEFAULT)
          val redirectUrlWithPayload =
            Uri.parse(redirectUrl)
              .buildUpon()
              .appendQueryParameter("payload", payloadBase64)
              .appendQueryParameter(
                "returnUrl",
                context!!.getString(R.string.custom_flow_return_url)
              )
              .build()
              .toString()

          Log.d(TAG, "Redirect Url with Payload: $redirectUrlWithPayload")
          binding.redirectBtn.setOnClickListener {
            FragmentUtil.getHost(this@CHIPLedgerDetailsFragment, Callback::class.java)
              ?.handleCustomFlowRedirectClicked(redirectUrlWithPayload)
          }

          // enable redirect button
          binding.redirectBtn.visibility = View.VISIBLE
        },
        { error ->
          Log.e(TAG, "Dcl request failed: $error")
          binding.commissioningFlowUrlTv.text =
            context!!.getString(R.string.chip_ledger_info_commissioning_flow_url_not_available)
        }
      )
    queue.add(jsonObjectRequest)

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  /** Interface for notifying the host. */
  interface Callback {
    /** Notifies listener of Custom flow redirect button click. */
    fun handleCustomFlowRedirectClicked(redirectUrl: String)
  }

  companion object {
    private const val TAG = "CUSTOM_FLOW"
    private const val ARG_DEVICE_INFO = "device_info"

    @JvmStatic
    fun newInstance(deviceInfo: CHIPDeviceInfo): CHIPLedgerDetailsFragment {
      return CHIPLedgerDetailsFragment().apply {
        arguments = Bundle(1).apply { putParcelable(ARG_DEVICE_INFO, deviceInfo) }
      }
    }
  }
}
