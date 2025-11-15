
import cython

from .._utils.time cimport current_time_millis, millis_to_seconds
from .answers cimport AnswerGroup, construct_outgoing_multicast_answers


cdef bint TYPE_CHECKING
cdef tuple MULTICAST_DELAY_RANDOM_INTERVAL
cdef object RAND_INT

cdef class MulticastOutgoingQueue:

    cdef object zc
    cdef public object queue
    cdef public object _multicast_delay_random_min
    cdef public object _multicast_delay_random_max
    cdef object _additional_delay
    cdef object _aggregation_delay

    @cython.locals(last_group=AnswerGroup, random_int=cython.uint)
    cpdef void async_add(self, double now, cython.dict answers)

    @cython.locals(pending=AnswerGroup)
    cdef void _remove_answers_from_queue(self, cython.dict answers)

    cpdef void async_ready(self)
