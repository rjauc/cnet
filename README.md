# cnet
This is a small library for TCP network communication. It provides Server and Client
classes, which provide easy-to-use interfaces for creating a server to host a service,
and for a client to connect and communicate with a server. All of the methods are 
non-blocking, which allows for a simple interface to communicate with multiple clients 
at once.

## Security
Currently all of the data is sent INSECURELY. All data sent is in the form of PLAIN
bytes and NOT ENCRYPTED in any way. Users are encouraged to implement their own
encryption and decryption if that is required by their usecase.

## Examples
A documented example of a server and a client are provided in the source code.
