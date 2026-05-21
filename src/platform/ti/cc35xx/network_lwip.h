/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NETWORK_LWIP_H_
#define NETWORK_LWIP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
    \brief network init

    This function initialize the network stack and rise the LWIP thread
    Should be called at the begining of the code
*/
void network_stack_init();

/*!
    \brief returns an handler for the station network if

    returns an handler for the station network if

    \return         pointer to the station network if
*/
void * network_get_sta_if();

/*!
    \brief returns an handler for the access point network if

    returns an handler for the access point network if

    \return         pointer to the access point network if
*/
void * network_get_ap_if();

/*!
    \brief set the interface up

    This function sets the interface up, should be called when
    there is a link
    \param[in]      newif   handler to the network interface

    \sa             network_set_down
*/
void network_set_up(void * newif);

/*!
    \brief set the interface down

    This function sets the interface down, should be called when
    there link is lost (wlan disconnected

    \param[in]      newif   handler to the network interface

    \sa             network_set_up
*/
void network_set_down(void * newif);

/*!
    \brief adds station

    This function adds the station network interface to the network stack
    should be called when role station is up (before connection)

    \sa             network_stack_remove_if_sta
*/
void network_stack_add_if_sta();

/*!
    \brief removes station

    This function removes the station network interface to the network stack
    should be called when role station is down

    \sa             network_stack_add_if_sta
*/
void network_stack_remove_if_sta();

/*!
    \brief adds access point

    This function adds the access point network interface to the network stack
    should be called when role ap is up

    \sa             network_stack_remove_if_ap
*/
void network_stack_add_if_ap();

/*!
    \brief removes access point

    This function removes the access point network interface to the network stack
    should be called when role ap is down

    \sa             network_stack_add_if_ap
*/
void network_stack_remove_if_ap();

/*!
    \brief rsend arp request

*/

int update_arp(void * ip_addr);

#ifdef __cplusplus
}
#endif
#endif /* NETWORK_LWIP_H_ */
