Description
============
Two applications with command line interface 
talk to each other using signed zmq messages.

Requirements
============
```
echo 'deb http://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/xUbuntu_18.04/ /' | sudo tee /etc/apt/sources.list.d/network:messaging:zeromq:release-stable.list
curl -fsSL https://download.opensuse.org/repositories/network:messaging:zeromq:release-stable/xUbuntu_18.04/Release.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/network:messaging:zeromq:release-stable.gpg > /dev/null
sudo apt update
sudo apt install libzmq3-dev=4.3.2

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

Run in docker containers
==========
Build image:
```
docker build .
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
