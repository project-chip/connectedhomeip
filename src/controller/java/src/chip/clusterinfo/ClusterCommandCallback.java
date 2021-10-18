package chip.clusterinfo;
<<<<<<< HEAD

import java.util.List;

/**
 * Interface for making customized callback that implements both onSuccess and onFailure functions.
 */
=======
import java.util.*;

>>>>>>> 2d2bbd2e1 (new design solution)
public interface ClusterCommandCallback {
  void onSuccess(List<Object> responseValues);

  void onFailure(Exception exception);
<<<<<<< HEAD
<<<<<<< HEAD
}
=======
}
>>>>>>> 2d2bbd2e1 (new design solution)
=======
}
>>>>>>> df6a682b0 (Restyled by whitespace)
