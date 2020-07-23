FROM alpine:3.8

RUN apk add --update \
              ca-certificates \
              musl \
              build-base \
              python3 \
              python3-dev \
              bash \
              git \
              libxml2-dev \
              libxslt-dev \
              openssl-dev \
              opus-dev \
              pulseaudio-dev \
              alsa-lib-dev \
              cmake \
              gperf \
 && pip3.6 install --upgrade pip \
 && rm /var/cache/apk/*

# make us compatible with manylinux wheels and create some useful symlinks that are expected to exist
RUN echo "manylinux1_compatible = True" > /usr/lib/python3.6/_manylinux.py \
 && cd /usr/bin \
 && ln -sf easy_install-3.6 easy_install \
 && ln -sf idle3.6 idle \
 && ln -sf pydoc3.6 pydoc \
 && ln -sf python3.6 python \
 && ln -sf python-config3.6 python-config \
 && ln -sf pip3.6 pip \
 && ln -sf /usr/include/locale.h /usr/include/xlocale.h

WORKDIR /usr/src

# Build libtgvoip
RUN set -ex; git clone https://github.com/gabomdq/libtgvoip.git
WORKDIR /usr/src/libtgvoip
RUN set -ex; ./configure && make -j4 && make install

# Build tdlib
WORKDIR /usr/src
RUN set -ex; git clone https://github.com/tdlib/td.git
WORKDIR /usr/src/td
RUN set -ex; mkdir build 
WORKDIR /usr/src/td/build
RUN set -ex; cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN set -ex; cmake --build . -- -j4

# python-telegram
WORKDIR /usr/src
RUN git clone https://github.com/gabomdq/python-telegram.git
RUN mkdir -p python-telegram/telegram/lib/linux && cp td/build/libtdjson.so python-telegram/telegram/lib/linux
RUN set -ex; cd python-telegram;python setup.py install

# pytgvoip
WORKDIR /usr/src
RUN git clone https://github.com/gabomdq/pytgvoip.git
RUN cd pytgvoip;python setup.py install

CMD python


ARG VCS_REF
ARG VCS_URL
ARG BUILD_DATE
LABEL org.label-schema.vcs-ref=${VCS_REF} \
      org.label-schema.vcs-url=${VCS_URL} \
      org.label-schema.build-date=${BUILD_DATE}
