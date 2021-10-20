package chip.clusterinfo;

import java.util.List;

public interface ClusterCommandCallback {
  void onSuccess(List<Object> responseValues);

  void onFailure(Exception exception);
}
