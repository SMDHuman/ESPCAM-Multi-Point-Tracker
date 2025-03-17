#ifndef SLIP_H
#define SLIP_H

//-----------------------------------------------------------------------------
#include <cstdint>
#include <cstddef>
#include <string.h>

//-----------------------------------------------------------------------------
#define S_END 0xC0
#define S_ESC 0xDB
#define S_ESC_END 0xDC
#define S_ESC_ESC 0xDD

//-----------------------------------------------------------------------------
struct slip_buffer_header_t
{   
    uint32_t len;
    uint32_t size;
    uint32_t checksum;
    uint8_t ready;
    uint8_t esc_flag;
    uint8_t checksum_enable;
    uint8_t overflow;
};

//-----------------------------------------------------------------------------
void slip_init(uint8_t *buffer, uint32_t size, uint8_t checksum_enable = true);
void slip_push(uint8_t *buffer, uint8_t data);
uint32_t slip_get_size(uint8_t *buffer);
uint8_t *slip_get_buffer(uint8_t *buffer);
void slip_reset(uint8_t *buffer);
uint8_t slip_is_ready(uint8_t *buffer);

#endif

//-----------------------------------------------------------------------------
#ifdef SLIP_IMPLEMENTATION
    //-----------------------------------------------------------------------------
    void slip_init(uint8_t *buffer, uint32_t size, uint8_t checksum_enable){
        slip_buffer_header_t slip_buffer_header;
        slip_buffer_header.len = 0;
        slip_buffer_header.size = size;
        slip_buffer_header.checksum = 0;
        slip_buffer_header.ready = false;
        slip_buffer_header.esc_flag = false;
        slip_buffer_header.checksum_enable = checksum_enable;
        memcpy(buffer, &slip_buffer_header, sizeof(slip_buffer_header_t));   
    }

    //-----------------------------------------------------------------------------
    void slip_push(uint8_t *buffer, uint8_t data){
        slip_buffer_header_t *slip_buffer_header = (slip_buffer_header_t *)buffer;
        uint8_t *data_buffer = buffer + sizeof(slip_buffer_header_t);
        if(slip_buffer_header->size <= slip_buffer_header->len){
            slip_reset(buffer);
            slip_buffer_header->overflow = true;
        }
        if(slip_buffer_header->ready){
            slip_reset(buffer);
        }
        if(slip_buffer_header->esc_flag){
            if(data == S_ESC_END){
                data_buffer[slip_buffer_header->len++] = S_END;
                slip_buffer_header->checksum += S_END;
            }
            else if(data == S_ESC_ESC){
                data_buffer[slip_buffer_header->len++] = S_ESC;
                slip_buffer_header->checksum += S_ESC;
            }
            slip_buffer_header->esc_flag = false;
        }
        else if(data == S_ESC){
            slip_buffer_header->esc_flag = true;
        }
        else if(data == S_END){
            slip_buffer_header->ready = true;
            if(slip_buffer_header->checksum_enable == true){
                slip_buffer_header->len -= 4;
            }
        }
        else{
            data_buffer[slip_buffer_header->len++] = data;
            slip_buffer_header->checksum += data;
        }
    }
    //-----------------------------------------------------------------------------
    void slip_reset(uint8_t *buffer){
        slip_buffer_header_t *slip_buffer_header = (slip_buffer_header_t *)buffer;
        slip_buffer_header->len = 0;
        slip_buffer_header->checksum = 0;
        slip_buffer_header->ready = false;
        slip_buffer_header->esc_flag = false;
        slip_buffer_header->overflow = false;
    }
    //-----------------------------------------------------------------------------
    uint8_t slip_is_ready(uint8_t *buffer){
        slip_buffer_header_t *slip_buffer_header = (slip_buffer_header_t *)buffer;
        if(slip_buffer_header->ready){
            if(slip_buffer_header->checksum_enable == true){
                uint32_t *checksum;
                memcpy(&checksum, buffer + slip_buffer_header->len, 4);
                if(slip_buffer_header->checksum == *checksum){
                    return(true);
                }else{
                    slip_reset(buffer);
                    return(false);
                }
            }else{
                return(true);
            }
        }
        return(false);
    }
    //-----------------------------------------------------------------------------
    uint32_t slip_get_size(uint8_t *buffer){
        slip_buffer_header_t *slip_buffer_header = (slip_buffer_header_t *)buffer;
        return(slip_buffer_header->len);
    }
    //-----------------------------------------------------------------------------
    uint8_t *slip_get_buffer(uint8_t *buffer){
        return(buffer + sizeof(slip_buffer_header_t));
    }
    
#endif