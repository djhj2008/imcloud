/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IM_DATAFORM_H__
#define __IM_DATAFORM_H__


struct waveform{
	/* Input Waveforms */
	uint64_t time_stamp;
	uint16_t data[FRAMES_GROUP][ADC_SAMPLE_CHANNEL*SAMPLES_FRAME];
};

/* =================================== API ======================================= */

#endif
