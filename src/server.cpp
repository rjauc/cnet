#include "server.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "logging.hpp"

namespace CNET
{
    Server::Server() {
        internal::GlobalState::Initialize();
        LOG_SET_NAME("Server");
        // LOG_SET_FILE_NAME("Server");
        LOG_INFO("Initialized.");
    }

    Server::~Server() {
        if (m_socket != INVALID_SOCKET)
            Stop();
        internal::GlobalState::Terminate();
        LOG_INFO("Terminated.");
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
        if (m_socket == INVALID_SOCKET) {
            freeaddrinfo(addrPtrList);
            return false;
        }

        unsigned long mode = 1;
        if (ioctlsocket(m_socket, FIONBIO, &mode) != NO_ERROR) {
            freeaddrinfo(addrPtrList);
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }

        result = bind(m_socket, addrPtr->ai_addr, addrPtr->ai_addrlen);
        if (result == SOCKET_ERROR) {
            freeaddrinfo(addrPtrList);
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }
        freeaddrinfo(addrPtrList);

        result = listen(m_socket, SOMAXCONN);
        if (result == SOCKET_ERROR) {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }

        LOG_INFO("Started, listening to port {}.", port);
        return true;
    }

    void Server::Stop() {
        if (m_socket == INVALID_SOCKET)
            return;
        
        // Client destructor takes care to close its socket properly
        LOG_INFO("Disconnecting {} clients.", m_clients.size());
        m_clients.clear();

        shutdown(m_socket, SD_BOTH);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        LOG_INFO("Stopped.");
    }

    bool Server::IsClientWaitingToConnect() {
        if (m_socket == INVALID_SOCKET)
            return false;

        fd_set readFDSet;
        FD_ZERO(&readFDSet);
        FD_SET(m_socket, &readFDSet);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        bool isClientWaiting = false;
        if (select(0, &readFDSet, nullptr, nullptr, &timeout) > 0)
            if (FD_ISSET(m_socket, &readFDSet))
                isClientWaiting = true;

        // LOG_INFO("There {} client waiting to connect.", isClientWaiting ? "is a" : "is no");
        return isClientWaiting;
    }

    bool Server::AcceptNewClient() {
        if (m_socket == INVALID_SOCKET)
            return false;

        SOCKET clientSocket = accept(m_socket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            LOG_WARNING("Failed to accept a client.");
            return false;
        }

        m_clients.emplace_back(clientSocket);
        LOG_INFO("Accepted a client from {}.", m_clients.back().GetIP());
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
