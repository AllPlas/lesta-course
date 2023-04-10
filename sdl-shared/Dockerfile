FROM gcc:latest as build

RUN apt update && \
    apt install -y \
    cmake \
    ninja-build && \
    git clone https://github.com/libsdl-org/SDL.git && \
    cd SDL && mkdir build && cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DSDL_STATIC=ON -DSDL_STATIC_PIC=ON .. && \
    cmake --build . && cmake --install .


WORKDIR /app

COPY src src
COPY CMakeLists.txt .

RUN mkdir build && cd build  && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

FROM ubuntu:latest as run

RUN apt update && \
    apt install -y python3

WORKDIR /app

COPY tests tests
COPY --from=build /app/build/sdl_shared .
COPY --from=build /usr/local/lib lib

ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/app/lib

ENTRYPOINT ["/bin/sh", "-c", "./sdl_shared | python3 tests/test.py"]