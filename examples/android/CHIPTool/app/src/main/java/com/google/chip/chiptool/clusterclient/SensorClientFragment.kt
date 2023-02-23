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
import chip.devicecontroller.ChipClusters
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.AttestationTestFragmentBinding
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

private typealias ReadCallback = ChipClusters.IntegerAttributeCallback
private typealias TemperatureReadCallback =
    ChipClusters.TemperatureMeasurementCluster.MeasuredValueAttributeCallback

private typealias PressureReadCallback =
    ChipClusters.PressureMeasurementCluster.MeasuredValueAttributeCallback

private typealias RelativeHumidityReadCallback =
    ChipClusters.RelativeHumidityMeasurementCluster.MeasuredValueAttributeCallback

class SensorClientFragment : Fragment() {
  private lateinit var scope: CoroutineScope

  // History of sensor values
  private val sensorData = LineGraphSeries<DataPoint>()

  // Device whose attribute is subscribed
  private var subscribedDevicePtr = 0L

  private var _binding: SensorClientFragmentBinding? = null
  private val binding get() = _binding!!

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
    binding.clusterNameSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
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
    binding.sensorGraph.viewport.setMaxX(currentTime.toDouble() + MIN_REFRESH_PERIOD_S * 1000 * MAX_DATA_POINTS)
    binding.sensorGraph.gridLabelRenderer.padding = 30
    binding.sensorGraph.gridLabelRenderer.numHorizontalLabels = 4
    binding.sensorGraph.gridLabelRenderer.setHorizontalLabelsAngle(150)
    binding.sensorGraph.gridLabelRenderer.labelFormatter = object : LabelFormatter {
      override fun setViewport(viewport: Viewport?) = Unit
      override fun formatLabel(value: Double, isValueX: Boolean): String {
        if (isValueX)
          return SimpleDateFormat("H:mm:ss").format(Date(value.toLong())).toString()
        if (value >= 100.0)
          return "%.1f".format(value)
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
    ).apply {
      setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
    }
  }

  private suspend fun readSensorCluster() {
    try {
      val deviceId = binding.deviceIdEd.text.toString().toULong().toLong()
      val endpointId = binding.endpointIdEd.text.toString().toInt()
      val clusterName = binding.clusterNameSpinner.selectedItem.toString()
      val clusterRead = CLUSTERS[clusterName]!!["read"] as (Long, Int, ReadCallback) -> Unit
      val device = ChipClient.getConnectedDevicePointer(requireContext(), deviceId)
      val callback = makeReadCallback(clusterName, false)

      clusterRead(device, endpointId, callback)
    } catch (ex: Exception) {
      showMessage(R.string.sensor_client_read_error_text, ex.toString())
    }
  }

  private suspend fun subscribeSensorCluster() {
    try {
      val deviceId = binding.deviceIdEd.text.toString().toULong().toLong()
      val endpointId = binding.endpointIdEd.text.toString().toInt()
      val clusterName = binding.clusterNameSpinner.selectedItem.toString()
      val clusterSubscribe = CLUSTERS[clusterName]!!["subscribe"] as (Long, Int, ReadCallback) -> Unit
      val device = ChipClient.getConnectedDevicePointer(requireContext(), deviceId)
      val callback = makeReadCallback(clusterName, true)

      clusterSubscribe(device, endpointId, callback)
      subscribedDevicePtr = device
    } catch (ex: Exception) {
      showMessage(R.string.sensor_client_subscribe_error_text, ex.toString())
    }
  }

  private fun unsubscribeSensorCluster() {
    if (subscribedDevicePtr == 0L)
      return

    try {
      ChipClient.getDeviceController(requireContext()).shutdownSubscriptions(subscribedDevicePtr)
      subscribedDevicePtr = 0
    } catch (ex: Exception) {
      showMessage(R.string.sensor_client_unsubscribe_error_text, ex.toString())
    }
  }

  private fun makeReadCallback(clusterName: String, addToGraph: Boolean): ReadCallback {
    return object : ReadCallback {
      val clusterConfig = CLUSTERS[clusterName]!!

      override fun onSuccess(value: Int) {
        val unitValue = clusterConfig["unitValue"] as Double
        val unitSymbol = clusterConfig["unitSymbol"] as String
        consumeSensorValue(value * unitValue, unitSymbol, addToGraph)
      }

      override fun onError(ex: Exception) {
        showMessage(R.string.sensor_client_read_error_text, ex.toString())
      }
    }
  }

  private fun consumeSensorValue(value: Double, unitSymbol: String, addToGraph: Boolean) {
    requireActivity().runOnUiThread {
      binding.lastValueTv.text = requireContext().getString(
          R.string.sensor_client_last_value_text, value, unitSymbol
      )

      if (addToGraph) {
        val isFirstSample = sensorData.isEmpty
        val dataPoint = DataPoint(Calendar.getInstance().time, value)
        sensorData.appendData(dataPoint, true, MAX_DATA_POINTS)
        if (isFirstSample) {
          // Make the graph visible on the first sample. Also, workaround a bug in graphview
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
    private val CLUSTERS = mapOf(
        "Temperature" to mapOf(
            "read" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.TemperatureMeasurementCluster(device, endpointId)
              cluster.readMeasuredValueAttribute(makeTemperatureReadCallback(callback))
            },
            "subscribe" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.TemperatureMeasurementCluster(device, endpointId)
              cluster.subscribeMeasuredValueAttribute(makeTemperatureReadCallback(callback),
                  MIN_REFRESH_PERIOD_S,
                  MAX_REFRESH_PERIOD_S)
            },
            "unitValue" to 0.01,
            "unitSymbol" to "\u00B0C"
        ),
        "Pressure" to mapOf(
            "read" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.PressureMeasurementCluster(device, endpointId)
              cluster.readMeasuredValueAttribute(makePressureReadCallback(callback))
            },
            "subscribe" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.PressureMeasurementCluster(device, endpointId)
              cluster.subscribeMeasuredValueAttribute(makePressureReadCallback(callback),
                  MIN_REFRESH_PERIOD_S,
                  MAX_REFRESH_PERIOD_S)
            },
            "unitValue" to 1.0,
            "unitSymbol" to "hPa"
        ),
        "Relative Humidity" to mapOf(
            "read" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.RelativeHumidityMeasurementCluster(device, endpointId)
              cluster.readMeasuredValueAttribute(makeHumidityReadCallback(callback))
            },
            "subscribe" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.RelativeHumidityMeasurementCluster(device, endpointId)
              cluster.subscribeMeasuredValueAttribute(makeHumidityReadCallback(callback),
                  MIN_REFRESH_PERIOD_S,
                  MAX_REFRESH_PERIOD_S)
            },
            "unitValue" to 0.01,
            "unitSymbol" to "%"
        )
    )

    private fun makeTemperatureReadCallback(callback: ReadCallback): TemperatureReadCallback {
      return object : TemperatureReadCallback {
        override fun onSuccess(value: Int?) {
          value?.let { callback.onSuccess(it) }
        }

        override fun onError(error: java.lang.Exception?) {
          callback.onError(error)
        }
      }
    }

    private fun makePressureReadCallback(callback: ReadCallback): PressureReadCallback {
      return object : PressureReadCallback {
        override fun onSuccess(value: Int?) {
          value?.let { callback.onSuccess(it) }
        }

        override fun onError(error: java.lang.Exception?) {
          callback.onError(error)
        }
      }
    }

    private fun makeHumidityReadCallback(callback: ReadCallback): RelativeHumidityReadCallback {
      return object : RelativeHumidityReadCallback {
        override fun onSuccess(value: Int?) {
          value?.let { callback.onSuccess(it) }
        }

        override fun onError(error: java.lang.Exception?) {
          callback.onError(error)
        }
      }
    }

    fun newInstance(): SensorClientFragment = SensorClientFragment()
  }
}
