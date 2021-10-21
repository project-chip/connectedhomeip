package chip.clusterinfo;
<<<<<<< HEAD
<<<<<<< HEAD

import java.util.List;
<<<<<<< HEAD

<<<<<<< HEAD
<<<<<<< HEAD
/**
 * Interface for making customized callback that implements both onSuccess and onFailure functions.
 */
=======
=======

>>>>>>> 46c963914 (Restyled by google-java-format)
import java.util.*;
=======
>>>>>>> ef97f0702 (resolve type, nullable and format comments)

>>>>>>> 2d2bbd2e1 (new design solution)
=======
/** Interface for making customized callback that implements both onSuccess and onFailure function */
>>>>>>> 2185ce827 (add descriptive documentation on each new class)
=======
/**
 * Interface for making customized callback that implements both onSuccess and onFailure functions.
 */
>>>>>>> e3803522e (Restyled by google-java-format)
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
