package com.matter.tv.server.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.JsonReader;
import android.util.JsonWriter;
import android.util.Log;
import com.matter.tv.app.api.SupportedCluster;
import com.matter.tv.server.model.ContentApp;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class EndpointsDataStore {

  private static final String MATTER_APPPLATFORM_ENDPOINTS_PREF = "matter.appplatform.endpoints";
  private static final String KEY_NAME = "N";
  private static final String KEY_VENDORID = "VID";
  private static final String KEY_VENDORNAME = "VN";
  private static final String KEY_PRODUCTID = "PID";
  private static final String KEY_VERSION = "VER";
  private static final String KEY_ENDPOINTID = "EPID";
  private static final String KEY_SUPPORTED_CLUSTERS = "supportedClusters";
  private static final String KEY_CLUSTER_IDENTIFIER = "clusterIdentifier";
  private static final String KEY_FEATURES = "features";
  private static final String KEY_OPTIONAL_COMMAND_IDENTIFIERS = "optionalCommandIdentifiers";
  private static final String KEY_OPTIONAL_ATTRIBUTES_IDENTIFIERS = "optionalAttributesIdentifiers";
  private static EndpointsDataStore instance;
  private final SharedPreferences discoveredEndpoints;
  Map<String, ContentApp> persistedContentApps = new HashMap<>();

  private EndpointsDataStore(Context context) {
    discoveredEndpoints =
        context.getSharedPreferences(MATTER_APPPLATFORM_ENDPOINTS_PREF, Context.MODE_PRIVATE);
    // Take care of versioning when required
    Map<String, String> appMetadata = (Map<String, String>) discoveredEndpoints.getAll();
    for (Map.Entry<String, String> contentAppEntry : appMetadata.entrySet()) {
      persistedContentApps.put(
          contentAppEntry.getKey(),
          deserializeContentApp(contentAppEntry.getKey(), contentAppEntry.getValue()));
    }
  }

  public static synchronized EndpointsDataStore getInstance(Context context) {
    if (instance == null) {
      instance = new EndpointsDataStore(context);
    }
    return instance;
  }

  public Map<String, ContentApp> getAllPersistedContentApps() {
    return Collections.unmodifiableMap(persistedContentApps);
  }

  public void persistContentAppEndpoint(ContentApp app) {
    Log.i(EndpointsDataStore.class.toString(), "Persist Content App Endpoint " + app.getAppName());
    persistedContentApps.put(app.getAppName(), app);
    discoveredEndpoints.edit().putString(app.getAppName(), serializeContentApp(app)).apply();
  }

  private String serializeContentApp(ContentApp app) {
    StringWriter stringWriter = new StringWriter();
    JsonWriter jsonWriter = new JsonWriter(stringWriter);
    try {
      jsonWriter.beginObject();
      jsonWriter.name(KEY_VENDORID);
      jsonWriter.value(app.getVendorId());
      jsonWriter.name(KEY_VENDORNAME);
      jsonWriter.value(app.getVendorName());
      jsonWriter.name(KEY_PRODUCTID);
      jsonWriter.value(app.getProductId());
      jsonWriter.name(KEY_VERSION);
      jsonWriter.value(app.getVersion());
      jsonWriter.name(KEY_ENDPOINTID);
      jsonWriter.value(app.getEndpointId());
      jsonWriter.name(KEY_SUPPORTED_CLUSTERS);
      serializeSupportedClusters(jsonWriter, app.getSupportedClusters());
      jsonWriter.endObject();
      jsonWriter.flush();
      jsonWriter.close();
    } catch (IOException e) {
      // Cannot happen
    }
    return stringWriter.toString();
  }

  private ContentApp deserializeContentApp(String appName, String appMetadata) {
    JsonReader jsonReader = new JsonReader(new StringReader(appMetadata));
    ContentApp app = null;
    try {
      jsonReader.beginObject();
      String version = "";
      String vendorName = "";
      int vendorId = 0;
      int productId = 0;
      int endpoint = ContentApp.INVALID_ENDPOINTID;
      Set<SupportedCluster> supportedClusters = new HashSet<>();
      while (jsonReader.hasNext()) {
        String name = jsonReader.nextName();
        switch (name) {
          case KEY_VENDORID:
            vendorId = jsonReader.nextInt();
            break;
          case KEY_VENDORNAME:
            vendorName = jsonReader.nextString();
            break;
          case KEY_PRODUCTID:
            productId = jsonReader.nextInt();
            break;
          case KEY_VERSION:
            version = jsonReader.nextString();
            break;
          case KEY_ENDPOINTID:
            endpoint = jsonReader.nextInt();
            break;
          case KEY_SUPPORTED_CLUSTERS:
            supportedClusters = deserializeSupportedClusters(jsonReader);
            break;
        }
      }
      app = new ContentApp(appName, vendorName, vendorId, productId, version, supportedClusters);
      jsonReader.endObject();
      jsonReader.close();
    } catch (IOException e) {
      // Cannot happen
    }
    return app;
  }

  private void serializeSupportedClusters(
      JsonWriter jsonWriter, Set<SupportedCluster> supportedClusters) throws IOException {
    if (supportedClusters != null) {
      jsonWriter.beginArray();
      for (SupportedCluster supportedCluster : supportedClusters) {
        if (supportedCluster != null) {
          serializeSupportedCluster(jsonWriter, supportedCluster);
        }
      }
      jsonWriter.endArray();
    }
  }

  private Set<SupportedCluster> deserializeSupportedClusters(JsonReader jsonReader)
      throws IOException {
    Set<SupportedCluster> supportedClusters = new HashSet<>();
    jsonReader.beginArray();
    while (jsonReader.hasNext()) {
      supportedClusters.add(deserializeSupportedCluster(jsonReader));
    }
    jsonReader.endArray();
    return supportedClusters;
  }

  private void serializeSupportedCluster(JsonWriter jsonWriter, SupportedCluster supportedCluster)
      throws IOException {
    jsonWriter.beginObject();
    jsonWriter.name(KEY_CLUSTER_IDENTIFIER);
    jsonWriter.value(supportedCluster.clusterIdentifier);
    jsonWriter.name(KEY_FEATURES);
    jsonWriter.value(supportedCluster.features);
    jsonWriter.name(KEY_OPTIONAL_COMMAND_IDENTIFIERS);
    serializeIntArray(jsonWriter, supportedCluster.optionalCommandIdentifiers);
    jsonWriter.name(KEY_OPTIONAL_ATTRIBUTES_IDENTIFIERS);
    serializeIntArray(jsonWriter, supportedCluster.optionalAttributesIdentifiers);
    jsonWriter.endObject();
  }

  private SupportedCluster deserializeSupportedCluster(JsonReader jsonReader) throws IOException {
    SupportedCluster supportedCluster = new SupportedCluster();
    jsonReader.beginObject();
    while (jsonReader.hasNext()) {
      String name = jsonReader.nextName();
      switch (name) {
        case KEY_CLUSTER_IDENTIFIER:
          supportedCluster.clusterIdentifier = jsonReader.nextInt();
          break;
        case KEY_FEATURES:
          supportedCluster.features = jsonReader.nextInt();
          break;
        case KEY_OPTIONAL_COMMAND_IDENTIFIERS:
          supportedCluster.optionalCommandIdentifiers = deserializeIntArray(jsonReader);
          break;
        case KEY_OPTIONAL_ATTRIBUTES_IDENTIFIERS:
          supportedCluster.optionalAttributesIdentifiers = deserializeIntArray(jsonReader);
          break;
      }
    }
    jsonReader.endObject();
    return supportedCluster;
  }

  private void serializeIntArray(JsonWriter jsonWriter, int[] array) throws IOException {
    jsonWriter.beginArray();
    if (array != null) {
      for (int value : array) {
        jsonWriter.value(value);
      }
    }
    jsonWriter.endArray();
  }

  private int[] deserializeIntArray(JsonReader jsonReader) throws IOException {
    List<Integer> dynamicArray = new ArrayList<>();
    jsonReader.beginArray();
    while (jsonReader.hasNext()) {
      dynamicArray.add(jsonReader.nextInt());
    }
    jsonReader.endArray();
    return dynamicArray.stream().mapToInt(Integer::intValue).toArray();
  }
}
