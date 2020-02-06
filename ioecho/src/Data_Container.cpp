#include <Data_Container.h>
#include <types.h>
#include <algorithm>
Data_Container::Data_Container() {
    memset (pool, 0, max_data_size);
    actual_size = 0;
    estimate_size = 0;
    header_size = 0;
    write_index = 0;
}

void Data_Container::reset() {
    memset (pool, 0, max_data_size);
    actual_size = 0;
    estimate_size = 0;
    header_size = 0;
    write_index = 0;
}

char *Data_Container::get_place_to_read() {
    return &pool [actual_size];
}

size_t Data_Container::get_size_to_read() const {
    if (estimate_size != 0) {
        return estimate_size + header_size - actual_size;
    }
    else {
        return  max_data_size - actual_size - std::max<size_t>(sizeof(tcp_header_t), sizeof(udp_header_t));
    }
}

const char *Data_Container::get_place_to_write() const {
    return &pool [write_index];
}

size_t Data_Container::get_size_to_write() const {
    return actual_size - write_index;
}

void Data_Container::update_estimate(size_t estimate) {
    estimate_size = (estimate > max_data_size) ? max_data_size : estimate;
}

bool Data_Container::is_data_complete_read() const {
    return (estimate_size == 0) ? false : (actual_size == estimate_size + header_size);
}

bool Data_Container::is_data_complete_write() const {
    return (write_index == actual_size);
}
