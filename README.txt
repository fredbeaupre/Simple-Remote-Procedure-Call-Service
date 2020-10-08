To run the program, do the following:
    1 - In a terminal, run 'make rpc'.
    2 - Start the server by running './frontend <host_ip> <host_port>
    3-  Start the client in another terminal by running './frontend <host_ip> <host_port>'.
    4 - To start more clients, repeat step 3

Quit/Shutdown/Exit functionality:
    'quit' & 'exit' - Typing these commands in the client will have the same effect,
                         they will close the client instantly with a goodbye message.
    'shutdown' - Typing shutdown in the client will send a shutdown signal to the server. 
                The server will retain its connection with existing clients, 
                but the next connection by a new client will  
                then shutdown the server.

