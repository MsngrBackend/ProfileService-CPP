# Multi-stage build for Profile Service
# Using Ubuntu 24.04 for better package support

# Stage 1: Builder
FROM ubuntu:24.04 AS builder

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

# Build minio-cpp
RUN git clone --depth 1 https://github.com/minio/minio-cpp.git /minio-cpp && \
    cd /minio-cpp && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON && \
    cmake --build build -j$(nproc) && \
    cmake --install build

WORKDIR /app
COPY . .

# Build the application
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libpqxx-dev \
    libboost-system1.83.0 \
    libboost-program-options1.83.0 \
    libboost-log1.83.0 \
    libboost-thread1.83.0 \
    libboost-filesystem1.83.0 \
    libboost-regex1.83.0 \
    libboost-date-time1.83.0 \
    libcurl4t64 \
    libcurlpp-dev \
    libssl3t64 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/profile_service /app/
COPY --from=builder /usr/local/lib/libminiocpp* /usr/local/lib/
COPY --from=builder /usr/local/lib/cmake/miniocpp /usr/local/lib/cmake/miniocpp 2>/dev/null || true

ENV LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}

EXPOSE 8082

CMD ["./profile_service"]