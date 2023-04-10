FROM gcc:latest as build

RUN apt update && \
    apt install -y \
    cmake \
    ninja-build

WORKDIR /app

COPY src src
COPY CMakeLists.txt .

RUN mkdir build && cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

FROM ubuntu:latest as run

RUN apt update && \
    apt install -y \
    cmake

WORKDIR /app

COPY --from=build /app .

RUN groupadd -r user && useradd -r -g user user && \
    chmod 777 /app && chown -R user /app

USER user

WORKDIR /app/build

ENTRYPOINT ["ctest", "--output-on-failure"]