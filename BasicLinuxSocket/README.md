Source file for simplistic host side programs written in C++ are included in this folder.

This is Linux friendly, not based on WINSOCK. For more portability, a complete program could be written with MACROs to distinguish between OS environment.

As is, build with your favorite compiler, e.g.
  g++ -o tcpHost ./tcpServer.cc
and run the program,
then run a client program to connect to the host, send a message, receive a response, and then close the connection with the host.
I.E. the server program merely accepts a connection with a client, receives a message, and then echoes it back.
  The host program will terminate upon a 0 sized message from the connected client, an event that occurs when the client terminates connection.

NOTE on large message sizes:
  Variable sized large message buffering has been fixed by requiring the size of a stream to each receiving socket be stored as an unsigned 32 bit integer within the first four  bytes of the sent message.
  Therefore, in client code, simply have a uint32_t of the message size and ensure the first four bytes of the message are free to override. Then std::memcpy(address_of_message, address_of_uint32t_size, 4) to copy the 4 bytes to the message before sending.
