FROM debian:bookworm-slim AS deps

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    pkg-config \
    ca-certificates \
    libboost-system-dev \
    libboost-program-options-dev \
    libboost-log-dev \
    libboost-thread-dev \
    libboost-filesystem-dev \
    nlohmann-json3-dev \
    libpqxx-dev \
    libpq-dev \
  && rm -rf /var/lib/apt/lists/*

FROM deps AS builder

COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  && cmake --build build --config Release -j"$(nproc)"

FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    libboost-system1.74.0 \
    libboost-program-options1.74.0 \
    libboost-log1.74.0 \
    libboost-thread1.74.0 \
    libboost-filesystem1.74.0 \
    libpqxx-6.4 \
    libpq5 \
    ca-certificates \
  && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/profile_service /usr/local/bin/profile_service

EXPOSE 8082

ENTRYPOINT ["profile_service"]
CMD ["--address", "0.0.0.0", "--port", "8082"]
