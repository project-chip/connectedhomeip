package com.chip.casting.app;

import android.content.Context;
import android.net.nsd.NsdManager;
import android.widget.LinearLayout;
import androidx.fragment.app.FragmentActivity;

public class CastingContext {
  private FragmentActivity fragmentActivity;

  public CastingContext(FragmentActivity fragmentActivity) {
    this.fragmentActivity = fragmentActivity;
  }

  public Context getApplicationContext() {
    return fragmentActivity.getApplicationContext();
  }

  public FragmentActivity getFragmentActivity() {
    return fragmentActivity;
  }

  public NsdManager getNsdManager() {
    return (NsdManager)
        fragmentActivity.getApplicationContext().getSystemService(Context.NSD_SERVICE);
  }

  public LinearLayout getCommissionersLayout() {
    return (LinearLayout) fragmentActivity.findViewById(R.id.castingCommissioners);
  }
}
