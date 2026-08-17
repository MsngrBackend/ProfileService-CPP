# Profile Service

A C++ microservice for managing user profiles, contacts, favorites, and notification settings.

**To build and run use Docker Compose:**

```bash
docker compose up -d --build
```

**Supported Platforms**

Tested on Ubuntu 24.04 LTS only. Other Linux distributions may work but are not officially supported.

**Quick Start**

***Using Docker Compose (Recommended)***

```
# Clone the repository
git clone <repository-url>
cd profile-service

# HTTP version (default)
docker compose --profile http up -d

# HTTPS version (auto-generates certificates)
docker compose --profile https up -d

# Both HTTP and HTTPS
docker compose --profile both up -d

# Check logs
docker compose logs -f app-http

# Stop services
docker compose down
```

***Using Helper Scripts***

```
# Make scripts executable
chmod +x scripts/*.sh

# Build specific version
./scripts/build.sh http
./scripts/build.sh https
./scripts/build.sh both

# Run specific version
./scripts/run.sh http
./scripts/run.sh https
./scripts/run.sh both
```

***Using Makefile***

**Docker Build Options**

***HTTP Version (Default)***

```
# Build HTTP image
docker build -f Dockerfile.http -t profile-service:http .

# Or using docker-compose
docker compose --profile http up -d
```

***HTTPS Version (Auto-generated certificates)***

```
# Build HTTPS image with self-signed certificates
docker build -f Dockerfile.https -t profile-service:https .

# Or using docker-compose
docker compose --profile https up -d
```

***Both HTTP and HTTPS***

```
# Build both versions
docker build --build-arg ENABLE_HTTPS=true -f Dockerfile -t profile-service:both .

# Or using docker-compose
docker compose --profile both up -d
```

**Certificate Management**

The HTTPS Dockerfile automatically generates self-signed certificates during the build process:

- server.crt - Certificate file

- server.key - Private key file

- Generated with 2048-bit RSA encryption

- Valid for 365 days

- Subject: CN=localhost

**Custom Certificates**

To use your own certificates:

1) Create a certs directory

2) Place your server.crt and server.key files

3) Mount them as volumes in docker-compose.yml:

```
volumes:
  - ./certs/server.crt:/app/server.crt:ro
  - ./certs/server.key:/app/server.key:ro
```

**Dependencies**

***System Dependencies (for native build)***

```bash
# Install required system packages
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    g++ \
    libboost-all-dev \
    libpqxx-dev \
    libssl-dev \
    nlohmann-json3-dev \
    postgresql \
    postgresql-contrib \
    docker.io \
    docker-compose-plugin
```

**Building from Source**

```
# Clone the repository
git clone <repository-url>
cd profile-service

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . -- -j$(nproc)

# Run tests
ctest --test-dir .
```

**Docker Compose (Recommended)**

```bash
# Build and start all services
docker compose up -d --build

# View logs
docker compose logs -f app

# Stop services
docker compose down
```

**Environment Variables**

| Variable           | Description                  | Required        | Default          |
|--------------------|------------------------------|-----------------|------------------|
| DATABASE_URL       | PostgreSQL connection string | Yes             | -                |
| MINIO_ENDPOINT     | MinIO server endpoint        | No              | localhost:9000   |
| MINIO_ACCESS_KEY   | MinIO access key             | No              | -                |
| MINIO_SECRET_KEY   | MinIO secret key             | No              | -                |

**Command Line Options**

| Option              | Description                | Default     |
|---------------------|----------------------------|-------------|
| `--address`, `-a`   | Bind address               | 0.0.0.0     |
| `--port`, `-p`      | Bind port                  | 8082        |
| `--tls`             | Enable TLS/HTTPS           | false       |
| `--cert`            | TLS certificate file       | server.crt  |
| `--key`             | TLS private key file       | server.key  |
| `--dhparam`         | TLS DH parameters file     | (empty)     |

**Example with TLS**

```bash
# Generate self-signed certificate for testing
openssl req -x509 -newkey rsa:2048 -nodes \
    -keyout server.key -out server.crt \
    -days 365 -subj "/CN=localhost"

# Generate DH parameters (optional, for perfect forward secrecy)
openssl dhparam -out dhparam.pem 2048

# Start with TLS
docker compose up -d --build

# Or run natively with TLS
./profile_service --address 0.0.0.0 --port 8443 --tls
```

**Authentication**

All endpoints (except /internal/profiles) require an X-User-ID header with a valid user ID.

**Testing**

```bash
# Run all tests
ctest --test-dir build

# Run specific test executables
./build/tests/api_spec_boost
./build/tests/api_spec_catch2
./build/tests/api_spec_gtest
```

**View Docker logs**

```bash
# View all logs
docker compose logs

# View specific service
docker compose logs app-http
docker compose logs app-https
docker compose logs postgres
docker compose logs minio

# Follow logs
docker compose logs -f app-http

# View logs with timestamps
docker compose logs --timestamps
```

**Test Endpoints**

```bash
# Enter running container
docker compose exec app-http /bin/bash

# Check if service is running
curl http://localhost:8082/me -H "X-User-ID: test-user"

# Test HTTPS endpoint (ignore self-signed cert)
curl -k https://localhost:8443/me -H "X-User-ID: test-user"

# Check database connection
docker compose exec postgres psql -U profile_user -d profile_service -c "SELECT 1"
```