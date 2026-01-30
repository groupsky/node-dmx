# Multi-stage build for minimal production image
FROM node:24.13.0-alpine3.23 AS builder

# Install build dependencies and libftdi development files
# --virtual creates a virtual package for easy cleanup
RUN apk add --no-cache --virtual .build-deps \
    python3 \
    make \
    g++ \
    gcc \
    libftdi1-dev \
    libusb-dev \
    pkgconf && \
    # Create symlinks for Debian compatibility (Alpine uses libftdi1, Debian uses libftdi)
    ln -s /usr/lib/libftdi1.so /usr/lib/libftdi.so && \
    ln -s /usr/include/libftdi1/ftdi.h /usr/include/ftdi.h

WORKDIR /app

# Copy package files and source code
COPY package*.json ./
COPY binding.gyp ./
COPY dmx.cc dmx.h ./

# Update npm to latest version for security fixes in build dependencies (node-gyp)
RUN npm install -g npm@latest

# Install dependencies and build native addon
# --omit=dev skips devDependencies, --ignore-scripts prevents postinstall vulnerabilities
RUN npm ci --omit=dev --ignore-scripts && \
    npm rebuild --build-from-source && \
    test -f dmx_native.node || (echo "Build failed: dmx_native.node not found" && exit 1)

# Note: Build dependencies remain in builder stage but are not copied to runtime stage

# ============================================================================
# Runtime stage - minimal production image
FROM node:24.13.0-alpine3.23

# Install only runtime dependencies (libftdi1 and libusb)
RUN apk add --no-cache \
    libftdi1 \
    libusb

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
