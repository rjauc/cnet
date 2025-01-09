#include "client.hpp"

#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace CNET
{
    static inline intptr_t id = 0;

    Client::Client() {
    
    }

    Client::Client(internal::SocketHandle socket) {
        m_socket = socket;
    }

    Client::Client(Client&& other) noexcept {
        this->m_socket = other.m_socket;
        other.Invalidate();
    }

    Client::~Client() {
        if (m_socket != internal::EMPTY_SOCKET_HANDLE)
            Disconnect();
    }

    Client& Client::operator=(Client&& other) noexcept {
        this->m_socket = other.m_socket;
        other.Invalidate();
        return *this;
    }

    bool Client::Connect(const std::string& address, const std::string& port) {
        if (m_socket != internal::EMPTY_SOCKET_HANDLE) {
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
            if (m_socket == internal::EMPTY_SOCKET_HANDLE)
                break;

            result = connect(m_socket, addrPtr->ai_addr, addrPtr->ai_addrlen);
            if (result == -1) {
                close(m_socket);
                m_socket = internal::EMPTY_SOCKET_HANDLE;
                continue;
            }
            break;
        }
        freeaddrinfo(addrPtrList);
        if (m_socket == internal::EMPTY_SOCKET_HANDLE)
            return false;

        int flags = fcntl(m_socket, F_GETFL, 0);
        if (fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
            close(m_socket);
            m_socket = internal::EMPTY_SOCKET_HANDLE;
            return false;
        }
        return true;
    }

    void Client::Disconnect() {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE)
            return;
        shutdown(m_socket, SHUT_RDWR);
        close(m_socket);
        m_socket = internal::EMPTY_SOCKET_HANDLE;
    }

    bool Client::IsConnected() {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE) {
            return false;
        }

        char data;
        int result = recv(m_socket, &data, sizeof(data), MSG_PEEK);
        bool isConnected = false;
        if (result > 0) {
            // There is still data to be read
            isConnected = true;
        } else if (result == 0) {
            // Proper shutdown
            isConnected = false;
        } else if (result < 0) {
            // Error
            if (errno == ENOTCONN || errno == ENOTSOCK) {
                isConnected = false;
            } else if (errno == EWOULDBLOCK) {
                isConnected = true;
            } else {
                // Error not handled
            }
        }
        return isConnected;
    }

    std::string Client::GetIP() {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE) {
            return "Disconnected";
        }

        sockaddr_in addr;
        socklen_t addrLen = sizeof(addr);
        int result = getpeername(m_socket, reinterpret_cast<sockaddr*>(&addr), &addrLen);
        if (result == 0) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
            return ip;
        } else {
        }
        return "Unknown";
    }

    int Client::GetReceiveSize() {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE)
            return -1;

        int bytesPending;
        if (ioctl(m_socket, FIONREAD, &bytesPending) < 0)
            return -1;
        return bytesPending;
    }

    bool Client::IsReceiveReady() {
        bool isPending = GetReceiveSize() > 0;
        return isPending;
    }

    bool Client::Send(const void* data, size_t size) {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE)
            return false;

        int result = send(m_socket, reinterpret_cast<const char*>(data), size, 0);
        if (result == -1) 
            return false;
        return true;
    }

    bool Client::Receive(void* data, size_t maxSize) {
        if (m_socket == internal::EMPTY_SOCKET_HANDLE)
            return false;
        
        int result = recv(m_socket, reinterpret_cast<char*>(data), maxSize, 0);
        if (result == -1)
            return false;
        return true;
    }

    void Client::Invalidate() {
        m_socket = internal::EMPTY_SOCKET_HANDLE;
        m_isInvalid = true;
    }
}
