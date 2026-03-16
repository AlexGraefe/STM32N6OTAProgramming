#ifndef VENC_H
#define VENC_H

#include "stm32n6xx_hal.h"

#define VIDEO_FRAME_NB 600

#define VENC_HW_MODE_ENABLE (1u)
#define VENC_LINE_BUF_DEPTH (1u)

int Encode_frame(uint32_t img_addr);
int encoder_prepare(uint32_t width, uint32_t height, int framerate, uint32_t * output_buffer);
int get_frame_nb(void);
int enc_end_reached();
int encoder_end(void);
const uint8_t *encoder_get_last_frame_data(void);
uint32_t encoder_get_last_frame_size(void);

#endif /* VENC_H */