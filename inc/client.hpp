#pragma once
#include "cnet.hpp"

#include <string>

namespace CNET
{
    class Client {
        public:
            Client(internal::SocketHandle socket = internal::EMPTY_SOCKET_HANDLE);
            Client(const Client&) = delete;
            Client(Client&& other) noexcept;

            ~Client();

            Client& operator=(const Client&) = delete;
            Client& operator=(Client&& other) noexcept;

            bool Connect(const std::string& address, const std::string& port);
            void Disconnect();
            bool IsConnected();

            std::string GetIP();

            int GetReceivePendingSize();
            bool IsReceivePending();

            bool Send(const void* data, size_t size);
            bool Receive(void* data, size_t maxSize);

        private:
            void Invalidate();
        private:
            internal::SocketHandle m_socket;
            bool m_isInvalid = false;
    };
}
