#include <iostream>

#include <client.hpp>

int main() {
    // Connect client to a server
    CNET::Client client;
    std::cout << "Connecting to server ... ";
    while(!client.Connect("127.0.0.1", "27272")) {}
    std::cout << "Connected!\n";

    // Read user input
    std::cout << "Input: ";
    std::string input;
    while(std::getline(std::cin, input)) {
        // Send user input to server
        client.Send(input.data(), input.size());

        // Wait for the client to receive echo
        while (!client.IsReceivePending()) {
            // Break out if server has disconnected the client
            if (!client.IsConnected()) {
                break;
            }
        }

        // Receive server echo
        auto recvSize = client.GetReceivePendingSize();
        std::string recvBuffer(recvSize, '\0');
        client.Receive(recvBuffer.data(), recvBuffer.size());
        std::cout << recvBuffer << '\n';

        // Additional disconnection check
        if (!client.IsConnected()) {
            break;
        }
        std::cout << "Input: ";
    }

    // Disconnect the client
    client.Disconnect();
}
