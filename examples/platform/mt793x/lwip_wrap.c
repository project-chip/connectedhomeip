

#include <FreeRTOS.h>

#include <stdlib.h>

/****************************************************************************/

#include <memory_map.h>

#define portCACHELINE_ALIGNMENT HAL_CACHE_LINE_SIZE

#if 0
void * pvPortMallocExt(BaseType_t xRegion, size_t xWantedSize)
{
    (void) xRegion;

    return pvPortMalloc(xWantedSize);
}

void vPortFreeExt(BaseType_t xRegion, void * pv)
{
    (void) xRegion;

    vPortFree(pv);
}

#include <hal_cache.h>

static void vPortMallocCacheInvalidate(void * pvAddr, size_t xMemSize)
{
    hal_cache_status_t status;

    status = hal_cache_invalidate_multiple_cache_lines((uint32_t) pvAddr, xMemSize);
    if (HAL_CACHE_STATUS_OK != status)
    {
        configASSERT(0);
    }
}

void * pvPortMallocNCExt(BaseType_t xRegion, size_t xWantedSize)
{
    /* +------+---+--------------+-----+
     * |      |   |              |     |
     * +------+---+--------------+-----+
     * P0     P1  P2             P3    P4
     *
     * P0 = pvRealAddr, returned by pvPortMallocExt()
     * P1 = pv
     * P2 = pvReturn, aligned to cache line, returned to caller.
     * P3 = the end of caller's buffer, if xWantedSize is a multiple of
     *      the size of a cache line, P3 = P4.
     * P4 = the end of real buffer.
     *
     * [P0, P1) - unused buffer for aligning P2~P3 with cache line.
     * [P1, P2) - keeps the address of P0.
     * [P2, P3) - caller wanted buffer
     * [P3, P4) - unused buffer for cache line alignment
     */

#define CACHE_ALIGN (16)
#define CACHE_ALIGN_PAD_MASK (CACHE_ALIGN - 1)
#define CACHE_ALIGN_MASK (~CACHE_ALIGN_PAD_MASK)

    const size_t xAddrSize = sizeof(void *);
    const size_t xHeadSize = xAddrSize + CACHE_ALIGN - 1;
    size_t xTailSize       = CACHE_ALIGN - (xWantedSize & CACHE_ALIGN_PAD_MASK);
    size_t xRealSize;
    void * pvRealAddr;
    void * pvReturn;

    xRealSize = xHeadSize + xWantedSize + xTailSize;

    pvRealAddr = pvPortMallocExt(xRegion, xRealSize);

    pvReturn = (void *) (((uint32_t) pvRealAddr + xHeadSize) & CACHE_ALIGN_MASK);

    // keep real addr for free()
    *(uint32_t *) (pvReturn - xAddrSize) = (uint32_t) pvRealAddr;

    vPortMallocCacheInvalidate(pvReturn, xWantedSize);

    if (pvReturn != NULL)
    {
        pvReturn = (char *) HAL_CACHE_VIRTUAL_TO_PHYSICAL((uint32_t) pvReturn);
    }

    return (void *) pvReturn;
}
#endif
