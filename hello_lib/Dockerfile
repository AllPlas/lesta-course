FROM gcc:12 as build

RUN apt update && \
    apt install -y \
    cmake \
    ninja-build \
    python3-pip && \
    pip3 install conan

WORKDIR /app

COPY conanfile.txt .
RUN mkdir build && cd build && \
    conan profile detect && \
    conan install .. --build=missing --output-folder=. -s compiler.cppstd=20

COPY CMakeLists.txt .
COPY hello_bin hello_bin
COPY hello_lib hello_lib
RUN cd build && cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

WORKDIR /app/build/hello_lib

RUN groupadd -r user && useradd -r -g user user && \
    chmod 777 /app && chown -R user /app
USER user

ENTRYPOINT ["ctest", "--output-on-failure"]
