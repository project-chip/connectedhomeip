/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file contains a preprocessor macro that resolves to a
 *      constant expression that evaluates to the size of an array
 *      element with alignment padding.
 */

#ifndef SYSTEMALIGNSIZE_H
#define SYSTEMALIGNSIZE_H

#define WEAVE_SYSTEM_ALIGN_SIZE(ELEMENT, ALIGNMENT) (((ELEMENT) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))

#endif // defined(SYSTEMALIGNSIZE_H)
