#ifndef DATA_CONTAINER_H
#define DATA_CONTAINER_H

#include <cstring>
#include <algorithm>
#include <types.h>

struct Data_Container
{
    static constexpr size_t max_data_size = 65537;
    char pool [max_data_size];
    size_t actual_size;
    size_t estimate_size;
    size_t header_size;
    size_t write_index;
    
    Data_Container ();
    
    void reset ();
    
    char* get_place_to_read ();
    size_t get_size_to_read () const;

    const char* get_place_to_write () const;
    size_t get_size_to_write () const;
    
    void update_estimate (size_t estimate);
    
    bool is_data_complete_read () const;
    bool is_data_complete_write () const;
};

#endif //DATA_CONTAINER_H
