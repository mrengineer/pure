<<<<<<< HEAD
# Raspberry Pi Web Server

Simple web server in C++ for Raspberry Pi using the Mongoose library. Supports Server-Sent Events (SSE) for real-time time updates and command handling via POST requests in JSON.

## Features

- Display status and current time.
- "Ping" button for testing commands (returns "Pong" to the specific client).
- Universal command handling in `logic.cpp`.
- JSON data exchange.
- Support for static files (images, JS).

## Build and Run

### Requirements

- CMake
- C++ compiler (g++)
- Raspberry Pi (or POSIX Linux)

### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Run

```bash
./server
```

The server will start on port 8000. Open http://localhost:8000 in your browser.

## Project Structure

- `main.cpp`: Main server code, HTTP/SSE handling.
- `logic.cpp`: Command logic.
- `mongoose.h` / `mongoose.c`: Mongoose library.
- `CMakeLists.txt`: Build configuration.
- `static/index.html`: HTML page.
- `.gitignore`: Ignored files.

## Usage

- The page displays status and time (updates every 10 seconds).
- Click "Ping" to send a command and receive "Pong" in the span.

## License

[MIT](LICENSE) or add your own.
=======
# pure
App for raspberry pi to control DIY air purifier
>>>>>>> origin/main
