#pragma once
#include <stdint.h>
#include <stdexcept>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace CNET {
    namespace internal {
        static const uint8_t WSA_VERSION_MAJOR = 2;
        static const uint8_t WSA_VERSION_MINOR = 2;

        class GlobalState {
            public:
                GlobalState() = delete;
                GlobalState(const GlobalState&) = delete;
                ~GlobalState() = delete;

                static void Initialize() {
                    GlobalState::m_usageCounter += 1;
                    if (GlobalState::m_usageCounter > 1)
                        return;

                    int result;
                    WSADATA wsaData;
                    result = WSAStartup(MAKEWORD(WSA_VERSION_MAJOR, WSA_VERSION_MINOR), &wsaData);
                    if (result != 0)
                        throw std::runtime_error("Failed to start up WinSock.");
                }

                static void Terminate() {
                    if (GlobalState::m_usageCounter > 0) {
                        GlobalState::m_usageCounter -= 1;
                        return;
                    }

                    WSACleanup();
                }
            private:
                static inline uint16_t m_usageCounter = 0;
        };
    }
}
