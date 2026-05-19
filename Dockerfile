# Stage 1: Builder
FROM ubuntu:24.04 AS builder

# Install system build dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake pkg-config git curl ca-certificates \
    libssl-dev zlib1g-dev libcurl4-openssl-dev \
    libpqxx-dev libboost-all-dev nlohmann-json3-dev \
    libpugixml-dev libinih-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone minio-cpp and set up local vcpkg
WORKDIR /tmp/minio-cpp
RUN git clone https://github.com/minio/minio-cpp.git . && \
    git clone https://github.com/microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# Configure and build minio-cpp (static library)
RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake && \
    cmake --build build -j$(nproc) && \
    cmake --install build

# Build your project
WORKDIR /build
COPY . .
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y \
    ca-certificates curl libpqxx-dev libboost-system1.83.0 \
    libboost-program-options1.83.0 libboost-log1.83.0 \
    libboost-thread1.83.0 libboost-filesystem1.83.0 \
    libboost-regex1.83.0 libboost-date-time1.83.0 \
    libssl3t64 libcurl4t64 libcurlpp0t64 libinih1 \
    libpugixml1v5 zlib1g && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local/lib/libminiocpp.a /usr/local/lib/
COPY --from=builder /usr/local/include/minio-cpp/ /usr/local/include/minio-cpp/
COPY --from=builder /build/build/profile_service /usr/local/bin/

RUN ldconfig
EXPOSE 8082
CMD ["/usr/local/bin/profile_service"]