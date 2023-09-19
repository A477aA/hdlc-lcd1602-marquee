#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include "hdlc.h"

enum HDLCState {
    UNINITIALIZED_STATE = 0,  // состояние до инцилизации
    IDLE_STATE, // Состояние ожидания начала
    READY_STATE, // Состояние принятия
    CONNECTING, // состояние соединения
    DISCONNECTING, // состояния отключения
    RECIVING // состояние приема и отправки
};

struct Client {
    enum HDLCState state;
    int connecting_frame_timeout; //-1
    uint8_t current_index_frame;
    hdlc_state_t current_state_hdlc;
    hdlc_control_t frameS;
    hdlc_control_t frameI;
    uint8_t* data_i_frame;
    size_t len_data_i_frame;
    //    hdlc_control_t frame3;
    //    hdlc_control_t frame4;
    hdlc_control_t frame_rej;
};

//название функций
void init_hdlc_client(struct Client* client, int connecting_frame_timeout);
void hdlc_connect(struct Client* client);
int hdlc_send_data(struct Client* client, uint8_t data[], size_t data_len);
int hdlc_get_raw_frame(struct Client* client, uint8_t* buffer, size_t lenBuffer);
//принимает буффер с уарта
int hdlc_decode_recived_raw_data(struct Client* client, uint8_t* buffer, size_t len_buffer, uint8_t* recived_data, size_t* len_recived_data);
int hdlc_timeout_handler(struct Client* client, int delta_time);

#endif //CLIENT_H