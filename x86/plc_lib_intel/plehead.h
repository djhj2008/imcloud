/*
 ================================================================

    PowerLine Signal Codec Encoder: API header
                                                  (plehead.h)

 ----------------------------------------------------------------

  [Function]
    This encodes current (and voltage) data.

  [Input]
    Current (and Voltage) waveforms

  [Output]
    Encoded data

 ----------------------------------------------------------------

    2013.01.24.[Thu] Created by T.SHIBUYA
    2013.06.21.[Fri] Changed by T.SHIBUYA

 ----------------------------------------------------------------
    RCS $Id: plehead.h,v 1.11 2013/06/26 02:11:34 shibuya Exp $
 ----------------------------------------------------------------
           Copyright (C) 2012-2013  SONY Corporation
          Any unauthorized use is strictly prohibited.
 ================================================================
*/
#ifndef PLEHEAD_H_0001
#define PLEHEAD_H_0001

/* ----------------------------------------------------------------
                             Includes
 ---------------------------------------------------------------- */
#include "plchead.h"

/* ----------------------------------------------------------------
                                Timer
 ---------------------------------------------------------------- */
/* if 1, this library expires on the date */
#define PLE_SET_EXPIRE          0
#define PLE_EXPIRE_YEAR         2013
#define PLE_EXPIRE_MONTH        10
#define PLE_EXPIRE_DAY          1

/* ----------------------------------------------------------------
                               Errors 
 ---------------------------------------------------------------- */
#define PLE_SUCCESS                 0
#define PLE_ERROR_GENERIC        -100
#define PLE_ERROR_EXPIRED        -200
#define PLE_ERROR_FILE           -300
#define PLE_ERROR_CREATION       -400
#define PLE_ERROR_NULLPOINTER    -500
#define PLE_ERROR_PREPROCESS     -600
#define PLE_ERROR_ALLOC          -700
#define PLE_ERROR_MEMORY         -800
#define PLE_ERROR_OVERFLOW       -900
#define PLE_ERROR_UNDERFLOW     -1000
#define PLE_ERROR_OUT_OF_RANGE  -1100
#define PLE_ERROR_INVALID_VALUE -1200
#define PLE_ERROR_ID            -1300
#define PLE_ERROR_VERSION       -1400
#define PLE_ERROR_MODE          -1500
#define PLE_ERROR_EOF           -1600
#define PLE_ERROR_OS            -1700
#define PLE_ERROR_FORMAT        -1800
#define PLE_ERROR_SAMPLE_RATE   -1900
#define PLE_ERROR_CHANNELS      -2000
#define PLE_ERROR_LENGTH        -2100
#define PLE_ERROR_STRUCT        -2200

/* ----------------------------------------------------------------
                           Return Codes
 ---------------------------------------------------------------- */
#define PLE_RESULT_NONE          0
#define PLE_RESULT_EXIST         1

/* ----------------------------------------------------------------
                          Variable types
 ---------------------------------------------------------------- */
#ifndef PLE_DEF_TYPES
#define PLE_DEF_TYPES
#include <stdint.h>
#include <stddef.h>
typedef int      ple_code_t;   /* for return code */
typedef void     ple_void_t;
typedef int8_t   ple_int8_t;
typedef uint8_t  ple_uint8_t;
typedef int16_t  ple_int16_t;
typedef uint16_t ple_uint16_t;
typedef int32_t  ple_int32_t;
typedef uint32_t ple_uint32_t;
typedef int64_t  ple_int64_t;
typedef uint64_t ple_uint64_t;
typedef float    ple_float_t;
typedef size_t   ple_size_t;
#endif /* PLE_DEF_TYPES */

/* ----------------------------------------------------------------
                            Main Handle
 ---------------------------------------------------------------- */
typedef struct ple_struct       *HPLE;

/* ----------------------------------------------------------------
                          version string 
 ---------------------------------------------------------------- */
#define PLE_VERSION_SIZE        64  /* bytes */

/* ----------------------------------------------------------------
                             Interfaces
 ---------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Preparation */
ple_code_t
PLEInitialize(HPLE *phPle, ple_void_t *pMem,
              const ple_size_t nMemSize);

ple_code_t
PLEFinalize(HPLE *phPle);

ple_code_t
PLEPreProcess(HPLE hPle,
              const ple_uint8_t ucFundamentalFrq,
              const ple_uint8_t ucCurrentChannels,
              const ple_uint8_t ucVoltageChannels,
              const ple_uint8_t ucSamplesPerFrame,
              const ple_uint8_t ucFramesPerGroup);

ple_code_t
PLEPostProcess(HPLE hPle);

/* Interfaces */
ple_code_t
PLEPutData(HPLE hPle,
           const ple_uint16_t **ppusWave);

ple_code_t
PLEGetResultSize(HPLE hPle, plc_size_t *pnItemSize);
  /* if <return> == 0, no output exists */

ple_code_t
PLEGetResult(HPLE hPle, plc_uint8_t *pucItem);
  /* if <return> == 0, no output exists */

/* Utilities */
ple_code_t
PLEGetVersion(ple_uint8_t *pszVersionString,
              const ple_size_t nSize);

ple_size_t
PLEGetStructSize(const ple_uint8_t ucCurrentChannels,
                 const ple_uint8_t ucVoltageChannels,
                 const ple_uint8_t ucFramesPerGroup);
  /* if inputs are no proper, <return> == 0 */

plc_size_t
PLEGetResultSizeMax(const ple_uint8_t ucCurrentChannels,
                    const ple_uint8_t ucVoltageChannels,
                    const ple_uint8_t ucFramesPerGroup);
  /* if inputs are no proper, <return> == 0 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*  PLEHEAD_H_0001 */
/* ----------------------------------------------------------------
                End of File : SONY Confidential
 ---------------------------------------------------------------- */

