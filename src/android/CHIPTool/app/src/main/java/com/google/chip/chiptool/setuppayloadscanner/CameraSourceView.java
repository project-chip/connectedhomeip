/*
 *   Copyright (c) 2020 Project CHIP Authors
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

package com.google.chip.chiptool.setuppayloadscanner;

import android.Manifest;
import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup;
import androidx.annotation.RequiresPermission;
import com.google.android.gms.vision.CameraSource;
import java.io.IOException;

/** Class that displays the camera view. */
public class CameraSourceView extends ViewGroup {

  private static final String TAG = CameraSourceView.class.getSimpleName();

  private SurfaceView surfaceView;
  private boolean startRequested;
  private boolean surfaceAvailable;
  private CameraSource cameraSource;

  public CameraSourceView(Context context, AttributeSet attrs) {
    super(context, attrs);

    surfaceView = new SurfaceView(context);
    surfaceView.getHolder().addCallback(new SurfaceCallback());
    addView(surfaceView);
  }

  /**
   * Attempts to start the camera source.
   *
   * @param cameraSource the camera source
   * @throws IOException if the camera doesn't work
   * @throws SecurityException if app doesn't have permission to access camera
   */
  @RequiresPermission(Manifest.permission.CAMERA)
  public void start(CameraSource cameraSource) throws IOException, SecurityException {
    if (cameraSource == null) {
      stop();
    }

    this.cameraSource = cameraSource;

    if (this.cameraSource != null) {
      startRequested = true;
      startIfReady();
    }
  }

  /** Stops the camera source. */
  public void stop() {
    if (cameraSource != null) {
      cameraSource.stop();
    }
  }

  /** Releases the camera source. */
  public void release() {
    if (cameraSource != null) {
      cameraSource.release();
      cameraSource = null;
    }
  }

  @RequiresPermission(Manifest.permission.CAMERA)
  @Override
  protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
    final int layoutWidth = right - left;
    final int layoutHeight = bottom - top;

    for (int position = 0; position < getChildCount(); ++position) {
      getChildAt(position).layout(0, 0, layoutWidth, layoutHeight);
    }

    startIfReady();
  }

  @RequiresPermission(Manifest.permission.CAMERA)
  private void startIfReady() {
    try {
      if (startRequested && surfaceAvailable) {
        cameraSource.start(surfaceView.getHolder());
        startRequested = false;
      }
    } catch (SecurityException se) {
      Log.e(TAG, "Do not have permission to start the camera", se);
    } catch (IOException ioe) {
      Log.e(TAG, "Could not start camera source.", ioe);
    }
  }

  /** Class to handle surface traits and availability */
  private class SurfaceCallback implements SurfaceHolder.Callback {

    @RequiresPermission(Manifest.permission.CAMERA)
    @Override
    public void surfaceCreated(SurfaceHolder surface) {
      surfaceAvailable = true;
      startIfReady();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surface) {
      surfaceAvailable = false;
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {}
  }
}
