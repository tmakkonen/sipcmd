FROM debian:stretch-slim

LABEL Description="SIP" Vendor="Daniel Drexlmaier" Version="1.6" maintainer="dd@dbe.academy"
ENV DEBIAN_FRONTEND noninteractive
ENV LOCALE en_EN.UTF-8

RUN echo "deb http://archive.debian.org/debian/ stretch contrib main non-free" > /etc/apt/sources.list
RUN apt-get update && apt-get -y upgrade &&  apt-get install --allow-downgrades -y git libopal-dev libpt-dev libsdl1.2-dev libpulse-dev libcaca-dev libglib2.0-dev libslang2-dev zlib1g-dev zlib1g=1:1.2.8.dfsg-5


RUN git clone https://github.com/tmakkonen/sipcmd.git && \
cd sipcmd && \
apt install -y make g++

#COPY alert1.wav /sipcmd/alert1.wav
#COPY alert2.wav /sipcmd/alert2.wav
#COPY alert3.wav /sipcmd/alert3.wav
#COPY Makefile /sipcmd/Makefile

RUN cd sipcmd && make
CMD tail -f /etc/debian_version

EXPOSE 5060
