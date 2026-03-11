
import cython

from .._cache cimport DNSCache
from .._dns cimport DNSAddress, DNSPointer, DNSQuestion, DNSRecord, DNSRRSet
from .._history cimport QuestionHistory
from .._protocol.incoming cimport DNSIncoming
from .._services.info cimport ServiceInfo
from .._services.registry cimport ServiceRegistry
from .answers cimport (
    QuestionAnswers,
    construct_outgoing_multicast_answers,
    construct_outgoing_unicast_answers,
)
from .multicast_outgoing_queue cimport MulticastOutgoingQueue


cdef bint TYPE_CHECKING
cdef cython.uint _ONE_SECOND, _TYPE_PTR, _TYPE_ANY, _TYPE_A, _TYPE_AAAA, _TYPE_SRV, _TYPE_TXT
cdef str _SERVICE_TYPE_ENUMERATION_NAME
cdef cython.set _RESPOND_IMMEDIATE_TYPES
cdef cython.set _ADDRESS_RECORD_TYPES
cdef object IPVersion, _IPVersion_ALL
cdef object _TYPE_PTR, _CLASS_IN, _DNS_OTHER_TTL

cdef unsigned int _ANSWER_STRATEGY_SERVICE_TYPE_ENUMERATION
cdef unsigned int _ANSWER_STRATEGY_POINTER
cdef unsigned int _ANSWER_STRATEGY_ADDRESS
cdef unsigned int _ANSWER_STRATEGY_SERVICE
cdef unsigned int _ANSWER_STRATEGY_TEXT

cdef list _EMPTY_SERVICES_LIST
cdef list _EMPTY_TYPES_LIST

cdef class _AnswerStrategy:

    cdef public DNSQuestion question
    cdef public unsigned int strategy_type
    cdef public list types
    cdef public list services


cdef class _QueryResponse:

    cdef bint _is_probe
    cdef cython.list _questions
    cdef double _now
    cdef DNSCache _cache
    cdef cython.dict _additionals
    cdef cython.set _ucast
    cdef cython.set _mcast_now
    cdef cython.set _mcast_aggregate
    cdef cython.set _mcast_aggregate_last_second

    @cython.locals(record=DNSRecord)
    cdef void add_qu_question_response(self, cython.dict answers)

    cdef void add_ucast_question_response(self, cython.dict answers)

    @cython.locals(answer=DNSRecord, question=DNSQuestion)
    cdef void add_mcast_question_response(self, cython.dict answers)

    @cython.locals(maybe_entry=DNSRecord)
    cdef bint _has_mcast_within_one_quarter_ttl(self, DNSRecord record)

    @cython.locals(maybe_entry=DNSRecord)
    cdef bint _has_mcast_record_in_last_second(self, DNSRecord record)

    cdef QuestionAnswers answers(self)

cdef class QueryHandler:

    cdef object zc
    cdef ServiceRegistry registry
    cdef DNSCache cache
    cdef QuestionHistory question_history
    cdef MulticastOutgoingQueue out_queue
    cdef MulticastOutgoingQueue out_delay_queue

    @cython.locals(service=ServiceInfo)
    cdef void _add_service_type_enumeration_query_answers(self, list types, cython.dict answer_set, DNSRRSet known_answers)

    @cython.locals(service=ServiceInfo)
    cdef void _add_pointer_answers(self, list services, cython.dict answer_set, DNSRRSet known_answers)

    @cython.locals(service=ServiceInfo, dns_address=DNSAddress)
    cdef void _add_address_answers(self, list services, cython.dict answer_set, DNSRRSet known_answers, cython.uint type_)

    @cython.locals(question_lower_name=str, type_=cython.uint, service=ServiceInfo)
    cdef cython.dict _answer_question(self, DNSQuestion question, unsigned int strategy_type, list types, list services, DNSRRSet known_answers)

    @cython.locals(
        msg=DNSIncoming,
        msgs=list,
        strategy=_AnswerStrategy,
        question=DNSQuestion,
        answer_set=cython.dict,
        known_answers=DNSRRSet,
        known_answers_set=cython.set,
        is_unicast=bint,
        is_probe=object,
        now=double
    )
    cpdef QuestionAnswers async_response(self, cython.list msgs, cython.bint unicast_source)

    @cython.locals(name=str, question_lower_name=str)
    cdef list _get_answer_strategies(self, DNSQuestion question)

    @cython.locals(
        first_packet=DNSIncoming,
        ucast_source=bint,
    )
    cpdef void handle_assembled_query(
        self,
        list packets,
        object addr,
        object port,
        object transport,
        tuple v6_flow_scope
    )
