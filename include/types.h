#ifndef TYPES_H
#define TYPES_H

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

#endif // TYPES_H
