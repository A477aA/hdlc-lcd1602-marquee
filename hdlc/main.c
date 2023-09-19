//#include "hdlc.h"
#include "stdio.h"
#include "client.h"

int main(){
    struct Client hdlc;
    init_hdlc_client(&hdlc, 200);
    hdlc_control_t frame;
    hdlc_connect(&hdlc, &frame);
    uint8_t buffer_for_ex[20];
    uint8_t fake_buffer;
    hdlc_get_raw_frame(&hdlc, &frame, &buffer_for_ex, sizeof(buffer_for_ex));

    for (int i = 0; i < 200; i++){
        int z = hdlc_timeout_handler(&hdlc, 1);
        hdlc_decode_recived_raw_data(&hdlc, &fake_buffer, sizeof(fake_buffer), 0, 0);
    }

    hdlc_get_raw_frame(&hdlc, &frame, &buffer_for_ex, sizeof(buffer_for_ex));
    int i = hdlc_decode_recived_raw_data(&hdlc, &buffer_for_ex, sizeof(buffer_for_ex), 0, 0);
    if (i < 0){
        printf("err connect: %d\n", i);
    }

    hdlc_control_t frame_data;
    uint8_t data = 33;
    hdlc_send_data(&hdlc, &frame_data, &data, sizeof(data));
    uint8_t buffer_for_ex_data[7];

    hdlc_get_raw_frame(&hdlc, &frame_data, &buffer_for_ex_data, sizeof(buffer_for_ex_data));

    //printf("first_ex:%d\n", buffer_for_ex_data[0]);
    uint8_t recived_data;
    size_t len_recived_data;
    int x = hdlc_decode_recived_raw_data(&hdlc, &buffer_for_ex_data, sizeof(buffer_for_ex_data), &recived_data, &len_recived_data);
    if (x < 0){
        printf("err send: %d\n", x);
    }
    printf("recived data: %d\n", recived_data);




//    uint8_t send[64];
//    uint8_t buffer[134];
//    for (int i = 0; i < sizeof(send); i++) {
//        send[i] = (uint8_t) (rand() % 0x70);
//    }
//    send_data(&hdlc, send, sizeof(send_data));
//    //get_frame(&hdlc, buffer, sizeof(buffer), send_data, sizeof(send_data));
//
//    hdlc_get_raw_data(&hdlc, buffer, sizeof(buffer));

    // test 1

//    int ret;
//    uint8_t frame_data[8], recv_data[8];
//    size_t i, frame_length = 0, recv_length = 0;
//    hdlc_control_t control_send, control_recv;

//    // Run through the supported sequence numbers (3-bit)
//    for (i = 0; i <= 7; i++) {
//        // Initialize the control field structure with frame type and sequence number
//        control_send.frame = HDLC_FRAME_ACK;
//        control_send.seq_no = i;
//
//        // Create an empty frame with the control field information
//        ret = hdlc_frame_data(&control_send, NULL, 0, frame_data, &frame_length);
//
//        // Get the data from the frame
//        ret = hdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
//                              &recv_length);
//
//        // Result should be frame_length minus start flag to be discarded and no bytes received
//        if(ret != (int )frame_length - 1){
//            printf("err");
//        }
//    }
//    if (recv_length != 0){
//        printf("err2");
//    }
//
//    if (control_send.frame != control_recv.frame){
//        printf("err3");
//    }
//
//    if (control_send.seq_no != control_recv.seq_no){
//        printf("err4");
//    }

    // test 2

    // Run through the supported sequence numbers (3-bit)
//    for (i = 0; i <= 7; i++) {
//        // Initialize the control field structure with frame type and sequence number
//        control_send.frame = HDLC_FRAME_DATA;
//        control_send.seq_no = i;
//
//        char* input = "311";
//        uint8_t data = (uint8_t)atoi(input);
//
//        // Create an empty frame with the control field information
//        ret = hdlc_frame_data(&control_send, &data, 3, frame_data, &frame_length);
//        if (ret != 0){
//            printf("err123\n");
//        }
//
//        // Get the data from the frame
//        ret = hdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
//                            &recv_length);
//
//        // Result should be frame_length minus start flag to be discarded and no bytes received
//        if(ret != (int )frame_length - 1){
//            printf("err333\n");
//        }
//        if (recv_length != 0){
//            printf("err2\n");
//        }
//
//        // Verify the control field information
//        if (control_send.frame != control_recv.frame){
//            printf("err3\n");
//        }
//
//        if (control_send.seq_no != control_recv.seq_no){
//            printf("err4\n");
//        }
//    }

//    int ret;
//    hdlc_control_t control;
//    uint8_t send_data[512], frame_data[530], recv_data[530];
//    size_t i, frame_length = 0, recv_length = 0, buf_length = 16;
//
//    // Initialize data to be send with random values (up to 0x70 to keep below the values to be escaped)
//    for (i = 0; i < sizeof(send_data); i++) {
//        send_data[i] = (uint8_t) (rand() % 0x70);
//    }
//
//    // Initialize control field structure and create frame
//    control.frame = HDLC_FRAME_DATA;
//    ret = hdlc_frame_data(&control, send_data, sizeof(send_data), frame_data,
//                            &frame_length);
//
//    // Check that frame length is maximum 2 bytes larger than data due to escape of FCS value
//    if(frame_length >= ((sizeof(send_data) + 6) + 2)){
//        printf("1");
//    }
//    if(ret != 0){
//        printf("2");
//    }
//
//    // Run though the different buffers (simulating decode of buffers from UART)
//    for (i = 0; i <= sizeof(send_data); i += buf_length) {
//        // Decode the data
//        ret = hdlc_get_data(&control, &frame_data[i], buf_length, recv_data,
//                              &recv_length);
//
//        printf("%zu: %s\n", i, recv_data);
//
//        if (i < sizeof(send_data)) {
//            // All chunks until the last should return no message and zero length
//            if (ret != -ENOMSG){
//                printf("3");
//            }
//            if (recv_length != 0){
//                printf("1231");
//            }
//        } else {
//            if (ret > 7){
//                printf("332");
//            }
//            if (recv_length != sizeof(send_data)){
//                printf("88888");
//            }
//            // The last chunk should return max 6 frame bytes - 1 start flag sequence byte + 2 byte for the possible
//            // escaped FCS = 7 bytes
////            BOOST_CHECK(ret <= 7);
////            BOOST_CHECK_EQUAL(recv_length, sizeof(send_data));
//            //printf("5");
//        }
//    }
}