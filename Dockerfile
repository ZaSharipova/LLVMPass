FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    wget gnupg cmake ninja-build git \
    lsb-release \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 19 all && \
    rm llvm.sh

# RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
#         > /etc/apt/trusted.gpg.d/apt.llvm.org.asc && \
#     echo "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-19 main" \
#         > /etc/apt/sources.list.d/llvm.list && \
#     apt-get update && apt-get install -y \
#         clang-19 \
#         llvm-19-dev \
#         lld-19 \
#     && rm -rf /var/lib/apt/lists/*

RUN ln -s /usr/bin/clang-19    /usr/bin/clang && \
    ln -s /usr/bin/clang++-19  /usr/bin/clang++

RUN apt-get update && apt-get install -y \
    python3 python3-pygraphviz \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project
