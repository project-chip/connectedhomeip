/* Protected allocation
 malloc/ICall_heapMalloc  --> ti_heap_wrapper --> bget protected by critical section
*/
void * pvPortMalloc(size_t xWantedSize);

/* Protected Deallocation
 Free/ICall_heapFree --> ti_heap_wrapper --> brel protected by critical section
 */
void vPortFree(void * pv);
void * pvPortRealloc(void * pv, size_t size);
