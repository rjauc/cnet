#include "cnet.hpp"

#include <stdexcept>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "logging.hpp"

namespace CNET
{
    namespace internal
    {
        void GlobalState::Initialize() {
            GlobalState::m_usageCounter += 1;
            if (GlobalState::m_usageCounter > 1)
                return;

            int result;
            WSADATA wsaData;
            result = WSAStartup(MAKEWORD(WSA_VERSION_MAJOR, WSA_VERSION_MINOR), &wsaData);
            if (result != 0)
                throw std::runtime_error("Failed to start up WinSock.");
        }

        void GlobalState::Terminate() {
            LOG_DECLARE();
            LOG_SET_NAME("GlobalState");
            LOG_DEBUG("Usage counter: {}", GlobalState::m_usageCounter-1);

            if (GlobalState::m_usageCounter > 0) {
                GlobalState::m_usageCounter -= 1;
                return;
            }

            WSACleanup();
        }
    }
}
