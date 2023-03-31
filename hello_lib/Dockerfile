FROM gcc:12 as build

RUN apt update && \
    apt install -y \
    cmake \
    ninja-build \
    python3-pip && \
    pip3 install conan==1.59.0

COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing -s build_type=Release -s compiler.libcxx=libstdc++11

COPY CMakeLists.txt /app/
COPY hello_bin /app/hello_bin
COPY hello_lib /app/hello_lib
RUN cd /app/build && cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .


FROM ubuntu:latest as run


RUN apt update && \
    apt install -y \
    cmake


RUN groupadd -r user && useradd -r -g user user
USER user

COPY --from=build /app/build /app/build

ENTRYPOINT ["/bin/bash"]