# Plugin parts of the ZCL Advanced Platform

These plugins contain generic logic behind the implementation of the specific ZCL clusters. They are either client or server and should be
platform and underlying-layer independent. 

If you find yourself using a stack-dependent APIs, you're very much
encouraged to create yourself a separate library in the `zap-lib` specific for a given platform.