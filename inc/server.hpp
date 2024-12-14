#pragma once
#include "cnet.hpp"
#include "client.hpp"

#include <string>
#include <vector>

namespace CNET
{
    class Server {
        public:
            Server();
            ~Server();

            bool Start(const std::string& port);
            void Stop();

            bool IsClientWaitingToConnect();
            bool AcceptNewClient();
            std::vector<Client>& GetClients();

            void RemoveDisconnectedClients();

        private:
            internal::SocketHandle m_socket;
            std::vector<Client> m_clients;
    };
}

