# Multi-stage build for Profile Service

# Stage 1: Builder
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    cmake \
    pkg-config \
    git \
    curl \
    libcurl4-openssl-dev \
    libcurlpp-dev \
    zip \
    unzip \
    tar \
    libpqxx-dev \
    libboost-all-dev \
    libssl-dev \
    zlib1g-dev \
    nlohmann-json3-dev \
    libpugixml-dev \
    libinih-dev \
    && rm -rf /var/lib/apt/lists/*

# Build minio-cpp (still needs vcpkg or manual build)
RUN git clone https://github.com/minio/minio-cpp.git /minio-cpp && \
    cd /minio-cpp && \
    # Manual build without vcpkg
    cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc) && \
    cmake --install build

WORKDIR /app
COPY . .

# Build the application
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpqxx-dev \
    libboost-system1.74.0 \
    libboost-program-options1.74.0 \
    libboost-log1.74.0 \
    libboost-thread1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-regex1.74.0 \
    libboost-date-time1.74.0 \
    libcurl4t64 \
    libcurlpp-dev \
    libssl3t64 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/profile_service /app/
COPY --from=builder /usr/local/lib/libminiocpp* /usr/local/lib/

ENV LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}

EXPOSE 8082

CMD ["./profile_service"]
