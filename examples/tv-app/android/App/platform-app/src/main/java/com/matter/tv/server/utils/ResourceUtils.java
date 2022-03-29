package com.matter.tv.server.utils;

import android.content.res.Resources;
import android.util.Log;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/** Util class for dealing with android Resources. */
public class ResourceUtils {
  private static final String TAG = "ResourceUtils";
  private static final ResourceUtils resourceUtils = new ResourceUtils();

  private ResourceUtils() {}

  public static ResourceUtils getInstance() {
    return resourceUtils;
  }

  /**
   * Attempts to open the resource given by the resourceId as a raw resource and return its contents
   * as a string.
   *
   * @param resources Resources object containing the resource
   * @param resId resourceId
   * @return String containing the contents of the resource file. Null if an error occurred.
   */
  public String getRawTextResource(final Resources resources, final int resId) {
    // TODO: Enforce some size limit on raw resource text? We don't want to
    // potentially take a long time reading an image file or something like that.

    // openRawResource cannot return null, it either succeeds or throws an exception
    try (BufferedReader reader =
        new BufferedReader(new InputStreamReader(resources.openRawResource(resId)))) {

      String line;
      StringBuilder result = new StringBuilder();

      while ((line = reader.readLine()) != null) {
        result.append(line).append('\n');
      }
      return result.toString();
    } catch (IOException e) {
      Log.e(TAG, "Error reading raw resource for id " + resId);
    } catch (Resources.NotFoundException e) {
      Log.e(TAG, "Could not find raw resource for id " + resId);
    }

    return null;
  }
}
