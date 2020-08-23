FROM ubuntu:18.04

# install zmq
RUN apt update && \
    apt install curl  -y
RUN echo "deb http://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/xUbuntu_18.04/ /" | tee /etc/apt/sources.list.d/network:messaging:zeromq:release-stable.list
RUN apt update --allow-insecure-repositories
RUN apt-get install libzmq3-dev=4.3.2 --allow-unauthenticated -y

# install rest
RUN apt install g++ -y && \
    apt-get install libboost-all-dev  -y && \
    apt-get install cmake -y && \
    apt-get install libssl-dev

COPY . .

RUN cmake -B./container_build -S.
RUN cp /public.pem /container_build/
RUN cp /private.ec.key /container_build/

WORKDIR "/container_build/"
RUN cmake --build .



