#include "server.hpp"

#include <algorithm>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace CNET
{
    Server::Server() {
    }

    Server::~Server() {
        if (m_socket != internal::EMPTY_SOCKET_HANDLE)
            Stop();
    }

    bool Server::Start(const std::string& port) {
        addrinfo* addrPtrList = nullptr;
        addrinfo addrHints { 0 };
        addrHints.ai_family = AF_INET;
        addrHints.ai_socktype = SOCK_STREAM;
        addrHints.ai_protocol = IPPROTO_TCP;
        addrHints.ai_flags = AI_PASSIVE;

        int result = getaddrinfo(nullptr, port.c_str(), &addrHints, &addrPtrList);
        if (result != 0)
            return false;

        auto addrPtr = addrPtrList;
        m_socket = socket(addrPtr->ai_family, addrPtr->ai_socktype, addrPtr->ai_protocol);
        if (m_socket == internal::EMPTY_SOCKET_HANDLE) {
            freeaddrinfo(addrPtrList);
            return false;
        }

        int flags = fcntl(m_socket, F_GETFL, 0);
        if (fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
            freeaddrinfo(addrPtrList);
            close(m_socket);
            m_socket = internal::EMPTY_SOCKET_HANDLE;
            return false;
        }

        result = bind(m_socket, addrPtr->ai_addr, addrPtr->ai_addrlen);
        if (result == -1) {
            freeaddrinfo(addrPtrList);
            close(m_socket);
            m_socket = internal::EMPTY_SOCKET_HANDLE;
            return false;
        }
        freeaddrinfo(addrPtrList);

        result = listen(m_socket, SOMAXCONN);
        if (result == -1) {
            close(m_socket);
            m_socket = internal::EMPTY_SOCKET_HANDLE;
            return false;
        }
        return true;
    }

    void Server::Stop() {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE)
            return;
        
        // Client destructor takes care to close its socket properly
        m_clients.clear();

        shutdown(m_socket, SHUT_RDWR);
        close(m_socket);
        m_socket = internal::EMPTY_SOCKET_HANDLE;
    }

    bool Server::IsClientPending() {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE)
            return false;

        fd_set readFDSet;
        FD_ZERO(&readFDSet);
        FD_SET(m_socket, &readFDSet);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        bool isClientWaiting = false;
        if (select(m_socket + 1, &readFDSet, nullptr, nullptr, &timeout) > 0)
            if (FD_ISSET(m_socket, &readFDSet))
                isClientWaiting = true;

        return isClientWaiting;
    }

    bool Server::AcceptClient() {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE)
            return false;

        internal::SocketHandle clientSocket = accept(m_socket, nullptr, nullptr);
        if (clientSocket == internal::EMPTY_SOCKET_HANDLE) {
            return false;
        }

        m_clients.push_back(Client(clientSocket));
        return true;
    }

    std::vector<Client>& Server::GetClients() {
        return m_clients;
    }

    void Server::RemoveDisconnectedClients() {
        m_clients.erase(
            std::remove_if(m_clients.begin(), m_clients.end(),
                [](Client& c) { return !c.IsConnected(); }), m_clients.end());
    }
}
