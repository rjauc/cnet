#pragma once
#include <stdint.h>

namespace CNET
{
    namespace internal
    {
        using SocketHandle = uint64_t;
        inline const SocketHandle EMPTY_SOCKET_HANDLE = (SocketHandle)(~0);
    }
}
