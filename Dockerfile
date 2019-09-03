# build
FROM heroku/heroku:18
LABEL maintainer=michel.promonet@free.fr

WORKDIR /streamer

ENV PATH="/webrtc/depot_tools:${PATH}"

RUN export DEBIAN_FRONTEND=noninteractive \
        && apt-get update && apt-get install -y --no-install-recommends g++ autoconf automake libtool xz-utils libasound2-dev libgtk-3-dev cmake p7zip-full vim sudo

RUN     git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git /webrtc/depot_tools \
        && export PATH=/webrtc/depot_tools:$PATH \
        && cd /webrtc \
        && fetch --no-history --nohooks webrtc \
        && sed -i -e "s|'src/resources'],|'src/resources'],'condition':'rtc_include_tests==true',|" src/DEPS \
        && gclient sync

# For automated builds in background...
RUN apt-get install -y inotify-tools

# COPY . /streamer
# EXPOSE 8000

ENTRYPOINT [ "/bin/bash" ]


