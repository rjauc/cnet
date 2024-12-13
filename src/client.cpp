#include "client.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "logging.hpp"

namespace CNET
{
    static inline intptr_t id = 0;

    Client::Client(internal::SocketHandle socket) {
        internal::GlobalState::Initialize();
        m_socket = socket;

        if (socket == INVALID_SOCKET)
            LOG_SET_NAME("Client({})", id++);
        else 
            LOG_SET_NAME("(Server)Client({})", id++);
        // LOG_SET_FILE_NAME("Client");
        LOG_INFO("Initialized.");
    }

    Client::Client(Client&& other) noexcept {
        LOG_MOVE(this, other);
        this->m_socket = other.m_socket;
        other.Invalidate();
    }

    Client::~Client() {
        if (m_socket != INVALID_SOCKET)
            Disconnect();
        if (!m_isInvalid)
            internal::GlobalState::Terminate();
        LOG_INFO("Terminated.");
    }

    Client& Client::operator=(Client&& other) noexcept {
        LOG_MOVE(this, other);
        this->m_socket = other.m_socket;
        other.Invalidate();
        return *this;
    }

    bool Client::Connect(const std::string& address, const std::string& port) {
        if (m_socket != INVALID_SOCKET) {
            Disconnect();
        }

        addrinfo* addrPtrList = nullptr;
        addrinfo addrHints { 0 };
        addrHints.ai_family = AF_INET;
        addrHints.ai_socktype = SOCK_STREAM;
        addrHints.ai_protocol = IPPROTO_TCP;

        int result = getaddrinfo(address.c_str(), port.c_str(), &addrHints, &addrPtrList);
        if (result != 0)
            return false;
        for (auto addrPtr = addrPtrList; addrPtr != nullptr; addrPtr = addrPtr->ai_next) {
            m_socket = socket(addrPtr->ai_family, addrPtr->ai_socktype, addrPtr->ai_protocol);
            if (m_socket == INVALID_SOCKET)
                break;

            result = connect(m_socket, addrPtr->ai_addr, addrPtr->ai_addrlen);
            if (result == SOCKET_ERROR) {
                closesocket(m_socket);
                m_socket = INVALID_SOCKET;
                continue;
            }
            break;
        }
        freeaddrinfo(addrPtrList);
        if (m_socket == INVALID_SOCKET)
            return false;

        unsigned long mode = 1;
        if (ioctlsocket(m_socket, FIONBIO, &mode) != NO_ERROR) {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }
        LOG_INFO("Connnected to {}::{}", address, port);
        return true;
    }

    void Client::Disconnect() {
        if (m_socket == INVALID_SOCKET)
            return;
        shutdown(m_socket, SD_BOTH);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        LOG_INFO("Disconnected.");
    }

    bool Client::IsConnected() {
        if (m_socket == INVALID_SOCKET) {
            LOG_INFO("Is not connected (IVS)");
            return false;
        }

        // if (GetReceivePendingSize() >= 0) {
        //     LOG_INFO("Is connected (GRPS)");
        //     return true;
        // }

        char data;
        int result = recv(m_socket, &data, sizeof(data), MSG_PEEK);
        bool isConnected = false;
        if (result > 0) {
            // There is still data to be read
            isConnected = true;
            LOG_INFO("Data still not read.");
        } else if (result == 0) {
            // Proper shutdown
            isConnected = false;
            LOG_INFO("Proper shutdown.");
        } else if (result < 0) {
            // Error
            auto lastError = WSAGetLastError();
            LOG_DEBUG("Error {} occured.", lastError);
            if (lastError == WSAENOTCONN || lastError == WSAENOTSOCK) {
                isConnected = false;
                LOG_DEBUG("Erroneous shutdown(?) ASSUME DISCONNECTED");
            } else if (lastError == WSAEWOULDBLOCK) {
                isConnected = true;
                LOG_DEBUG("Still connected maybe(?) ASSUME CONNECTED");
            } else {
                LOG_ERROR("Error {} not handled.", lastError);
            }
        }

        LOG_INFO("Is {}connected", isConnected ? "" : "not ");
        return isConnected;
    }

    std::string Client::GetIP() {
        if (m_socket == INVALID_SOCKET) {
            return "Disconnected";
        }

        sockaddr_in addr;
        int addrLen = sizeof(addr);
        int result = getpeername(m_socket, reinterpret_cast<sockaddr*>(&addr), &addrLen);
        if (result == 0) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
            return ip;
        } else {
            auto lastError = WSAGetLastError();
            LOG_ERROR("Error {} occured.", lastError);
        }
        return "Unknown";
    }

    int Client::GetReceivePendingSize() {
        if (m_socket == INVALID_SOCKET)
            return -1;

        unsigned long bytesPending;
        int result = ioctlsocket(m_socket, FIONREAD, &bytesPending);

        if (result != 0) {
            auto lastError = WSAGetLastError();
            LOG_ERROR("Error {} occured.", lastError);
            return -1;
        }
        
        if (bytesPending)
            LOG_INFO("There are {} bytes pending read.", bytesPending);
        return bytesPending;
    }

    bool Client::IsReceivePending() {
        bool isPending = GetReceivePendingSize() > 0;
        return isPending;
    }

    bool Client::Send(const void* data, size_t size) {
        if (m_socket == INVALID_SOCKET)
            return false;

        int result = send(m_socket, reinterpret_cast<const char*>(data), size, 0);
        if (result == SOCKET_ERROR) 
            return false;
        LOG_INFO("Sent {} bytes.", result);
        return true;
    }

    bool Client::Receive(void* data, size_t maxSize) {
        if (m_socket == INVALID_SOCKET)
            return false;
        
        int result = recv(m_socket, reinterpret_cast<char*>(data), maxSize, 0);
        if (result == SOCKET_ERROR)
            return false;
        LOG_INFO("Received {} bytes.", result);
        return true;
    }

    void Client::Invalidate() {
        m_socket = INVALID_SOCKET;
        m_isInvalid = true;
    }
}
