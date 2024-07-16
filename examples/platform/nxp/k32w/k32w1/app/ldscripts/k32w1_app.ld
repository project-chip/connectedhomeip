/*
** Copyright (c) 2023 Project CHIP Authors
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
** http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

m_smu2_data_start                       = 0x489C0080;
m_smu2_data_end                         = 0x489C537B;
m_smu2_data_size                        = m_smu2_data_end - m_smu2_data_start + 1;

/* Specify the extra application specific memory areas */
MEMORY
{
  m_smu2_data               (RW)  : ORIGIN = m_smu2_data_start, LENGTH = m_smu2_data_size
}


/* Define the extra application specific output sections */
SECTIONS
{
  .smu2 (NOLOAD) :
  {
    /* This is used by the startup in order to initialize the free .smu2 section */
    . = ALIGN(4);
    __START_SMU2 = .;
    __smu2_start__ = .;
    *(.smu2)
    /* These input section descriptions should not be changed as they match */
    /* specific Matter instances/global variables. */
    *(.bss.*chip*Server*sServer*)
    *(*gImageProcessor)
    *(*gApplicationProcessor)
    *(.bss.*ThreadStackManagerImpl*sInstance*)
    . = ALIGN(4);
    __smu2_end__ = .;
    __END_SMU2 = .;
    ASSERT(__smu2_end__ > 18K, "SMU2 section unexpected end address, check variable names");
  } > m_smu2_data
}


INCLUDE connectivity.ld
