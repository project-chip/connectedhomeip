#ifndef SL_MEMORY_CONFIG_H
#define SL_MEMORY_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h> Memory configuration

// <o SL_STACK_SIZE> Stack size for the application.
// <i> Default: 4096
// <i> The stack size configured here will be used by the stack that the
// <i> application uses when coming out of a reset.
#ifndef SL_STACK_SIZE
  #define SL_STACK_SIZE  4096
#endif

// <o SL_HEAP_SIZE> Minimum heap size for the application.
// <i> Default: 2048
// <i> Note that this value will configure the c heap which is normally used by
// <i> malloc() and free() from the c library. The value defines a minimum heap
// <i> size that is guaranteed to be available. The available heap may be larger
// <i> to make use of any memory that would otherwise remain unused.
#ifndef SL_HEAP_SIZE
  #define SL_HEAP_SIZE   2048
#endif

// </h>
// <<< end of configuration section >>>

#endif
