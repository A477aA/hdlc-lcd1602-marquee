//
// Created by 79513 on 15.12.2023.
//

#ifndef HDLC_H
#define HDLC_H

#include "fcs.h"
#include <errno.h>
#include "stdint.h"

/** HDLC start/end flag sequence */
#define HDLC_FLAG_SEQUENCE 0x7E

/** HDLC control escape value */
#define HDLC_CONTROL_ESCAPE 0x7D

/** HDLC all station address */
#define HDLC_ALL_STATION_ADDR 0xFF

/** Supported HDLC frame types */
typedef enum {
    I_FRAME,
    S_FRAME,
    S_FRAME_NACK,
} hdlc_frame_t;

/** Control field information */
typedef struct {
    hdlc_frame_t frame;
    unsigned char seq_no :3;
} hdlc_control_t;

/** Variables used in hdlc_get_data and hdlc_get_data_with_state
 * to keep track of received buffers
 */
typedef struct {
    char control_escape;
    FCS_SIZE fcs;
    int start_index;
    int end_index;
    int src_index;
    int dest_index;
} hdlc_state_t;

/**
 * Retrieves data from specified buffer containing the HDLC frame. Frames can be
 * parsed from multiple buffers e.g. when received via UART.
 *
 * @param[out] control Control field structure with frame type and sequence number
 * @param[in] src Source buffer with frame
 * @param[in] src_len Source buffer length
 * @param[out] dest Destination buffer (should be able to contain max frame size)
 * @param[out] dest_len Destination buffer length
 * @retval >=0 Success (size of returned value should be discarded from source buffer)
 * @retval -EINVAL Invalid parameter
 * @retval -ENOMSG Invalid message
 * @retval -EIO Invalid FCS (size of dest_len should be discarded from source buffer)
 *
 * @see hdlc_get_data_with_state
 */
int hdlc_get_data(hdlc_control_t *control, uint8_t *src,
                    size_t src_len, uint8_t *dest, size_t *dest_len);

/**
 * Retrieves data from specified buffer containing the HDLC frame. Frames can be
 * parsed from multiple buffers e.g. when received via UART.
 *
 * This function is a variation of @ref hdlc_get_data
 * The difference is only in first argument: hdlc_state_t *state
 * Data under that pointer is used to keep track of internal buffers.
 *
 * @see hdlc_get_data
 */
int hdlc_get_data_with_state(hdlc_state_t *state, hdlc_control_t *control, uint8_t *src,
                             size_t src_len, uint8_t *dest, size_t *dest_len);

/**
 * Resets values used in yahdlc_get_data function to keep track of received buffers
 */
void hdlc_get_data_reset();

/**
 * This is a variation of @ref hdlc_get_data_reset
 * Resets state values that are under the pointer provided as argument
 *
 * This function need to be called before the first call to hdlc_get_data_with_state
 * when custom state storage is used.
 *
 * @see hdlc_get_data_reset
 */
void hdlc_get_data_reset_with_state(hdlc_state_t *state);

/**
 * Creates HDLC frame with specified data buffer.
 *
 * @param[in] control Control field structure with frame type and sequence number
 * @param[in] src Source buffer with data
 * @param[in] src_len Source buffer length
 * @param[out] dest Destination buffer (should be bigger than source buffer)
 * @param[out] dest_len Destination buffer length
 * @retval 0 Success
 * @retval -EINVAL Invalid parameter
 */
int hdlc_frame_data(hdlc_control_t *control, uint8_t *src,
                      size_t src_len, uint8_t *dest, size_t *dest_len);

#endif //HDLC_H
