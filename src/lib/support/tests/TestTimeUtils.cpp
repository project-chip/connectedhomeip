/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements a process to effect a functional test for
 *      the CHIP date and time support utilities.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <support/TimeUtils.h>
#include <support/UnitTestRegistration.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;

static void Abort()
{
    abort();
}

void TestAssert(bool assert, const char * msg)
{
    if (!assert)
    {
        printf("%s\n", msg);
        Abort();
    }
}

struct OrdinalDateTestValue
{
    uint16_t DayOfYear;
    uint8_t Month;
    uint8_t DayOfMonth;
};

// clang-format off
OrdinalDateTestValue StandardYearOrdinalDates[] =
{
    { 1, 1, 1 },
    { 2, 1, 2 },
    { 3, 1, 3 },
    { 4, 1, 4 },
    { 5, 1, 5 },
    { 6, 1, 6 },
    { 7, 1, 7 },
    { 8, 1, 8 },
    { 9, 1, 9 },
    { 10, 1, 10 },
    { 11, 1, 11 },
    { 12, 1, 12 },
    { 13, 1, 13 },
    { 14, 1, 14 },
    { 15, 1, 15 },
    { 16, 1, 16 },
    { 17, 1, 17 },
    { 18, 1, 18 },
    { 19, 1, 19 },
    { 20, 1, 20 },
    { 21, 1, 21 },
    { 22, 1, 22 },
    { 23, 1, 23 },
    { 24, 1, 24 },
    { 25, 1, 25 },
    { 26, 1, 26 },
    { 27, 1, 27 },
    { 28, 1, 28 },
    { 29, 1, 29 },
    { 30, 1, 30 },
    { 31, 1, 31 },
    { 32, 2, 1 },
    { 33, 2, 2 },
    { 34, 2, 3 },
    { 35, 2, 4 },
    { 36, 2, 5 },
    { 37, 2, 6 },
    { 38, 2, 7 },
    { 39, 2, 8 },
    { 40, 2, 9 },
    { 41, 2, 10 },
    { 42, 2, 11 },
    { 43, 2, 12 },
    { 44, 2, 13 },
    { 45, 2, 14 },
    { 46, 2, 15 },
    { 47, 2, 16 },
    { 48, 2, 17 },
    { 49, 2, 18 },
    { 50, 2, 19 },
    { 51, 2, 20 },
    { 52, 2, 21 },
    { 53, 2, 22 },
    { 54, 2, 23 },
    { 55, 2, 24 },
    { 56, 2, 25 },
    { 57, 2, 26 },
    { 58, 2, 27 },
    { 59, 2, 28 },
    { 60, 3, 1 },
    { 61, 3, 2 },
    { 62, 3, 3 },
    { 63, 3, 4 },
    { 64, 3, 5 },
    { 65, 3, 6 },
    { 66, 3, 7 },
    { 67, 3, 8 },
    { 68, 3, 9 },
    { 69, 3, 10 },
    { 70, 3, 11 },
    { 71, 3, 12 },
    { 72, 3, 13 },
    { 73, 3, 14 },
    { 74, 3, 15 },
    { 75, 3, 16 },
    { 76, 3, 17 },
    { 77, 3, 18 },
    { 78, 3, 19 },
    { 79, 3, 20 },
    { 80, 3, 21 },
    { 81, 3, 22 },
    { 82, 3, 23 },
    { 83, 3, 24 },
    { 84, 3, 25 },
    { 85, 3, 26 },
    { 86, 3, 27 },
    { 87, 3, 28 },
    { 88, 3, 29 },
    { 89, 3, 30 },
    { 90, 3, 31 },
    { 91, 4, 1 },
    { 92, 4, 2 },
    { 93, 4, 3 },
    { 94, 4, 4 },
    { 95, 4, 5 },
    { 96, 4, 6 },
    { 97, 4, 7 },
    { 98, 4, 8 },
    { 99, 4, 9 },
    { 100, 4, 10 },
    { 101, 4, 11 },
    { 102, 4, 12 },
    { 103, 4, 13 },
    { 104, 4, 14 },
    { 105, 4, 15 },
    { 106, 4, 16 },
    { 107, 4, 17 },
    { 108, 4, 18 },
    { 109, 4, 19 },
    { 110, 4, 20 },
    { 111, 4, 21 },
    { 112, 4, 22 },
    { 113, 4, 23 },
    { 114, 4, 24 },
    { 115, 4, 25 },
    { 116, 4, 26 },
    { 117, 4, 27 },
    { 118, 4, 28 },
    { 119, 4, 29 },
    { 120, 4, 30 },
    { 121, 5, 1 },
    { 122, 5, 2 },
    { 123, 5, 3 },
    { 124, 5, 4 },
    { 125, 5, 5 },
    { 126, 5, 6 },
    { 127, 5, 7 },
    { 128, 5, 8 },
    { 129, 5, 9 },
    { 130, 5, 10 },
    { 131, 5, 11 },
    { 132, 5, 12 },
    { 133, 5, 13 },
    { 134, 5, 14 },
    { 135, 5, 15 },
    { 136, 5, 16 },
    { 137, 5, 17 },
    { 138, 5, 18 },
    { 139, 5, 19 },
    { 140, 5, 20 },
    { 141, 5, 21 },
    { 142, 5, 22 },
    { 143, 5, 23 },
    { 144, 5, 24 },
    { 145, 5, 25 },
    { 146, 5, 26 },
    { 147, 5, 27 },
    { 148, 5, 28 },
    { 149, 5, 29 },
    { 150, 5, 30 },
    { 151, 5, 31 },
    { 152, 6, 1 },
    { 153, 6, 2 },
    { 154, 6, 3 },
    { 155, 6, 4 },
    { 156, 6, 5 },
    { 157, 6, 6 },
    { 158, 6, 7 },
    { 159, 6, 8 },
    { 160, 6, 9 },
    { 161, 6, 10 },
    { 162, 6, 11 },
    { 163, 6, 12 },
    { 164, 6, 13 },
    { 165, 6, 14 },
    { 166, 6, 15 },
    { 167, 6, 16 },
    { 168, 6, 17 },
    { 169, 6, 18 },
    { 170, 6, 19 },
    { 171, 6, 20 },
    { 172, 6, 21 },
    { 173, 6, 22 },
    { 174, 6, 23 },
    { 175, 6, 24 },
    { 176, 6, 25 },
    { 177, 6, 26 },
    { 178, 6, 27 },
    { 179, 6, 28 },
    { 180, 6, 29 },
    { 181, 6, 30 },
    { 182, 7, 1 },
    { 183, 7, 2 },
    { 184, 7, 3 },
    { 185, 7, 4 },
    { 186, 7, 5 },
    { 187, 7, 6 },
    { 188, 7, 7 },
    { 189, 7, 8 },
    { 190, 7, 9 },
    { 191, 7, 10 },
    { 192, 7, 11 },
    { 193, 7, 12 },
    { 194, 7, 13 },
    { 195, 7, 14 },
    { 196, 7, 15 },
    { 197, 7, 16 },
    { 198, 7, 17 },
    { 199, 7, 18 },
    { 200, 7, 19 },
    { 201, 7, 20 },
    { 202, 7, 21 },
    { 203, 7, 22 },
    { 204, 7, 23 },
    { 205, 7, 24 },
    { 206, 7, 25 },
    { 207, 7, 26 },
    { 208, 7, 27 },
    { 209, 7, 28 },
    { 210, 7, 29 },
    { 211, 7, 30 },
    { 212, 7, 31 },
    { 213, 8, 1 },
    { 214, 8, 2 },
    { 215, 8, 3 },
    { 216, 8, 4 },
    { 217, 8, 5 },
    { 218, 8, 6 },
    { 219, 8, 7 },
    { 220, 8, 8 },
    { 221, 8, 9 },
    { 222, 8, 10 },
    { 223, 8, 11 },
    { 224, 8, 12 },
    { 225, 8, 13 },
    { 226, 8, 14 },
    { 227, 8, 15 },
    { 228, 8, 16 },
    { 229, 8, 17 },
    { 230, 8, 18 },
    { 231, 8, 19 },
    { 232, 8, 20 },
    { 233, 8, 21 },
    { 234, 8, 22 },
    { 235, 8, 23 },
    { 236, 8, 24 },
    { 237, 8, 25 },
    { 238, 8, 26 },
    { 239, 8, 27 },
    { 240, 8, 28 },
    { 241, 8, 29 },
    { 242, 8, 30 },
    { 243, 8, 31 },
    { 244, 9, 1 },
    { 245, 9, 2 },
    { 246, 9, 3 },
    { 247, 9, 4 },
    { 248, 9, 5 },
    { 249, 9, 6 },
    { 250, 9, 7 },
    { 251, 9, 8 },
    { 252, 9, 9 },
    { 253, 9, 10 },
    { 254, 9, 11 },
    { 255, 9, 12 },
    { 256, 9, 13 },
    { 257, 9, 14 },
    { 258, 9, 15 },
    { 259, 9, 16 },
    { 260, 9, 17 },
    { 261, 9, 18 },
    { 262, 9, 19 },
    { 263, 9, 20 },
    { 264, 9, 21 },
    { 265, 9, 22 },
    { 266, 9, 23 },
    { 267, 9, 24 },
    { 268, 9, 25 },
    { 269, 9, 26 },
    { 270, 9, 27 },
    { 271, 9, 28 },
    { 272, 9, 29 },
    { 273, 9, 30 },
    { 274, 10, 1 },
    { 275, 10, 2 },
    { 276, 10, 3 },
    { 277, 10, 4 },
    { 278, 10, 5 },
    { 279, 10, 6 },
    { 280, 10, 7 },
    { 281, 10, 8 },
    { 282, 10, 9 },
    { 283, 10, 10 },
    { 284, 10, 11 },
    { 285, 10, 12 },
    { 286, 10, 13 },
    { 287, 10, 14 },
    { 288, 10, 15 },
    { 289, 10, 16 },
    { 290, 10, 17 },
    { 291, 10, 18 },
    { 292, 10, 19 },
    { 293, 10, 20 },
    { 294, 10, 21 },
    { 295, 10, 22 },
    { 296, 10, 23 },
    { 297, 10, 24 },
    { 298, 10, 25 },
    { 299, 10, 26 },
    { 300, 10, 27 },
    { 301, 10, 28 },
    { 302, 10, 29 },
    { 303, 10, 30 },
    { 304, 10, 31 },
    { 305, 11, 1 },
    { 306, 11, 2 },
    { 307, 11, 3 },
    { 308, 11, 4 },
    { 309, 11, 5 },
    { 310, 11, 6 },
    { 311, 11, 7 },
    { 312, 11, 8 },
    { 313, 11, 9 },
    { 314, 11, 10 },
    { 315, 11, 11 },
    { 316, 11, 12 },
    { 317, 11, 13 },
    { 318, 11, 14 },
    { 319, 11, 15 },
    { 320, 11, 16 },
    { 321, 11, 17 },
    { 322, 11, 18 },
    { 323, 11, 19 },
    { 324, 11, 20 },
    { 325, 11, 21 },
    { 326, 11, 22 },
    { 327, 11, 23 },
    { 328, 11, 24 },
    { 329, 11, 25 },
    { 330, 11, 26 },
    { 331, 11, 27 },
    { 332, 11, 28 },
    { 333, 11, 29 },
    { 334, 11, 30 },
    { 335, 12, 1 },
    { 336, 12, 2 },
    { 337, 12, 3 },
    { 338, 12, 4 },
    { 339, 12, 5 },
    { 340, 12, 6 },
    { 341, 12, 7 },
    { 342, 12, 8 },
    { 343, 12, 9 },
    { 344, 12, 10 },
    { 345, 12, 11 },
    { 346, 12, 12 },
    { 347, 12, 13 },
    { 348, 12, 14 },
    { 349, 12, 15 },
    { 350, 12, 16 },
    { 351, 12, 17 },
    { 352, 12, 18 },
    { 353, 12, 19 },
    { 354, 12, 20 },
    { 355, 12, 21 },
    { 356, 12, 22 },
    { 357, 12, 23 },
    { 358, 12, 24 },
    { 359, 12, 25 },
    { 360, 12, 26 },
    { 361, 12, 27 },
    { 362, 12, 28 },
    { 363, 12, 29 },
    { 364, 12, 30 },
    { 365, 12, 31 },
    { 0, 0, 0 }
};

OrdinalDateTestValue LeapYearOrdinalDates[] =
{
    { 1, 1, 1 },
    { 2, 1, 2 },
    { 3, 1, 3 },
    { 4, 1, 4 },
    { 5, 1, 5 },
    { 6, 1, 6 },
    { 7, 1, 7 },
    { 8, 1, 8 },
    { 9, 1, 9 },
    { 10, 1, 10 },
    { 11, 1, 11 },
    { 12, 1, 12 },
    { 13, 1, 13 },
    { 14, 1, 14 },
    { 15, 1, 15 },
    { 16, 1, 16 },
    { 17, 1, 17 },
    { 18, 1, 18 },
    { 19, 1, 19 },
    { 20, 1, 20 },
    { 21, 1, 21 },
    { 22, 1, 22 },
    { 23, 1, 23 },
    { 24, 1, 24 },
    { 25, 1, 25 },
    { 26, 1, 26 },
    { 27, 1, 27 },
    { 28, 1, 28 },
    { 29, 1, 29 },
    { 30, 1, 30 },
    { 31, 1, 31 },
    { 32, 2, 1 },
    { 33, 2, 2 },
    { 34, 2, 3 },
    { 35, 2, 4 },
    { 36, 2, 5 },
    { 37, 2, 6 },
    { 38, 2, 7 },
    { 39, 2, 8 },
    { 40, 2, 9 },
    { 41, 2, 10 },
    { 42, 2, 11 },
    { 43, 2, 12 },
    { 44, 2, 13 },
    { 45, 2, 14 },
    { 46, 2, 15 },
    { 47, 2, 16 },
    { 48, 2, 17 },
    { 49, 2, 18 },
    { 50, 2, 19 },
    { 51, 2, 20 },
    { 52, 2, 21 },
    { 53, 2, 22 },
    { 54, 2, 23 },
    { 55, 2, 24 },
    { 56, 2, 25 },
    { 57, 2, 26 },
    { 58, 2, 27 },
    { 59, 2, 28 },
    { 60, 2, 29 },
    { 61, 3, 1 },
    { 62, 3, 2 },
    { 63, 3, 3 },
    { 64, 3, 4 },
    { 65, 3, 5 },
    { 66, 3, 6 },
    { 67, 3, 7 },
    { 68, 3, 8 },
    { 69, 3, 9 },
    { 70, 3, 10 },
    { 71, 3, 11 },
    { 72, 3, 12 },
    { 73, 3, 13 },
    { 74, 3, 14 },
    { 75, 3, 15 },
    { 76, 3, 16 },
    { 77, 3, 17 },
    { 78, 3, 18 },
    { 79, 3, 19 },
    { 80, 3, 20 },
    { 81, 3, 21 },
    { 82, 3, 22 },
    { 83, 3, 23 },
    { 84, 3, 24 },
    { 85, 3, 25 },
    { 86, 3, 26 },
    { 87, 3, 27 },
    { 88, 3, 28 },
    { 89, 3, 29 },
    { 90, 3, 30 },
    { 91, 3, 31 },
    { 92, 4, 1 },
    { 93, 4, 2 },
    { 94, 4, 3 },
    { 95, 4, 4 },
    { 96, 4, 5 },
    { 97, 4, 6 },
    { 98, 4, 7 },
    { 99, 4, 8 },
    { 100, 4, 9 },
    { 101, 4, 10 },
    { 102, 4, 11 },
    { 103, 4, 12 },
    { 104, 4, 13 },
    { 105, 4, 14 },
    { 106, 4, 15 },
    { 107, 4, 16 },
    { 108, 4, 17 },
    { 109, 4, 18 },
    { 110, 4, 19 },
    { 111, 4, 20 },
    { 112, 4, 21 },
    { 113, 4, 22 },
    { 114, 4, 23 },
    { 115, 4, 24 },
    { 116, 4, 25 },
    { 117, 4, 26 },
    { 118, 4, 27 },
    { 119, 4, 28 },
    { 120, 4, 29 },
    { 121, 4, 30 },
    { 122, 5, 1 },
    { 123, 5, 2 },
    { 124, 5, 3 },
    { 125, 5, 4 },
    { 126, 5, 5 },
    { 127, 5, 6 },
    { 128, 5, 7 },
    { 129, 5, 8 },
    { 130, 5, 9 },
    { 131, 5, 10 },
    { 132, 5, 11 },
    { 133, 5, 12 },
    { 134, 5, 13 },
    { 135, 5, 14 },
    { 136, 5, 15 },
    { 137, 5, 16 },
    { 138, 5, 17 },
    { 139, 5, 18 },
    { 140, 5, 19 },
    { 141, 5, 20 },
    { 142, 5, 21 },
    { 143, 5, 22 },
    { 144, 5, 23 },
    { 145, 5, 24 },
    { 146, 5, 25 },
    { 147, 5, 26 },
    { 148, 5, 27 },
    { 149, 5, 28 },
    { 150, 5, 29 },
    { 151, 5, 30 },
    { 152, 5, 31 },
    { 153, 6, 1 },
    { 154, 6, 2 },
    { 155, 6, 3 },
    { 156, 6, 4 },
    { 157, 6, 5 },
    { 158, 6, 6 },
    { 159, 6, 7 },
    { 160, 6, 8 },
    { 161, 6, 9 },
    { 162, 6, 10 },
    { 163, 6, 11 },
    { 164, 6, 12 },
    { 165, 6, 13 },
    { 166, 6, 14 },
    { 167, 6, 15 },
    { 168, 6, 16 },
    { 169, 6, 17 },
    { 170, 6, 18 },
    { 171, 6, 19 },
    { 172, 6, 20 },
    { 173, 6, 21 },
    { 174, 6, 22 },
    { 175, 6, 23 },
    { 176, 6, 24 },
    { 177, 6, 25 },
    { 178, 6, 26 },
    { 179, 6, 27 },
    { 180, 6, 28 },
    { 181, 6, 29 },
    { 182, 6, 30 },
    { 183, 7, 1 },
    { 184, 7, 2 },
    { 185, 7, 3 },
    { 186, 7, 4 },
    { 187, 7, 5 },
    { 188, 7, 6 },
    { 189, 7, 7 },
    { 190, 7, 8 },
    { 191, 7, 9 },
    { 192, 7, 10 },
    { 193, 7, 11 },
    { 194, 7, 12 },
    { 195, 7, 13 },
    { 196, 7, 14 },
    { 197, 7, 15 },
    { 198, 7, 16 },
    { 199, 7, 17 },
    { 200, 7, 18 },
    { 201, 7, 19 },
    { 202, 7, 20 },
    { 203, 7, 21 },
    { 204, 7, 22 },
    { 205, 7, 23 },
    { 206, 7, 24 },
    { 207, 7, 25 },
    { 208, 7, 26 },
    { 209, 7, 27 },
    { 210, 7, 28 },
    { 211, 7, 29 },
    { 212, 7, 30 },
    { 213, 7, 31 },
    { 214, 8, 1 },
    { 215, 8, 2 },
    { 216, 8, 3 },
    { 217, 8, 4 },
    { 218, 8, 5 },
    { 219, 8, 6 },
    { 220, 8, 7 },
    { 221, 8, 8 },
    { 222, 8, 9 },
    { 223, 8, 10 },
    { 224, 8, 11 },
    { 225, 8, 12 },
    { 226, 8, 13 },
    { 227, 8, 14 },
    { 228, 8, 15 },
    { 229, 8, 16 },
    { 230, 8, 17 },
    { 231, 8, 18 },
    { 232, 8, 19 },
    { 233, 8, 20 },
    { 234, 8, 21 },
    { 235, 8, 22 },
    { 236, 8, 23 },
    { 237, 8, 24 },
    { 238, 8, 25 },
    { 239, 8, 26 },
    { 240, 8, 27 },
    { 241, 8, 28 },
    { 242, 8, 29 },
    { 243, 8, 30 },
    { 244, 8, 31 },
    { 245, 9, 1 },
    { 246, 9, 2 },
    { 247, 9, 3 },
    { 248, 9, 4 },
    { 249, 9, 5 },
    { 250, 9, 6 },
    { 251, 9, 7 },
    { 252, 9, 8 },
    { 253, 9, 9 },
    { 254, 9, 10 },
    { 255, 9, 11 },
    { 256, 9, 12 },
    { 257, 9, 13 },
    { 258, 9, 14 },
    { 259, 9, 15 },
    { 260, 9, 16 },
    { 261, 9, 17 },
    { 262, 9, 18 },
    { 263, 9, 19 },
    { 264, 9, 20 },
    { 265, 9, 21 },
    { 266, 9, 22 },
    { 267, 9, 23 },
    { 268, 9, 24 },
    { 269, 9, 25 },
    { 270, 9, 26 },
    { 271, 9, 27 },
    { 272, 9, 28 },
    { 273, 9, 29 },
    { 274, 9, 30 },
    { 275, 10, 1 },
    { 276, 10, 2 },
    { 277, 10, 3 },
    { 278, 10, 4 },
    { 279, 10, 5 },
    { 280, 10, 6 },
    { 281, 10, 7 },
    { 282, 10, 8 },
    { 283, 10, 9 },
    { 284, 10, 10 },
    { 285, 10, 11 },
    { 286, 10, 12 },
    { 287, 10, 13 },
    { 288, 10, 14 },
    { 289, 10, 15 },
    { 290, 10, 16 },
    { 291, 10, 17 },
    { 292, 10, 18 },
    { 293, 10, 19 },
    { 294, 10, 20 },
    { 295, 10, 21 },
    { 296, 10, 22 },
    { 297, 10, 23 },
    { 298, 10, 24 },
    { 299, 10, 25 },
    { 300, 10, 26 },
    { 301, 10, 27 },
    { 302, 10, 28 },
    { 303, 10, 29 },
    { 304, 10, 30 },
    { 305, 10, 31 },
    { 306, 11, 1 },
    { 307, 11, 2 },
    { 308, 11, 3 },
    { 309, 11, 4 },
    { 310, 11, 5 },
    { 311, 11, 6 },
    { 312, 11, 7 },
    { 313, 11, 8 },
    { 314, 11, 9 },
    { 315, 11, 10 },
    { 316, 11, 11 },
    { 317, 11, 12 },
    { 318, 11, 13 },
    { 319, 11, 14 },
    { 320, 11, 15 },
    { 321, 11, 16 },
    { 322, 11, 17 },
    { 323, 11, 18 },
    { 324, 11, 19 },
    { 325, 11, 20 },
    { 326, 11, 21 },
    { 327, 11, 22 },
    { 328, 11, 23 },
    { 329, 11, 24 },
    { 330, 11, 25 },
    { 331, 11, 26 },
    { 332, 11, 27 },
    { 333, 11, 28 },
    { 334, 11, 29 },
    { 335, 11, 30 },
    { 336, 12, 1 },
    { 337, 12, 2 },
    { 338, 12, 3 },
    { 339, 12, 4 },
    { 340, 12, 5 },
    { 341, 12, 6 },
    { 342, 12, 7 },
    { 343, 12, 8 },
    { 344, 12, 9 },
    { 345, 12, 10 },
    { 346, 12, 11 },
    { 347, 12, 12 },
    { 348, 12, 13 },
    { 349, 12, 14 },
    { 350, 12, 15 },
    { 351, 12, 16 },
    { 352, 12, 17 },
    { 353, 12, 18 },
    { 354, 12, 19 },
    { 355, 12, 20 },
    { 356, 12, 21 },
    { 357, 12, 22 },
    { 358, 12, 23 },
    { 359, 12, 24 },
    { 360, 12, 25 },
    { 361, 12, 26 },
    { 362, 12, 27 },
    { 363, 12, 28 },
    { 364, 12, 29 },
    { 365, 12, 30 },
    { 366, 12, 31 },
    { 0, 0, 0 }
};
// clang-format on

void TestOrdinalDateConversion()
{
    for (uint16_t year = 0; year <= 10000; year++)
    {
        OrdinalDateTestValue * testValue = (IsLeapYear(year)) ? LeapYearOrdinalDates : StandardYearOrdinalDates;

        for (; testValue->DayOfYear != 0; testValue++)
        {
            uint16_t outDayOfYear = 0;
            uint8_t outMonth = 0, outDayOfMonth = 0;

            OrdinalDateToCalendarDate(year, testValue->DayOfYear, outMonth, outDayOfMonth);

            TestAssert(outMonth == testValue->Month, "OrdinalDateToCalendarDate returned invalid month");
            TestAssert(outDayOfMonth == testValue->DayOfMonth, "OrdinalDateToCalendarDate returned invalid day-of-month");

            CalendarDateToOrdinalDate(year, testValue->Month, testValue->DayOfMonth, outDayOfYear);

            TestAssert(outDayOfYear == testValue->DayOfYear, "CalendarDateToOrdinalDate returned invalid day-of-year");
        }
    }
}

void TestDaysSinceEpochConversion()
{
    uint32_t daysSinceEpoch = 0;

    for (uint16_t year = kUnixEpochYear; year <= kMaxYearInDaysSinceUnixEpoch32; year++)
    {
        for (uint8_t month = kJanuary; month <= kDecember; month++)
        {
            for (uint8_t dayOfMonth = 1; dayOfMonth <= DaysInMonth(year, month); dayOfMonth++)
            {
                // Test CalendarDateToDaysSinceUnixEpoch()
                {
                    uint32_t calculatedDaysSinceEpoch;

                    CalendarDateToDaysSinceUnixEpoch(year, month, dayOfMonth, calculatedDaysSinceEpoch);

                    if (calculatedDaysSinceEpoch != daysSinceEpoch)
                        printf("%04u/%02u/%02u %" PRIu32 " %" PRIu32 "\n", year, month, dayOfMonth, daysSinceEpoch,
                               calculatedDaysSinceEpoch);

                    TestAssert(calculatedDaysSinceEpoch == daysSinceEpoch,
                               "CalendarDateToDaysSinceUnixEpoch() returned unexpected value");
                }

                // Test DaysSinceUnixEpochToCalendarDate()
                {
                    uint16_t calculatedYear;
                    uint8_t calculatedMonth, calculatedDayOfMonth;

                    DaysSinceUnixEpochToCalendarDate(daysSinceEpoch, calculatedYear, calculatedMonth, calculatedDayOfMonth);

                    if (calculatedYear != year || calculatedMonth != month || calculatedDayOfMonth != dayOfMonth)
                        printf("%04u/%02u/%02u %04u/%02u/%02u\n", year, month, dayOfMonth, calculatedYear, calculatedMonth,
                               calculatedDayOfMonth);

                    TestAssert(calculatedYear == year, "DaysSinceUnixEpochToCalendarDate() returned unexpected year value");
                    TestAssert(calculatedMonth == month, "DaysSinceUnixEpochToCalendarDate() returned unexpected month value");
                    TestAssert(calculatedDayOfMonth == dayOfMonth,
                               "DaysSinceUnixEpochToCalendarDate() returned unexpected dayOfMonth value");
                }

                daysSinceEpoch++;
            }
        }
    }
}

void TestSecondsSinceEpochConversion()
{
    uint32_t daysSinceEpoch = 0;
    uint32_t timeOfDay      = 0; // in seconds

    for (uint16_t year = kUnixEpochYear; year <= kMaxYearInSecondsSinceUnixEpoch32; year++)
    {
        for (uint8_t month = kJanuary; month <= kDecember; month++)
        {
            for (uint8_t dayOfMonth = 1; dayOfMonth <= DaysInMonth(year, month); dayOfMonth++)
            {
                // Test 10 different times of day per calendar day (this keeps the total runtime manageable).
                for (uint8_t i = 0; i < 10; i++)
                {
                    uint32_t secondsSinceEpoch = daysSinceEpoch * kSecondsPerDay + timeOfDay;

                    uint8_t hour   = static_cast<uint8_t>(timeOfDay / kSecondsPerHour);
                    uint8_t minute = static_cast<uint8_t>((timeOfDay - (hour * kSecondsPerHour)) / kSecondsPerMinute);
                    uint8_t second = static_cast<uint8_t>(timeOfDay - (hour * kSecondsPerHour + minute * kSecondsPerMinute));

#define VERIFY_TEST_AGAINST_GMTTIME 0
#if VERIFY_TEST_AGAINST_GMTTIME
                    {
                        time_t epochTimeT = static_cast<time_t>(secondsSinceEpoch);

                        struct tm * gmt = gmtime(&epochTimeT);

                        TestAssert(year == gmt->tm_year + 1900, "gmtime() mismatch: year");
                        TestAssert(month == gmt->tm_mon + 1, "gmtime() mismatch: month");
                        TestAssert(dayOfMonth == gmt->tm_mday, "gmtime() mismatch: dayOfMonth");
                        TestAssert(hour == gmt->tm_hour, "gmtime() mismatch: hour");
                        TestAssert(minute == gmt->tm_min, "gmtime() mismatch: minute");
                        TestAssert(second == gmt->tm_sec, "gmtime() mismatch: second");
                    }
#endif

                    // Test SecondsSinceUnixEpochToCalendarTime()
                    {
                        uint16_t calculatedYear;
                        uint8_t calculatedMonth, calculatedDayOfMonth, calculatedHour, calculatedMinute, calculatedSecond;

                        SecondsSinceUnixEpochToCalendarTime(secondsSinceEpoch, calculatedYear, calculatedMonth,
                                                            calculatedDayOfMonth, calculatedHour, calculatedMinute,
                                                            calculatedSecond);

                        TestAssert(calculatedYear == year, "SecondsSinceUnixEpochToCalendarTime() returned unexpected year value");
                        TestAssert(calculatedMonth == month,
                                   "SecondsSinceUnixEpochToCalendarTime() returned unexpected month value");
                        TestAssert(calculatedDayOfMonth == dayOfMonth,
                                   "SecondsSinceUnixEpochToCalendarTime() returned unexpected dayOfMonth value");
                        TestAssert(calculatedHour == hour, "SecondsSinceUnixEpochToCalendarTime() returned unexpected hour value");
                        TestAssert(calculatedMinute == minute,
                                   "SecondsSinceUnixEpochToCalendarTime() returned unexpected minute value");
                        TestAssert(calculatedSecond == second,
                                   "SecondsSinceUnixEpochToCalendarTime() returned unexpected second value");
                    }

                    // Test CalendarTimeToSecondsSinceUnixEpoch()

                    {
                        uint32_t calculatedSecondsSinceEpoch;

                        CalendarTimeToSecondsSinceUnixEpoch(year, month, dayOfMonth, hour, minute, second,
                                                            calculatedSecondsSinceEpoch);

                        TestAssert(calculatedSecondsSinceEpoch == secondsSinceEpoch,
                                   "CalendarTimeToSecondsSinceUnixEpoch() returned unexpected value");
                    }

                    // Iterate through times of day by skipping a large prime number of seconds.
                    timeOfDay = (timeOfDay + 10007) % kSecondsPerDay;
                }

                daysSinceEpoch++;
            }
        }
    }
}

void TestChipEpochTimeConversion()
{
    uint32_t daysSinceEpoch = 0;
    uint32_t timeOfDay      = 0; // in seconds

    for (uint16_t year = kChipEpochBaseYear; year <= kChipEpochMaxYear; year++)
    {
        for (uint8_t month = kJanuary; month <= kDecember; month++)
        {
            for (uint8_t dayOfMonth = 1; dayOfMonth <= DaysInMonth(year, month); dayOfMonth++)
            {
                // Test 10 different times of day per calendar day (this keeps the total runtime manageable).
                for (uint8_t i = 0; i < 10; i++)
                {
                    uint32_t chipEpochTime = daysSinceEpoch * kSecondsPerDay + timeOfDay;

                    uint8_t hour   = static_cast<uint8_t>(timeOfDay / kSecondsPerHour);
                    uint8_t minute = static_cast<uint8_t>((timeOfDay - (hour * kSecondsPerHour)) / kSecondsPerMinute);
                    uint8_t second = static_cast<uint8_t>(timeOfDay - (hour * kSecondsPerHour + minute * kSecondsPerMinute));

#if VERIFY_TEST_AGAINST_GMTTIME
                    {
                        time_t epochTimeT = static_cast<time_t>(chipEpochTime) + kChipEpochSecondsSinceUnixEpoch;

                        struct tm * gmt = gmtime(&epochTimeT);

                        TestAssert(year == gmt->tm_year + 1900, "chip epoch gmtime() mismatch: year");
                        TestAssert(month == gmt->tm_mon + 1, "chip epoch gmtime() mismatch: month");
                        TestAssert(dayOfMonth == gmt->tm_mday, "chip epoch gmtime() mismatch: dayOfMonth");
                        TestAssert(hour == gmt->tm_hour, "chip epoch gmtime() mismatch: hour");
                        TestAssert(minute == gmt->tm_min, "chip epoch gmtime() mismatch: minute");
                        TestAssert(second == gmt->tm_sec, "chip epoch gmtime() mismatch: second");
                    }
#endif

                    // Test ChipEpochToCalendarTime()
                    {
                        uint16_t calculatedYear;
                        uint8_t calculatedMonth, calculatedDayOfMonth, calculatedHour, calculatedMinute, calculatedSecond;

                        ChipEpochToCalendarTime(chipEpochTime, calculatedYear, calculatedMonth, calculatedDayOfMonth,
                                                calculatedHour, calculatedMinute, calculatedSecond);

                        TestAssert(calculatedYear == year, "ChipEpochToCalendarTime() returned unexpected year value");
                        TestAssert(calculatedMonth == month, "ChipEpochToCalendarTime() returned unexpected month value");
                        TestAssert(calculatedDayOfMonth == dayOfMonth,
                                   "ChipEpochToCalendarTime() returned unexpected dayOfMonth value");
                        TestAssert(calculatedHour == hour, "ChipEpochToCalendarTime() returned unexpected hour value");
                        TestAssert(calculatedMinute == minute, "ChipEpochToCalendarTime() returned unexpected minute value");
                        TestAssert(calculatedSecond == second, "ChipEpochToCalendarTime() returned unexpected second value");
                    }

                    // Test CalendarTimeToSecondsSinceUnixEpoch()

                    {
                        uint32_t calculatedChipEpochTime;

                        CalendarToChipEpochTime(year, month, dayOfMonth, hour, minute, second, calculatedChipEpochTime);

                        TestAssert(calculatedChipEpochTime == chipEpochTime,
                                   "CalendarTimeToSecondsSinceUnixEpoch() returned unexpected value");
                    }

                    // Iterate through times of day by skipping a large prime number of seconds.
                    timeOfDay = (timeOfDay + 10007) % kSecondsPerDay;
                }

                daysSinceEpoch++;
            }
        }
    }
}

int TestTimeUtils(void)
{
    TestOrdinalDateConversion();
    TestDaysSinceEpochConversion();
    TestSecondsSinceEpochConversion();
    TestChipEpochTimeConversion();

    printf("All tests passed\n");

    return (0);
}

CHIP_REGISTER_TEST_SUITE(TestTimeUtils);
