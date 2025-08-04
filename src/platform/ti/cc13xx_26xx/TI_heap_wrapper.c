#include "bget.h"
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SwiP.h>

typedef unsigned int dpl_CSState;

typedef union _dpl_cs_state_union_t
{
    /** critical section variable as declared in the interface */
    dpl_CSState state;
    /** @internal field used to access internal data */
    struct _dpl_cs_state_aggr_t
    {
        /** field to store Swi_disable() return value */
        uint_least16_t swikey;
        /** field to store Hwi_disable() return value */
        uint_least16_t hwikey;
    } each;
} dpl_CSStateUnion;

/* This is enter critical section for DPL supported devices */
dpl_CSState dpl_enterCSImpl(void)
{

    dpl_CSStateUnion cu;
    cu.each.swikey = (uint_least16_t) SwiP_disable();
    cu.each.hwikey = (uint_least16_t) HwiP_disable();
    return cu.state;
}

/* This is exit critical section for DPL supported devices */
void dpl_leaveCSImpl(dpl_CSState key)
{
    dpl_CSStateUnion * cu = (dpl_CSStateUnion *) &key;
    HwiP_restore((uint32_t) cu->each.hwikey);
    SwiP_restore((uint32_t) cu->each.swikey);
}

/* Protected allocation */
void * pvPortMalloc(size_t xWantedSize)
{
    void * retVal = NULL;

    dpl_CSState state;
    state = dpl_enterCSImpl();

    retVal = bget(xWantedSize);

    dpl_leaveCSImpl(state);
    return retVal;
}

/* Protected Deallocation */
void vPortFree(void * pv)
{
    dpl_CSState state;
    state = dpl_enterCSImpl();

    brel(pv);

    dpl_leaveCSImpl(state);
}

void * pvPortRealloc(void * pv, size_t size)
{
    void * retVal = NULL;

    dpl_CSState state;
    state = dpl_enterCSImpl();

    retVal = bgetr(pv, size);

    dpl_leaveCSImpl(state);
    return retVal;
}
