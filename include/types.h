#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>

enum class ECHO_SOCKET_ACTION {
    NONE,
    ADD_NEW_SOCKET,
    ONE_SHOOT_ACTION,
    SAVE_SOCKET_AND_DATA,
    DELETE_SOCKET,
    ANSWER_AND_DELETE,
    KEEP_TRYING
};

enum class ECHO_PROTO {
    UDP,
    TCP
};

enum class ECHO_RET_CODE {
    OK,
    NEW,
    IN_PROGRESS,
    CLOSE,
    FAIL
};

struct tcp_header_t {
    size_t message_length;
};

struct udp_header_t {
    bool is_last;
};

#endif // TYPES_H
