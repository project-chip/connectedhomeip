package chip.clusterinfo;

/** Interface for setting customized ClusterCommandCallback in ClusterInteractionFragment.kt */
public interface DelegatedClusterCallback {
  void setCallbackDelegate(ClusterCommandCallback callback);
}
