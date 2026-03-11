
import cython


cdef class Signal:

    cdef list _handlers

cdef class SignalRegistrationInterface:

    cdef list _handlers
