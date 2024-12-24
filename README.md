<h1>Modbus TCP/IP Server</h1>
After working with some modbus devices at my day-job, I decided to create a simple tcp/ip server that commmunicates via the modbus protocol.
Modbus is a protocol commonly used by industrial devices and PLCs for data acquisition. It was established in 1979 by Modicon, and is still
quite prevalent today despite its age.

I spent a majority of my time in making the base server functionality to be robust enough to handle multiple client connections through multithreaded responses.
The server will listen for a client connection and spin the connection off to a separate thread to begin responding to client requests once the connection is established.
