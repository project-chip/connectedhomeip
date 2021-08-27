package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipClusters
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.DeviceIdUtil
import com.jjoe64.graphview.LabelFormatter
import com.jjoe64.graphview.Viewport
import com.jjoe64.graphview.series.DataPoint
import com.jjoe64.graphview.series.LineGraphSeries
import kotlinx.android.synthetic.main.sensor_client_fragment.*
import kotlinx.android.synthetic.main.sensor_client_fragment.view.*
import kotlinx.coroutines.*
import java.text.SimpleDateFormat
import java.util.*

private typealias ReadCallback = ChipClusters.IntegerAttributeCallback

class SensorClientFragment : Fragment() {
  private val scope = CoroutineScope(Dispatchers.Main + Job())

  // Job for sending periodic sensor read requests
  private var sensorWatchJob: Job? = null

  // History of sensor values
  private val sensorData = LineGraphSeries<DataPoint>()

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.sensor_client_fragment, container, false).apply {
      clusterNameSpinner.adapter = makeClusterNamesAdapter()
      clusterNameSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
        override fun onNothingSelected(parent: AdapterView<*>?) = Unit
        override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
          resetSensorGraph() // reset the graph on cluster change
        }
      }
      readSensorBtn.setOnClickListener { scope.launch { readSensorButtonClick() } }
      watchSensorBtn.setOnCheckedChangeListener { _, isChecked ->
        if (isChecked) {
          watchSensorButtonChecked()
        } else {
          watchSensorButtonUnchecked()
        }
      }
      val currentTime = Calendar.getInstance().time.time
      sensorGraph.addSeries(sensorData)
      sensorGraph.viewport.isXAxisBoundsManual = true
      sensorGraph.viewport.setMinX(currentTime.toDouble())
      sensorGraph.viewport.setMaxX(currentTime.toDouble() + REFRESH_PERIOD_MS * MAX_DATA_POINTS)
      sensorGraph.gridLabelRenderer.padding = 20
      sensorGraph.gridLabelRenderer.numHorizontalLabels = 4
      sensorGraph.gridLabelRenderer.setHorizontalLabelsAngle(150)
      sensorGraph.gridLabelRenderer.labelFormatter = object : LabelFormatter {
        override fun setViewport(viewport: Viewport?) = Unit
        override fun formatLabel(value: Double, isValueX: Boolean): String {
          if (!isValueX)
            return "%.2f".format(value)
          return SimpleDateFormat("H:mm:ss").format(Date(value.toLong())).toString()
        }
      }
    }
  }

  override fun onStart() {
    super.onStart()
    deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
  }

  override fun onStop() {
    super.onStop()
    scope.cancel()
    resetSensorGraph() // reset the graph on fragment exit
  }

  private fun resetSensorGraph() {
    watchSensorBtn.isChecked = false
    sensorGraph.visibility = View.INVISIBLE
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

  private suspend fun readSensorButtonClick() {
    try {
      readSensorCluster(clusterNameSpinner.selectedItem.toString(), false)
    } catch (ex: Exception) {
      showMessage(R.string.sensor_client_read_error_text, ex.toString())
    }
  }

  private fun watchSensorButtonChecked() {
    sensorWatchJob = scope.launch {
      while (isActive) {
        try {
          readSensorCluster(clusterNameSpinner.selectedItem.toString(), true)
        } catch (ex: Exception) {
          showMessage(R.string.sensor_client_read_error_text, ex.toString())
        }
        delay(REFRESH_PERIOD_MS)
      }
    }
  }

  private fun watchSensorButtonUnchecked() {
    sensorWatchJob?.cancel()
    sensorWatchJob = null
  }

  private suspend fun readSensorCluster(clusterName: String, addToGraph: Boolean) {
    val deviceId = deviceIdEd.text.toString().toULong().toLong()
    val endpointId = endpointIdEd.text.toString().toInt()
    val clusterConfig = CLUSTERS[clusterName]
    val clusterRead = clusterConfig!!["read"] as (Long, Int, ReadCallback) -> Unit

    val device = ChipClient.getConnectedDevicePointer(requireContext(), deviceId)

    clusterRead(device, endpointId, object : ReadCallback {
      override fun onSuccess(value: Int) {
        val unitValue = clusterConfig["unitValue"] as Double
        val unitSymbol = clusterConfig["unitSymbol"] as String
        consumeSensorValue(value * unitValue, unitSymbol, addToGraph)
      }

      override fun onError(ex: Exception) {
        showMessage(R.string.sensor_client_read_error_text, ex.toString())
      }
    })
  }

  private fun consumeSensorValue(value: Double, unitSymbol: String, addToGraph: Boolean) {
    requireActivity().runOnUiThread {
      lastValueTv.text = requireContext().getString(
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
          sensorGraph.visibility = View.VISIBLE
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
    private const val REFRESH_PERIOD_MS = 3000L
    private const val MAX_DATA_POINTS = 60
    private val CLUSTERS = mapOf(
        "Temperature" to mapOf(
            "read" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.TemperatureMeasurementCluster(device, endpointId)
              cluster.readMeasuredValueAttribute(callback)
            },
            "unitValue" to 0.01,
            "unitSymbol" to "\u00B0C"
        ),
        "Pressure" to mapOf(
            "read" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.PressureMeasurementCluster(device, endpointId)
              cluster.readMeasuredValueAttribute(callback)
            },
            "unitValue" to 1.0,
            "unitSymbol" to "hPa"
        ),
        "Relative Humidity" to mapOf(
            "read" to { device: Long, endpointId: Int, callback: ReadCallback ->
              val cluster = ChipClusters.RelativeHumidityMeasurementCluster(device, endpointId)
              cluster.readMeasuredValueAttribute(callback)
            },
            "unitValue" to 0.01,
            "unitSymbol" to "%"
        )
    )

    fun newInstance(): SensorClientFragment = SensorClientFragment()
  }
}
