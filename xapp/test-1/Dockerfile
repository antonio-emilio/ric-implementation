# Use a base image with a C++ compiler
FROM gcc:latest AS builder

# Set the working directory
WORKDIR /usr/src/app

# Copy the source code
COPY . .

# Create a build directory
RUN mkdir build
WORKDIR /usr/src/app/build

# Configure and build the xApp
RUN cmake ..
RUN make

# Final stage
FROM debian:buster-slim

# Set the working directory
WORKDIR /usr/app

# Copy the compiled xApp from the builder stage
COPY --from=builder /usr/src/app/build/hello-xapp .

# Set the command to run the xApp
CMD ["./hello-xapp"]
