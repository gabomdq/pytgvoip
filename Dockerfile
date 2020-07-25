FROM debian:buster

# Install compiling tools, pyhton3 and pip
RUN apt update \
&& apt --no-install-recommends -y install make \
      git \
      zlib1g-dev \
      libssl-dev \
      gperf \
      php \
      cmake \
      clang \
      libc++-dev \
      libc++abi-dev \
      libopus-dev \
      libpulse-dev \
      libasound-dev \
      python3 \
      python3-pip \
      python3-dev

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
&& cmake --build . --target prepare_cross_compiling \
&& cd .. \
&& php SplitSource.php \
&& cd build \
&& cmake --build . --target install \
&& cd .. \
&& php SplitSource.php --undo \
&& cd .. \
&& ls -l td/tdlib
#&& export CXXFLAGS="-stdlib=libc++" \
#&& CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=../tdlib .. \
#&& cmake --build . --target install \
#&& cd .. \
#&& cd .. \
#&& ls -l td/tdlib

# Build libtgvoip
WORKDIR /usr/src
RUN set -ex; git clone https://github.com/gabomdq/libtgvoip.git

WORKDIR /usr/src/libtgvoip
RUN apt --no-install-recommends -y install 
RUN set -ex; ./configure && make -j4 && make install

# Copy python scripts and tgvoip.cpp
COPY *.py /root/
COPY requirements.txt /root/
COPY tgvoip.cpp /root/

# python-telegram
WORKDIR /root
RUN pip3 install setuptools wheel
RUN pip3 install -r requirements.txt

# pytgvoip
CMD python3 /root/setup.py install

ARG VCS_REF
ARG VCS_URL
ARG BUILD_DATE
LABEL org.label-schema.vcs-ref=${VCS_REF} \
      org.label-schema.vcs-url=${VCS_URL} \
      org.label-schema.build-date=${BUILD_DATE}

# Purge unuseful packages
RUN apt -y purge make git zlib1g-dev libssl-dev gperf php cmake clang libc++-dev libc++abi-dev && apt -y autoremove

# Purge tdlib and source code
RUN rm -rf /usr/src/td
RUN rm -rf /usr/src/libtgvoip

# Purge dowloaded packages
RUN rm -rf /var/lib/apt/lists/*

RUN ls -lh
CMD python3 /root/tgcall.py
#RUN while true; do sleep 1000; done
