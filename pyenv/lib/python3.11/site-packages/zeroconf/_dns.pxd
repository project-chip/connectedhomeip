
import cython

from ._protocol.outgoing cimport DNSOutgoing


cdef cython.uint _LEN_BYTE
cdef cython.uint _LEN_SHORT
cdef cython.uint _LEN_INT

cdef cython.uint _NAME_COMPRESSION_MIN_SIZE
cdef cython.uint _BASE_MAX_SIZE

cdef cython.uint _EXPIRE_FULL_TIME_MS
cdef cython.uint _EXPIRE_STALE_TIME_MS
cdef cython.uint _RECENT_TIME_MS

cdef cython.uint _TYPE_ANY

cdef cython.uint _CLASS_UNIQUE
cdef cython.uint _CLASS_MASK

cdef object current_time_millis

cdef class DNSEntry:

    cdef public str key
    cdef public str name
    cdef public cython.uint type
    cdef public cython.uint class_
    cdef public bint unique

    cdef _fast_init_entry(self, str name, cython.uint type_, cython.uint class_)

    cdef bint _dns_entry_matches(self, DNSEntry other)

cdef class DNSQuestion(DNSEntry):

    cdef public cython.int _hash

    cdef _fast_init(self, str name, cython.uint type_, cython.uint class_)

    cpdef bint answered_by(self, DNSRecord rec)

cdef class DNSRecord(DNSEntry):

    cdef public unsigned int ttl
    cdef public double created

    cdef _fast_init_record(self, str name, cython.uint type_, cython.uint class_, unsigned int ttl, double created)

    cdef bint _suppressed_by_answer(self, DNSRecord answer)

    @cython.locals(
        answers=cython.list,
    )
    cpdef bint suppressed_by(self, object msg)

    cpdef get_remaining_ttl(self, double now)

    cpdef double get_expiration_time(self, cython.uint percent)

    cpdef bint is_expired(self, double now)

    cpdef bint is_stale(self, double now)

    cpdef bint is_recent(self, double now)

    cdef _set_created_ttl(self, double now, unsigned int ttl)

cdef class DNSAddress(DNSRecord):

    cdef public cython.int _hash
    cdef public bytes address
    cdef public object scope_id

    cdef _fast_init(self, str name, cython.uint type_, cython.uint class_, unsigned int ttl, bytes address, object scope_id, double created)

    cdef bint _eq(self, DNSAddress other)

    cpdef write(self, DNSOutgoing out)


cdef class DNSHinfo(DNSRecord):

    cdef public cython.int _hash
    cdef public str cpu
    cdef public str os

    cdef _fast_init(self, str name, cython.uint type_, cython.uint class_, unsigned int ttl, str cpu, str os, double created)

    cdef bint _eq(self, DNSHinfo other)

    cpdef write(self, DNSOutgoing out)

cdef class DNSPointer(DNSRecord):

    cdef public cython.int _hash
    cdef public str alias
    cdef public str alias_key

    cdef _fast_init(self, str name, cython.uint type_, cython.uint class_, unsigned int ttl, str alias, double created)

    cdef bint _eq(self, DNSPointer other)

    cpdef write(self, DNSOutgoing out)

cdef class DNSText(DNSRecord):

    cdef public cython.int _hash
    cdef public bytes text

    cdef _fast_init(self, str name, cython.uint type_, cython.uint class_, unsigned int ttl, bytes text, double created)

    cdef bint _eq(self, DNSText other)

    cpdef write(self, DNSOutgoing out)

cdef class DNSService(DNSRecord):

    cdef public cython.int _hash
    cdef public cython.uint priority
    cdef public cython.uint weight
    cdef public cython.uint port
    cdef public str server
    cdef public str server_key

    cdef _fast_init(self, str name, cython.uint type_, cython.uint class_, unsigned int ttl, cython.uint priority, cython.uint weight, cython.uint port, str server, double created)

    cdef bint _eq(self, DNSService other)

    cpdef write(self, DNSOutgoing out)

cdef class DNSNsec(DNSRecord):

    cdef public cython.int _hash
    cdef public str next_name
    cdef public cython.list rdtypes

    cdef _fast_init(self, str name, cython.uint type_, cython.uint class_, unsigned int ttl, str next_name, cython.list rdtypes, double created)

    cdef bint _eq(self, DNSNsec other)

    cpdef write(self, DNSOutgoing out)

cdef class DNSRRSet:

    cdef cython.list _records
    cdef cython.dict _lookup

    @cython.locals(other=DNSRecord)
    cpdef bint suppresses(self, DNSRecord record)

    @cython.locals(
        record=DNSRecord,
        record_sets=cython.list,
    )
    cdef cython.dict _get_lookup(self)

    cpdef cython.set lookup_set(self)
