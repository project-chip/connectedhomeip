include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

# freertos component uses the #if checks against undefined macros.
# so, we need to disable the -Wundef warnings.

# This target provides the backend for pw::sync::Mutex.
pw_add_library(pw_sync.esp32 INTERFACE
    HEADERS
      public/pw_sync_esp32/mutex_inline.h
      public/pw_sync_esp32/mutex_native.h
      public_overrides/pw_sync_backend/mutex_inline.h
      public_overrides/pw_sync_backend/mutex_native.h
    PUBLIC_INCLUDES
      public
      public_overrides
    PUBLIC_DEPS
      pw_assert
      pw_sync.mutex.facade
      pw_third_party.freertos
    PUBLIC_COMPILE_OPTIONS
      -Wno-undef
)

# This target provides the backend for pw::sync::InterruptSpinLock.
pw_add_library(pw_sync.esp32_interrupt_spin_lock STATIC
    HEADERS
      public/pw_sync_esp32/interrupt_spin_lock_inline.h
      public/pw_sync_esp32/interrupt_spin_lock_native.h
      public_overrides/pw_sync_backend/interrupt_spin_lock_inline.h
      public_overrides/pw_sync_backend/interrupt_spin_lock_native.h
    PUBLIC_INCLUDES
      public
      public_overrides
    PUBLIC_DEPS
      pw_assert
      pw_sync.interrupt_spin_lock.facade
      pw_sync.yield_core
      pw_third_party.freertos
    PUBLIC_COMPILE_OPTIONS
      -Wno-undef
    SOURCES
      interrupt_spin_lock.cc
    PRIVATE_COMPILE_OPTIONS
      -Wno-undef
)
