package com.matter.tv.server.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.JsonReader;
import android.util.JsonWriter;
import com.matter.tv.server.model.ContentApp;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class EndpointsDataStore {

  private static final String MATTER_APPPLATFORM_ENDPOINTS_PREF = "matter.appplatform.endpoints";
  private static final String KEY_NAME = "N";
  private static final String KEY_VENDORID = "VID";
  private static final String KEY_VENDORNAME = "VN";
  private static final String KEY_PRODUCTID = "PID";
  private static final String KEY_VERSION = "VER";
  private static final String KEY_ENDPOINTID = "EPID";
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
    persistedContentApps.put(app.getAppName(), app);
    discoveredEndpoints.edit().putString(app.getAppName(), serializeContentApp(app)).apply();
  }

  private String serializeContentApp(ContentApp app) {
    StringWriter stringWriter = new StringWriter();
    JsonWriter jsonWriter = new JsonWriter(stringWriter);
    try {
      jsonWriter
          .beginObject()
          .name(KEY_VENDORID)
          .value(app.getVendorId())
          .name(KEY_VENDORNAME)
          .value(app.getVendorName())
          .name(KEY_PRODUCTID)
          .value(app.getProductId())
          .name(KEY_VERSION)
          .value(app.getVersion())
          .name(KEY_ENDPOINTID)
          .value(app.getEndpointId())
          .endObject();
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
        }
      }
      app = new ContentApp(appName, vendorName, vendorId, productId, version);
      jsonReader.endObject();
      jsonReader.close();
    } catch (IOException e) {
      // Cannot happen
    }
    return app;
  }
}
