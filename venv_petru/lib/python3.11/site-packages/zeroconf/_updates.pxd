
import cython


cdef class RecordUpdateListener:

    cpdef void async_update_records(self, object zc, double now, cython.list records)

    cpdef void async_update_records_complete(self)
