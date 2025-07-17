/**
  ******************************************************************************
  * @file    nvm_emul.c
  * @author  MCD Application Team
  * @brief   This file implements the RAM version of the NVM manager for
  *          STM32WBX. It is made for test purpose.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "app_common.h"
#include "nvm.h"

/*****************************************************************************/

static uint32_t *NVM_buffer;
/* static */ uint32_t NVM_offset, NVM_trig_level;

#if CFG_NVM_ALIGN != 0
static uint16_t NVM_size;       /* d */
#endif
static uint16_t NVM_max_size;   /* w */

/*****************************************************************************/

/* Number of 32-bit words in NVM flash area */
#if (CFG_NVM_ALIGN == 0) && (!defined CFG_NVM_MAX_SIZE)
#error NVM: CFG_NVM_MAX_SIZE must be defined!
#endif

/*****************************************************************************/

/* Macro to get the blank status of a record */
#define NVM_BLANK(ptr)          (!(*(ptr)))

/* Macro to get the size of a record */
#define NVM_SIZE(ptr)           ((*(ptr)) & 0xFFFFUL)

/* Macro to get the type of a record */
#define NVM_TYPE(ptr)           (((*(ptr)) >> 16) & 0xFFUL)

/* Macro to get the validity of a record */
#define NVM_VALID(ptr)          (((*(ptr)) >> 24) & 0xFFUL)

/* Macro to get the offset in words from a size in bytes */
#if CFG_NVM_ALIGN != 0
#define NVM_OFFSET(size)        (2*DIVC(size + 4, 8))
#else
#define NVM_OFFSET(size)        DIVC(size + 4, 4)
#endif

/*****************************************************************************/

#if CFG_NVM_ALIGN != 0
void NVM_Init( uint64_t* buffer,
               uint16_t size,
               uint16_t max_size )
#else
void NVM_Init( uint32_t* buffer )
#endif
{
  /* Save buffer info */
  NVM_buffer = (uint32_t*)buffer;
#if CFG_NVM_ALIGN != 0
  NVM_size = size;
  NVM_max_size = 2*max_size;
  NVM_buffer[2*size] = 0;
#else
  NVM_max_size = CFG_NVM_MAX_SIZE;
#endif

  /* Initialize warning trigger level */
  NVM_trig_level = NVM_max_size - 1;
}

/*****************************************************************************/

int NVM_Add( uint8_t type,
             const uint8_t* data,
             uint16_t size,
             const uint8_t* extra_data,
             uint16_t extra_size )
{
  uint32_t *ptr, next, left, total_size, offset, *start_ptr, removed;

  if ( data && size )
  {
    total_size = size + extra_size;
    offset = 1 + NVM_OFFSET(total_size);

    while ( 1 )
    {
      /* Search the end of data in the buffer */
      ptr = NVM_buffer;
      left = NVM_max_size;
      removed = 0;
      while ( !NVM_BLANK(ptr) )
      {
        next = NVM_OFFSET(NVM_SIZE(ptr));

        if  ( next >= left )
        {
          /* Return if we exceed buffer size (memory corruption case) */
          return NVM_ERROR;
        }

        if ( !NVM_VALID(ptr) )
          removed += next;

        ptr += next;
        left -= next;
      }

      /* Test if there is enough room for the new record */
      if ( offset <= left )
      {
        break;
      }

      if ( removed == 0 )
      {
        /* Return if there is no more room in NVM */
        return NVM_FULL;
      }

      /* Search for an invalid record */
      ptr = NVM_buffer;
      left = NVM_max_size;
      while ( (!NVM_BLANK(ptr)) && NVM_VALID(ptr) )
      {
        next = NVM_OFFSET(NVM_SIZE(ptr));
        ptr += next;
        left -= next;
      }

      if ( NVM_BLANK(ptr) )
      {
        /* Return if we have reached the end of the buffer */
        return NVM_FULL;
      }

      /* Move records to get room */
      next = NVM_OFFSET(NVM_SIZE(ptr));
      memmove( ptr, ptr + next, 4*(left - next) );
      removed -= next;

#if CFG_NVM_ALIGN != 0
      NVM_size -= next/2;
      NVMCB_Store( ptr, NVM_size );
#else
      NVMCB_Store( ptr, left - next );
#endif
    }

    /* Write the record */
    *ptr = 0x01000000UL | (((uint32_t)type) << 16) | total_size;
    start_ptr = ptr;

    memcpy( ptr + 1, data, size );

    if ( extra_data )
    {
      memcpy( ((uint8_t*)(ptr + 1)) + size, extra_data, extra_size );
    }

    /* Set next record as blank */
    ptr += NVM_OFFSET(NVM_SIZE(ptr));
    *ptr = 0;
#if CFG_NVM_ALIGN != 0
    NVM_size = (ptr - NVM_buffer) / 2;
    NVMCB_Store( start_ptr, NVM_size );
#else
    NVMCB_Store( start_ptr, ptr + 1 - start_ptr );
#endif

    /* Update trigger level */
    NVM_trig_level = MIN(NVM_trig_level, NVM_max_size + 1 - offset);

    /* Check amount of NVM used */
    if ( (ptr + 1 - NVM_buffer - removed) > NVM_trig_level )
    {
      /* Return warning to indicate that NVM is near to be full */
      return NVM_WARN;
    }
  }

  return NVM_OK;
}

/*****************************************************************************/

int NVM_Get( uint8_t mode,
             uint8_t type,
             uint16_t offset,
             uint8_t* data,
             uint16_t size )
{
  uint32_t *ptr, next;
  int remain_size, copy_size;

  /* Point at buffer start */
  ptr = NVM_buffer;

  if ( mode == NVM_FIRST )
  {
    NVM_offset = 0;
  }
  else
  {
    /* Check current offset */
    if  ( NVM_offset >= NVM_max_size - 1 )
    {
      NVM_offset = 0;
      return NVM_EOF;
    }

    /* Point on the current record */
    ptr += NVM_offset;
  }

  if ( mode != NVM_CURRENT )
  {
    if ( mode == NVM_NEXT )
    {
      /* Point on the next record */
      next = NVM_OFFSET(NVM_SIZE(ptr));
      ptr += next;
      NVM_offset += next;

      if  ( NVM_offset >= NVM_max_size )
      {
        /* Return if we exceed buffer size (memory corruption case) */
        return NVM_ERROR;
      }
    }

    /* Search the first next record with matching type */
    while ( !(NVM_BLANK(ptr) || (NVM_VALID(ptr) && (NVM_TYPE(ptr) == type))) )
    {
      /* Point on the next record */
      next = NVM_OFFSET(NVM_SIZE(ptr));
      ptr += next;
      NVM_offset += next;

      if  ( NVM_offset >= NVM_max_size )
      {
        /* Return if we exceed buffer size (memory corruption case) */
       return NVM_ERROR;
      }
    }
  }

  if ( NVM_BLANK(ptr) )
  {
    /* Return if we have reached the end of data in the buffer */
    return NVM_EOF;
  }

  if ( !(NVM_VALID(ptr) && (NVM_TYPE(ptr) == type)) )
  {
    /* Return if the record is not valid (memory corruption case) */
    return NVM_ERROR;
  }

  /* Compute the record remaining size (after the offset) */
  remain_size = (int)NVM_SIZE(ptr) - (int)offset;

  /* Compute the number of bytes to copy */
  copy_size = 0;

  if ( remain_size > 0 )
  {
    copy_size = (int)size;

    if ( remain_size < copy_size )
    {
      copy_size = remain_size;
    }

    /* Retrieve the record data */
    if ( data )
    {
      memcpy( data, ((uint8_t*)ptr) + 4 + offset, copy_size );
    }
  }

  /* Return the number of bytes copied */
  return copy_size;
}

/*****************************************************************************/

int NVM_Compare( uint16_t offset,
                 const uint8_t* data,
                 uint16_t size )
{
  uint32_t *ptr;
  int remain_size;

  /* Point on the current record */
  ptr = NVM_buffer + NVM_offset;

  /* Check current position in the database */
  if  ( (NVM_offset >= NVM_max_size - 1) || NVM_BLANK(ptr) )
  {
    /* Return if we have reached the end of data in the buffer */
    return NVM_EOF;
  }

  /* Compute the record remaining size (after the offset) */
  remain_size = NVM_SIZE(ptr) - (int)offset;

  if ( (size > remain_size) ||
       (memcmp( data, ((uint8_t*)ptr) + 4 + offset, size ) != 0) )
  {
    return (int)size;
  }

  return NVM_OK;
}

/*****************************************************************************/

void NVM_Discard( uint8_t mode )
{
  uint32_t *ptr, *ptr_next, next, size;

  ptr = NVM_buffer;

  if ( mode == NVM_CURRENT )
  {
    /* Check current offset */
    if  ( NVM_offset >= NVM_max_size - 1 )
    {
      NVM_offset = 0;
      return;
    }

    /* Point on the current record */
    ptr += NVM_offset;

    if ( !NVM_BLANK(ptr) )
    {
      /* Point on the next record */
      next = NVM_OFFSET(NVM_SIZE(ptr));
      ptr_next = ptr + next;

      if  ( (NVM_offset + next) >= NVM_max_size )
      {
        /* Return if we exceed buffer size (memory corruption case) */
        return;
      }

      if ( NVM_BLANK(ptr_next) )
      {
        /* Set current record as blank if next record is blank. */
        *ptr = 0;

#if CFG_NVM_ALIGN != 0
        NVM_size = (ptr - NVM_buffer) / 2;
#endif
      }
      else
      {
        size = NVM_SIZE(ptr);

        if ( !NVM_VALID(ptr_next) )
        {
          next += NVM_OFFSET(NVM_SIZE(ptr_next));

          if ( next <= 0x4000UL )
          {
            /* Update size of current record if next record is invalid. */
            size = (next - 1) * 4;
          }
        }

        /* Invalidate the current record */
        *ptr = size;
      }
    }
  }
  else if ( mode == NVM_ALL )
  {
    /* Set initial record as blank */
    *ptr = 0;

#if CFG_NVM_ALIGN != 0
    NVM_size = 0;
#endif
  }

#if CFG_NVM_ALIGN != 0
  NVMCB_Store( ptr, NVM_size );
#else
  NVMCB_Store( ptr, 1 );
#endif
}

/*****************************************************************************/

/*
 * Callbacks
 */

__WEAK void NVMCB_Store( const uint32_t* ptr, uint32_t size )
{
}

/*****************************************************************************/
