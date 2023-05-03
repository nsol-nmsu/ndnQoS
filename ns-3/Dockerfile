ARG VERSION=latest
FROM ndnsim:$VERSION

WORKDIR /cosim

COPY src ndnsim_src

RUN ./waf configure -d optimized --enable-examples

CMD ["./waf --run=ndn-20node-power"]