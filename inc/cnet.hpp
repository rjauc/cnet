#pragma once
#include <stdint.h>

namespace CNET
{
    namespace internal
    {
        using SocketHandle = uint64_t;
        static const SocketHandle EMPTY_SOCKET_HANDLE = (SocketHandle)(~0);

        static const uint8_t WSA_VERSION_MAJOR = 2;
        static const uint8_t WSA_VERSION_MINOR = 2;

        class GlobalState {
            public:
                GlobalState(const GlobalState&) = delete;

                static void Initialize();
                static void Terminate();
            private:
                static inline uint16_t m_usageCounter = 0;
        };
    }
}
