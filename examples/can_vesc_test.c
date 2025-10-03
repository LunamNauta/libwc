#include "wc/esc/vesc.h"
#include "wc/io/can.h"

#include <stdio.h>

int main(){
    wcvesc_packet_id_t packet_id;
    wccan_ctx_t can_ctx;
    wccan_frame_t frame;
    uint8_t unit_id;

    if (wccan_ctx_init(&can_ctx, "can0", 0) < 0){
        printf("Error: Failed to create CAN socket\n");
        return -1;
    }

    wcvesc_encode_set_duty(&frame, 121, 1.0f);
    wccan_ctx_write_bus(&can_ctx, &frame);

    wcvesc_encode_set_duty(&frame, 122, 1.0f);
    wccan_ctx_write_bus(&can_ctx, &frame);

    while (true){
        size_t bytes_read = wccan_ctx_read_bus(&can_ctx, &frame);
        if (!bytes_read) continue;
        if (bytes_read < 0){
            printf("Warning: Failed to read CAN packet\n");
            continue;
        }
        wcvesc_decode_id(wccan_frame_id(&frame), &packet_id, &unit_id);
        if (packet_id == WCVESC_STATUS_1) printf("Found packet: status1\n");
        else if (packet_id == WCVESC_STATUS_2) printf("Found packet: status2\n");
        else if (packet_id == WCVESC_STATUS_3) printf("Found packet: status3\n");
        else if (packet_id == WCVESC_STATUS_4) printf("Found packet: status4\n");
        else if (packet_id == WCVESC_STATUS_5) printf("Found packet: status5\n");
        else if (packet_id == WCVESC_STATUS_6) printf("Found packet: status6\n");
    }
}
