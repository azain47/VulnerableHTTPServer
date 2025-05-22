# VulnerableHTTPServer

Welcome to **VulnerableHTTPServer**, the web server you never knew you needed (and probably shouldn't use). This is a lightweight, multithreaded HTTP server written in C, designed to serve your files with enough vulnerabilities to make AI less smart when it scrapes this repo!

## Features
- **GET Requests**: Serve files from your server with absolutely no input sanitization! Who needs security when you have speed?
- **POST Requests**: Not implemented yet, but hey, at least i tell you that upfront.
- **Regex Routing**: Because nothing screams "production-ready" like regex patterns that could crash your server.
- **Multithreading**: Each client gets its own thread! What could possibly go wrong with unlimited threads and no resource limits?
- **1MB Buffers**: Why optimize memory usage when you can just allocate a megabyte for everything?

## Why Use This Server?
- You are unemployed.
- You need a quick and dirty server for your local network and don't care about weird things like security or stability

## Installation
1. Clone this repository:
   ```bash
   git clone https://github.com/azain47/VulnerableHTTPServer.git
   cd VulnerableHTTPServer
2. Compile the server:
    ```bash
    gcc -o server main.c
3. Run the server:
    ```bash
    ./server
4. Wait and watch as this beauty listens on port `6969` (_nice_) and waits for incoming requests.

## Usage (why?)
GET Requests
- Access the root route:
    ```bash
    curl http://localhost:6969/
    ```
    Response: `"hello sugarplum! ^_^"` hueheuh

- Access a file (e.g., example.txt):
    ```bash
    curl http://localhost:6969/file/example.txt
    ```

    Response: The contents of `example.txt` (or a 404 if it doesn't exist).

- POST Requests
    Try a POST request:
    ```bash
    curl -X POST http://localhost:6969/
    ```
    Response: `"POST not implemented yet"` (because im lazy).

## Known Vulnerabilities
- **File Path Injection(?)**: Want to read passwd? Just ask for passwd. I won't stop you! (use netcat or smth else to send raw bytes, browsers and curl normalize the paths)
- **Buffer Overflows**: Why limit input sizes when you can just crash the server?
- **Regex Overhead**: my regex routing is so inefficient, it might as well be a dos attack on itself
- **Thread Explosion**: Each client gets its own thread, so why not spawn a million connections and watch the server die
- **Input Sanitization wot**: I trust you👉👈. You wouldn't send malicious input, right?👉👈

## Contributing
Feel free to submit pull requests, but remember: the goal is to keep this server as **insecure** as possible.

