Requirements
============
```
echo 'deb http://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/xUbuntu_18.04/ /' | sudo tee /etc/apt/sources.list.d/network:messaging:zeromq:release-stable.list
curl -fsSL https://download.opensuse.org/repositories/network:messaging:zeromq:release-stable/xUbuntu_18.04/Release.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/network:messaging:zeromq:release-stable.gpg > /dev/null
sudo apt update
sudo apt install libzmq3-dev

sudo apt-get install libboost-all-dev
```

Make
==========
```
cmake .
cmake --build .
```

Use
==========
Run client:
```
./client.out
```
Run server:
```
./server.out
```
You should see client-server communication log in the commandline.
Press Enter in server command line to send stop instruction to client. 

Generating Keys
==========
```
openssl ecparam -name prime256v1 -genkey -noout -out private.ec.key
openssl ec -in private.ec.key -pubout -out public.pem
```