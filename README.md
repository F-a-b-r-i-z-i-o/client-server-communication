# client-server-communication

Client program and a server program.
The two processes interact using AF_UNIX sockets.

SERVER:
- handles multiple connections at a time via threads

- for each connection it launches a new thread that will manage the connection with the client

- the reply message is a string equal to the original message but each character has inverted the "letter case", i.e., uppercase characters become lowercase and vice versa

- Example: received string "hello" -> returned string "hello"

- The server checks if the string contains "white-space" characters ('', \ f, \ t, \ r, \ t, \ v). If any are found, it does NOT perform the conversion and returns an error to the client

CLIENT:
- opens a connection to the server and sends a string
- wait for the result before sending a new string
- ends when it receives the string "quit" as input (from the user)

