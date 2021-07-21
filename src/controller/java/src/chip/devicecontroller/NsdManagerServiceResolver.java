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
package chip.devicecontroller;

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;

public class NsdManagerServiceResolver implements ServiceResolver
{
  private final String TAG = NsdManagerServiceResolver.class.getSimpleName();
  private final NsdManager nsdManager;

  public NsdManagerServiceResolver(Context context) {
    this.nsdManager = (NsdManager) context.getSystemService(Context.NSD_SERVICE);
  }

  @Override
  public void resolve(final String instanceName, final String serviceType, final long callbackHandle, final long contextHandle) {
    NsdServiceInfo serviceInfo = new NsdServiceInfo();
    serviceInfo.setServiceName(instanceName);
    serviceInfo.setServiceType(serviceType);

    this.nsdManager.resolveService(serviceInfo, new NsdManager.ResolveListener() {
      @Override
      public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
        Log.w(TAG, "Failed to resolve service '" + serviceInfo.getServiceName() + "': " + errorCode);
        ChipDeviceController.handleServiceResolve(
            instanceName,
            serviceType,
            null,
            0,
            callbackHandle,
            contextHandle);
      }

      @Override
      public void onServiceResolved(NsdServiceInfo serviceInfo) {
        Log.i(TAG, "Resolved service '" + serviceInfo.getServiceName() + "' to " + serviceInfo.getHost());
        ChipDeviceController.handleServiceResolve(
            instanceName,
            serviceType,
            serviceInfo.getHost().getHostAddress(),
            serviceInfo.getPort(),
            callbackHandle,
            contextHandle);
      }
    });
  }
}
