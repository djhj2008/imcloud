/* Preparation */
#include "plehead.h"
ple_code_t
PLEInitialize(HPLE *phPle, ple_void_t *pMem,
              const ple_size_t nMemSize)
{
	return 0;
}

ple_code_t
PLEFinalize(HPLE *phPle)
{
	return 0;
}

ple_code_t
PLEPreProcess(HPLE hPle,
              const ple_uint8_t ucFundamentalFrq,
              const ple_uint8_t ucCurrentChannels,
              const ple_uint8_t ucVoltageChannels,
              const ple_uint8_t ucSamplesPerFrame,
              const ple_uint8_t ucFramesPerGroup)
{
	return 0;
}
              
   
ple_code_t
PLEPostProcess(HPLE hPle)
{
	return 0;
}

/* Interfaces */
ple_code_t
PLEPutData(HPLE hPle,
           const ple_uint16_t **ppusWave)
{
	return 0;
}

ple_code_t
PLEGetResultSize(HPLE hPle, plc_size_t *pnItemSize)
{
	return 0;
}
  /* if <return> == 0, no output exists */

ple_code_t
PLEGetResult(HPLE hPle, plc_uint8_t *pucItem)
{
	return 0;
}
  /* if <return> == 0, no output exists */

/* Utilities */
ple_code_t
PLEGetVersion(ple_uint8_t *pszVersionString,
              const ple_size_t nSize)
{
	return 0;
}

ple_size_t
PLEGetStructSize(const ple_uint8_t ucCurrentChannels,
                 const ple_uint8_t ucVoltageChannels,
                 const ple_uint8_t ucFramesPerGroup)
{
	return 0;
}
  /* if inputs are no proper, <return> == 0 */

plc_size_t
PLEGetResultSizeMax(const ple_uint8_t ucCurrentChannels,
                    const ple_uint8_t ucVoltageChannels,
                    const ple_uint8_t ucFramesPerGroup)
{
	return 0;
}
