//#include "Lсd_print/lcd.h"
#include "client.h"
#include "uart.h"
#include "stdbool.h"
//#include "stdio.h"
#include "lcd_print.h"
//#include "protocol.h"

int main() {
    struct Client hdlc;
    bool flag_connection = false;

    Lcd_inciliation();
    UART_init();
    init_hdlc_client(&hdlc, 200);

//    while(true){
//        if (!flag_connection){
//        }
//    }

    hdlc_connect(&hdlc);
    uint8_t buffer[10];

    hdlc_get_raw_frame(&hdlc, buffer, sizeof buffer);

    UART_send(buffer, sizeof buffer);

    while(!flag_connection){
        uint8_t* buffer_recive[10];
        UART_receive(buffer_recive, sizeof buffer_recive);

        int err = hdlc_timeout_handler(&hdlc, 1);
        if (err == ERR_FRAME_TIME_OUT){
            hdlc_get_raw_frame(&hdlc, buffer, sizeof buffer);
            UART_send(buffer, sizeof buffer);
            continue;
        }

        err = hdlc_decode_recived_raw_data(&hdlc, buffer_recive, sizeof buffer_recive, 0, 0);
        if (err < 0){
            if (err == ERR_INVALID_SEQ_NUMBER_FRAME){
                uint8_t* buffer_rej[10];
                hdlc_get_raw_frame(&hdlc, buffer_rej, sizeof buffer_rej);
                UART_send(buffer_rej, sizeof buffer_rej);
                return err;
            }
            return err;
        }

        if (hdlc.state == READY_STATE){
            flag_connection = true;
        }
    }

    struct message mess;
    mess.numbers[0] = 1.23;
    mess.numbers[1] = 22.1;
    mess.numbers[2] = 100;
    mess.len_numbers = 3;
    mess.str = "word war in new world io ex";
    mess.len_str = sizeof mess.str;

    uint8_t data[64];
    size_t len_data;
    protocol_encode(mess, data, &len_data);

    hdlc_send_data(&hdlc, data, len_data);
    uint8_t buffer_data[72];
    hdlc_get_raw_frame(&hdlc, buffer_data, sizeof buffer_data);

    UART_send(buffer_data, sizeof buffer_data);

    bool flag_recive = false;
    uint8_t data_recive[64];
    size_t len_data_recive;
    while (!flag_recive){
        uint8_t* buffer_recive_data[72];
        UART_receive(buffer_recive_data, sizeof buffer_recive_data);

        int err = hdlc_timeout_handler(&hdlc, 1);
        if (err == ERR_FRAME_TIME_OUT){
            hdlc_get_raw_frame(&hdlc, buffer_data, sizeof buffer_data);
            UART_send(buffer_data, sizeof buffer_data);
            continue;
        }

        err = hdlc_decode_recived_raw_data(&hdlc, buffer_recive_data, sizeof buffer_recive_data, data_recive, &len_data_recive);
        if (err < 0){
            if (err == ERR_INVALID_SEQ_NUMBER_FRAME){
                uint8_t* buffer_rej[10];
                hdlc_get_raw_frame(&hdlc, buffer_rej, sizeof buffer_rej);
                UART_send(buffer_rej, sizeof buffer_rej);
                return err;
            }
            return err;
        }

        if (hdlc.state == SEND){
            flag_recive = true;
        }
    }

    struct message resp;
    protocol_decode(data_recive, len_data_recive, &resp);

    printLcd(&resp);

    return 0;
}
