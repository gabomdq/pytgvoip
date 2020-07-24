FROM debian:buster

RUN apt update

RUN apt -y install make git zlib1g-dev libssl-dev gperf php cmake clang libc++-dev libc++abi-dev python3

# Install libtgvoip
RUN apt -y install libtgvoip-dev

#WORKDIR /usr/src

#RUN set -ex; git clone https://salsa.debian.org/debian/libtgvoip.git
#WORKDIR /usr/src/libtgvoip
#RUN ./install-sh

# Build tdlib
WORKDIR /usr/src
RUN set -ex; git clone https://github.com/tdlib/td.git
WORKDIR /usr/src/td
RUN git checkout v1.6.0 \
&& rm -rf build \
&& mkdir build \
&& cd build \
&& export CXXFLAGS="-stdlib=libc++" \
&& CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=../tdlib .. \
&& cmake --build . --target install \
&& cd .. \
&& cd .. \
&& ls -l td/tdlib \

# python-telegram
RUN python3 -m pip install python-telegram

# pytgvoip

CMD python


ARG VCS_REF
ARG VCS_URL
ARG BUILD_DATE
LABEL org.label-schema.vcs-ref=${VCS_REF} \
      org.label-schema.vcs-url=${VCS_URL} \
      org.label-schema.build-date=${BUILD_DATE}
