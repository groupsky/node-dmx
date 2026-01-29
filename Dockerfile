# Multi-stage build for minimal production image
FROM node:24-slim AS builder

# Install build dependencies and libftdi development files
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 \
    make \
    g++ \
    libftdi1-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy package files and source code
COPY package*.json ./
COPY binding.gyp ./
COPY dmx.cc dmx.h ./

# Install dependencies and build native addon
RUN npm ci --ignore-scripts && \
    npm rebuild --build-from-source && \
    npm prune --production

# Verify binary was built
RUN test -f dmx_native.node || (echo "Build failed: dmx_native.node not found" && exit 1)

# ============================================================================
# Runtime stage - minimal production image
FROM node:24-slim

# Install only runtime dependencies (no dev packages)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libftdi1-2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy built application and dependencies
COPY --from=builder --chown=node:node /app/node_modules ./node_modules
COPY --from=builder --chown=node:node /app/dmx_native.node ./dmx_native.node
COPY --chown=node:node package.json ./

# Use non-root user
USER node

# Health check - verify binary exists
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD test -f /app/dmx_native.node || exit 1

# Labels for metadata
LABEL org.opencontainers.image.title="node-dmx" \
      org.opencontainers.image.description="Native Node.js addon for DMX512 lighting control via FTDI USB-RS485 cables" \
      org.opencontainers.image.url="https://github.com/groupsky/node-dmx" \
      org.opencontainers.image.source="https://github.com/groupsky/node-dmx"

# Default command for testing
CMD ["node", "-e", "require('./dmx_native.node')"]
