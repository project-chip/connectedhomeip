
import cython

from .info cimport ServiceInfo


cdef class ServiceRegistry:

    cdef cython.dict _services
    cdef public cython.dict types
    cdef public cython.dict servers
    cdef public bint has_entries

    @cython.locals(
        record_list=cython.list,
    )
    cdef cython.list _async_get_by_index(self, cython.dict records, str key)

    cdef _add(self, ServiceInfo info)

    @cython.locals(
        info=ServiceInfo,
        old_service_info=ServiceInfo
    )
    cdef _remove(self, cython.list infos)

    cpdef ServiceInfo async_get_info_name(self, str name)

    cpdef cython.list async_get_types(self)

    cpdef cython.list async_get_infos_type(self, str type_)

    cpdef cython.list async_get_infos_server(self, str server)
