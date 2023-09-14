FROM ubuntu:20.04
RUN mkdir var/log/erss
RUN apt-get update && apt-get -y install g++ make
RUN mkdir /src
WORKDIR /src

CMD ["./proxy"]