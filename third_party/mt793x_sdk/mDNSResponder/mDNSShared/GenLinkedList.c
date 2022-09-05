/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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

    File:		GenLinkedList.c

    Contains:	implementation of generic linked lists.

    Version:	1.0
    Tabs:		4 spaces
 */

#include "GenLinkedList.h"


// Return the link pointer contained within element e at offset o.
#define     GETLINK( e, o)          ( *(void**)((char*) (e) + (o)) )

// Assign the link pointer l to element e at offset o.
#define     ASSIGNLINK( e, l, o)    ( *((void**)((char*) (e) + (o))) = (l))


//		GenLinkedList		/////////////////////////////////////////////////////////////

void        InitLinkedList( GenLinkedList *pList, size_t linkOffset)
/* Initialize the block of memory pointed to by pList as a linked list. */
{
    pList->Head = NULL;
    pList->Tail = NULL;
    pList->LinkOffset = linkOffset;
}


void        AddToTail( GenLinkedList *pList, void *elem)
/* Add a linked list element to the tail of the list. */
{
    if ( pList->Tail) {
        ASSIGNLINK( pList->Tail, elem, pList->LinkOffset);
    } else
        pList->Head = elem;
    ASSIGNLINK( elem, NULL, pList->LinkOffset);

    pList->Tail = elem;
}


void        AddToHead( GenLinkedList *pList, void *elem)
/* Add a linked list element to the head of the list. */
{
    ASSIGNLINK( elem, pList->Head, pList->LinkOffset);
    if ( pList->Tail == NULL)
        pList->Tail = elem;

    pList->Head = elem;
}


int     RemoveFromList( GenLinkedList *pList, void *elem)
/* Remove a linked list element from the list. Return 0 if it was not found. */
/* If the element is removed, its link will be set to NULL. */
{
    void    *iElem, *lastElem;

    for ( iElem = pList->Head, lastElem = NULL; iElem; iElem = GETLINK( iElem, pList->LinkOffset)) {
        if ( iElem == elem) {
            if ( lastElem) {        // somewhere past the head
                ASSIGNLINK( lastElem, GETLINK( elem, pList->LinkOffset), pList->LinkOffset);
            } else {                // at the head
                pList->Head = GETLINK( elem, pList->LinkOffset);
            }
            if ( pList->Tail == elem)
                pList->Tail = lastElem ? lastElem : NULL;
            ASSIGNLINK( elem, NULL, pList->LinkOffset);         // maybe catch a stale reference bug.
            return 1;
        }
        lastElem = iElem;
    }

    return 0;
}


int         ReplaceElem( GenLinkedList *pList, void *elemInList, void *newElem)
/* Replace an element in the list with a new element, in the same position. */
{
    void    *iElem, *lastElem;

    if ( elemInList == NULL || newElem == NULL)
        return 0;

    for ( iElem = pList->Head, lastElem = NULL; iElem; iElem = GETLINK( iElem, pList->LinkOffset))
    {
        if ( iElem == elemInList)
        {
            ASSIGNLINK( newElem, GETLINK( elemInList, pList->LinkOffset), pList->LinkOffset);
            if ( lastElem)      // somewhere past the head
            {
                ASSIGNLINK( lastElem, newElem, pList->LinkOffset);
            }
            else                // at the head
            {
                pList->Head = newElem;
            }
            if ( pList->Tail == elemInList)
                pList->Tail = newElem;
            return 1;
        }
        lastElem = iElem;
    }

    return 0;
}


//		GenDoubleLinkedList		/////////////////////////////////////////////////////////

void        InitDoubleLinkedList( GenDoubleLinkedList *pList, size_t fwdLinkOffset,
                                  size_t backLinkOffset)
/* Initialize the block of memory pointed to by pList as a double linked list. */
{
    pList->Head = NULL;
    pList->Tail = NULL;
    pList->FwdLinkOffset = fwdLinkOffset;
    pList->BackLinkOffset = backLinkOffset;
}


void            DLLAddToHead( GenDoubleLinkedList *pList, void *elem)
/* Add a linked list element to the head of the list. */
{
    void        *pNext;

    pNext = pList->Head;

    // fix up the forward links
    ASSIGNLINK( elem, pList->Head, pList->FwdLinkOffset);
    pList->Head = elem;

    // fix up the backward links
    if ( pNext) {
        ASSIGNLINK( pNext, elem, pList->BackLinkOffset);
    } else
        pList->Tail = elem;
    ASSIGNLINK( elem, NULL, pList->BackLinkOffset);
}


void            DLLRemoveFromList( GenDoubleLinkedList *pList, void *elem)
/* Remove a linked list element from the list. */
/* When the element is removed, its link will be set to NULL. */
{
    void        *pNext, *pPrev;

    pNext = GETLINK( elem, pList->FwdLinkOffset);
    pPrev = GETLINK( elem, pList->BackLinkOffset);

    // fix up the forward links
    if ( pPrev)
        ASSIGNLINK( pPrev, pNext, pList->FwdLinkOffset);
    else
        pList->Head = pNext;

    // fix up the backward links
    if ( pNext)
        ASSIGNLINK( pNext, pPrev, pList->BackLinkOffset);
    else
        pList->Tail = pPrev;

    ASSIGNLINK( elem, NULL, pList->FwdLinkOffset);
    ASSIGNLINK( elem, NULL, pList->BackLinkOffset);
}


//		GenLinkedOffsetList		/////////////////////////////////////////////////////

// Extract the Next offset from element
#define     GETOFFSET( e, o)    ( *(size_t*)((char*) (e) + (o)) )

static void     AssignOffsetLink( void *elem, void *link, size_t linkOffset);


static void AssignOffsetLink( void *elem, void *link, size_t linkOffset)
// Assign link to elem as an offset from elem. Assign 0 to elem if link is NULL.
{
    GETOFFSET( elem, linkOffset) = link ? (size_t) link - (size_t) elem : 0;
}


void        *GetHeadPtr( GenLinkedOffsetList *pList)
/* Return a pointer to the head element of a list, or NULL if none. */
{
    return pList->Head ? ( (char*) (pList) + pList->Head) : NULL;
}


void        *GetTailPtr( GenLinkedOffsetList *pList)
/* Return a pointer to the tail element of a list, or NULL if none. */
{
    return pList->Tail ? ( (char*) (pList) + pList->Tail) : NULL;
}


void        *GetOffsetLink( GenLinkedOffsetList *pList, void *elem)
/* Return the link pointer contained within element e for pList, or NULL if it is 0. */
{
    size_t nextOffset;

    nextOffset = GETOFFSET( elem, pList->LinkOffset);

    return nextOffset ? (char*) elem + nextOffset : NULL;
}


void        InitLinkedOffsetList( GenLinkedOffsetList *pList, size_t linkOffset)
/* Initialize the block of memory pointed to by pList as a linked list. */
{
    pList->Head = 0;
    pList->Tail = 0;
    pList->LinkOffset = linkOffset;
}


void        OffsetAddToTail( GenLinkedOffsetList *pList, void *elem)
/* Add a linked list element to the tail of the list. */
{
    if ( pList->Tail) {
        AssignOffsetLink( GetTailPtr( pList), elem, pList->LinkOffset);
    } else
        pList->Head = (size_t) elem - (size_t) pList;
    AssignOffsetLink( elem, NULL, pList->LinkOffset);

    pList->Tail = (size_t) elem - (size_t) pList;
}


void        OffsetAddToHead( GenLinkedOffsetList *pList, void *elem)
/* Add a linked list element to the head of the list. */
{
    AssignOffsetLink( elem, GetHeadPtr( pList), pList->LinkOffset);
    if ( pList->Tail == 0)
        pList->Tail = (size_t) elem - (size_t) pList;

    pList->Head = (size_t) elem - (size_t) pList;
}


int     OffsetRemoveFromList( GenLinkedOffsetList *pList, void *elem)
/* Remove a linked list element from the list. Return 0 if it was not found. */
/* If the element is removed, its link will be set to NULL. */
{
    void    *iElem, *lastElem;

    for ( iElem = GetHeadPtr( pList), lastElem = NULL; iElem;
          iElem = GetOffsetLink( pList, iElem))
    {
        if ( iElem == elem) {
            if ( lastElem) {        // somewhere past the head
                AssignOffsetLink( lastElem, GetOffsetLink( pList, elem), pList->LinkOffset);
            } else {                // at the head
                iElem = GetOffsetLink( pList, elem);
                pList->Head = iElem ? (size_t) iElem - (size_t) pList : 0;
            }
            if ( GetTailPtr( pList) == elem)
                pList->Tail = lastElem ? (size_t) lastElem - (size_t) pList : 0;
            AssignOffsetLink( elem, NULL, pList->LinkOffset);   // maybe catch a stale reference bug.
            return 1;
        }
        lastElem = iElem;
    }

    return 0;
}


int         OffsetReplaceElem( GenLinkedOffsetList *pList, void *elemInList, void *newElem)
/* Replace an element in the list with a new element, in the same position. */
{
    void    *iElem, *lastElem;

    if ( elemInList == NULL || newElem == NULL)
        return 0;

    for ( iElem = GetHeadPtr( pList), lastElem = NULL; iElem;
          iElem = GetOffsetLink( pList, iElem))
    {
        if ( iElem == elemInList)
        {
            AssignOffsetLink( newElem, GetOffsetLink( pList, elemInList), pList->LinkOffset);
            if ( lastElem)      // somewhere past the head
            {
                AssignOffsetLink( lastElem, newElem, pList->LinkOffset);
            }
            else                // at the head
            {
                pList->Head = (size_t) newElem - (size_t) pList;
            }
            if ( GetTailPtr( pList) == elemInList)
                pList->Tail = (size_t) newElem - (size_t) pList;
            return 1;
        }
        lastElem = iElem;
    }

    return 0;
}


