from .._dns cimport DNSAddress

cdef bint TYPE_CHECKING


cpdef get_ip_address_object_from_record(DNSAddress record)


@cython.locals(address_str=str)
cpdef str_without_scope_id(object addr)


cpdef ip_bytes_and_scope_to_address(object addr, object scope_id)


cdef object cached_ip_addresses_wrapper
