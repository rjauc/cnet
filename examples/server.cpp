#include <iostream>

#include <server.hpp>

int main() {
    // Start a server on port 27272
    std::cout << "Starting server ... " << std::flush;
    CNET::Server server;
    server.Start("27272");
    std::cout << "Started!\n";

    bool running = true;
    while (running) {
        // Accept any pending clients
        while (server.IsClientPending()) {
            std::cout << "Accepted a new client.\n";
            server.AcceptClient();
        }
        // Remove clients that are no longer connected from internal buffer
        server.RemoveDisconnectedClients();

        // Loop through all of the connected clients
        auto& clients = server.GetClients();
        for (auto& client: clients) {
            // Skip client if it hasn't sent any data
            if (!client.IsReceiveReady()) {
                continue;
            }

            // Receive pending client data
            auto recvSize = client.GetReceiveSize();
            std::string recvBuffer(recvSize, '\0');
            client.Receive(recvBuffer.data(), recvBuffer.size());
            std::cout << "Received: " << recvBuffer << '\n';

            // Act on received data
            if (recvBuffer == "Disconnect") {
                // Disconnect client
                client.Disconnect();
                continue;
            } else if (recvBuffer == "Stop") {
                // Stop server
                running = false;
                break;
            }

            // Echo client's message back to the client
            client.Send(recvBuffer.data(), recvBuffer.size());
        }
    }

    // Stop the server
    server.Stop();
}
