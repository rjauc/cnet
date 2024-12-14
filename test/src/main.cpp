#include <iostream>
#include <thread>

#include "logging.hpp"
#include "args.hpp"

#include <cnet.hpp>
#include <client.hpp>
#include <server.hpp>

int main(int argc, char** argv) {
    LOG_DECLARE();

    ARG::ArgParser args;
    args.Add("Type",       { .opt="-t", .longOpt="--type", .description="Specify whether to run a Server or a Client.", .isMandatory=true });
    args.Add("Address",    { .opt="-a", .longOpt="--address", .value="127.0.0.1", .description="Network address to connect to as a Server." });
    args.Add("Port",       { .opt="-p", .longOpt="--port", .value="6969", .description="Network port to use." });
    args.Add("Message",    { .longOpt="--message", .description="Custom message for the client to send to the server on startup." });
    if (!args.Parse(argc, argv))
        return 0;

    if (args["Type"] == "Client") {
        LOG_SET_FILE_NAME(args["Type"]);
        CNET::Client client;
        while (!client.Connect(args["Address"], args["Port"])) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::string sendBuffer;
        if (args.IsSet("Message")) {
            sendBuffer = args["Message"];
        } else {
            sendBuffer = "Hello from the client!"
                + std::format("{:#010x}", reinterpret_cast<intptr_t>(&client)); 
        }
        client.Send(sendBuffer.data(), sendBuffer.size()+1);

        while (!client.IsReceivePending()) {
            if (!client.IsConnected()) {
                return 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        auto recvSize = client.GetReceivePendingSize();
        std::string recvBuffer(recvSize, '\0');
        client.Receive(recvBuffer.data(), recvBuffer.capacity());
        std::cout << "Client Received: " << recvBuffer << '\n';

        client.IsConnected();
        client.Disconnect();
        client.IsConnected();
    } else if (args["Type"] == "Server") {
        LOG_SET_FILE_NAME(args["Type"]);
        CNET::Server server;
        if (!server.Start(args["Port"]))
            return 4;
        
        bool running = true;
        while(running) {
            while (server.IsClientWaitingToConnect()) {
                server.AcceptNewClient();
            }
            server.RemoveDisconnectedClients();

            auto& clients = server.GetClients();
            for (auto& client: clients) {
                if (!client.IsConnected()) {
                    LOG_WARNING("Disconnected client is still here ...");
                    continue;
                }

                if (!client.IsReceivePending()) {
                    auto pendingBytes = client.GetReceivePendingSize();
                    if (pendingBytes < 0)
                        LOG_ERROR("Was not pending but ERROR!");
                    if (pendingBytes > 0)
                        LOG_WARNING("Was not pending but has {} bytes to receive.", pendingBytes);
                    continue;
                }

                auto recvSize = client.GetReceivePendingSize();
                if (recvSize >= 10000 || recvSize <= 0) {
                    LOG_ERROR("Client recv ERROR/overflow. Total of {} bytes.", recvSize);
                    continue;
                }
                std::string recvBuffer(recvSize, '\0');
                client.Receive(recvBuffer.data(), recvBuffer.capacity());
                std::cout << "Server Received: " << recvBuffer << '\n';
                if (recvBuffer.contains("Message=fuck+you")) {
                    running = false;
                    break;
                }

                std::string sendBuffer;
                if (recvBuffer.starts_with("POST / HTTP/1.1") || recvBuffer.starts_with("GET / HTTP/1.1")) {
                    sendBuffer = "HTTP/1.1 200 OK\r\n";
                    sendBuffer += "Content-Type: text/html; charset=UTF-8\r\n";
                    
                    static int serveCounter = 0;
                    serveCounter += 1;

                    std::ifstream f("./index.html");
                    std::stringstream ss;
                    ss << f.rdbuf();
                    auto str = ss.str();
                    str += "<h2>Your public IPv4 is: " + client.GetIP() + "</h2>\r\n";
                    str += "<h2>Website was served " + std::to_string(serveCounter) + " times.</h2>\r\n";
                    str += "</body>\r\n</html>\r\n\r\n";

                    sendBuffer += "Content-length: " + std::to_string(str.length()) + "\r\n\r\n";
                    sendBuffer += str;
                } else {
                        sendBuffer = "HTTP/1.1 404 Not Found\r\n";
                }
                client.Send(sendBuffer.data(), sendBuffer.size()+1);
                LOG_DEBUG("Just served client with IP {}", client.GetIP());
                client.Disconnect();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } else {
        LOG_ERROR("Invalid command-line arguments.");
        return 2;
    }
}
