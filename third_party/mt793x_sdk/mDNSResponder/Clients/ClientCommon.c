/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 * ("Apple") in consideration of your agreement to the following terms, and your
 * use, installation, modification or redistribution of this Apple software
 * constitutes acceptance of these terms.  If you do not agree with these terms,
 * please do not use, install, modify or redistribute this Apple software.
 *
 * In consideration of your agreement to abide by the following terms, and subject
 * to these terms, Apple grants you a personal, non-exclusive license, under Apple's
 * copyrights in this original Apple software (the "Apple Software"), to use,
 * reproduce, modify and redistribute the Apple Software, with or without
 * modifications, in source and/or binary forms; provided that if you redistribute
 * the Apple Software in its entirety and without modifications, you must retain
 * this notice and the following text and disclaimers in all such redistributions of
 * the Apple Software.  Neither the name, trademarks, service marks or logos of
 * Apple Computer, Inc. may be used to endorse or promote products derived from the
 * Apple Software without specific prior written permission from Apple.  Except as
 * expressly stated in this notice, no other rights or licenses, express or implied,
 * are granted by Apple herein, including but not limited to any patent rights that
 * may be infringed by your derivative works or by other works in which the Apple
 * Software may be incorporated.
 *
 * The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 * COMBINATION WITH YOUR PRODUCTS.
 *
 * IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
 * OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdio.h>          // For stdout, stderr

#include "ClientCommon.h"

const char *GetNextLabel(const char *cstr, char label[64])
{
    char *ptr = label;
    while (*cstr && *cstr != '.')               // While we have characters in the label...
    {
        char c = *cstr++;
        if (c == '\\' && *cstr)                 // If we have a backslash, and it's not the last character of the string
        {
            c = *cstr++;
            if (isdigit(cstr[-1]) && isdigit(cstr[0]) && isdigit(cstr[1]))
            {
                int v0 = cstr[-1] - '0';                        // then interpret as three-digit decimal
                int v1 = cstr[ 0] - '0';
                int v2 = cstr[ 1] - '0';
                int val = v0 * 100 + v1 * 10 + v2;
                // If valid three-digit decimal value, use it
                // Note that although ascii nuls are possible in DNS labels
                // we're building a C string here so we have no way to represent that
                if (val == 0) val = '-';
                if (val <= 255) { c = (char)val; cstr += 2; }
            }
        }
        *ptr++ = c;
        if (ptr >= label+64) { label[63] = 0; return(NULL); }   // Illegal label more than 63 bytes
    }
    *ptr = 0;                                                   // Null-terminate label text
    if (ptr == label) return(NULL);                             // Illegal empty label
    if (*cstr) cstr++;                                          // Skip over the trailing dot (if present)
    return(cstr);
}
