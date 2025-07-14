# hello-xapp

A minimalist FlexRIC xApp example.

## Build

### Prerequisites

- Docker
- CMake
- C++ Compiler (g++)

### Build with Docker

```bash
docker build -t hello-xapp .
```

### Build Natively

```bash
mkdir build
cd build
cmake ..
make
```

## Run

### Run with Docker

```bash
docker run --rm -it --name hello-xapp hello-xapp
```

### Run Natively

```bash
./build/hello-xapp
```

## Integration with FlexRIC

1.  **Start FlexRIC:** Follow the FlexRIC documentation to start the RIC.
2.  **Run the xApp:** Start the `hello-xapp` using either Docker or the native method.
3.  **Observe the output:** The xApp will print messages to the console as it connects to FlexRIC, subscribes to the KPM service, and receives indications.
