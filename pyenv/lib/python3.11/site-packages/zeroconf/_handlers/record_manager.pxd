
import cython

from .._cache cimport DNSCache
from .._dns cimport DNSQuestion, DNSRecord
from .._protocol.incoming cimport DNSIncoming
from .._updates cimport RecordUpdateListener
from .._utils.time cimport current_time_millis
from .._record_update cimport RecordUpdate

cdef unsigned int _DNS_PTR_MIN_TTL
cdef cython.uint _TYPE_PTR
cdef object _ADDRESS_RECORD_TYPES
cdef bint TYPE_CHECKING
cdef object _TYPE_PTR


cdef class RecordManager:

    cdef public object zc
    cdef public DNSCache cache
    cdef public cython.set listeners

    cpdef void async_updates(self, object now, list records)

    cpdef void async_updates_complete(self, bint notify)

    @cython.locals(
        cache=DNSCache,
        record=DNSRecord,
        answers=cython.list,
        maybe_entry=DNSRecord,
        rec_update=RecordUpdate
    )
    cpdef void async_updates_from_response(self, DNSIncoming msg)

    cpdef void async_add_listener(self, RecordUpdateListener listener, object question)

    cpdef void async_remove_listener(self, RecordUpdateListener listener)

    @cython.locals(question=DNSQuestion, record=DNSRecord)
    cdef void _async_update_matching_records(self, RecordUpdateListener listener, cython.list questions)
