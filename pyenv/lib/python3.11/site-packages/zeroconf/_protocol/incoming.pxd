
import cython


cdef cython.uint DNS_COMPRESSION_HEADER_LEN
cdef cython.uint MAX_DNS_LABELS
cdef cython.uint DNS_COMPRESSION_POINTER_LEN
cdef cython.uint MAX_NAME_LENGTH

cdef cython.uint _TYPE_A
cdef cython.uint _TYPE_CNAME
cdef cython.uint _TYPE_PTR
cdef cython.uint _TYPE_TXT
cdef cython.uint _TYPE_SRV
cdef cython.uint _TYPE_HINFO
cdef cython.uint _TYPE_AAAA
cdef cython.uint _TYPE_NSEC
cdef cython.uint _FLAGS_QR_MASK
cdef cython.uint _FLAGS_QR_MASK
cdef cython.uint _FLAGS_TC
cdef cython.uint _FLAGS_QR_QUERY
cdef cython.uint _FLAGS_QR_RESPONSE

cdef object DECODE_EXCEPTIONS

cdef object IncomingDecodeError

from .._dns cimport (
    DNSAddress,
    DNSEntry,
    DNSHinfo,
    DNSNsec,
    DNSPointer,
    DNSQuestion,
    DNSRecord,
    DNSService,
    DNSText,
)
from .._utils.time cimport current_time_millis


cdef class DNSIncoming:

    cdef bint _did_read_others
    cdef public unsigned int flags
    cdef cython.uint offset
    cdef public bytes data
    cdef const unsigned char [:] view
    cdef unsigned int _data_len
    cdef cython.dict _name_cache
    cdef cython.list _questions
    cdef cython.list _answers
    cdef public cython.uint id
    cdef cython.uint _num_questions
    cdef cython.uint _num_answers
    cdef cython.uint _num_authorities
    cdef cython.uint _num_additionals
    cdef public bint valid
    cdef public double now
    cdef public object scope_id
    cdef public object source
    cdef bint _has_qu_question

    @cython.locals(
        question=DNSQuestion
    )
    cpdef bint has_qu_question(self)

    cpdef bint is_query(self)

    cpdef bint is_probe(self)

    cpdef list answers(self)

    cpdef bint is_response(self)

    @cython.locals(
        off="unsigned int",
        label_idx="unsigned int",
        length="unsigned int",
        link="unsigned int",
        link_data="unsigned int",
        link_py_int=object,
        linked_labels=cython.list
    )
    cdef unsigned int _decode_labels_at_offset(self, unsigned int off, cython.list labels, cython.set seen_pointers)

    @cython.locals(offset="unsigned int")
    cdef void _read_header(self)

    cdef void _initial_parse(self)

    @cython.locals(
        end="unsigned int",
        length="unsigned int",
        offset="unsigned int"
    )
    cdef void _read_others(self)

    @cython.locals(offset="unsigned int", question=DNSQuestion)
    cdef _read_questions(self)

    @cython.locals(
        length="unsigned int",
    )
    cdef str _read_character_string(self)

    cdef bytes _read_string(self, unsigned int length)

    @cython.locals(
        name_start="unsigned int",
        offset="unsigned int",
        address_rec=DNSAddress,
        pointer_rec=DNSPointer,
        text_rec=DNSText,
        srv_rec=DNSService,
        hinfo_rec=DNSHinfo,
        nsec_rec=DNSNsec,
    )
    cdef _read_record(self, str domain, unsigned int type_, unsigned int class_, unsigned int ttl, unsigned int length)

    @cython.locals(
        offset="unsigned int",
        offset_plus_one="unsigned int",
        offset_plus_two="unsigned int",
        window="unsigned int",
        bit="unsigned int",
        byte="unsigned int",
        i="unsigned int",
        bitmap_length="unsigned int",
    )
    cdef list _read_bitmap(self, unsigned int end)

    cdef str _read_name(self)
