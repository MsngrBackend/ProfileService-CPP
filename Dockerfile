FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    cmake \
    pkg-config \
    git \
    curl \
    zip \
    unzip \
    tar \
    libpqxx-dev \
    libboost-all-dev \
    libssl-dev \
    zlib1g-dev \
    nlohmann-json3-dev

# Устанавливаем vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /vcpkg && \
    /vcpkg/bootstrap-vcpkg.sh -disableMetrics

# Устанавливаем зависимости minio через vcpkg
RUN /vcpkg/vcpkg install curlpp pugixml inih

# Собираем minio-cpp
RUN git clone https://github.com/minio/minio-cpp.git /minio-cpp && \
    cd /minio-cpp && \
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    cmake --build build -j$(nproc) && \
    cmake --install build

WORKDIR /app
COPY . .

RUN find / -name "libminio*" 2>/dev/null | head -20 && \
    find /usr/local/include -type f -name "*.h" 2>/dev/null | head -20 && \
    find /minio-cpp -name "client.h" 2>/dev/null

RUN grep -n "PutObject\|UploadObject\|ApiArgs" /usr/local/include/miniocpp/baseclient.h | head -30

RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    cmake --build build -j$(nproc)

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libpqxx-dev \
    libboost-system1.74.0 \
    libboost-program-options1.74.0 \
    libboost-log1.74.0 \
    libboost-thread1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-regex1.74.0 \
    libboost-date-time1.74.0 \
    libcurl4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/profile_service .
COPY --from=builder /usr/local/lib/libminio* /usr/local/lib/
COPY --from=builder /vcpkg/installed/x64-linux/lib/libcurlpp* /usr/local/lib/
RUN ldconfig

EXPOSE 8082
CMD ["./profile_service"]