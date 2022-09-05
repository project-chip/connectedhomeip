/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2011 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"

// Disable certain benign warnings with Microsoft compilers
#if (defined(_MSC_VER))
// Disable "conditional expression is constant" warning for debug macros.
// Otherwise, this generates warnings for the perfectly natural construct "while(1)"
// If someone knows a variant way of writing "while(1)" that doesn't generate warning messages, please let us know
    #pragma warning(disable:4127)
#endif


// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Byte Swapping Functions
#endif

mDNSlocal mDNSu16 NToH16(mDNSu8 * bytes)
{
    return (mDNSu16)((mDNSu16)bytes[0] << 8 | (mDNSu16)bytes[1]);
}

mDNSlocal mDNSu32 NToH32(mDNSu8 * bytes)
{
    return (mDNSu32)((mDNSu32) bytes[0] << 24 | (mDNSu32) bytes[1] << 16 | (mDNSu32) bytes[2] << 8 | (mDNSu32)bytes[3]);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - MD5 Hash Functions
#endif


/* The source for the has is derived CommonCrypto files CommonDigest.h, md32_common.h, md5_locl.h, md5_locl.h, and openssl/md5.h.
 * The following changes have been made to the original sources:
 *    replaced CC_LONG w/ mDNSu32
 *    replaced CC_MD5* with MD5*
 *    replaced CC_LONG w/ mDNSu32, removed conditional #defines from md5.h
 *    removed extern decls for MD5_Init/Update/Final from CommonDigest.h
 *    removed APPLE_COMMON_DIGEST specific #defines from md5_locl.h
 *
 * Note: machine archetecure specific conditionals from the original sources are turned off, but are left in the code
 * to aid in platform-specific optimizations and debugging.
 * Sources originally distributed under the following license headers:
 * CommonDigest.h - APSL
 *
 * md32_Common.h
 * ====================================================================
 * Copyright (c) 1999-2002 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * md5_dgst.c, md5_locl.h
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 * Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 *
 */

//from CommonDigest.h



// from openssl/md5.h

#define MD5_CBLOCK  64
#define MD5_LBLOCK  (MD5_CBLOCK/4)
#define MD5_DIGEST_LENGTH 16

void MD5_Transform(MD5_CTX *c, const unsigned char *b);

// From md5_locl.h

#ifndef MD5_LONG_LOG2
#define MD5_LONG_LOG2 2 /* default to 32 bits */
#endif

#ifdef MD5_ASM
# if defined(__i386) || defined(__i386__) || defined(_M_IX86) || defined(__INTEL__)
#  define md5_block_host_order md5_block_asm_host_order
# elif defined(__sparc) && defined(OPENSSL_SYS_ULTRASPARC)
void md5_block_asm_data_order_aligned (MD5_CTX *c, const mDNSu32 *p,int num);
#  define HASH_BLOCK_DATA_ORDER_ALIGNED md5_block_asm_data_order_aligned
# endif
#endif

void md5_block_host_order (MD5_CTX *c, const void *p,int num);
void md5_block_data_order (MD5_CTX *c, const void *p,int num);

#if defined(__i386) || defined(__i386__) || defined(_M_IX86) || defined(__INTEL__)
/*
 * *_block_host_order is expected to handle aligned data while
 * *_block_data_order - unaligned. As algorithm and host (x86)
 * are in this case of the same "endianness" these two are
 * otherwise indistinguishable. But normally you don't want to
 * call the same function because unaligned access in places
 * where alignment is expected is usually a "Bad Thing". Indeed,
 * on RISCs you get punished with BUS ERROR signal or *severe*
 * performance degradation. Intel CPUs are in turn perfectly
 * capable of loading unaligned data without such drastic side
 * effect. Yes, they say it's slower than aligned load, but no
 * exception is generated and therefore performance degradation
 * is *incomparable* with RISCs. What we should weight here is
 * costs of unaligned access against costs of aligning data.
 * According to my measurements allowing unaligned access results
 * in ~9% performance improvement on Pentium II operating at
 * 266MHz. I won't be surprised if the difference will be higher
 * on faster systems:-)
 *
 *				<appro@fy.chalmers.se>
 */
#define md5_block_data_order md5_block_host_order
#endif

#define DATA_ORDER_IS_LITTLE_ENDIAN

#define HASH_LONG       mDNSu32
#define HASH_LONG_LOG2  MD5_LONG_LOG2
#define HASH_CTX        MD5_CTX
#define HASH_CBLOCK     MD5_CBLOCK
#define HASH_LBLOCK     MD5_LBLOCK

#define HASH_UPDATE     MD5_Update
#define HASH_TRANSFORM  MD5_Transform
#define HASH_FINAL      MD5_Final

#define HASH_MAKE_STRING(c,s)   do {    \
        unsigned long ll;       \
        ll=(c)->A; HOST_l2c(ll,(s));    \
        ll=(c)->B; HOST_l2c(ll,(s));    \
        ll=(c)->C; HOST_l2c(ll,(s));    \
        ll=(c)->D; HOST_l2c(ll,(s));    \
} while (0)
#define HASH_BLOCK_HOST_ORDER   md5_block_host_order
#if !defined(L_ENDIAN) || defined(md5_block_data_order)
#define HASH_BLOCK_DATA_ORDER   md5_block_data_order
/*
 * Little-endians (Intel and Alpha) feel better without this.
 * It looks like memcpy does better job than generic
 * md5_block_data_order on copying-n-aligning input data.
 * But frankly speaking I didn't expect such result on Alpha.
 * On the other hand I've got this with egcs-1.0.2 and if
 * program is compiled with another (better?) compiler it
 * might turn out other way around.
 *
 *				<appro@fy.chalmers.se>
 */
#endif


// from md32_common.h

/*
 * This is a generic 32 bit "collector" for message digest algorithms.
 * Whenever needed it collects input character stream into chunks of
 * 32 bit values and invokes a block function that performs actual hash
 * calculations.
 *
 * Porting guide.
 *
 * Obligatory macros:
 *
 * DATA_ORDER_IS_BIG_ENDIAN or DATA_ORDER_IS_LITTLE_ENDIAN
 *	this macro defines byte order of input stream.
 * HASH_CBLOCK
 *	size of a unit chunk HASH_BLOCK operates on.
 * HASH_LONG
 *	has to be at lest 32 bit wide, if it's wider, then
 *	HASH_LONG_LOG2 *has to* be defined along
 * HASH_CTX
 *	context structure that at least contains following
 *	members:
 *		typedef struct {
 *			...
 *			HASH_LONG	Nl,Nh;
 *			HASH_LONG	data[HASH_LBLOCK];
 *			int		num;
 *			...
 *			} HASH_CTX;
 * HASH_UPDATE
 *	name of "Update" function, implemented here.
 * HASH_TRANSFORM
 *	name of "Transform" function, implemented here.
 * HASH_FINAL
 *	name of "Final" function, implemented here.
 * HASH_BLOCK_HOST_ORDER
 *	name of "block" function treating *aligned* input message
 *	in host byte order, implemented externally.
 * HASH_BLOCK_DATA_ORDER
 *	name of "block" function treating *unaligned* input message
 *	in original (data) byte order, implemented externally (it
 *	actually is optional if data and host are of the same
 *	"endianess").
 * HASH_MAKE_STRING
 *	macro convering context variables to an ASCII hash string.
 *
 * Optional macros:
 *
 * B_ENDIAN or L_ENDIAN
 *	defines host byte-order.
 * HASH_LONG_LOG2
 *	defaults to 2 if not states otherwise.
 * HASH_LBLOCK
 *	assumed to be HASH_CBLOCK/4 if not stated otherwise.
 * HASH_BLOCK_DATA_ORDER_ALIGNED
 *	alternative "block" function capable of treating
 *	aligned input message in original (data) order,
 *	implemented externally.
 *
 * MD5 example:
 *
 *	#define DATA_ORDER_IS_LITTLE_ENDIAN
 *
 *	#define HASH_LONG		mDNSu32
 *	#define HASH_LONG_LOG2	mDNSu32_LOG2
 *	#define HASH_CTX		MD5_CTX
 *	#define HASH_CBLOCK		MD5_CBLOCK
 *	#define HASH_LBLOCK		MD5_LBLOCK
 *	#define HASH_UPDATE		MD5_Update
 *	#define HASH_TRANSFORM		MD5_Transform
 *	#define HASH_FINAL		MD5_Final
 *	#define HASH_BLOCK_HOST_ORDER	md5_block_host_order
 *	#define HASH_BLOCK_DATA_ORDER	md5_block_data_order
 *
 *					<appro@fy.chalmers.se>
 */

#if !defined(DATA_ORDER_IS_BIG_ENDIAN) && !defined(DATA_ORDER_IS_LITTLE_ENDIAN)
#error "DATA_ORDER must be defined!"
#endif

#ifndef HASH_CBLOCK
#error "HASH_CBLOCK must be defined!"
#endif
#ifndef HASH_LONG
#error "HASH_LONG must be defined!"
#endif
#ifndef HASH_CTX
#error "HASH_CTX must be defined!"
#endif

#ifndef HASH_UPDATE
#error "HASH_UPDATE must be defined!"
#endif
#ifndef HASH_TRANSFORM
#error "HASH_TRANSFORM must be defined!"
#endif
#ifndef HASH_FINAL
#error "HASH_FINAL must be defined!"
#endif

#ifndef HASH_BLOCK_HOST_ORDER
#error "HASH_BLOCK_HOST_ORDER must be defined!"
#endif

#if 0
/*
 * Moved below as it's required only if HASH_BLOCK_DATA_ORDER_ALIGNED
 * isn't defined.
 */
#ifndef HASH_BLOCK_DATA_ORDER
#error "HASH_BLOCK_DATA_ORDER must be defined!"
#endif
#endif

#ifndef HASH_LBLOCK
#define HASH_LBLOCK (HASH_CBLOCK/4)
#endif

#ifndef HASH_LONG_LOG2
#define HASH_LONG_LOG2  2
#endif

/*
 * Engage compiler specific rotate intrinsic function if available.
 */
#undef ROTATE
#ifndef PEDANTIC
# if 0 /* defined(_MSC_VER) */
#  define ROTATE(a,n)   _lrotl(a,n)
# elif defined(__MWERKS__)
#  if defined(__POWERPC__)
#   define ROTATE(a,n)  (unsigned MD32_REG_T)__rlwinm((int)a,n,0,31)
#  elif defined(__MC68K__)
/* Motorola specific tweak. <appro@fy.chalmers.se> */
#   define ROTATE(a,n)  (n<24 ? __rol(a,n) : __ror(a,32-n))
#  else
#   define ROTATE(a,n)  __rol(a,n)
#  endif
# elif defined(__GNUC__) && __GNUC__>=2 && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
/*
 * Some GNU C inline assembler templates. Note that these are
 * rotates by *constant* number of bits! But that's exactly
 * what we need here...
 *
 *                  <appro@fy.chalmers.se>
 */
/*
 * LLVM is more strict about compatibility of types between input & output constraints,
 * but we want these to be rotations of 32 bits, not 64, so we explicitly drop the
 * most significant bytes by casting to an unsigned int.
 */
#  if defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)
#   define ROTATE(a,n)  ({ register unsigned int ret;   \
                           asm (           \
                               "roll %1,%0"        \
                               : "=r" (ret)     \
                               : "I" (n), "0" ((unsigned int)a)  \
                               : "cc");        \
                           ret;             \
                         })
#  elif defined(__powerpc) || defined(__ppc)
#   define ROTATE(a,n)  ({ register unsigned int ret;   \
                           asm (           \
                               "rlwinm %0,%1,%2,0,31"  \
                               : "=r" (ret)     \
                               : "r" (a), "I" (n));  \
                           ret;             \
                         })
#  endif
# endif

/*
 * Engage compiler specific "fetch in reverse byte order"
 * intrinsic function if available.
 */
# if defined(__GNUC__) && __GNUC__>=2 && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
/* some GNU C inline assembler templates by <appro@fy.chalmers.se> */
#  if (defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)) && !defined(I386_ONLY)
#   define BE_FETCH32(a)    ({ register unsigned int l=(a); \
                               asm (           \
                                   "bswapl %0"     \
                                   : "=r" (l) : "0" (l));    \
                               l;                \
                             })
#  elif defined(__powerpc)
#   define LE_FETCH32(a)    ({ register unsigned int l; \
                               asm (           \
                                   "lwbrx %0,0,%1"     \
                                   : "=r" (l)       \
                                   : "r" (a));      \
                               l;               \
                             })

#  elif defined(__sparc) && defined(OPENSSL_SYS_ULTRASPARC)
#  define LE_FETCH32(a) ({ register unsigned int l;     \
                           asm (               \
                               "lda [%1]#ASI_PRIMARY_LITTLE,%0" \
                               : "=r" (l)           \
                               : "r" (a));          \
                           l;                   \
                         })
#  endif
# endif
#endif /* PEDANTIC */

#if HASH_LONG_LOG2==2   /* Engage only if sizeof(HASH_LONG)== 4 */
/* A nice byte order reversal from Wei Dai <weidai@eskimo.com> */
#ifdef ROTATE
/* 5 instructions with rotate instruction, else 9 */
#define REVERSE_FETCH32(a,l)    (                   \
        l=*(const HASH_LONG *)(a),              \
        ((ROTATE(l,8)&0x00FF00FF)|(ROTATE((l&0x00FF00FF),24)))  \
        )
#else
/* 6 instructions with rotate instruction, else 8 */
#define REVERSE_FETCH32(a,l)    (               \
        l=*(const HASH_LONG *)(a),          \
        l=(((l>>8)&0x00FF00FF)|((l&0x00FF00FF)<<8)),    \
        ROTATE(l,16)                    \
        )
/*
 * Originally the middle line started with l=(((l&0xFF00FF00)>>8)|...
 * It's rewritten as above for two reasons:
 *	- RISCs aren't good at long constants and have to explicitely
 *	  compose 'em with several (well, usually 2) instructions in a
 *	  register before performing the actual operation and (as you
 *	  already realized:-) having same constant should inspire the
 *	  compiler to permanently allocate the only register for it;
 *	- most modern CPUs have two ALUs, but usually only one has
 *	  circuitry for shifts:-( this minor tweak inspires compiler
 *	  to schedule shift instructions in a better way...
 *
 *				<appro@fy.chalmers.se>
 */
#endif
#endif

#ifndef ROTATE
#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#endif

/*
 * Make some obvious choices. E.g., HASH_BLOCK_DATA_ORDER_ALIGNED
 * and HASH_BLOCK_HOST_ORDER ought to be the same if input data
 * and host are of the same "endianess". It's possible to mask
 * this with blank #define HASH_BLOCK_DATA_ORDER though...
 *
 *				<appro@fy.chalmers.se>
 */
#if defined(B_ENDIAN)
#  if defined(DATA_ORDER_IS_BIG_ENDIAN)
#    if !defined(HASH_BLOCK_DATA_ORDER_ALIGNED) && HASH_LONG_LOG2==2
#      define HASH_BLOCK_DATA_ORDER_ALIGNED HASH_BLOCK_HOST_ORDER
#    endif
#  elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)
#    ifndef HOST_FETCH32
#      ifdef LE_FETCH32
#        define HOST_FETCH32(p,l)   LE_FETCH32(p)
#      elif defined(REVERSE_FETCH32)
#        define HOST_FETCH32(p,l)   REVERSE_FETCH32(p,l)
#      endif
#    endif
#  endif
#elif defined(L_ENDIAN)
#  if defined(DATA_ORDER_IS_LITTLE_ENDIAN)
#    if !defined(HASH_BLOCK_DATA_ORDER_ALIGNED) && HASH_LONG_LOG2==2
#      define HASH_BLOCK_DATA_ORDER_ALIGNED HASH_BLOCK_HOST_ORDER
#    endif
#  elif defined(DATA_ORDER_IS_BIG_ENDIAN)
#    ifndef HOST_FETCH32
#      ifdef BE_FETCH32
#        define HOST_FETCH32(p,l)   BE_FETCH32(p)
#      elif defined(REVERSE_FETCH32)
#        define HOST_FETCH32(p,l)   REVERSE_FETCH32(p,l)
#      endif
#    endif
#  endif
#endif

#if !defined(HASH_BLOCK_DATA_ORDER_ALIGNED)
#ifndef HASH_BLOCK_DATA_ORDER
#error "HASH_BLOCK_DATA_ORDER must be defined!"
#endif
#endif

// None of the invocations of the following macros actually use the result,
// so cast them to void to avoid any compiler warnings/errors about not using
// the result (e.g. when using clang).
// If the resultant values need to be used at some point, these must be changed.
#define HOST_c2l(c,l) ((void)_HOST_c2l(c,l))
#define HOST_l2c(l,c) ((void)_HOST_l2c(l,c))

#if defined(DATA_ORDER_IS_BIG_ENDIAN)

#define _HOST_c2l(c,l)  (l =(((unsigned long)(*((c)++)))<<24),      \
                         l|=(((unsigned long)(*((c)++)))<<16),      \
                         l|=(((unsigned long)(*((c)++)))<< 8),      \
                         l|=(((unsigned long)(*((c)++)))    ),      \
                         l)
#define HOST_p_c2l(c,l,n)   {                   \
        switch (n) {                    \
        case 0: l =((unsigned long)(*((c)++)))<<24; \
        case 1: l|=((unsigned long)(*((c)++)))<<16; \
        case 2: l|=((unsigned long)(*((c)++)))<< 8; \
        case 3: l|=((unsigned long)(*((c)++)));     \
        } }
#define HOST_p_c2l_p(c,l,sc,len) {                  \
        switch (sc) {                   \
        case 0: l =((unsigned long)(*((c)++)))<<24; \
            if (--len == 0) break;                                                 \
        case 1: l|=((unsigned long)(*((c)++)))<<16; \
            if (--len == 0) break;                                                 \
        case 2: l|=((unsigned long)(*((c)++)))<< 8; \
        } }
/* NOTE the pointer is not incremented at the end of this */
#define HOST_c2l_p(c,l,n)   {                   \
        l=0; (c)+=n;                    \
        switch (n) {                    \
        case 3: l =((unsigned long)(*(--(c))))<< 8; \
        case 2: l|=((unsigned long)(*(--(c))))<<16; \
        case 1: l|=((unsigned long)(*(--(c))))<<24; \
        } }
#define _HOST_l2c(l,c)  (*((c)++)=(unsigned char)(((l)>>24)&0xff),  \
                         *((c)++)=(unsigned char)(((l)>>16)&0xff),  \
                         *((c)++)=(unsigned char)(((l)>> 8)&0xff),  \
                         *((c)++)=(unsigned char)(((l)    )&0xff),  \
                         l)

#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)

#define _HOST_c2l(c,l)  (l =(((unsigned long)(*((c)++)))    ),      \
                         l|=(((unsigned long)(*((c)++)))<< 8),      \
                         l|=(((unsigned long)(*((c)++)))<<16),      \
                         l|=(((unsigned long)(*((c)++)))<<24),      \
                         l)
#define HOST_p_c2l(c,l,n)   {                   \
        switch (n) {                    \
        case 0: l =((unsigned long)(*((c)++)));     \
        case 1: l|=((unsigned long)(*((c)++)))<< 8; \
        case 2: l|=((unsigned long)(*((c)++)))<<16; \
        case 3: l|=((unsigned long)(*((c)++)))<<24; \
        } }
#define HOST_p_c2l_p(c,l,sc,len) {                  \
        switch (sc) {                   \
        case 0: l =((unsigned long)(*((c)++)));     \
            if (--len == 0) break;                                                 \
        case 1: l|=((unsigned long)(*((c)++)))<< 8; \
            if (--len == 0) break;                                                 \
        case 2: l|=((unsigned long)(*((c)++)))<<16; \
        } }
/* NOTE the pointer is not incremented at the end of this */
#define HOST_c2l_p(c,l,n)   {                   \
        l=0; (c)+=n;                    \
        switch (n) {                    \
        case 3: l =((unsigned long)(*(--(c))))<<16; \
        case 2: l|=((unsigned long)(*(--(c))))<< 8; \
        case 1: l|=((unsigned long)(*(--(c))));     \
        } }
#define _HOST_l2c(l,c)  (*((c)++)=(unsigned char)(((l)    )&0xff),  \
                         *((c)++)=(unsigned char)(((l)>> 8)&0xff),  \
                         *((c)++)=(unsigned char)(((l)>>16)&0xff),  \
                         *((c)++)=(unsigned char)(((l)>>24)&0xff),  \
                         l)

#endif

/*
 * Time for some action:-)
 */

int HASH_UPDATE (HASH_CTX *c, const void *data_, unsigned long len)
{
    const unsigned char *data=(const unsigned char *)data_;
    register HASH_LONG * p;
    register unsigned long l;
    int sw,sc,ew,ec;

    if (len==0) return 1;

    l=(c->Nl+(len<<3))&0xffffffffL;
    /* 95-05-24 eay Fixed a bug with the overflow handling, thanks to
     * Wei Dai <weidai@eskimo.com> for pointing it out. */
    if (l < c->Nl) /* overflow */
        c->Nh++;
    c->Nh+=(len>>29);
    c->Nl=l;

    if (c->num != 0)
    {
        p=c->data;
        sw=c->num>>2;
        sc=c->num&0x03;

        if ((c->num+len) >= HASH_CBLOCK)
        {
            l=p[sw]; HOST_p_c2l(data,l,sc); p[sw++]=l;
            for (; sw<HASH_LBLOCK; sw++)
            {
                HOST_c2l(data,l); p[sw]=l;
            }
            HASH_BLOCK_HOST_ORDER (c,p,1);
            len-=(HASH_CBLOCK-c->num);
            c->num=0;
            /* drop through and do the rest */
        }
        else
        {
            c->num+=len;
            if ((sc+len) < 4) /* ugly, add char's to a word */
            {
                l=p[sw]; HOST_p_c2l_p(data,l,sc,len); p[sw]=l;
            }
            else
            {
                ew=(c->num>>2);
                ec=(c->num&0x03);
                if (sc)
                    l=p[sw];
                HOST_p_c2l(data,l,sc);
                p[sw++]=l;
                for (; sw < ew; sw++)
                {
                    HOST_c2l(data,l); p[sw]=l;
                }
                if (ec)
                {
                    HOST_c2l_p(data,l,ec); p[sw]=l;
                }
            }
            return 1;
        }
    }

    sw=(int)(len/HASH_CBLOCK);
    if (sw > 0)
    {
#if defined(HASH_BLOCK_DATA_ORDER_ALIGNED)
        /*
         * Note that HASH_BLOCK_DATA_ORDER_ALIGNED gets defined
         * only if sizeof(HASH_LONG)==4.
         */
        if ((((unsigned long)data)%4) == 0)
        {
            /* data is properly aligned so that we can cast it: */
            HASH_BLOCK_DATA_ORDER_ALIGNED (c,(HASH_LONG *)data,sw);
            sw*=HASH_CBLOCK;
            data+=sw;
            len-=sw;
        }
        else
#if !defined(HASH_BLOCK_DATA_ORDER)
            while (sw--)
            {
                mDNSPlatformMemCopy(p=c->data,data,HASH_CBLOCK);
                HASH_BLOCK_DATA_ORDER_ALIGNED(c,p,1);
                data+=HASH_CBLOCK;
                len-=HASH_CBLOCK;
            }
#endif
#endif
#if defined(HASH_BLOCK_DATA_ORDER)
        {
            HASH_BLOCK_DATA_ORDER(c,data,sw);
            sw*=HASH_CBLOCK;
            data+=sw;
            len-=sw;
        }
#endif
    }

    if (len!=0)
    {
        p = c->data;
        c->num = (int)len;
        ew=(int)(len>>2);   /* words to copy */
        ec=(int)(len&0x03);
        for (; ew; ew--,p++)
        {
            HOST_c2l(data,l); *p=l;
        }
        HOST_c2l_p(data,l,ec);
        *p=l;
    }
    return 1;
}


void HASH_TRANSFORM (HASH_CTX *c, const unsigned char *data)
{
#if defined(HASH_BLOCK_DATA_ORDER_ALIGNED)
    if ((((unsigned long)data)%4) == 0)
        /* data is properly aligned so that we can cast it: */
        HASH_BLOCK_DATA_ORDER_ALIGNED (c,(HASH_LONG *)data,1);
    else
#if !defined(HASH_BLOCK_DATA_ORDER)
    {
        mDNSPlatformMemCopy(c->data,data,HASH_CBLOCK);
        HASH_BLOCK_DATA_ORDER_ALIGNED (c,c->data,1);
    }
#endif
#endif
#if defined(HASH_BLOCK_DATA_ORDER)
    HASH_BLOCK_DATA_ORDER (c,data,1);
#endif
}


int HASH_FINAL (unsigned char *md, HASH_CTX *c)
{
    register HASH_LONG *p;
    register unsigned long l;
    register int i,j;
    static const unsigned char end[4]={0x80,0x00,0x00,0x00};
    const unsigned char *cp=end;

    /* c->num should definitly have room for at least one more byte. */
    p=c->data;
    i=c->num>>2;
    j=c->num&0x03;

#if 0
    /* purify often complains about the following line as an
     * Uninitialized Memory Read.  While this can be true, the
     * following p_c2l macro will reset l when that case is true.
     * This is because j&0x03 contains the number of 'valid' bytes
     * already in p[i].  If and only if j&0x03 == 0, the UMR will
     * occur but this is also the only time p_c2l will do
     * l= *(cp++) instead of l|= *(cp++)
     * Many thanks to Alex Tang <altitude@cic.net> for pickup this
     * 'potential bug' */
#ifdef PURIFY
    if (j==0) p[i]=0; /* Yeah, but that's not the way to fix it:-) */
#endif
    l=p[i];
#else
    l = (j==0) ? 0 : p[i];
#endif
    HOST_p_c2l(cp,l,j); p[i++]=l; /* i is the next 'undefined word' */

    if (i>(HASH_LBLOCK-2)) /* save room for Nl and Nh */
    {
        if (i<HASH_LBLOCK) p[i]=0;
        HASH_BLOCK_HOST_ORDER (c,p,1);
        i=0;
    }
    for (; i<(HASH_LBLOCK-2); i++)
        p[i]=0;

#if   defined(DATA_ORDER_IS_BIG_ENDIAN)
    p[HASH_LBLOCK-2]=c->Nh;
    p[HASH_LBLOCK-1]=c->Nl;
#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)
    p[HASH_LBLOCK-2]=c->Nl;
    p[HASH_LBLOCK-1]=c->Nh;
#endif
    HASH_BLOCK_HOST_ORDER (c,p,1);

#ifndef HASH_MAKE_STRING
#error "HASH_MAKE_STRING must be defined!"
#else
    HASH_MAKE_STRING(c,md);
#endif

    c->num=0;
    /* clear stuff, HASH_BLOCK may be leaving some stuff on the stack
     * but I'm not worried :-)
       OPENSSL_cleanse((void *)c,sizeof(HASH_CTX));
     */
    return 1;
}

#ifndef MD32_REG_T
#define MD32_REG_T long
/*
 * This comment was originaly written for MD5, which is why it
 * discusses A-D. But it basically applies to all 32-bit digests,
 * which is why it was moved to common header file.
 *
 * In case you wonder why A-D are declared as long and not
 * as mDNSu32. Doing so results in slight performance
 * boost on LP64 architectures. The catch is we don't
 * really care if 32 MSBs of a 64-bit register get polluted
 * with eventual overflows as we *save* only 32 LSBs in
 * *either* case. Now declaring 'em long excuses the compiler
 * from keeping 32 MSBs zeroed resulting in 13% performance
 * improvement under SPARC Solaris7/64 and 5% under AlphaLinux.
 * Well, to be honest it should say that this *prevents*
 * performance degradation.
 *				<appro@fy.chalmers.se>
 * Apparently there're LP64 compilers that generate better
 * code if A-D are declared int. Most notably GCC-x86_64
 * generates better code.
 *				<appro@fy.chalmers.se>
 */
#endif


// from md5_locl.h (continued)

/*
 #define	F(x,y,z)	(((x) & (y))  |  ((~(x)) & (z)))
 #define	G(x,y,z)	(((x) & (z))  |  ((y) & (~(z))))
 */

/* As pointed out by Wei Dai <weidai@eskimo.com>, the above can be
 * simplified to the code below.  Wei attributes these optimizations
 * to Peter Gutmann's SHS code, and he attributes it to Rich Schroeppel.
 */
#define F(b,c,d)    ((((c) ^ (d)) & (b)) ^ (d))
#define G(b,c,d)    ((((b) ^ (c)) & (d)) ^ (c))
#define H(b,c,d)    ((b) ^ (c) ^ (d))
#define I(b,c,d)    (((~(d)) | (b)) ^ (c))

#define R0(a,b,c,d,k,s,t) { \
        a+=((k)+(t)+F((b),(c),(d))); \
        a=ROTATE(a,s); \
        a+=b; }; \

#define R1(a,b,c,d,k,s,t) { \
        a+=((k)+(t)+G((b),(c),(d))); \
        a=ROTATE(a,s); \
        a+=b; };

#define R2(a,b,c,d,k,s,t) { \
        a+=((k)+(t)+H((b),(c),(d))); \
        a=ROTATE(a,s); \
        a+=b; };

#define R3(a,b,c,d,k,s,t) { \
        a+=((k)+(t)+I((b),(c),(d))); \
        a=ROTATE(a,s); \
        a+=b; };

// from md5_dgst.c


/* Implemented from RFC1321 The MD5 Message-Digest Algorithm
 */

#define INIT_DATA_A (unsigned long)0x67452301L
#define INIT_DATA_B (unsigned long)0xefcdab89L
#define INIT_DATA_C (unsigned long)0x98badcfeL
#define INIT_DATA_D (unsigned long)0x10325476L

int MD5_Init(MD5_CTX *c)
{
    c->A=INIT_DATA_A;
    c->B=INIT_DATA_B;
    c->C=INIT_DATA_C;
    c->D=INIT_DATA_D;
    c->Nl=0;
    c->Nh=0;
    c->num=0;
    return 1;
}

#ifndef md5_block_host_order
void md5_block_host_order (MD5_CTX *c, const void *data, int num)
{
    const mDNSu32 *X=(const mDNSu32 *)data;
    register unsigned MD32_REG_T A,B,C,D;

    A=c->A;
    B=c->B;
    C=c->C;
    D=c->D;

    for (; num--; X+=HASH_LBLOCK)
    {
        /* Round 0 */
        R0(A,B,C,D,X[ 0], 7,0xd76aa478L);
        R0(D,A,B,C,X[ 1],12,0xe8c7b756L);
        R0(C,D,A,B,X[ 2],17,0x242070dbL);
        R0(B,C,D,A,X[ 3],22,0xc1bdceeeL);
        R0(A,B,C,D,X[ 4], 7,0xf57c0fafL);
        R0(D,A,B,C,X[ 5],12,0x4787c62aL);
        R0(C,D,A,B,X[ 6],17,0xa8304613L);
        R0(B,C,D,A,X[ 7],22,0xfd469501L);
        R0(A,B,C,D,X[ 8], 7,0x698098d8L);
        R0(D,A,B,C,X[ 9],12,0x8b44f7afL);
        R0(C,D,A,B,X[10],17,0xffff5bb1L);
        R0(B,C,D,A,X[11],22,0x895cd7beL);
        R0(A,B,C,D,X[12], 7,0x6b901122L);
        R0(D,A,B,C,X[13],12,0xfd987193L);
        R0(C,D,A,B,X[14],17,0xa679438eL);
        R0(B,C,D,A,X[15],22,0x49b40821L);
        /* Round 1 */
        R1(A,B,C,D,X[ 1], 5,0xf61e2562L);
        R1(D,A,B,C,X[ 6], 9,0xc040b340L);
        R1(C,D,A,B,X[11],14,0x265e5a51L);
        R1(B,C,D,A,X[ 0],20,0xe9b6c7aaL);
        R1(A,B,C,D,X[ 5], 5,0xd62f105dL);
        R1(D,A,B,C,X[10], 9,0x02441453L);
        R1(C,D,A,B,X[15],14,0xd8a1e681L);
        R1(B,C,D,A,X[ 4],20,0xe7d3fbc8L);
        R1(A,B,C,D,X[ 9], 5,0x21e1cde6L);
        R1(D,A,B,C,X[14], 9,0xc33707d6L);
        R1(C,D,A,B,X[ 3],14,0xf4d50d87L);
        R1(B,C,D,A,X[ 8],20,0x455a14edL);
        R1(A,B,C,D,X[13], 5,0xa9e3e905L);
        R1(D,A,B,C,X[ 2], 9,0xfcefa3f8L);
        R1(C,D,A,B,X[ 7],14,0x676f02d9L);
        R1(B,C,D,A,X[12],20,0x8d2a4c8aL);
        /* Round 2 */
        R2(A,B,C,D,X[ 5], 4,0xfffa3942L);
        R2(D,A,B,C,X[ 8],11,0x8771f681L);
        R2(C,D,A,B,X[11],16,0x6d9d6122L);
        R2(B,C,D,A,X[14],23,0xfde5380cL);
        R2(A,B,C,D,X[ 1], 4,0xa4beea44L);
        R2(D,A,B,C,X[ 4],11,0x4bdecfa9L);
        R2(C,D,A,B,X[ 7],16,0xf6bb4b60L);
        R2(B,C,D,A,X[10],23,0xbebfbc70L);
        R2(A,B,C,D,X[13], 4,0x289b7ec6L);
        R2(D,A,B,C,X[ 0],11,0xeaa127faL);
        R2(C,D,A,B,X[ 3],16,0xd4ef3085L);
        R2(B,C,D,A,X[ 6],23,0x04881d05L);
        R2(A,B,C,D,X[ 9], 4,0xd9d4d039L);
        R2(D,A,B,C,X[12],11,0xe6db99e5L);
        R2(C,D,A,B,X[15],16,0x1fa27cf8L);
        R2(B,C,D,A,X[ 2],23,0xc4ac5665L);
        /* Round 3 */
        R3(A,B,C,D,X[ 0], 6,0xf4292244L);
        R3(D,A,B,C,X[ 7],10,0x432aff97L);
        R3(C,D,A,B,X[14],15,0xab9423a7L);
        R3(B,C,D,A,X[ 5],21,0xfc93a039L);
        R3(A,B,C,D,X[12], 6,0x655b59c3L);
        R3(D,A,B,C,X[ 3],10,0x8f0ccc92L);
        R3(C,D,A,B,X[10],15,0xffeff47dL);
        R3(B,C,D,A,X[ 1],21,0x85845dd1L);
        R3(A,B,C,D,X[ 8], 6,0x6fa87e4fL);
        R3(D,A,B,C,X[15],10,0xfe2ce6e0L);
        R3(C,D,A,B,X[ 6],15,0xa3014314L);
        R3(B,C,D,A,X[13],21,0x4e0811a1L);
        R3(A,B,C,D,X[ 4], 6,0xf7537e82L);
        R3(D,A,B,C,X[11],10,0xbd3af235L);
        R3(C,D,A,B,X[ 2],15,0x2ad7d2bbL);
        R3(B,C,D,A,X[ 9],21,0xeb86d391L);

        A = c->A += A;
        B = c->B += B;
        C = c->C += C;
        D = c->D += D;
    }
}
#endif

#ifndef md5_block_data_order
#ifdef X
#undef X
#endif
void md5_block_data_order (MD5_CTX *c, const void *data_, int num)
{
    const unsigned char *data=data_;
    register unsigned MD32_REG_T A,B,C,D,l;
#ifndef MD32_XARRAY
    /* See comment in crypto/sha/sha_locl.h for details. */
    unsigned MD32_REG_T XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7,
                        XX8, XX9,XX10,XX11,XX12,XX13,XX14,XX15;
# define X(i)   XX ## i
#else
    mDNSu32 XX[MD5_LBLOCK];
# define X(i)   XX[i]
#endif

    A=c->A;
    B=c->B;
    C=c->C;
    D=c->D;

    for (; num--;)
    {
        HOST_c2l(data,l); X( 0)=l;      HOST_c2l(data,l); X( 1)=l;
        /* Round 0 */
        R0(A,B,C,D,X( 0), 7,0xd76aa478L);   HOST_c2l(data,l); X( 2)=l;
        R0(D,A,B,C,X( 1),12,0xe8c7b756L);   HOST_c2l(data,l); X( 3)=l;
        R0(C,D,A,B,X( 2),17,0x242070dbL);   HOST_c2l(data,l); X( 4)=l;
        R0(B,C,D,A,X( 3),22,0xc1bdceeeL);   HOST_c2l(data,l); X( 5)=l;
        R0(A,B,C,D,X( 4), 7,0xf57c0fafL);   HOST_c2l(data,l); X( 6)=l;
        R0(D,A,B,C,X( 5),12,0x4787c62aL);   HOST_c2l(data,l); X( 7)=l;
        R0(C,D,A,B,X( 6),17,0xa8304613L);   HOST_c2l(data,l); X( 8)=l;
        R0(B,C,D,A,X( 7),22,0xfd469501L);   HOST_c2l(data,l); X( 9)=l;
        R0(A,B,C,D,X( 8), 7,0x698098d8L);   HOST_c2l(data,l); X(10)=l;
        R0(D,A,B,C,X( 9),12,0x8b44f7afL);   HOST_c2l(data,l); X(11)=l;
        R0(C,D,A,B,X(10),17,0xffff5bb1L);   HOST_c2l(data,l); X(12)=l;
        R0(B,C,D,A,X(11),22,0x895cd7beL);   HOST_c2l(data,l); X(13)=l;
        R0(A,B,C,D,X(12), 7,0x6b901122L);   HOST_c2l(data,l); X(14)=l;
        R0(D,A,B,C,X(13),12,0xfd987193L);   HOST_c2l(data,l); X(15)=l;
        R0(C,D,A,B,X(14),17,0xa679438eL);
        R0(B,C,D,A,X(15),22,0x49b40821L);
        /* Round 1 */
        R1(A,B,C,D,X( 1), 5,0xf61e2562L);
        R1(D,A,B,C,X( 6), 9,0xc040b340L);
        R1(C,D,A,B,X(11),14,0x265e5a51L);
        R1(B,C,D,A,X( 0),20,0xe9b6c7aaL);
        R1(A,B,C,D,X( 5), 5,0xd62f105dL);
        R1(D,A,B,C,X(10), 9,0x02441453L);
        R1(C,D,A,B,X(15),14,0xd8a1e681L);
        R1(B,C,D,A,X( 4),20,0xe7d3fbc8L);
        R1(A,B,C,D,X( 9), 5,0x21e1cde6L);
        R1(D,A,B,C,X(14), 9,0xc33707d6L);
        R1(C,D,A,B,X( 3),14,0xf4d50d87L);
        R1(B,C,D,A,X( 8),20,0x455a14edL);
        R1(A,B,C,D,X(13), 5,0xa9e3e905L);
        R1(D,A,B,C,X( 2), 9,0xfcefa3f8L);
        R1(C,D,A,B,X( 7),14,0x676f02d9L);
        R1(B,C,D,A,X(12),20,0x8d2a4c8aL);
        /* Round 2 */
        R2(A,B,C,D,X( 5), 4,0xfffa3942L);
        R2(D,A,B,C,X( 8),11,0x8771f681L);
        R2(C,D,A,B,X(11),16,0x6d9d6122L);
        R2(B,C,D,A,X(14),23,0xfde5380cL);
        R2(A,B,C,D,X( 1), 4,0xa4beea44L);
        R2(D,A,B,C,X( 4),11,0x4bdecfa9L);
        R2(C,D,A,B,X( 7),16,0xf6bb4b60L);
        R2(B,C,D,A,X(10),23,0xbebfbc70L);
        R2(A,B,C,D,X(13), 4,0x289b7ec6L);
        R2(D,A,B,C,X( 0),11,0xeaa127faL);
        R2(C,D,A,B,X( 3),16,0xd4ef3085L);
        R2(B,C,D,A,X( 6),23,0x04881d05L);
        R2(A,B,C,D,X( 9), 4,0xd9d4d039L);
        R2(D,A,B,C,X(12),11,0xe6db99e5L);
        R2(C,D,A,B,X(15),16,0x1fa27cf8L);
        R2(B,C,D,A,X( 2),23,0xc4ac5665L);
        /* Round 3 */
        R3(A,B,C,D,X( 0), 6,0xf4292244L);
        R3(D,A,B,C,X( 7),10,0x432aff97L);
        R3(C,D,A,B,X(14),15,0xab9423a7L);
        R3(B,C,D,A,X( 5),21,0xfc93a039L);
        R3(A,B,C,D,X(12), 6,0x655b59c3L);
        R3(D,A,B,C,X( 3),10,0x8f0ccc92L);
        R3(C,D,A,B,X(10),15,0xffeff47dL);
        R3(B,C,D,A,X( 1),21,0x85845dd1L);
        R3(A,B,C,D,X( 8), 6,0x6fa87e4fL);
        R3(D,A,B,C,X(15),10,0xfe2ce6e0L);
        R3(C,D,A,B,X( 6),15,0xa3014314L);
        R3(B,C,D,A,X(13),21,0x4e0811a1L);
        R3(A,B,C,D,X( 4), 6,0xf7537e82L);
        R3(D,A,B,C,X(11),10,0xbd3af235L);
        R3(C,D,A,B,X( 2),15,0x2ad7d2bbL);
        R3(B,C,D,A,X( 9),21,0xeb86d391L);

        A = c->A += A;
        B = c->B += B;
        C = c->C += C;
        D = c->D += D;
    }
}
#endif


// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - base64 -> binary conversion
#endif

static const char Base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char Pad64 = '=';


#define mDNSisspace(x) (x == '\t' || x == '\n' || x == '\v' || x == '\f' || x == '\r' || x == ' ')

mDNSlocal const char *mDNSstrchr(const char *s, int c)
{
    while (1)
    {
        if (c == *s) return s;
        if (!*s) return mDNSNULL;
        s++;
    }
}

// skips all whitespace anywhere.
// converts characters, four at a time, starting at (or after)
// src from base - 64 numbers into three 8 bit bytes in the target area.
// it returns the number of data bytes stored at the target, or -1 on error.
// adapted from BIND sources

mDNSlocal mDNSs32 DNSDigest_Base64ToBin(const char *src, mDNSu8 *target, mDNSu32 targsize)
{
    int tarindex, state, ch;
    const char *pos;

    state = 0;
    tarindex = 0;

    while ((ch = *src++) != '\0') {
        if (mDNSisspace(ch))    /* Skip whitespace anywhere. */
            continue;

        if (ch == Pad64)
            break;

        pos = mDNSstrchr(Base64, ch);
        if (pos == 0)       /* A non-base64 character. */
            return (-1);

        switch (state) {
        case 0:
            if (target) {
                if ((mDNSu32)tarindex >= targsize)
                    return (-1);
                target[tarindex] = (mDNSu8)((pos - Base64) << 2);
            }
            state = 1;
            break;
        case 1:
            if (target) {
                if ((mDNSu32)tarindex + 1 >= targsize)
                    return (-1);
                target[tarindex]   |=  (pos - Base64) >> 4;
                target[tarindex+1]  = (mDNSu8)(((pos - Base64) & 0x0f) << 4);
            }
            tarindex++;
            state = 2;
            break;
        case 2:
            if (target) {
                if ((mDNSu32)tarindex + 1 >= targsize)
                    return (-1);
                target[tarindex]   |=  (pos - Base64) >> 2;
                target[tarindex+1]  = (mDNSu8)(((pos - Base64) & 0x03) << 6);
            }
            tarindex++;
            state = 3;
            break;
        case 3:
            if (target) {
                if ((mDNSu32)tarindex >= targsize)
                    return (-1);
                target[tarindex] |= (pos - Base64);
            }
            tarindex++;
            state = 0;
            break;
        default:
            return -1;
        }
    }

    /*
     * We are done decoding Base-64 chars.  Let's see if we ended
     * on a byte boundary, and/or with erroneous trailing characters.
     */

    if (ch == Pad64) {      /* We got a pad char. */
        ch = *src++;        /* Skip it, get next. */
        switch (state) {
        case 0:     /* Invalid = in first position */
        case 1:     /* Invalid = in second position */
            return (-1);

        case 2:     /* Valid, means one byte of info */
            /* Skip any number of spaces. */
            for ((void)mDNSNULL; ch != '\0'; ch = *src++)
                if (!mDNSisspace(ch))
                    break;
            /* Make sure there is another trailing = sign. */
            if (ch != Pad64)
                return (-1);
            ch = *src++;        /* Skip the = */
        /* Fall through to "single trailing =" case. */
        /* FALLTHROUGH */

        case 3:     /* Valid, means two bytes of info */
            /*
             * We know this char is an =.  Is there anything but
             * whitespace after it?
             */
            for ((void)mDNSNULL; ch != '\0'; ch = *src++)
                if (!mDNSisspace(ch))
                    return (-1);

            /*
             * Now make sure for cases 2 and 3 that the "extra"
             * bits that slopped past the last full byte were
             * zeros.  If we don't check them, they become a
             * subliminal channel.
             */
            if (target && target[tarindex] != 0)
                return (-1);
        }
    } else {
        /*
         * We ended by seeing the end of the string.  Make sure we
         * have no partial bytes lying around.
         */
        if (state != 0)
            return (-1);
    }

    return (tarindex);
}


// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - API exported to mDNS Core
#endif

// Constants
#define HMAC_IPAD   0x36
#define HMAC_OPAD   0x5c
#define MD5_LEN     16

#define HMAC_MD5_AlgName (*(const domainname*) "\010" "hmac-md5" "\007" "sig-alg" "\003" "reg" "\003" "int")

// Adapted from Appendix, RFC 2104
mDNSlocal void DNSDigest_ConstructHMACKey(DomainAuthInfo *info, const mDNSu8 *key, mDNSu32 len)
{
    MD5_CTX k;
    mDNSu8 buf[MD5_LEN];
    int i;

    // If key is longer than HMAC_LEN reset it to MD5(key)
    if (len > HMAC_LEN)
    {
        MD5_Init(&k);
        MD5_Update(&k, key, len);
        MD5_Final(buf, &k);
        key = buf;
        len = MD5_LEN;
    }

    // store key in pads
    mDNSPlatformMemZero(info->keydata_ipad, HMAC_LEN);
    mDNSPlatformMemZero(info->keydata_opad, HMAC_LEN);
    mDNSPlatformMemCopy(info->keydata_ipad, key, len);
    mDNSPlatformMemCopy(info->keydata_opad, key, len);

    // XOR key with ipad and opad values
    for (i = 0; i < HMAC_LEN; i++)
    {
        info->keydata_ipad[i] ^= HMAC_IPAD;
        info->keydata_opad[i] ^= HMAC_OPAD;
    }

}

mDNSexport mDNSs32 DNSDigest_ConstructHMACKeyfromBase64(DomainAuthInfo *info, const char *b64key)
{
    mDNSu8 keybuf[1024];
    mDNSs32 keylen = DNSDigest_Base64ToBin(b64key, keybuf, sizeof(keybuf));
    if (keylen < 0) return(keylen);
    DNSDigest_ConstructHMACKey(info, keybuf, (mDNSu32)keylen);
    return(keylen);
}

mDNSexport void DNSDigest_SignMessage(DNSMessage *msg, mDNSu8 **end, DomainAuthInfo *info, mDNSu16 tcode)
{
    AuthRecord tsig;
    mDNSu8  *rdata, *const countPtr = (mDNSu8 *)&msg->h.numAdditionals; // Get existing numAdditionals value
    mDNSu32 utc32;
    mDNSu8 utc48[6];
    mDNSu8 digest[MD5_LEN];
    mDNSu8 *ptr = *end;
    mDNSu32 len;
    mDNSOpaque16 buf;
    MD5_CTX c;
    mDNSu16 numAdditionals = (mDNSu16)((mDNSu16)countPtr[0] << 8 | countPtr[1]);

    // Init MD5 context, digest inner key pad and message
    MD5_Init(&c);
    MD5_Update(&c, info->keydata_ipad, HMAC_LEN);
    MD5_Update(&c, (mDNSu8 *)msg, (unsigned long)(*end - (mDNSu8 *)msg));

    // Construct TSIG RR, digesting variables as apporpriate
    mDNS_SetupResourceRecord(&tsig, mDNSNULL, 0, kDNSType_TSIG, 0, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);

    // key name
    AssignDomainName(&tsig.namestorage, &info->keyname);
    MD5_Update(&c, info->keyname.c, DomainNameLength(&info->keyname));

    // class
    tsig.resrec.rrclass = kDNSQClass_ANY;
    buf = mDNSOpaque16fromIntVal(kDNSQClass_ANY);
    MD5_Update(&c, buf.b, sizeof(mDNSOpaque16));

    // ttl
    tsig.resrec.rroriginalttl = 0;
    MD5_Update(&c, (mDNSu8 *)&tsig.resrec.rroriginalttl, sizeof(tsig.resrec.rroriginalttl));

    // alg name
    AssignDomainName(&tsig.resrec.rdata->u.name, &HMAC_MD5_AlgName);
    len = DomainNameLength(&HMAC_MD5_AlgName);
    rdata = tsig.resrec.rdata->u.data + len;
    MD5_Update(&c, HMAC_MD5_AlgName.c, len);

    // time
    // get UTC (universal time), convert to 48-bit unsigned in network byte order
    utc32 = (mDNSu32)mDNSPlatformUTC();
    if (utc32 == (unsigned)-1) { LogMsg("ERROR: DNSDigest_SignMessage - mDNSPlatformUTC returned bad time -1"); *end = mDNSNULL; }
    utc48[0] = 0;
    utc48[1] = 0;
    utc48[2] = (mDNSu8)((utc32 >> 24) & 0xff);
    utc48[3] = (mDNSu8)((utc32 >> 16) & 0xff);
    utc48[4] = (mDNSu8)((utc32 >>  8) & 0xff);
    utc48[5] = (mDNSu8)( utc32        & 0xff);

    mDNSPlatformMemCopy(rdata, utc48, 6);
    rdata += 6;
    MD5_Update(&c, utc48, 6);

    // 300 sec is fudge recommended in RFC 2485
    rdata[0] = (mDNSu8)((300 >> 8)  & 0xff);
    rdata[1] = (mDNSu8)( 300        & 0xff);
    MD5_Update(&c, rdata, sizeof(mDNSOpaque16));
    rdata += sizeof(mDNSOpaque16);

    // digest error (tcode) and other data len (zero) - we'll add them to the rdata later
    buf.b[0] = (mDNSu8)((tcode >> 8) & 0xff);
    buf.b[1] = (mDNSu8)( tcode       & 0xff);
    MD5_Update(&c, buf.b, sizeof(mDNSOpaque16));  // error
    buf.NotAnInteger = 0;
    MD5_Update(&c, buf.b, sizeof(mDNSOpaque16));  // other data len

    // finish the message & tsig var hash
    MD5_Final(digest, &c);

    // perform outer MD5 (outer key pad, inner digest)
    MD5_Init(&c);
    MD5_Update(&c, info->keydata_opad, HMAC_LEN);
    MD5_Update(&c, digest, MD5_LEN);
    MD5_Final(digest, &c);

    // set remaining rdata fields
    rdata[0] = (mDNSu8)((MD5_LEN >> 8)  & 0xff);
    rdata[1] = (mDNSu8)( MD5_LEN        & 0xff);
    rdata += sizeof(mDNSOpaque16);
    mDNSPlatformMemCopy(rdata, digest, MD5_LEN);                          // MAC
    rdata += MD5_LEN;
    rdata[0] = msg->h.id.b[0];                                            // original ID
    rdata[1] = msg->h.id.b[1];
    rdata[2] = (mDNSu8)((tcode >> 8) & 0xff);
    rdata[3] = (mDNSu8)( tcode       & 0xff);
    rdata[4] = 0;                                                         // other data len
    rdata[5] = 0;
    rdata += 6;

    tsig.resrec.rdlength = (mDNSu16)(rdata - tsig.resrec.rdata->u.data);
    *end = PutResourceRecordTTLJumbo(msg, ptr, &numAdditionals, &tsig.resrec, 0);
    if (!*end) { LogMsg("ERROR: DNSDigest_SignMessage - could not put TSIG"); *end = mDNSNULL; return; }

    // Write back updated numAdditionals value
    countPtr[0] = (mDNSu8)(numAdditionals >> 8);
    countPtr[1] = (mDNSu8)(numAdditionals &  0xFF);
}

mDNSexport mDNSBool DNSDigest_VerifyMessage(DNSMessage *msg, mDNSu8 *end, LargeCacheRecord * lcr, DomainAuthInfo *info, mDNSu16 * rcode, mDNSu16 * tcode)
{
    mDNSu8          *   ptr = (mDNSu8*) &lcr->r.resrec.rdata->u.data;
    mDNSs32 now;
    mDNSs32 then;
    mDNSu8 thisDigest[MD5_LEN];
    mDNSu8 thatDigest[MD5_LEN];
    mDNSOpaque16 buf;
    mDNSu8 utc48[6];
    mDNSs32 delta;
    mDNSu16 fudge;
    domainname      *   algo;
    MD5_CTX c;
    mDNSBool ok = mDNSfalse;

    // We only support HMAC-MD5 for now

    algo = (domainname*) ptr;

    if (!SameDomainName(algo, &HMAC_MD5_AlgName))
    {
        LogMsg("ERROR: DNSDigest_VerifyMessage - TSIG algorithm not supported: %##s", algo->c);
        *rcode = kDNSFlag1_RC_NotAuth;
        *tcode = TSIG_ErrBadKey;
        ok = mDNSfalse;
        goto exit;
    }

    ptr += DomainNameLength(algo);

    // Check the times

    now = mDNSPlatformUTC();
    if (now == -1)
    {
        LogMsg("ERROR: DNSDigest_VerifyMessage - mDNSPlatformUTC returned bad time -1");
        *rcode = kDNSFlag1_RC_NotAuth;
        *tcode = TSIG_ErrBadTime;
        ok = mDNSfalse;
        goto exit;
    }

    // Get the 48 bit time field, skipping over the first word

    utc48[0] = *ptr++;
    utc48[1] = *ptr++;
    utc48[2] = *ptr++;
    utc48[3] = *ptr++;
    utc48[4] = *ptr++;
    utc48[5] = *ptr++;

    then  = (mDNSs32)NToH32(utc48 + sizeof(mDNSu16));

    fudge = NToH16(ptr);

    ptr += sizeof(mDNSu16);

    delta = (now > then) ? now - then : then - now;

    if (delta > fudge)
    {
        LogMsg("ERROR: DNSDigest_VerifyMessage - time skew > %d", fudge);
        *rcode = kDNSFlag1_RC_NotAuth;
        *tcode = TSIG_ErrBadTime;
        ok = mDNSfalse;
        goto exit;
    }

    // MAC size

    ptr += sizeof(mDNSu16);

    // MAC

    mDNSPlatformMemCopy(thatDigest, ptr, MD5_LEN);

    // Init MD5 context, digest inner key pad and message

    MD5_Init(&c);
    MD5_Update(&c, info->keydata_ipad, HMAC_LEN);
    MD5_Update(&c, (mDNSu8*) msg, (unsigned long)(end - (mDNSu8*) msg));

    // Key name

    MD5_Update(&c, lcr->r.resrec.name->c, DomainNameLength(lcr->r.resrec.name));

    // Class name

    buf = mDNSOpaque16fromIntVal(lcr->r.resrec.rrclass);
    MD5_Update(&c, buf.b, sizeof(mDNSOpaque16));

    // TTL

    MD5_Update(&c, (mDNSu8*) &lcr->r.resrec.rroriginalttl, sizeof(lcr->r.resrec.rroriginalttl));

    // Algorithm

    MD5_Update(&c, algo->c, DomainNameLength(algo));

    // Time

    MD5_Update(&c, utc48, 6);

    // Fudge

    buf = mDNSOpaque16fromIntVal(fudge);
    MD5_Update(&c, buf.b, sizeof(mDNSOpaque16));

    // Digest error and other data len (both zero) - we'll add them to the rdata later

    buf.NotAnInteger = 0;
    MD5_Update(&c, buf.b, sizeof(mDNSOpaque16));  // error
    MD5_Update(&c, buf.b, sizeof(mDNSOpaque16));  // other data len

    // Finish the message & tsig var hash

    MD5_Final(thisDigest, &c);

    // perform outer MD5 (outer key pad, inner digest)

    MD5_Init(&c);
    MD5_Update(&c, info->keydata_opad, HMAC_LEN);
    MD5_Update(&c, thisDigest, MD5_LEN);
    MD5_Final(thisDigest, &c);

    if (!mDNSPlatformMemSame(thisDigest, thatDigest, MD5_LEN))
    {
        LogMsg("ERROR: DNSDigest_VerifyMessage - bad signature");
        *rcode = kDNSFlag1_RC_NotAuth;
        *tcode = TSIG_ErrBadSig;
        ok = mDNSfalse;
        goto exit;
    }

    // set remaining rdata fields
    ok = mDNStrue;

exit:

    return ok;
}


#ifdef __cplusplus
}
#endif
