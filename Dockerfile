FROM debian:buster

# Install compiling tools
RUN apt update \
&& apt --no-install-recommends -y install make git zlib1g-dev libssl-dev gperf php cmake clang libc++-dev libc++abi-dev

# Install libtgvoip pyhton3 and pip
RUN apt --no-install-recommends -y install libtgvoip-dev python3 python3-pip

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
&& ls -l td/tdlib

# Python scripts:
COPY setup.py root/
COPY tgcall.py root/
COPY tgvoip.cpp root/

# python-telegram
CMD pip3 install -r requirements.txt
RUN pip3 install --user python-telegram

# pytgvoip
CMD python3 setup.py install --user

# Purge unuseful packages
RUN apt -y purge make git zlib1g-dev libssl-dev gperf php cmake clang libc++-dev libc++abi-dev && apt -y autoremove

ARG VCS_REF
ARG VCS_URL
ARG BUILD_DATE
LABEL org.label-schema.vcs-ref=${VCS_REF} \
      org.label-schema.vcs-url=${VCS_URL} \
      org.label-schema.build-date=${BUILD_DATE}

# Purge tdlib source code
RUN rm -rf /usr/src/td
RUN rm -rf /var/lib/apt/lists/*
