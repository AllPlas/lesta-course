FROM gcc:12 as build

RUN apt update && \
    apt install -y \
    cmake \
    ninja-build \
    python3-pip && \
    pip3 install conan


COPY conanfile.txt /home/user/app/
RUN mkdir /home/user/app/build && cd /home/user/app/build && \
    conan profile detect && \
    conan install .. --build=missing --output-folder=. -s compiler.cppstd=20

COPY CMakeLists.txt /home/user/app/
COPY hello_bin /home/user/app/hello_bin
COPY hello_lib /home/user/app/hello_lib
RUN cd /home/user/app/build && cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

WORKDIR /home/user/app/build/hello_lib
RUN groupadd -r user && useradd -r -g user user
USER user

ENTRYPOINT ["ctest", "--output-on-failure"]
