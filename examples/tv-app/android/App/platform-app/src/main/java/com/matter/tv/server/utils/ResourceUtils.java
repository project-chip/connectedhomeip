package com.matter.tv.server.utils;

import android.content.res.Resources;
import android.util.JsonReader;
import android.util.Log;
import com.matter.tv.app.api.SupportedCluster;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/** Util class for dealing with android Resources. */
public class ResourceUtils {
  private static final String TAG = "ResourceUtils";
  private static final ResourceUtils resourceUtils = new ResourceUtils();
  private static final String KEY_CLUSTERS = "clusters";
  private static final String KEY_CLUSTER_ID = "identifier";
  private static final String KEY_FEATURES = "features";
  private static final String KEY_FEATURE_FLAGS = "featureFlags";
  private static final String KEY_OPTIONAL_COMMANDS = "optionalCommands";
  private static final String KEY_OPTIONAL_ATTRIBUTES = "optionalAttributes";

  private ResourceUtils() {}

  public static ResourceUtils getInstance() {
    return resourceUtils;
  }

  /**
   * { "clusters": [ { "identifier": 1234, "features": ["CS"] }, { "identifier": 1235, "features":
   * ["AS"], "optionalCommands" : [4, 5] } ] }
   */

  /**
   * Attempts to open the resource given by the resourceId as a raw resource and return its contents
   * as a string.
   *
   * @param resources Resources object containing the resource
   * @param resId resourceId
   * @return Set<SupportedCluster> containing the clusters defined in the resource file. Empty set
   *     if there is a parsing error.
   */
  public Set<SupportedCluster> getSupportedClusters(final Resources resources, final int resId) {
    Set<SupportedCluster> supportedClusters = new HashSet<>();

    // openRawResource cannot return null, it either succeeds or throws an exception
    try (JsonReader reader =
        new JsonReader(new InputStreamReader(resources.openRawResource(resId)))) {

      reader.beginObject();

      if (reader.hasNext() && reader.nextName().equals(KEY_CLUSTERS)) {
        reader.beginArray();
        while (reader.hasNext()) {
          reader.beginObject();
          SupportedCluster cluster = new SupportedCluster();
          while (reader.hasNext()) {
            String name = reader.nextName();
            if (name.equals(KEY_CLUSTER_ID)) {
              cluster.clusterIdentifier = reader.nextInt();
            } else if (name.equals(KEY_FEATURE_FLAGS)) {
              cluster.features = reader.nextInt();
            } else if (name.equals(KEY_OPTIONAL_COMMANDS)) {
              List<Integer> commands = new ArrayList<>();
              reader.beginArray();
              while (reader.hasNext()) {
                commands.add(reader.nextInt());
              }
              reader.endArray();
              int[] commandIds = new int[commands.size()];
              int i = 0;
              for (Integer command : commands) {
                commandIds[i++] = command;
              }
              cluster.optionalCommandIdentifiers = commandIds;
            } else if (name.equals(KEY_OPTIONAL_ATTRIBUTES)) {
              List<Integer> attributes = new ArrayList<>();
              reader.beginArray();
              while (reader.hasNext()) {
                attributes.add(reader.nextInt());
              }
              reader.endArray();
              int[] attributeIds = new int[attributes.size()];
              int i = 0;
              for (Integer command : attributes) {
                attributeIds[i++] = command;
              }
              cluster.optionalAttributesIdentifiers = attributeIds;
            } else {
              reader.skipValue();
            }
          }
          supportedClusters.add(cluster);
          reader.endObject();
        }
        reader.endArray();
      } else {
        Log.e(TAG, "Invalid resource (Key:'clusters' not found) for id " + resId);
      }
      reader.endObject();
    } catch (IOException e) {
      Log.e(TAG, "Error reading raw resource for id " + resId);
    } catch (Resources.NotFoundException e) {
      Log.e(TAG, "Could not find raw resource for id " + resId);
    }
    return supportedClusters;
  }
}
