FROM ubuntu:18.04

RUN apt-get update && \
    apt-get install -y \
        git \
        build-essential \
        gcc \
        g++ \
        vim \
        python \
        python-dev \
        sqlite \
        sqlite3 \
        libsqlite3-dev


RUN mkdir /cosim
WORKDIR /cosim

COPY ./ /cosim