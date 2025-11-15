
import cython

from .._dns cimport DNSEntry, DNSPointer, DNSQuestion, DNSRecord
from .incoming cimport DNSIncoming


cdef cython.uint _CLASS_UNIQUE
cdef cython.uint _DNS_PACKET_HEADER_LEN
cdef cython.uint _FLAGS_QR_MASK
cdef cython.uint _FLAGS_QR_QUERY
cdef cython.uint _FLAGS_QR_RESPONSE
cdef cython.uint _FLAGS_TC
cdef cython.uint _MAX_MSG_ABSOLUTE
cdef cython.uint _MAX_MSG_TYPICAL


cdef bint TYPE_CHECKING

cdef unsigned int SHORT_CACHE_MAX

cdef object PACK_BYTE
cdef object PACK_SHORT
cdef object PACK_LONG

cdef unsigned int STATE_INIT
cdef unsigned int STATE_FINISHED

cdef object LOGGING_IS_ENABLED_FOR
cdef object LOGGING_DEBUG

cdef cython.tuple BYTE_TABLE
cdef cython.tuple SHORT_LOOKUP
cdef cython.dict LONG_LOOKUP

cdef class DNSOutgoing:

    cdef public unsigned int flags
    cdef public bint finished
    cdef public object id
    cdef public bint multicast
    cdef public cython.list packets_data
    cdef public cython.dict names
    cdef public cython.list data
    cdef public unsigned int size
    cdef public bint allow_long
    cdef public unsigned int state
    cdef public cython.list questions
    cdef public cython.list answers
    cdef public cython.list authorities
    cdef public cython.list additionals

    cpdef void _reset_for_next_packet(self)

    cdef void _write_byte(self, cython.uint value)

    cdef void _insert_short_at_start(self, unsigned int value)

    cdef void _replace_short(self, cython.uint index, cython.uint value)

    cdef _get_short(self, cython.uint value)

    cdef void _write_int(self, object value)

    cdef cython.bint _write_question(self, DNSQuestion question)

    @cython.locals(
        d=cython.bytes,
        data_view=cython.list,
        index=cython.uint,
        length=cython.uint
    )
    cdef cython.bint _write_record(self, DNSRecord record, double now)

    @cython.locals(class_=cython.uint)
    cdef void _write_record_class(self, DNSEntry record)

    @cython.locals(
        start_size_int=object
    )
    cdef cython.bint _check_data_limit_or_rollback(self, cython.uint start_data_length, cython.uint start_size)

    @cython.locals(questions_written=cython.uint)
    cdef cython.uint _write_questions_from_offset(self, unsigned int questions_offset)

    @cython.locals(answers_written=cython.uint)
    cdef cython.uint _write_answers_from_offset(self, unsigned int answer_offset)

    @cython.locals(records_written=cython.uint)
    cdef cython.uint _write_records_from_offset(self, cython.list records, unsigned int offset)

    cdef bint _has_more_to_add(self, unsigned int questions_offset, unsigned int answer_offset, unsigned int authority_offset, unsigned int additional_offset)

    cdef void _write_ttl(self, DNSRecord record, double now)

    @cython.locals(
        labels=cython.list,
        label=cython.str,
        index=cython.uint,
        start_size=cython.uint,
        name_length=cython.uint,
    )
    cpdef void write_name(self, cython.str name)

    cdef void _write_link_to_name(self, unsigned int index)

    cpdef void write_short(self, cython.uint value)

    cpdef void write_string(self, cython.bytes value)

    cpdef void write_character_string(self, cython.bytes value)

    @cython.locals(utfstr=bytes)
    cdef void _write_utf(self, cython.str value)

    @cython.locals(
        debug_enable=bint,
        made_progress=bint,
        has_more_to_add=bint,
        questions_offset="unsigned int",
        answer_offset="unsigned int",
        authority_offset="unsigned int",
        additional_offset="unsigned int",
        questions_written="unsigned int",
        answers_written="unsigned int",
        authorities_written="unsigned int",
        additionals_written="unsigned int",
    )
    cpdef packets(self)

    cpdef void add_question(self, DNSQuestion question)

    cpdef void add_answer(self, DNSIncoming inp, DNSRecord record)

    @cython.locals(now_double=double)
    cpdef void add_answer_at_time(self, DNSRecord record, double now)

    cpdef void add_authorative_answer(self, DNSPointer record)

    cpdef void add_additional_answer(self, DNSRecord record)

    cpdef bint is_query(self)

    cpdef bint is_response(self)
