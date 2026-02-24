#if defined(WF200_WIFI)
#include "lwipopts-wf200.h"
#elif defined(SL_MATTER_SIWX_WIFI_ENABLE)
#include "lwipopts-siwx.h"
#else
#error No platform has been specified for the LwIP configurations.
#endif
