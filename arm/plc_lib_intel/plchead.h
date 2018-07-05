/*
 ================================================================

    PowerLine Signal Codec Common Constants: API Header
                                                  (plchead.h)

 ----------------------------------------------------------------

 ----------------------------------------------------------------

    2013.01.24.[Thu] Created by T.SHIBUYA
    2013.06.21.[Fri] Changed by T.SHIBUYA

 ----------------------------------------------------------------
    RCS $Id: plchead.h,v 1.7 2013/06/24 03:58:54 shibuya Exp $
 ----------------------------------------------------------------
           Copyright (C) 2012-2013  SONY Corporation
          Any unauthorized use is strictly prohibited.
 ================================================================
*/
#ifndef PLCHEAD_H_0001
#define PLCHEAD_H_0001

/* ----------------------------------------------------------------
                             Constants
 ---------------------------------------------------------------- */
/* wave parameters */
#define PLC_BITS_PER_SAMPLE          14  /* bits */
#define PLC_SAMPLES_PER_FRAME_MIN    32  /* samples */
#define PLC_SAMPLES_PER_FRAME_MAX   128  /* samples */
#define PLC_FRAMES_PER_GROUP_MAX    255  /* frames */

/* ----------------------------------------------------------------
                           Variable types
 ---------------------------------------------------------------- */
#ifndef PLC_DEF_TYPES
#define PLC_DEF_TYPES
#include <stdint.h>
typedef uint8_t  plc_uint8_t;
typedef uint16_t plc_size_t;
#endif /* PLC_DEF_TYPES */

#endif /*  PLCHEAD_H_0001 */
/* ----------------------------------------------------------------
                End of File : SONY Confidential
 ---------------------------------------------------------------- */

