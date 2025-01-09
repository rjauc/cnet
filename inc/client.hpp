#pragma once
#include "cnet.hpp"

#include <string>

namespace CNET
{
    class Client {
        public:
            Client();
            Client(const Client&) = delete;
            Client(Client&& other) noexcept;
            ~Client();

            Client& operator=(const Client&) = delete;
            Client& operator=(Client&& other) noexcept;

            bool Connect(const std::string& address, const std::string& port);
            void Disconnect();
            bool IsConnected();

            std::string GetIP();

            int GetReceiveSize();
            bool IsReceiveReady();

            bool Send(const void* data, size_t size);
            bool Receive(void* data, size_t maxSize);

        private:
            Client(internal::SocketHandle socket);
            void Invalidate();

        private:
            internal::SocketHandle m_socket { internal::EMPTY_SOCKET_HANDLE };
            bool m_isInvalid { false };

        private:
            friend class Server;
    };
}
