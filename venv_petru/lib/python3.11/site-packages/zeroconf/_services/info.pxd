
import cython

from .._cache cimport DNSCache
from .._dns cimport (
    DNSAddress,
    DNSNsec,
    DNSPointer,
    DNSQuestion,
    DNSRecord,
    DNSService,
    DNSText,
)
from .._history cimport QuestionHistory
from .._protocol.outgoing cimport DNSOutgoing
from .._record_update cimport RecordUpdate
from .._updates cimport RecordUpdateListener
from .._utils.ipaddress cimport (
    get_ip_address_object_from_record,
    ip_bytes_and_scope_to_address,
    str_without_scope_id,
)
from .._utils.time cimport current_time_millis

cdef cython.set _TYPE_AAAA_RECORDS
cdef cython.set _TYPE_A_RECORDS
cdef cython.set _TYPE_A_AAAA_RECORDS

cdef object _resolve_all_futures_to_none

cdef object _TYPE_SRV
cdef object _TYPE_TXT
cdef object _TYPE_A
cdef object _TYPE_AAAA
cdef object _TYPE_PTR
cdef object _TYPE_NSEC
cdef object _CLASS_IN
cdef object _FLAGS_QR_QUERY

cdef object service_type_name

cdef object QU_QUESTION
cdef object QM_QUESTION

cdef object _IPVersion_All_value
cdef object _IPVersion_V4Only_value

cdef cython.set _ADDRESS_RECORD_TYPES

cdef unsigned int _DUPLICATE_QUESTION_INTERVAL

cdef bint TYPE_CHECKING
cdef object cached_ip_addresses

cdef object randint

cdef class ServiceInfo(RecordUpdateListener):

    cdef public cython.bytes text
    cdef public str type
    cdef str _name
    cdef public str key
    cdef public cython.list _ipv4_addresses
    cdef public cython.list _ipv6_addresses
    cdef public object port
    cdef public object weight
    cdef public object priority
    cdef public str server
    cdef public str server_key
    cdef public cython.dict _properties
    cdef public cython.dict _decoded_properties
    cdef public object host_ttl
    cdef public object other_ttl
    cdef public object interface_index
    cdef public cython.set _new_records_futures
    cdef public DNSPointer _dns_pointer_cache
    cdef public DNSService _dns_service_cache
    cdef public DNSText _dns_text_cache
    cdef public cython.list _dns_address_cache
    cdef public cython.set _get_address_and_nsec_records_cache
    cdef public cython.set _query_record_types

    @cython.locals(record_update=RecordUpdate, update=bint, cache=DNSCache)
    cpdef void async_update_records(self, object zc, double now, cython.list records)

    @cython.locals(cache=DNSCache)
    cpdef bint _load_from_cache(self, object zc, double now)

    @cython.locals(length="unsigned char", index="unsigned int", key_value=bytes, key_sep_value=tuple)
    cdef void _unpack_text_into_properties(self)

    @cython.locals(k=bytes, v=bytes)
    cdef void _generate_decoded_properties(self)

    @cython.locals(properties_contain_str=bint)
    cpdef void _set_properties(self, cython.dict properties)

    cdef void _set_text(self, cython.bytes text)

    @cython.locals(record=DNSAddress)
    cdef _get_ip_addresses_from_cache_lifo(self, object zc, double now, object type)

    @cython.locals(
        dns_service_record=DNSService,
        dns_text_record=DNSText,
        dns_address_record=DNSAddress
    )
    cdef bint _process_record_threadsafe(self, object zc, DNSRecord record, double now)

    @cython.locals(cache=DNSCache)
    cdef cython.list _get_address_records_from_cache_by_type(self, object zc, object _type)

    cdef void _set_ipv4_addresses_from_cache(self, object zc, double now)

    cdef void _set_ipv6_addresses_from_cache(self, object zc, double now)

    cdef cython.list _ip_addresses_by_version_value(self, object version_value)

    cpdef addresses_by_version(self, object version)

    cpdef ip_addresses_by_version(self, object version)

    @cython.locals(cacheable=cython.bint)
    cdef cython.list _dns_addresses(self, object override_ttls, object version)

    @cython.locals(cacheable=cython.bint)
    cdef DNSPointer _dns_pointer(self, object override_ttl)

    @cython.locals(cacheable=cython.bint)
    cdef DNSService _dns_service(self, object override_ttl)

    @cython.locals(cacheable=cython.bint)
    cdef DNSText _dns_text(self, object override_ttl)

    cdef DNSNsec _dns_nsec(self, cython.list missing_types, object override_ttl)

    @cython.locals(cacheable=cython.bint)
    cdef cython.set _get_address_and_nsec_records(self, object override_ttl)

    cpdef void async_clear_cache(self)

    @cython.locals(cache=DNSCache, history=QuestionHistory, out=DNSOutgoing, qu_question=bint)
    cdef DNSOutgoing _generate_request_query(self, object zc, double now, object question_type)

    @cython.locals(question=DNSQuestion, answer=DNSRecord)
    cdef void _add_question_with_known_answers(
        self,
        DNSOutgoing out,
        bint qu_question,
        QuestionHistory question_history,
        DNSCache cache,
        double now,
        str name,
        object type_,
        object class_,
        bint skip_if_known_answers
    )

    cdef double _get_initial_delay(self)

    cdef double _get_random_delay(self)

cdef class AddressResolver(ServiceInfo):
    pass

cdef class AddressResolverIPv6(ServiceInfo):
    pass

cdef class AddressResolverIPv4(ServiceInfo):
    pass
