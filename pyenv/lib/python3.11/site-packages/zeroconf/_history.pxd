import cython

from ._dns cimport DNSQuestion


cdef cython.double _DUPLICATE_QUESTION_INTERVAL

cdef class QuestionHistory:

    cdef cython.dict _history

    cpdef void add_question_at_time(self, DNSQuestion question, double now, cython.set known_answers)

    @cython.locals(than=double, previous_question=cython.tuple, previous_known_answers=cython.set)
    cpdef bint suppresses(self, DNSQuestion question, double now, cython.set known_answers)

    @cython.locals(than=double, now_known_answers=cython.tuple)
    cpdef void async_expire(self, double now)
