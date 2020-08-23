Description
============
Two applications with command line interface 
talk to each other using signed zmq messages.
```
~/zmqhb$ ./server.out
SERVER
Press Enter to send stop instruction

Send:
areYouAlive+1598196459
Received:
{"timestamp":"AliveAt1598196459","statuses":{"subsystem1":"ERROR","subsystem2":"OK"}}

Send:
areYouAlive+1598196460
Received:
{"timestamp":"AliveAt1598196460","statuses":{"subsystem1":"OFF","subsystem2":"WARNING"}}
```

```
~/zmqhb$ ./client.out
CLIENT
Received:
areYouAlive+1598196459
Send:
{"timestamp":"AliveAt1598196459","statuses":{"subsystem1":"ERROR","subsystem2":"OK"}}

Received:
areYouAlive+1598196460
Send:
{"timestamp":"AliveAt1598196460","statuses":{"subsystem1":"OFF","subsystem2":"WARNING"}}
```

Requirements
============
```
#install ccpzmq
echo 'deb http://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/xUbuntu_18.04/ /' | sudo tee /etc/apt/sources.list.d/network:messaging:zeromq:release-stable.list
curl -fsSL https://download.opensuse.org/repositories/network:messaging:zeromq:release-stable/xUbuntu_18.04/Release.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/network:messaging:zeromq:release-stable.gpg > /dev/null
sudo apt update
sudo apt install libzmq3-dev=4.3.2

#install the rest
sudo apt install g++
sudo apt-get install libboost-all-dev
sudo apt-get install cmake
sudo apt-get install libssl-dev
```

Make
==========
```
cmake .
cmake --build .
```

Use
==========
Run client in terminal:
```
./client.out
```
Run server in the other terminal:
```
./server.out
```
You should see client-server communication log in the terminal.
Press Enter in server command line to send stop instruction to client. 

The aplications read corresponsing keys from the source directory.

Run in docker containers
==========
Build image:
```
sudo docker build .
```
Run client terminal:
```
sudo docker-compose up client
```
Run server in the other terminal:
```
sudo docker-compose up server
```
You should see client-server communication log in the terminal.
Press Enter in server command line to send stop instruction to client.

Generating Keys
==========
```
openssl ecparam -name prime256v1 -genkey -noout -out private.ec.key
openssl ec -in private.ec.key -pubout -out public.pem
```
