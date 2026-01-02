## Server Process
This application is designed have a single server manage a maximum of 5 clients. You can configure this limit by changing the `#define MAX_CLIENTS 5` macro found in `./server.c`.

### Setup
cd into the `server/` directory (where you're currently at) and compile the server program
```
make server
```
After the server program compiles, run the server executable 
```
./server
```
On success, you'll see `STATUS: Server initiated` printed onto the terminal. Statements starting with `STATUS: ` are notifications from the server which can be errors or important updates.
Messages from clients are shown with `Chat: `.
