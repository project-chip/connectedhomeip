/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// Prevent multiple inclusion
#pragma once

// Default values for the attributes longer than a pointer,
// in a form of a binary blob
// Separate block is generated for big-endian and little-endian cases.
#if BIGENDIAN_CPU
#define GENERATED_DEFAULTS                                                                                                         \
    {                                                                                                                              \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00,                                                                                          \
            0x00, /* Default for cluster: "Color Control", attribute: "compensation text". side: server, big-endian */             \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                                              \
            0x00, /* Default for cluster: "IAS Zone", attribute: "IAS CIE address". side: server, big-endian */                    \
    }

#else // !BIGENDIAN_CPU
#define GENERATED_DEFAULTS                                                                                                         \
    {                                                                                                                              \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00,                                                                                          \
            0x00, /* Default for cluster: "Color Control", attribute: "compensation text". side: server, little-endian */          \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                                              \
            0x00, /* Default for cluster: "IAS Zone", attribute: "IAS CIE address". side: server, little-endian */                 \
    }

#endif // BIGENDIAN_CPU

#define GENERATED_DEFAULTS_COUNT (2)

#define ZAP_TYPE(type) ZCL_##type##_ATTRIBUTE_TYPE
#define ZAP_LONG_DEFAULTS_INDEX(index)                                                                                             \
    {                                                                                                                              \
        (uint8_t *) (&generatedDefaults[index])                                                                                    \
    }
#define ZAP_MIN_MAX_DEFAULTS_INDEX(index)                                                                                          \
    {                                                                                                                              \
        (uint8_t *) (&minMaxDefault[index])                                                                                        \
    }

// This is an array of EmberAfAttributeMinMaxValue structures.
#define GENERATED_MIN_MAX_DEFAULT_COUNT 0
#define GENERATED_MIN_MAX_DEFAULTS                                                                                                 \
    {                                                                                                                              \
    }

#define ZAP_ATTRIBUTE_MASK(mask) ATTRIBUTE_MASK_##mask
// This is an array of EmberAfAttributeMetadata structures.
#define GENERATED_ATTRIBUTE_COUNT 103
#define GENERATED_ATTRIBUTES                                                                                                       \
    {                                                                                                                              \
        { 0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 3 } }, /* Basic (client): cluster revision */     \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 3 } },                      /* Basic (server): cluster revision */     \
            { 0x0000, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x08 } },                    /* Basic (server): ZCL version */          \
            { 0x0007, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0x00 } },                    /* Basic (server): power source */         \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 2 }                                         \
            },                                                     /* Identify (client): cluster revision */                       \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 2 } }, /* Identify (server): cluster revision */                       \
            {                                                                                                                      \
                0x0000, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0x0000 }                                  \
            }, /* Identify (server): identify time */                                                                              \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 3 }                                         \
            },                                                      /* Groups (client): cluster revision */                        \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 3 } },  /* Groups (server): cluster revision */                        \
            { 0x0000, ZAP_TYPE(BITMAP8), 1, 0, { (uint8_t *) 0 } }, /* Groups (server): name support */                            \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 3 }                                         \
            },                                                          /* Scenes (client): cluster revision */                    \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 3 } },      /* Scenes (server): cluster revision */                    \
            { 0x0000, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x00 } },    /* Scenes (server): scene count */                         \
            { 0x0001, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x00 } },    /* Scenes (server): current scene */                       \
            { 0x0002, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x0000 } }, /* Scenes (server): current group */                       \
            { 0x0003, ZAP_TYPE(BOOLEAN), 1, 0, { (uint8_t *) 0x00 } },  /* Scenes (server): scene valid */                         \
            { 0x0004, ZAP_TYPE(BITMAP8), 1, 0, { (uint8_t *) 0 } },     /* Scenes (server): name support */                        \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 2 }                                         \
            },                                                         /* On/off (client): cluster revision */                     \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 2 } },     /* On/off (server): cluster revision */                     \
            { 0x0000, ZAP_TYPE(BOOLEAN), 1, 0, { (uint8_t *) 0x00 } }, /* On/off (server): on/off */                               \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 3 }                                         \
            },                                                       /* Level Control (client): cluster revision */                \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 3 } },   /* Level Control (server): cluster revision */                \
            { 0x0000, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x00 } }, /* Level Control (server): current level */                   \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 3 }                                         \
            },                                                      /* Door Lock (client): cluster revision */                     \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 3 } },  /* Door Lock (server): cluster revision */                     \
            { 0x0000, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0 } },   /* Door Lock (server): lock state */                           \
            { 0x0001, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0 } },   /* Door Lock (server): lock type */                            \
            { 0x0002, ZAP_TYPE(BOOLEAN), 1, 0, { (uint8_t *) 0 } }, /* Door Lock (server): actuator enabled */                     \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 0x0001 }                                    \
            },                                                          /* Barrier Control (client): cluster revision */           \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x0001 } }, /* Barrier Control (server): cluster revision */           \
            { 0x0001, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0 } },       /* Barrier Control (server): barrier moving state */       \
            { 0x0002, ZAP_TYPE(BITMAP16), 2, 0, { (uint8_t *) 0 } },    /* Barrier Control (server): barrier safety status */      \
            { 0x0003, ZAP_TYPE(BITMAP8), 1, 0, { (uint8_t *) 0 } },     /* Barrier Control (server): barrier capabilities */       \
            { 0x000A, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0 } },       /* Barrier Control (server): barrier position */           \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 3 }                                         \
            },                                                          /* Color Control (client): cluster revision */             \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 3 } },      /* Color Control (server): cluster revision */             \
            { 0x0000, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x00 } },    /* Color Control (server): current hue */                  \
            { 0x0001, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x00 } },    /* Color Control (server): current saturation */           \
            { 0x0002, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x0000 } }, /* Color Control (server): remaining time */               \
            { 0x0003, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x616B } }, /* Color Control (server): current x */                    \
            { 0x0004, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x607D } }, /* Color Control (server): current y */                    \
            { 0x0005, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0 } },       /* Color Control (server): drift compensation */           \
            {                                                                                                                      \
                0x0006, ZAP_TYPE(CHAR_STRING), 254, 0, { (uint8_t *) ZAP_LONG_DEFAULTS_INDEX(0) }                                  \
            },                                                          /* Color Control (server): compensation text */            \
            { 0x0007, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x00FA } }, /* Color Control (server): color temperature */            \
            { 0x0008, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0x01 } },    /* Color Control (server): color mode */                   \
            {                                                                                                                      \
                0x000F, ZAP_TYPE(BITMAP8), 1, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0x00 }                                   \
            },                                                     /* Color Control (server): color control options */             \
            { 0x0010, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0 } },  /* Color Control (server): number of primaries */               \
            { 0x0011, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 1 x */                       \
            { 0x0012, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 1 y */                       \
            { 0x0013, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0 } },  /* Color Control (server): primary 1 intensity */               \
            { 0x0015, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 2 x */                       \
            { 0x0016, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 2 y */                       \
            { 0x0017, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0 } },  /* Color Control (server): primary 2 intensity */               \
            { 0x0019, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 3 x */                       \
            { 0x001A, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 3 y */                       \
            { 0x001B, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0 } },  /* Color Control (server): primary 3 intensity */               \
            { 0x0020, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 4 x */                       \
            { 0x0021, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 4 y */                       \
            { 0x0022, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0 } },  /* Color Control (server): primary 4 intensity */               \
            { 0x0024, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 5 x */                       \
            { 0x0025, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 5 y */                       \
            { 0x0026, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0 } },  /* Color Control (server): primary 5 intensity */               \
            { 0x0028, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 6 x */                       \
            { 0x0029, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 } }, /* Color Control (server): primary 6 y */                       \
            { 0x002A, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0 } },  /* Color Control (server): primary 6 intensity */               \
            {                                                                                                                      \
                0x0030, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): white point x */                                                                         \
            {                                                                                                                      \
                0x0031, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): white point y */                                                                         \
            {                                                                                                                      \
                0x0032, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): color point r x */                                                                       \
            {                                                                                                                      \
                0x0033, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): color point r y */                                                                       \
            {                                                                                                                      \
                0x0034, ZAP_TYPE(INT8U), 1, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                        \
            }, /* Color Control (server): color point r intensity */                                                               \
            {                                                                                                                      \
                0x0036, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): color point g x */                                                                       \
            {                                                                                                                      \
                0x0037, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): color point g y */                                                                       \
            {                                                                                                                      \
                0x0038, ZAP_TYPE(INT8U), 1, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                        \
            }, /* Color Control (server): color point g intensity */                                                               \
            {                                                                                                                      \
                0x003A, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): color point b x */                                                                       \
            {                                                                                                                      \
                0x003B, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): color point b y */                                                                       \
            {                                                                                                                      \
                0x003C, ZAP_TYPE(INT8U), 1, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                        \
            }, /* Color Control (server): color point b intensity */                                                               \
            {                                                                                                                      \
                0x400D, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0 }                                                                  \
            }, /* Color Control (server): couple color temp to level min-mireds */                                                 \
            {                                                                                                                      \
                0x4010, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) 0 }                                       \
            }, /* Color Control (server): start up color temperature mireds */                                                     \
            { 0x4000, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x0000 } },   /* Color Control (server): enhanced current hue */       \
            { 0x4001, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0x01 } },      /* Color Control (server): enhanced color mode */        \
            { 0x4002, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x00 } },      /* Color Control (server): color loop active */          \
            { 0x4003, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x00 } },      /* Color Control (server): color loop direction */       \
            { 0x4004, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x0019 } },   /* Color Control (server): color loop time */            \
            { 0x400A, ZAP_TYPE(BITMAP16), 2, 0, { (uint8_t *) 0x0000 } }, /* Color Control (server): color capabilities */         \
            { 0x400B, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x0000 } },   /* Color Control (server): color temp physical min */    \
            { 0x400C, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0xFEFF } },   /* Color Control (server): color temp physical max */    \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 3 } },        /* Temperature Measurement (server): cluster revision */ \
            { 0x0000, ZAP_TYPE(INT16S), 2, 0, { (uint8_t *) 0x8000 } },   /* Temperature Measurement (server): measured value */   \
            { 0x0001, ZAP_TYPE(INT16S), 2, 0, { (uint8_t *) 0x8000 } }, /* Temperature Measurement (server): min measured value */ \
            { 0x0002, ZAP_TYPE(INT16S), 2, 0, { (uint8_t *) 0x8000 } }, /* Temperature Measurement (server): max measured value */ \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 2 } },      /* IAS Zone (server): cluster revision */                  \
            { 0x0000, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0x00 } },    /* IAS Zone (server): zone state */                        \
            { 0x0001, ZAP_TYPE(ENUM16), 2, 0, { (uint8_t *) 0 } },      /* IAS Zone (server): zone type */                         \
            { 0x0002, ZAP_TYPE(BITMAP16), 2, 0, { (uint8_t *) 0x0000 } }, /* IAS Zone (server): zone status */                     \
            {                                                                                                                      \
                0x0010, ZAP_TYPE(IEEE_ADDRESS), 8, ZAP_ATTRIBUTE_MASK(WRITABLE), { (uint8_t *) ZAP_LONG_DEFAULTS_INDEX(254) }      \
            },                                                       /* IAS Zone (server): IAS CIE address */                      \
            { 0x0011, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0xff } }, /* IAS Zone (server): Zone ID */                              \
            {                                                                                                                      \
                0xFFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(CLIENT), { (uint8_t *) 0x0001 }                                    \
            },                                                          /* Binding (client): cluster revision */                   \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 0x0001 } }, /* Binding (server): cluster revision */                   \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 3 } },      /* Basic (server): cluster revision */                     \
            { 0x0000, ZAP_TYPE(INT8U), 1, 0, { (uint8_t *) 0x08 } },    /* Basic (server): ZCL version */                          \
            { 0x0007, ZAP_TYPE(ENUM8), 1, 0, { (uint8_t *) 0x00 } },    /* Basic (server): power source */                         \
            { 0xFFFD, ZAP_TYPE(INT16U), 2, 0, { (uint8_t *) 2 } },      /* On/off (server): cluster revision */                    \
            { 0x0000, ZAP_TYPE(BOOLEAN), 1, 0, { (uint8_t *) 0x00 } },  /* On/off (server): on/off */                              \
    }

// This is an array of EmberAfCluster structures.
#define ZAP_ATTRIBUTE_INDEX(index) ((EmberAfAttributeMetadata *) (&generatedAttributes[index]))

// Cluster function static arrays
#define GENERATED_FUNCTION_ARRAYS                                                                                                  \
    const EmberAfGenericClusterFunction chipFuncArrayIdentifyServer[] = {                                                          \
        (EmberAfGenericClusterFunction) emberAfIdentifyClusterServerInitCallback,                                                  \
        (EmberAfGenericClusterFunction) emberAfIdentifyClusterServerAttributeChangedCallback,                                      \
    };                                                                                                                             \
    const EmberAfGenericClusterFunction chipFuncArrayGroupsServer[] = {                                                            \
        (EmberAfGenericClusterFunction) emberAfGroupsClusterServerInitCallback,                                                    \
    };                                                                                                                             \
    const EmberAfGenericClusterFunction chipFuncArrayScenesServer[] = {                                                            \
        (EmberAfGenericClusterFunction) emberAfScenesClusterServerInitCallback,                                                    \
    };                                                                                                                             \
    const EmberAfGenericClusterFunction chipFuncArrayOnOffServer[] = {                                                             \
        (EmberAfGenericClusterFunction) emberAfOnOffClusterServerInitCallback,                                                     \
    };                                                                                                                             \
    const EmberAfGenericClusterFunction chipFuncArrayLevelControlServer[] = {                                                      \
        (EmberAfGenericClusterFunction) emberAfLevelControlClusterServerInitCallback,                                              \
    };                                                                                                                             \
    const EmberAfGenericClusterFunction chipFuncArrayDoorLockServer[] = {                                                          \
        (EmberAfGenericClusterFunction) emberAfDoorLockClusterServerAttributeChangedCallback,                                      \
    };                                                                                                                             \
    const EmberAfGenericClusterFunction chipFuncArrayColorControlServer[] = {                                                      \
        (EmberAfGenericClusterFunction) emberAfColorControlClusterServerInitCallback,                                              \
    };                                                                                                                             \
    const EmberAfGenericClusterFunction chipFuncArrayIasZoneServer[] = {                                                           \
        (EmberAfGenericClusterFunction) emberAfIasZoneClusterServerInitCallback,                                                   \
        (EmberAfGenericClusterFunction) emberAfIasZoneClusterServerMessageSentCallback,                                            \
        (EmberAfGenericClusterFunction) emberAfIasZoneClusterServerPreAttributeChangedCallback,                                    \
    };

#define ZAP_CLUSTER_MASK(mask) CLUSTER_MASK_##mask
#define GENERATED_CLUSTER_COUNT 24
#define GENERATED_CLUSTERS                                                                                                         \
    {                                                                                                                              \
        { 0x0000, ZAP_ATTRIBUTE_INDEX(0), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL },     /* Endpoint: 1, Cluster: Basic (client) */   \
            { 0x0000, ZAP_ATTRIBUTE_INDEX(1), 3, 4, ZAP_CLUSTER_MASK(SERVER), NULL }, /* Endpoint: 1, Cluster: Basic (server) */   \
            {                                                                                                                      \
                0x0003, ZAP_ATTRIBUTE_INDEX(4), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL                                               \
            }, /* Endpoint: 1, Cluster: Identify (client) */                                                                       \
            { 0x0003,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(5),                                                                                              \
              2,                                                                                                                   \
              4,                                                                                                                   \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) | ZAP_CLUSTER_MASK(ATTRIBUTE_CHANGED_FUNCTION),           \
              chipFuncArrayIdentifyServer }, /* Endpoint: 1, Cluster: Identify (server) */                                         \
            { 0x0004, ZAP_ATTRIBUTE_INDEX(7), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL }, /* Endpoint: 1, Cluster: Groups (client) */  \
            { 0x0004,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(8),                                                                                              \
              2,                                                                                                                   \
              3,                                                                                                                   \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION),                                                          \
              chipFuncArrayGroupsServer },                                             /* Endpoint: 1, Cluster: Groups (server) */ \
            { 0x0005, ZAP_ATTRIBUTE_INDEX(10), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL }, /* Endpoint: 1, Cluster: Scenes (client) */ \
            { 0x0005,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(11),                                                                                             \
              6,                                                                                                                   \
              8,                                                                                                                   \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION),                                                          \
              chipFuncArrayScenesServer },                                             /* Endpoint: 1, Cluster: Scenes (server) */ \
            { 0x0006, ZAP_ATTRIBUTE_INDEX(17), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL }, /* Endpoint: 1, Cluster: On/off (client) */ \
            { 0x0006,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(18),                                                                                             \
              2,                                                                                                                   \
              3,                                                                                                                   \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION),                                                          \
              chipFuncArrayOnOffServer }, /* Endpoint: 1, Cluster: On/off (server) */                                              \
            {                                                                                                                      \
                0x0008, ZAP_ATTRIBUTE_INDEX(20), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL                                              \
            }, /* Endpoint: 1, Cluster: Level Control (client) */                                                                  \
            { 0x0008,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(21),                                                                                             \
              2,                                                                                                                   \
              3,                                                                                                                   \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION),                                                          \
              chipFuncArrayLevelControlServer }, /* Endpoint: 1, Cluster: Level Control (server) */                                \
            {                                                                                                                      \
                0x0101, ZAP_ATTRIBUTE_INDEX(23), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL                                              \
            }, /* Endpoint: 1, Cluster: Door Lock (client) */                                                                      \
            { 0x0101,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(24),                                                                                             \
              4,                                                                                                                   \
              5,                                                                                                                   \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(ATTRIBUTE_CHANGED_FUNCTION),                                             \
              chipFuncArrayDoorLockServer }, /* Endpoint: 1, Cluster: Door Lock (server) */                                        \
            {                                                                                                                      \
                0x0103, ZAP_ATTRIBUTE_INDEX(28), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL                                              \
            }, /* Endpoint: 1, Cluster: Barrier Control (client) */                                                                \
            {                                                                                                                      \
                0x0103, ZAP_ATTRIBUTE_INDEX(29), 5, 7, ZAP_CLUSTER_MASK(SERVER), NULL                                              \
            }, /* Endpoint: 1, Cluster: Barrier Control (server) */                                                                \
            {                                                                                                                      \
                0x0300, ZAP_ATTRIBUTE_INDEX(34), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL                                              \
            }, /* Endpoint: 1, Cluster: Color Control (client) */                                                                  \
            { 0x0300,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(35),                                                                                             \
              51,                                                                                                                  \
              336,                                                                                                                 \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION),                                                          \
              chipFuncArrayColorControlServer }, /* Endpoint: 1, Cluster: Color Control (server) */                                \
            {                                                                                                                      \
                0x0402, ZAP_ATTRIBUTE_INDEX(86), 4, 8, ZAP_CLUSTER_MASK(SERVER), NULL                                              \
            }, /* Endpoint: 1, Cluster: Temperature Measurement (server) */                                                        \
            { 0x0500,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(90),                                                                                             \
              6,                                                                                                                   \
              16,                                                                                                                  \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) | ZAP_CLUSTER_MASK(PRE_ATTRIBUTE_CHANGED_FUNCTION) |      \
                  ZAP_CLUSTER_MASK(MESSAGE_SENT_FUNCTION),                                                                         \
              chipFuncArrayIasZoneServer }, /* Endpoint: 1, Cluster: IAS Zone (server) */                                          \
            {                                                                                                                      \
                0xF000, ZAP_ATTRIBUTE_INDEX(96), 1, 2, ZAP_CLUSTER_MASK(CLIENT), NULL                                              \
            }, /* Endpoint: 1, Cluster: Binding (client) */                                                                        \
            {                                                                                                                      \
                0xF000, ZAP_ATTRIBUTE_INDEX(97), 1, 2, ZAP_CLUSTER_MASK(SERVER), NULL                                              \
            }, /* Endpoint: 1, Cluster: Binding (server) */                                                                        \
            { 0x0000, ZAP_ATTRIBUTE_INDEX(98), 3, 4, ZAP_CLUSTER_MASK(SERVER), NULL }, /* Endpoint: 2, Cluster: Basic (server) */  \
            { 0x0006,                                                                                                              \
              ZAP_ATTRIBUTE_INDEX(101),                                                                                            \
              2,                                                                                                                   \
              3,                                                                                                                   \
              ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION),                                                          \
              chipFuncArrayOnOffServer }, /* Endpoint: 2, Cluster: On/off (server) */                                              \
    }

#define ZAP_CLUSTER_INDEX(index) ((EmberAfCluster *) (&generatedClusters[index]))

// This is an array of EmberAfEndpointType structures.
#define GENERATED_ENDPOINT_TYPES                                                                                                   \
    {                                                                                                                              \
        { ZAP_CLUSTER_INDEX(0), 22, 419 }, { ZAP_CLUSTER_INDEX(22), 2, 7 },                                                        \
    }

// Largest attribute size is needed for various buffers
#define ATTRIBUTE_LARGEST (254)

// Total size of singleton attributes
#define ATTRIBUTE_SINGLETONS_SIZE (0)

// Total size of attribute storage
#define ATTRIBUTE_MAX_SIZE (426)

// Number of fixed endpoints
#define FIXED_ENDPOINT_COUNT (2)

// Array of endpoints that are supported, the data inside
// the array is the endpoint number.
#define FIXED_ENDPOINT_ARRAY                                                                                                       \
    {                                                                                                                              \
        0x0001, 0x0002                                                                                                             \
    }

// Array of profile ids
#define FIXED_PROFILE_IDS                                                                                                          \
    {                                                                                                                              \
        0x0103, 0x0103                                                                                                             \
    }

// Array of device ids
#define FIXED_DEVICE_IDS                                                                                                           \
    {                                                                                                                              \
        0, 0                                                                                                                       \
    }

// Array of device versions
#define FIXED_DEVICE_VERSIONS                                                                                                      \
    {                                                                                                                              \
        1, 1                                                                                                                       \
    }

// Array of endpoint types supported on each endpoint
#define FIXED_ENDPOINT_TYPES                                                                                                       \
    {                                                                                                                              \
        0, 1                                                                                                                       \
    }

// Array of networks supported on each endpoint
#define FIXED_NETWORKS                                                                                                             \
    {                                                                                                                              \
        0, 0                                                                                                                       \
    }

// Array of EmberAfCommandMetadata structs.
#define ZAP_COMMAND_MASK(mask) COMMAND_MASK_##mask
#define EMBER_AF_GENERATED_COMMAND_COUNT (171)
#define GENERATED_COMMANDS                                                                                                         \
    {                                                                                                                              \
        { 0x0000, 0x00,                                                                                                            \
          ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Basic (client): MfgSpecificPing */           \
            {                                                                                                                      \
                0x0000, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Basic (client): ResetToFactoryDefaults */                                                                        \
            { 0x0000, 0x00,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Basic (server): MfgSpecificPing */       \
            {                                                                                                                      \
                0x0000, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Basic (server): ResetToFactoryDefaults */                                                                        \
            {                                                                                                                      \
                0x0000, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            },                                                   /* Basic (server): ResetToFactoryDefaults */                      \
            { 0x0003, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Identify (client): Identify */                                 \
            { 0x0003, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Identify (client): IdentifyQueryResponse */                    \
            { 0x0003, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Identify (server): Identify */                                 \
            { 0x0003, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Identify (server): IdentifyQueryResponse */                    \
            { 0x0003, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Identify (client): IdentifyQuery */                            \
            { 0x0003, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Identify (server): IdentifyQuery */                            \
            { 0x0004, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (client): AddGroup */                                   \
            { 0x0004, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Groups (client): AddGroupResponse */                           \
            { 0x0004, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (server): AddGroup */                                   \
            { 0x0004, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Groups (server): AddGroupResponse */                           \
            { 0x0004, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (client): ViewGroup */                                  \
            { 0x0004, 0x01, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Groups (client): ViewGroupResponse */                          \
            { 0x0004, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (server): ViewGroup */                                  \
            { 0x0004, 0x01, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Groups (server): ViewGroupResponse */                          \
            { 0x0004, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (client): GetGroupMembership */                         \
            { 0x0004, 0x02, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Groups (client): GetGroupMembershipResponse */                 \
            { 0x0004, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (server): GetGroupMembership */                         \
            { 0x0004, 0x02, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Groups (server): GetGroupMembershipResponse */                 \
            { 0x0004, 0x03, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (client): RemoveGroup */                                \
            { 0x0004, 0x03, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Groups (client): RemoveGroupResponse */                        \
            { 0x0004, 0x03, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (server): RemoveGroup */                                \
            { 0x0004, 0x03, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Groups (server): RemoveGroupResponse */                        \
            { 0x0004, 0x04, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (client): RemoveAllGroups */                            \
            { 0x0004, 0x04, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (server): RemoveAllGroups */                            \
            { 0x0004, 0x05, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (client): AddGroupIfIdentifying */                      \
            { 0x0004, 0x05, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Groups (server): AddGroupIfIdentifying */                      \
            { 0x0005, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (client): AddScene */                                   \
            { 0x0005, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (client): AddSceneResponse */                           \
            { 0x0005, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (server): AddScene */                                   \
            { 0x0005, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (server): AddSceneResponse */                           \
            { 0x0005, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (client): ViewScene */                                  \
            { 0x0005, 0x01, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (client): ViewSceneResponse */                          \
            { 0x0005, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (server): ViewScene */                                  \
            { 0x0005, 0x01, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (server): ViewSceneResponse */                          \
            { 0x0005, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (client): RemoveScene */                                \
            { 0x0005, 0x02, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (client): RemoveSceneResponse */                        \
            { 0x0005, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (server): RemoveScene */                                \
            { 0x0005, 0x02, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (server): RemoveSceneResponse */                        \
            { 0x0005, 0x03, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (client): RemoveAllScenes */                            \
            { 0x0005, 0x03, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (client): RemoveAllScenesResponse */                    \
            { 0x0005, 0x03, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (server): RemoveAllScenes */                            \
            { 0x0005, 0x03, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (server): RemoveAllScenesResponse */                    \
            { 0x0005, 0x04, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (client): StoreScene */                                 \
            { 0x0005, 0x04, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (client): StoreSceneResponse */                         \
            { 0x0005, 0x04, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (server): StoreScene */                                 \
            { 0x0005, 0x04, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (server): StoreSceneResponse */                         \
            { 0x0005, 0x05, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (client): RecallScene */                                \
            { 0x0005, 0x05, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (server): RecallScene */                                \
            { 0x0005, 0x06, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (client): GetSceneMembership */                         \
            { 0x0005, 0x06, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (client): GetSceneMembershipResponse */                 \
            { 0x0005, 0x06, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Scenes (server): GetSceneMembership */                         \
            { 0x0005, 0x06, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Scenes (server): GetSceneMembershipResponse */                 \
            { 0x0006, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (client): Off */                                        \
            { 0x0006, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (server): Off */                                        \
            { 0x0006, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (server): Off */                                        \
            { 0x0006, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (client): On */                                         \
            { 0x0006, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (server): On */                                         \
            { 0x0006, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (server): On */                                         \
            { 0x0006, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (client): Toggle */                                     \
            { 0x0006, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (server): Toggle */                                     \
            { 0x0006, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* On/off (server): Toggle */                                     \
            { 0x0008, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (client): MoveToLevel */                         \
            { 0x0008, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (server): MoveToLevel */                         \
            { 0x0008, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (client): Move */                                \
            { 0x0008, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (server): Move */                                \
            { 0x0008, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (client): Step */                                \
            { 0x0008, 0x02, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (server): Step */                                \
            { 0x0008, 0x03, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (client): Stop */                                \
            { 0x0008, 0x03, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (server): Stop */                                \
            { 0x0008, 0x04, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (client): MoveToLevelWithOnOff */                \
            { 0x0008, 0x04, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (server): MoveToLevelWithOnOff */                \
            { 0x0008, 0x05, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (client): MoveWithOnOff */                       \
            { 0x0008, 0x05, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (server): MoveWithOnOff */                       \
            { 0x0008, 0x06, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (client): StepWithOnOff */                       \
            { 0x0008, 0x06, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (server): StepWithOnOff */                       \
            { 0x0008, 0x07, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (client): StopWithOnOff */                       \
            { 0x0008, 0x07, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Level Control (server): StopWithOnOff */                       \
            { 0x0101, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Door Lock (client): LockDoor */                                \
            { 0x0101, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Door Lock (client): LockDoorResponse */                        \
            { 0x0101, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Door Lock (server): LockDoor */                                \
            { 0x0101, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Door Lock (server): LockDoorResponse */                        \
            { 0x0101, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Door Lock (client): UnlockDoor */                              \
            { 0x0101, 0x01, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Door Lock (client): UnlockDoorResponse */                      \
            { 0x0101, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Door Lock (server): UnlockDoor */                              \
            { 0x0101, 0x01, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* Door Lock (server): UnlockDoorResponse */                      \
            { 0x0101, 0x03,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): UnlockWithTimeout */ \
            { 0x0101, 0x03,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): UnlockWithTimeout */ \
            { 0x0101, 0x04,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): GetLogRecord */      \
            { 0x0101, 0x04,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): GetLogRecord */      \
            { 0x0101, 0x05,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): SetPin */            \
            { 0x0101, 0x05,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): SetPin */            \
            { 0x0101, 0x06,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): GetPin */            \
            { 0x0101, 0x06,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): GetPin */            \
            { 0x0101, 0x07,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): ClearPin */          \
            { 0x0101, 0x07,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): ClearPin */          \
            { 0x0101, 0x08,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): ClearAllPins */      \
            { 0x0101, 0x08,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): ClearAllPins */      \
            {                                                                                                                      \
                0x0101, 0x0B, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): SetWeekdaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x0B, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): SetWeekdaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x0C, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): GetWeekdaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x0C, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): GetWeekdaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x0D, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): ClearWeekdaySchedule */                                                                      \
            {                                                                                                                      \
                0x0101, 0x0D, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): ClearWeekdaySchedule */                                                                      \
            {                                                                                                                      \
                0x0101, 0x0E, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): SetYeardaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x0E, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): SetYeardaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x0F, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): GetYeardaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x0F, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): GetYeardaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x10, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): ClearYeardaySchedule */                                                                      \
            {                                                                                                                      \
                0x0101, 0x10, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): ClearYeardaySchedule */                                                                      \
            {                                                                                                                      \
                0x0101, 0x11, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): SetHolidaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x11, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): SetHolidaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x12, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): GetHolidaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x12, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): GetHolidaySchedule */                                                                        \
            {                                                                                                                      \
                0x0101, 0x13, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Door Lock (client): ClearHolidaySchedule */                                                                      \
            {                                                                                                                      \
                0x0101, 0x13, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Door Lock (server): ClearHolidaySchedule */                                                                      \
            { 0x0101, 0x14,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): SetUserType */       \
            { 0x0101, 0x14,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): SetUserType */       \
            { 0x0101, 0x15,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): GetUserType */       \
            { 0x0101, 0x15,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): GetUserType */       \
            { 0x0101, 0x16,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): SetRfid */           \
            { 0x0101, 0x16,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): SetRfid */           \
            { 0x0101, 0x17,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): GetRfid */           \
            { 0x0101, 0x17,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): GetRfid */           \
            { 0x0101, 0x18,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): ClearRfid */         \
            { 0x0101, 0x18,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): ClearRfid */         \
            { 0x0101, 0x19,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Door Lock (client): ClearAllRfids */     \
            { 0x0101, 0x19,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Door Lock (server): ClearAllRfids */     \
            { 0x0103, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Barrier Control (client): BarrierControlGoToPercent */         \
            { 0x0103, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Barrier Control (server): BarrierControlGoToPercent */         \
            { 0x0103, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Barrier Control (client): BarrierControlStop */                \
            { 0x0103, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Barrier Control (server): BarrierControlStop */                \
            { 0x0300, 0x00,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Color Control (client): MoveToHue */     \
            { 0x0300, 0x00,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Color Control (server): MoveToHue */     \
            { 0x0300, 0x01,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Color Control (client): MoveHue */       \
            { 0x0300, 0x01,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Color Control (server): MoveHue */       \
            { 0x0300, 0x02,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Color Control (client): StepHue */       \
            { 0x0300, 0x02,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Color Control (server): StepHue */       \
            {                                                                                                                      \
                0x0300, 0x03, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Color Control (client): MoveToSaturation */                                                                      \
            {                                                                                                                      \
                0x0300, 0x03, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Color Control (server): MoveToSaturation */                                                                      \
            {                                                                                                                      \
                0x0300, 0x04, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Color Control (client): MoveSaturation */                                                                        \
            {                                                                                                                      \
                0x0300, 0x04, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Color Control (server): MoveSaturation */                                                                        \
            {                                                                                                                      \
                0x0300, 0x05, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Color Control (client): StepSaturation */                                                                        \
            {                                                                                                                      \
                0x0300, 0x05, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Color Control (server): StepSaturation */                                                                        \
            {                                                                                                                      \
                0x0300, 0x06, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Color Control (client): MoveToHueAndSaturation */                                                                \
            {                                                                                                                      \
                0x0300, 0x06, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            },                                                   /* Color Control (server): MoveToHueAndSaturation */              \
            { 0x0300, 0x07, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Color Control (client): MoveToColor */                         \
            { 0x0300, 0x07, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Color Control (server): MoveToColor */                         \
            { 0x0300, 0x08, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Color Control (client): MoveColor */                           \
            { 0x0300, 0x08, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Color Control (server): MoveColor */                           \
            { 0x0300, 0x09, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Color Control (client): StepColor */                           \
            { 0x0300, 0x09, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Color Control (server): StepColor */                           \
            {                                                                                                                      \
                0x0300, 0x0A, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Color Control (client): MoveToColorTemperature */                                                                \
            {                                                                                                                      \
                0x0300, 0x0A, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Color Control (server): MoveToColorTemperature */                                                                \
            { 0x0300, 0x47,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT) }, /* Color Control (client): StopMoveStep */  \
            { 0x0300, 0x47,                                                                                                        \
              ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER) }, /* Color Control (server): StopMoveStep */  \
            {                                                                                                                      \
                0x0300, 0x4B, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Color Control (client): MoveColorTemperature */                                                                  \
            {                                                                                                                      \
                0x0300, 0x4B, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            }, /* Color Control (server): MoveColorTemperature */                                                                  \
            {                                                                                                                      \
                0x0300, 0x4C, ZAP_COMMAND_MASK(INCOMING_CLIENT) | ZAP_COMMAND_MASK(OUTGOING_CLIENT)                                \
            }, /* Color Control (client): StepColorTemperature */                                                                  \
            {                                                                                                                      \
                0x0300, 0x4C, ZAP_COMMAND_MASK(INCOMING_SERVER) | ZAP_COMMAND_MASK(OUTGOING_SERVER)                                \
            },                                                   /* Color Control (server): StepColorTemperature */                \
            { 0x0500, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* IAS Zone (server): ZoneEnrollResponse */                       \
            { 0x0500, 0x00, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* IAS Zone (server): ZoneStatusChangeNotification */             \
            { 0x0500, 0x01, ZAP_COMMAND_MASK(INCOMING_CLIENT) }, /* IAS Zone (server): ZoneEnrollRequest */                        \
            { 0xF000, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Binding (client): Bind */                                      \
            { 0xF000, 0x00, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Binding (server): Bind */                                      \
            { 0xF000, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Binding (client): Unbind */                                    \
            { 0xF000, 0x01, ZAP_COMMAND_MASK(INCOMING_SERVER) }, /* Binding (server): Unbind */                                    \
    }

// Array of EmberAfManufacturerCodeEntry structures for commands.
#define GENERATED_COMMAND_MANUFACTURER_CODE_COUNT (2)
#define GENERATED_COMMAND_MANUFACTURER_CODES                                                                                       \
    {                                                                                                                              \
        { 0, 0x1002 }, { 2, 0x1002 },                                                                                              \
    }

// This is an array of EmberAfManufacturerCodeEntry structures for clusters.
#define GENERATED_CLUSTER_MANUFACTURER_CODE_COUNT (0)
#define GENERATED_CLUSTER_MANUFACTURER_CODES                                                                                       \
    {                                                                                                                              \
        {                                                                                                                          \
            0x00, 0x00                                                                                                             \
        }                                                                                                                          \
    }

// This is an array of EmberAfManufacturerCodeEntry structures for attributes.
#define GENERATED_ATTRIBUTE_MANUFACTURER_CODE_COUNT (0)
#define GENERATED_ATTRIBUTE_MANUFACTURER_CODES                                                                                     \
    {                                                                                                                              \
        {                                                                                                                          \
            0x00, 0x00                                                                                                             \
        }                                                                                                                          \
    }

// Array of EmberAfPluginReportingEntry structures.
#define ZRD(x) EMBER_ZCL_REPORTING_DIRECTION_##x
#define ZAP_REPORT_DIRECTION(x) ZRD(x)

// Use this macro to check if Reporting plugin is included
#define EMBER_AF_PLUGIN_REPORTING
// User options for plugin Reporting
#define EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE (10)
#define EMBER_AF_PLUGIN_REPORTING_ENABLE_GROUP_BOUND_REPORTS

#define EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE (10)
#define EMBER_AF_GENERATED_REPORTING_CONFIG_DEFAULTS                                                                               \
    {                                                                                                                              \
        {                                                                                                                          \
            ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0006, 0x0000, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }          \
        }, /* Reporting for cluster: "On/off", attribute: "on/off". side: server */                                                \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0008, 0x0000, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "Level Control", attribute: "current level". side: server */                              \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0101, 0x0000, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "Door Lock", attribute: "lock state". side: server */                                     \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0300, 0x0000, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "Color Control", attribute: "current hue". side: server */                                \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0300, 0x0001, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "Color Control", attribute: "current saturation". side: server */                         \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0300, 0x0003, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "Color Control", attribute: "current x". side: server */                                  \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0300, 0x0004, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "Color Control", attribute: "current y". side: server */                                  \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0300, 0x0007, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "Color Control", attribute: "color temperature". side: server */                          \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0001, 0x0402, 0x0000, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "Temperature Measurement", attribute: "measured value". side: server */                   \
            {                                                                                                                      \
                ZAP_REPORT_DIRECTION(REPORTED), 0x0002, 0x0006, 0x0000, ZAP_CLUSTER_MASK(SERVER), 0x0000, { { 0, 65344, 0 } }      \
            }, /* Reporting for cluster: "On/off", attribute: "on/off". side: server */                                            \
    }
