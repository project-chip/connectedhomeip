package com.chip.casting.dnssd;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import com.chip.casting.app.CastingContext;
import com.chip.casting.util.GlobalCastingConstants;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

@RunWith(MockitoJUnitRunner.class)
public class CommissionerDiscoveryListenerTest {
  @Mock private CastingContext castingContext;

  @Mock private NsdServiceInfo nsdService;

  @Mock private NsdManager nsdManager;

  @Test
  public void onServiceFound_callsResolve_whenCommissionerDiscovered() {
    when(nsdService.getServiceType()).thenReturn(GlobalCastingConstants.CommissionerServiceType);
    when(castingContext.getNsdManager()).thenReturn(nsdManager);
    doNothing().when(nsdManager).resolveService(any(), any());

    CommissionerDiscoveryListener listener = new CommissionerDiscoveryListener(castingContext);
    listener.onServiceFound(nsdService);

    verify(nsdManager, times(1)).resolveService(any(), any());
  }

  @Test
  public void onServiceFound_noCallsToResolve_whenNonCommissionerDiscovered() {
    when(nsdService.getServiceType()).thenReturn("_type._udp");
    CommissionerDiscoveryListener listener = new CommissionerDiscoveryListener(castingContext);
    listener.onServiceFound(nsdService);

    verify(nsdManager, times(0)).resolveService(any(), any());
  }

  @Test
  public void onStartDiscoveryFailed_callsStopServiceDiscovery() {
    when(castingContext.getNsdManager()).thenReturn(nsdManager);
    doNothing().when(nsdManager).stopServiceDiscovery(any());

    CommissionerDiscoveryListener listener = new CommissionerDiscoveryListener(castingContext);
    listener.onStartDiscoveryFailed("_test_type._udp", 1);

    verify(nsdManager, times(1)).stopServiceDiscovery(any());
  }

  @Test
  public void onStopDiscoveryFailed_callsStopServiceDiscovery() {
    when(castingContext.getNsdManager()).thenReturn(nsdManager);
    doNothing().when(nsdManager).stopServiceDiscovery(any());

    CommissionerDiscoveryListener listener = new CommissionerDiscoveryListener(castingContext);
    listener.onStopDiscoveryFailed("_test_type._udp", 1);

    verify(nsdManager, times(1)).stopServiceDiscovery(any());
  }
}
