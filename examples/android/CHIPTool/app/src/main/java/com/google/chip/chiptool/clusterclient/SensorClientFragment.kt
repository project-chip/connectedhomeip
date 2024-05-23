package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.EditorInfo
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterIDMapping.*
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.NodeState
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.SensorClientFragmentBinding
import com.google.chip.chiptool.util.DeviceIdUtil
import com.jjoe64.graphview.LabelFormatter
import com.jjoe64.graphview.Viewport
import com.jjoe64.graphview.series.DataPoint
import com.jjoe64.graphview.series.LineGraphSeries
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader

class SensorClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  // History of sensor values
  private val sensorData = LineGraphSeries<DataPoint>()

  // Device whose attribute is subscribed
  private var subscribedDevicePtr = 0L

  private var _binding: SensorClientFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = SensorClientFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    ChipClient.getDeviceController(requireContext()).setCompletionListener(null)
    binding.deviceIdEd.setOnEditorActionListener { textView, actionId, _ ->
      if (actionId == EditorInfo.IME_ACTION_DONE) {
        resetSensorGraph() // reset the graph on device change
      }
      actionId == EditorInfo.IME_ACTION_DONE
    }

    binding.endpointIdEd.setOnEditorActionListener { textView, actionId, _ ->
      if (actionId == EditorInfo.IME_ACTION_DONE)
        resetSensorGraph() // reset the graph on endpoint change
      actionId == EditorInfo.IME_ACTION_DONE
    }

    binding.clusterNameSpinner.adapter = makeClusterNamesAdapter()
    binding.clusterNameSpinner.onItemSelectedListener =
      object : AdapterView.OnItemSelectedListener {
        override fun onNothingSelected(parent: AdapterView<*>?) = Unit

        override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
          resetSensorGraph() // reset the graph on cluster change
        }
      }

    binding.readSensorBtn.setOnClickListener { scope.launch { readSensorCluster() } }
    binding.watchSensorBtn.setOnCheckedChangeListener { _, isChecked ->
      if (isChecked) {
        scope.launch { subscribeSensorCluster() }
      } else {
        unsubscribeSensorCluster()
      }
    }

    val currentTime = Calendar.getInstance().time.time
    binding.sensorGraph.addSeries(sensorData)
    binding.sensorGraph.viewport.isXAxisBoundsManual = true
    binding.sensorGraph.viewport.setMinX(currentTime.toDouble())
    binding.sensorGraph.viewport.setMaxX(
      currentTime.toDouble() + MIN_REFRESH_PERIOD_S * 1000 * MAX_DATA_POINTS
    )
    binding.sensorGraph.gridLabelRenderer.padding = 30
    binding.sensorGraph.gridLabelRenderer.numHorizontalLabels = 4
    binding.sensorGraph.gridLabelRenderer.setHorizontalLabelsAngle(150)
    binding.sensorGraph.gridLabelRenderer.labelFormatter =
      object : LabelFormatter {
        override fun setViewport(viewport: Viewport?) = Unit

        override fun formatLabel(value: Double, isValueX: Boolean): String {
          if (isValueX) return SimpleDateFormat("H:mm:ss").format(Date(value.toLong())).toString()
          if (value >= 100.0) return "%.1f".format(value)
          return "%.2f".format(value)
        }
      }
  }

  override fun onStart() {
    super.onStart()
    binding.deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
  }

  override fun onStop() {
    resetSensorGraph() // reset the graph on fragment exit
    super.onStop()
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private fun resetSensorGraph() {
    binding.watchSensorBtn.isChecked = false
    binding.sensorGraph.visibility = View.INVISIBLE
    sensorData.resetData(emptyArray())
  }

  private fun makeClusterNamesAdapter(): ArrayAdapter<String> {
    return ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        CLUSTERS.keys.toList()
      )
      .apply { setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item) }
  }

  private suspend fun readSensorCluster() {
    try {
      val deviceId = binding.deviceIdEd.text.toString().toULong().toLong()
      val endpointId = binding.endpointIdEd.text.toString().toInt()
      val clusterName = binding.clusterNameSpinner.selectedItem.toString()
      val clusterId = CLUSTERS[clusterName]!!["clusterId"] as Long
      val attributeId = CLUSTERS[clusterName]!!["attributeId"] as Long
      val device =
        try {
          ChipClient.getConnectedDevicePointer(requireContext(), deviceId)
        } catch (e: IllegalStateException) {
          Log.d(TAG, "getConnectedDevicePointer exception", e)
          return
        }
      val callback = makeReadCallback(clusterName, false)

      deviceController.readAttributePath(
        callback,
        device,
        listOf(ChipAttributePath.newInstance(endpointId, clusterId, attributeId)),
        0
      )
    } catch (ex: Exception) {
      Log.d(TAG, "Failed to read the sensor : ", ex)
      showMessage(R.string.sensor_client_read_error_text, ex.toString())
    }
  }

  private suspend fun subscribeSensorCluster() {
    try {
      val deviceId = binding.deviceIdEd.text.toString().toULong().toLong()
      val endpointId = binding.endpointIdEd.text.toString().toInt()
      val clusterName = binding.clusterNameSpinner.selectedItem.toString()
      val clusterId = CLUSTERS[clusterName]!!["clusterId"] as Long
      val attributeId = CLUSTERS[clusterName]!!["attributeId"] as Long
      val device =
        try {
          ChipClient.getConnectedDevicePointer(requireContext(), deviceId)
        } catch (e: IllegalStateException) {
          Log.d(TAG, "getConnectedDevicePointer exception", e)
          return
        }
      val callback = makeReadCallback(clusterName, true)

      deviceController.subscribeToAttributePath(
        { Log.d(TAG, "onSubscriptionEstablished") },
        callback,
        device,
        listOf(ChipAttributePath.newInstance(endpointId, clusterId, attributeId)),
        MIN_REFRESH_PERIOD_S,
        MAX_REFRESH_PERIOD_S,
        0
      )
      subscribedDevicePtr = device
    } catch (ex: Exception) {
      Log.d(TAG, "Failed to subscribe", ex)
      showMessage(R.string.sensor_client_subscribe_error_text, ex.toString())
    }
  }

  private fun unsubscribeSensorCluster() {
    if (subscribedDevicePtr == 0L) return

    try {
      ChipClient.getDeviceController(requireContext()).shutdownSubscriptions()
      subscribedDevicePtr = 0
    } catch (ex: Exception) {
      showMessage(R.string.sensor_client_unsubscribe_error_text, ex.toString())
    }
  }

  private fun makeReadCallback(clusterName: String, addToGraph: Boolean): ReportCallback {
    return object : ReportCallback {
      val clusterConfig = CLUSTERS[clusterName]!!
      val endpointId = binding.endpointIdEd.text.toString().toInt()
      val clusterId = clusterConfig["clusterId"] as Long
      val attributeId = clusterConfig["attributeId"] as Long

      override fun onReport(nodeState: NodeState?) {
        val tlv =
          nodeState
            ?.getEndpointState(endpointId)
            ?.getClusterState(clusterId)
            ?.getAttributeState(attributeId)
            ?.tlv
            ?: return
        // TODO : Need to be implement poj-to-tlv
        val value =
          try {
            TlvReader(tlv).getInt(AnonymousTag)
          } catch (ex: Exception) {
            showMessage(R.string.sensor_client_read_error_text, "value is null")
            return
          }
        val unitValue = clusterConfig["unitValue"] as Double
        val unitSymbol = clusterConfig["unitSymbol"] as String
        consumeSensorValue(value * unitValue, unitSymbol, addToGraph)
      }

      override fun onError(
        attributePath: ChipAttributePath?,
        eventPath: ChipEventPath?,
        ex: Exception
      ) {
        showMessage(R.string.sensor_client_read_error_text, ex.toString())
      }
    }
  }

  private fun consumeSensorValue(value: Double, unitSymbol: String, addToGraph: Boolean) {
    requireActivity().runOnUiThread {
      binding.lastValueTv.text =
        requireContext().getString(R.string.sensor_client_last_value_text, value, unitSymbol)

      if (addToGraph) {
        val isFirstSample = sensorData.isEmpty
        val dataPoint = DataPoint(Calendar.getInstance().time, value)
        sensorData.appendData(dataPoint, true, MAX_DATA_POINTS)
        if (isFirstSample) {
          // Make the graph visible on the first sample. Also, workaround a bug in
          // graphview
          // related to calculating the viewport when there is only one data point by
          // duplicating the first sample.
          sensorData.appendData(dataPoint, true, MAX_DATA_POINTS)
          binding.sensorGraph.visibility = View.VISIBLE
        }
      }
    }
  }

  private fun showMessage(msgResId: Int, stringArgs: String? = null) {
    requireActivity().runOnUiThread {
      val context = requireContext()
      val message = context.getString(msgResId, stringArgs)
      Toast.makeText(context, message, Toast.LENGTH_SHORT).show()
      Log.i(TAG, message)
    }
  }

  companion object {
    private const val TAG = "SensorClientFragment"
    private const val MIN_REFRESH_PERIOD_S = 2
    private const val MAX_REFRESH_PERIOD_S = 10
    private const val MAX_DATA_POINTS = 60
    private val CLUSTERS =
      mapOf(
        "Temperature" to
          mapOf(
            "clusterId" to TemperatureMeasurement.ID,
            "attributeId" to TemperatureMeasurement.Attribute.MeasuredValue.id,
            "unitValue" to 0.01,
            "unitSymbol" to "\u00B0C"
          ),
        "Pressure" to
          mapOf(
            "clusterId" to PressureMeasurement.ID,
            "attributeId" to PressureMeasurement.Attribute.MeasuredValue.id,
            "unitValue" to 1.0,
            "unitSymbol" to "hPa"
          ),
        "Relative Humidity" to
          mapOf(
            "clusterId" to RelativeHumidityMeasurement.ID,
            "attributeId" to RelativeHumidityMeasurement.Attribute.MeasuredValue.id,
            "unitValue" to 0.01,
            "unitSymbol" to "%"
          )
      )

    fun newInstance(): SensorClientFragment = SensorClientFragment()
  }
}
