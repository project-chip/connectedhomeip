package com.matter.casting;

import android.util.Log;
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.core.Endpoint;
import java.util.List;

/** A utility that selects an endpoint based on some criterion */
public class EndpointSelectorExample {
  private static final String TAG = EndpointSelectorExample.class.getSimpleName();
  private static final Integer SAMPLE_ENDPOINT_VID = 65521;

  /**
   * Returns the first Endpoint in the list of Endpoints associated with the selectedCastingPlayer
   * whose VendorID matches the EndpointSelectorExample.SAMPLE_ENDPOINT_VID
   */
  public static Endpoint selectFirstEndpointByVID(CastingPlayer selectedCastingPlayer) {
    Endpoint endpoint = null;
    if (selectedCastingPlayer != null) {
      List<Endpoint> endpoints = selectedCastingPlayer.getEndpoints();
      if (endpoints == null) {
        Log.e(TAG, "selectFirstEndpointByVID() No Endpoints found on CastingPlayer");
      } else {
        endpoint =
            endpoints
                .stream()
                .filter(e -> SAMPLE_ENDPOINT_VID.equals(e.getVendorId()))
                .findFirst()
                .orElse(null);
      }
    }
    return endpoint;
  }

  /**
   * Returns the Endpoint with the desired endpoint Id in the list of Endpoints associated with the
   * selectedCastingPlayer.
   */
  public static Endpoint selectEndpointById(
      CastingPlayer selectedCastingPlayer, int desiredEndpointId) {
    Endpoint endpoint = null;
    if (selectedCastingPlayer != null) {
      List<Endpoint> endpoints = selectedCastingPlayer.getEndpoints();
      if (endpoints == null) {
        Log.e(TAG, "selectEndpointById() No Endpoints found on CastingPlayer");
      } else {
        endpoint =
            endpoints.stream().filter(e -> desiredEndpointId == e.getId()).findFirst().orElse(null);
      }
    }
    return endpoint;
  }
}
