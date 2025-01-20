#include "DFUSync.h"

CHIP_ERROR DFUSync::Take(uint32_t & id)
{
    if (mIsTaken)
    {
        if (id == mOwnerId)
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_BUSY;
    }

    mIsTaken = true;
    /* Increment owner id to make sure that every allocation is unique. */
    mOwnerId++;
    id = mOwnerId;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DFUSync::Free(uint32_t id)
{
    /* Prevent free operation from the threads that do not own mutex. */
    if (id != mOwnerId)
    {
        return CHIP_ERROR_ACCESS_DENIED;
    }

    mIsTaken = false;

    return CHIP_NO_ERROR;
}
