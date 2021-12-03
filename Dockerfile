FROM  alpine:latest
RUN   adduser -S -D -H -h /xmrig miner
RUN   apk --no-cache upgrade && \
      apk --no-cache add git make cmake libstdc++ gcc g++ libuv-dev openssl-dev hwloc-dev && \
      git clone https://github.com/MoneroOcean/xmrig.git && \
      cd xmrig && \
      cmake -DCMAKE_BUILD_TYPE=Release . && \
      mkdir build && \
      make && \
      apk del git make cmake libstdc++ gcc g++

USER miner
WORKDIR /xmrig
entrypoint  ["./xmrig"]
