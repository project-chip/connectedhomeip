package com.matter.casting;

import android.util.Log;

import com.matter.casting.core.CastingPlayer;
import com.matter.casting.core.Endpoint;

import java.util.List;

/**
 * A utility that selects an endpoint based on some criterion
 */
public class EndpointSelectorExample {
    private static final String TAG = EndpointSelectorExample.class.getSimpleName();
    private static final Integer SAMPLE_ENDPOINT_VID = 65521;

    public static Endpoint selectEndpointByVID(CastingPlayer selectedCastingPlayer) {
        Endpoint endpoint = null;
        if (selectedCastingPlayer != null) {
            List<Endpoint> endpoints = selectedCastingPlayer.getEndpoints();
            if (endpoints == null) {
                Log.e(TAG, "No Endpoints found on CastingPlayer");
            } else {
                endpoint =
                        endpoints
                                .stream()
                                .filter(e -> SAMPLE_ENDPOINT_VID.equals(e.getVendorId()))
                                .findFirst()
                                .get();
            }
        }
        return endpoint;
    }
}
