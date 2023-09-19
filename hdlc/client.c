#include "client.h"
#include "hdlc.h"
#include "stdio.h"

#define ERR_INVALID_DATA_SIZE -1
#define ERR_ALL_BUFFERS_FILL -2
#define ERR_INVALID_PARAMS -3
#define ERR_INVALID_STATE -4
#define ERR_FRAME_TIME_OUT -5
#define ERR_INVALID_SEQ_NUMBER_FRAME -6
#define ERR_TIMEOUT_ANSWER -7

#define SIZE_DATA_BUFFERS 64

int connecting_frame_timeout_bf;

void init_hdlc_client(struct Client* client, int connecting_frame_timeout){
    client->state = IDLE_STATE;
    client->connecting_frame_timeout = connecting_frame_timeout;
    connecting_frame_timeout_bf = connecting_frame_timeout;
    client->current_index_frame = 20;

//    client->current_state_hdlc.control_escape = 0;
//    client->current_state_hdlc.fcs = FCS_INIT_VALUE;
//    client->current_state_hdlc.start_index = -1;
//    client->current_state_hdlc.end_index = -1;
//    client->current_state_hdlc.src_index = 0;
//    client->current_state_hdlc.dest_index = 0;
}

void hdlc_connect(struct Client* client, hdlc_control_t* frame){
    client->state = CONNECTING;

    client->frameS.seq_no = 0;
    client->frameS.frame = S_FRAME;
    client->current_index_frame = client->frameS.seq_no;

    *frame = client->frameS;
}

int hdlc_send_data(struct Client* client, hdlc_control_t* frame, uint8_t* data, size_t data_len){
    if (client->state != READY_STATE){
        return ERR_INVALID_STATE;
    }

    client->state = RECIVING;

    if (SIZE_DATA_BUFFERS < data_len){
        return ERR_INVALID_DATA_SIZE;
    }

    client->frameI.seq_no = 0;
    client->frameI.frame = I_FRAME;
    client->data_i_frame = *data;
    client->len_data_i_frame = data_len;
    client->current_index_frame = client->frameI.seq_no;

    *frame = client->frameI;

    client->state = RECIVING;
    return 0;
}

int hdlc_get_raw_frame(struct Client *client, hdlc_control_t* frame, uint8_t* buffer, size_t lenBuffer) {
    if (frame->frame == S_FRAME || frame->frame == S_FRAME_NACK){
        int ret = hdlc_frame_data(frame, NULL, 0, buffer, &lenBuffer);

        if (ret < 0){
            printf("err in get_frame: %d\n", ret);
        }
    } else {
        int ret = hdlc_frame_data(frame, &client->data_i_frame,
                                  client->len_data_i_frame, buffer, &lenBuffer);
        if (ret < 0){
            printf("err in get_frame: %d\n", ret);
        }
    }

    return 0;
}

int hdlc_decode_recived_raw_data(struct Client* client, uint8_t* buffer, size_t len_buffer, uint8_t* recived_data, size_t* len_recived_data){
    hdlc_control_t recv_control;
    uint8_t recive[len_buffer];

    int ret = hdlc_get_data(&recv_control, buffer, len_buffer, &recive,
                            &len_buffer);

    if (ret < 0) {
        return ret;
    }

    if (recv_control.seq_no != client->current_index_frame){
        client->state = DISCONNECTING;
        client->frame_rej.seq_no = 0;
        client->frame_rej.frame = S_FRAME_NACK;
        return ERR_INVALID_SEQ_NUMBER_FRAME;
    }

    switch (recv_control.frame) {
        case S_FRAME:
            client->state = READY_STATE;
            break;
        case I_FRAME:
            *recived_data = buffer[3];
            *len_recived_data = sizeof(buffer[3]);
            break;
        case S_FRAME_NACK:
            client->state = DISCONNECTING;
            client->frame_rej.seq_no = 0;
            client->frame_rej.frame = S_FRAME_NACK;
            return ERR_INVALID_SEQ_NUMBER_FRAME;
    }

    client->connecting_frame_timeout = connecting_frame_timeout_bf;
    return 0;
}

int hdlc_timeout_handler(struct Client* client, int delta_time){
    client->connecting_frame_timeout -= delta_time;

    if (client->connecting_frame_timeout <= 0){
        return ERR_FRAME_TIME_OUT;
    }
    return 0;
}