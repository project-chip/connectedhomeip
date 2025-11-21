
import cython

from .._cache cimport DNSCache
from .._history cimport QuestionHistory
from .._protocol.outgoing cimport DNSOutgoing, DNSPointer, DNSQuestion, DNSRecord
from .._record_update cimport RecordUpdate
from .._updates cimport RecordUpdateListener
from .._utils.time cimport current_time_millis, millis_to_seconds
from . cimport Signal, SignalRegistrationInterface


cdef bint TYPE_CHECKING
cdef object cached_possible_types
cdef cython.uint _EXPIRE_REFRESH_TIME_PERCENT, _MAX_MSG_TYPICAL, _DNS_PACKET_HEADER_LEN
cdef cython.uint _TYPE_PTR
cdef object _CLASS_IN
cdef object SERVICE_STATE_CHANGE_ADDED, SERVICE_STATE_CHANGE_REMOVED, SERVICE_STATE_CHANGE_UPDATED
cdef cython.set _ADDRESS_RECORD_TYPES
cdef float RESCUE_RECORD_RETRY_TTL_PERCENTAGE

cdef object _MDNS_PORT, _BROWSER_TIME

cdef object QU_QUESTION

cdef object _FLAGS_QR_QUERY

cdef object heappop, heappush

cdef class _ScheduledPTRQuery:

    cdef public str alias
    cdef public str name
    cdef public unsigned int ttl
    cdef public bint cancelled
    cdef public double expire_time_millis
    cdef public double when_millis

cdef class _DNSPointerOutgoingBucket:

    cdef public double now_millis
    cdef public DNSOutgoing out
    cdef public cython.uint bytes

    cpdef add(self, cython.uint max_compressed_size, DNSQuestion question, cython.set answers)


@cython.locals(cache=DNSCache, question_history=QuestionHistory, record=DNSRecord, qu_question=bint)
cpdef list generate_service_query(
    object zc,
    double now_millis,
    set types_,
    bint multicast,
    object question_type
)


@cython.locals(answer=DNSPointer, query_buckets=list, question=DNSQuestion, max_compressed_size=cython.uint, max_bucket_size=cython.uint, query_bucket=_DNSPointerOutgoingBucket)
cdef list _group_ptr_queries_with_known_answers(double now_millis, bint multicast, cython.dict question_with_known_answers)


cdef class QueryScheduler:

    cdef object _zc
    cdef set _types
    cdef str _addr
    cdef int _port
    cdef bint _multicast
    cdef tuple _first_random_delay_interval
    cdef double _min_time_between_queries_millis
    cdef object _loop
    cdef unsigned int _startup_queries_sent
    cdef public dict _next_scheduled_for_alias
    cdef public list _query_heap
    cdef object _next_run
    cdef double _clock_resolution_millis
    cdef object _question_type

    cdef void _schedule_ptr_refresh(self, DNSPointer pointer, double expire_time_millis, double refresh_time_millis)

    cdef void _schedule_ptr_query(self, _ScheduledPTRQuery scheduled_query)

    @cython.locals(scheduled=_ScheduledPTRQuery)
    cpdef void cancel_ptr_refresh(self, DNSPointer pointer)

    @cython.locals(current=_ScheduledPTRQuery, expire_time=double)
    cpdef void reschedule_ptr_first_refresh(self, DNSPointer pointer)

    @cython.locals(ttl_millis="unsigned int", additional_wait=double, next_query_time=double)
    cpdef void schedule_rescue_query(self, _ScheduledPTRQuery query, double now_millis, float additional_percentage)

    cpdef void _process_startup_queries(self)

    @cython.locals(query=_ScheduledPTRQuery, next_scheduled=_ScheduledPTRQuery, next_when=double)
    cpdef void _process_ready_types(self)

    cpdef void async_send_ready_queries(self, bint first_request, double now_millis, set ready_types)


cdef class _ServiceBrowserBase(RecordUpdateListener):

    cdef public cython.set types
    cdef public object zc
    cdef DNSCache _cache
    cdef object _loop
    cdef public cython.dict _pending_handlers
    cdef public object _service_state_changed
    cdef public QueryScheduler query_scheduler
    cdef public bint done
    cdef public object _query_sender_task

    cpdef void _enqueue_callback(self, object state_change, object type_, object name)

    @cython.locals(record_update=RecordUpdate, record=DNSRecord, cache=DNSCache, service=DNSRecord, pointer=DNSPointer)
    cpdef void async_update_records(self, object zc, double now, cython.list records)

    cpdef cython.list _names_matching_types(self, object types)

    cpdef _fire_service_state_changed_event(self, cython.tuple event)

    cpdef void async_update_records_complete(self)
