# UDP Network Quality Tester

A small C++17 project for trying out UDP sockets and seeing what happens to packets on a round trip.

There are two programs: a server that echoes packets back, and a client that sends them and measures the replies. At the end of a run, the client reports packet loss, round-trip time, jitter, and receive throughput.

The code uses Linux/POSIX sockets directly. Keeping the networking code small makes it easier to follow a packet from the client to the server and back.

## Build and run

You'll need Linux (or Ubuntu through WSL), a C++17 compiler, and CMake 3.14 or newer. The first build also needs internet access to download GoogleTest.

On Ubuntu, install the build tools with:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake ca-certificates
```

From the project folder, build everything:

```bash
cmake -S . -B build
cmake --build build
```

Start the server in one terminal:

```bash
./build/udp_server 9000
```

Then open another terminal in the same folder and run the client:

```bash
./build/udp_client 127.0.0.1 9000
```

This sends 100 packets to the server on your own machine. To choose a different count, add it as the last argument:

```bash
./build/udp_client 127.0.0.1 9000 1000
```

To test between two machines, run the server on one and use its IPv4 address in the client command on the other. The server's firewall needs to allow UDP traffic on the chosen port.

The client exits after printing its results. The server keeps listening until you stop it with `Ctrl+C`.

## How it works

The client sends one packet at a time and waits up to one second for a reply before moving on. The server sends each packet back to the address it came from.

```text
Client                              Server
  |                                   |
  |--- packet with a send timestamp -->|
  |                                   |
  |<--------- same packet back --------|
  |
  Calculate round-trip time
  and update the statistics
```

Both programs use the same packet structure:

```cpp
struct Packet {
    uint32_t sequenceNumber;
    uint64_t timestamp;
    char payload[1000];
};
```

The sequence number lets the client check which packet came back. The timestamp records when it was sent, using `std::chrono::steady_clock`. The payload adds 1,000 bytes of data to each packet.

The server doesn't need to interpret the timestamp or have its clock synchronized with the client. It just echoes the bytes, so both timing measurements happen on the client.

## Reading the results

**Packet loss** is the percentage of sent packets for which the client didn't record a matching reply:

```text
packet loss (%) = (sent - received) / sent * 100
```

A missing reply doesn't tell us where the packet was lost. The outgoing packet or its reply might have been dropped, or the reply might have arrived too late.

**Round-trip time (RTT)** is the time from sending a packet to receiving its echo. The report shows the average, minimum, and maximum RTT in milliseconds. Timing uses `steady_clock` so changes to the system clock don't affect it.

**Jitter** gives a rough idea of how much the delay varies. Here, it's the average absolute difference between consecutive recorded RTT values. This is a simple calculation for this project, not the RTP jitter algorithm.

**Throughput** is the number of echoed bytes received over the duration of the test, expressed in megabits per second:

```text
Mbps = (received bytes * 8) / elapsed seconds / 1,000,000
```

Because the client waits for each reply before sending again, this number depends on latency and timeouts. It isn't a measurement of your connection's maximum download speed.

Here's an example from a local run. Your timings will vary:

```text
## Results

Packets sent:       100
Packets received:   100
Packet loss:        0.00%

Average RTT:        0.06 ms
Minimum RTT:        0.05 ms
Maximum RTT:        0.71 ms
Jitter:             0.01 ms

Throughput:         129.83 Mbps
```

Testing against `127.0.0.1` checks that the programs work together, but the packets stay on your machine. Those results don't describe your Wi-Fi or internet connection.

## Finding your way around the code

```text
udp-network-tester/
|-- CMakeLists.txt
|-- README.md
|-- include/
|   |-- Packet.hpp
|   `-- Statistics.hpp
|-- src/
|   |-- client.cpp
|   |-- server.cpp
|   `-- Statistics.cpp
`-- tests/
    `-- test_statistics.cpp
```

Start with `server.cpp` to see the receive-and-echo loop, then look at `client.cpp` for packet sending, timeouts, and result reporting. `Packet.hpp` holds the shared packet format. The calculations live in `Statistics.cpp`, with the class declaration in `Statistics.hpp`.

## The networking basics

UDP sends separate messages called datagrams. It doesn't establish a connection first, and it doesn't guarantee delivery or ordering. A packet can be lost, duplicated, or arrive after a later packet. The application has to decide what to do about that.

TCP handles things differently: it provides an ordered stream of bytes and retransmits lost data. UDP is useful here because it lets the test observe missing replies without transport-level retransmissions hiding them.

A socket is the operating system's endpoint for network communication. Both programs create one with:

```cpp
socket(AF_INET, SOCK_DGRAM, 0)
```

`AF_INET` selects IPv4, `SOCK_DGRAM` selects datagrams, and `0` lets the system choose the default protocol, UDP in this case.

The IP address tells the client where to send packets; `127.0.0.1` points back to the same machine. The port identifies the receiving socket on that machine. In the commands above, the server listens on port `9000`, so the client uses `9000` too.

These are the socket calls you'll see in the code:

| Function | What it does here | Return value |
| --- | --- | --- |
| `socket()` | Creates a UDP socket for the client or server. | A file descriptor, or `-1` on error. |
| `bind()` | Gives the server a local address and port to listen on. It can fail if the port is already in use. | `0` on success, `-1` on error. |
| `sendto()` | Sends a packet to the server, or echoes it back to the client. | Number of bytes sent, or `-1` on error. |
| `recvfrom()` | Receives a packet and its sender's address, which the server uses to send the reply. | Number of bytes received, or `-1` on error or timeout. |
| `setsockopt()` | Sets the client's one-second receive timeout using `SO_RCVTIMEO`. | `0` on success, `-1` on error. |
| `inet_pton()` | Converts the client's text IPv4 address into the binary form sockets expect. | `1` on success, `0` for invalid input, `-1` on error. |
| `close()` | Releases the socket when the program is done with it. | `0` on success, `-1` on error. |

## Tests

Run the GoogleTest suite after building:

```bash
ctest --test-dir build --output-on-failure
```

The nine tests cover packet loss, RTT calculations, jitter, received-byte counts, and basic throughput behavior, including empty statistics. They run without a server or network connection. They don't test packet delivery; running the client and server together checks that separately.

To build just the client and server without downloading GoogleTest:

```bash
cmake -S . -B build -DBUILD_TESTING=OFF
cmake --build build
```
