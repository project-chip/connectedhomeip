package com.google.chip.chiptool.commissioner.thread.internal;

import android.os.Parcel;
import android.os.Parcelable;
import com.google.chip.chiptool.commissioner.thread.BorderAgentInfo;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkInfo;
import java.util.ArrayList;

class ThreadNetworkInfoHolder implements Parcelable {
  public ThreadNetworkInfo networkInfo;
  public ArrayList<BorderAgentInfo> borderAgents;

  public ThreadNetworkInfoHolder(BorderAgentInfo borderAgent) {
    networkInfo = new ThreadNetworkInfo(borderAgent.networkName, borderAgent.extendedPanId);
    borderAgents = new ArrayList<>();
    borderAgents.add(borderAgent);
  }

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    dest.writeParcelable(networkInfo, flags);
    dest.writeParcelableArray( (BorderAgentInfo[]) borderAgents.toArray(), flags);
  }
}
