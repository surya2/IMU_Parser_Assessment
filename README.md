# True Anomaly Assessment
#### Surya Shanmugaselvam

## Quick Disclaimer
I tried using /dev/tty1 for my serial port. However, as it is a serial port, my operating ssytem did not allow me to have 2 internal processes both writing to and reading from /dev/tty1.
For this reason, to simulate hardware communicating to my C++ parsing code, I used a command call `socat` to create a virtual serial port and 2 file descriptors called PTY devices to simulate a hardware connection. I had me LoopTester.py Python script open one PTY device and my C++ imu_parser code to open the other PTY device. This way, my C++ code received the UART frames my Python code was sending. I have included usage instructions below.

## Tasks:
### Task 1
Description: Write a parser to receive a byte stream from an IMU at 921600 baud rate and parse the packet fields\
Solution for Parsing function:
- Wrote function called `parsePacket` in *IMU_Parser/src/parse_functions.cpp*
- The function receives `uint8_t` buffer which represents the input received by the IMU device over a serial port
- The function calls the `findHeader` function given a position to start at in the buffer. The `findHeader` function will find the first occurrence of the start header *0x7F 0xF0 0x1C 0xAF* which signifies the beginning of a UART frame
- `findHeader` returns *true* if a packet is found in the buffer and sets the position at the start of the packet.
- `parsePacket` then creates a `ParsedPacket` struct and copies bytes 4-7 from the new position in the buffer for the packet count, bytes 8-11 for the X-axis Gyro Rate, byte 12-15 for the Y-axis Gyro Rate, and bytes 16-19 for the X-axis Gyro Rate (indexed from 0)
- The function returns the packet struct. Now because the struct object always has the same size regardless of whether it was written to or not, I decided to return a *std::pair* instead with the second field being a boolean signifying whether a packet was extracted
Solution for UART reader:
- I also wrote a function called `processIMU_Frames` in *IMU_Driver/src/imu_parser.cpp*
- The function reads any incoming bytes that have arrived at the seral port and copies them into a byte array called `stagingBuffer` using the `read` function from the *iostream* library
- The `stagingBuffer` is then copied/inserted at the end of `inputBuffer` which is a *uint8_t* byte vector, holding all bytes that have been received thu far. The reason for having a separate `stagingBuffer` and `inputBuffer` is because the `read` function that copies bytes to `stagingBuffer` may not receive an entire frame/packet and instead parts of an IMU packet at a given time. So it is inappropriate to immediately parse received data. Instead, data is inserted to `inputBuffer` so that all of the bytes can be retrieved and concatenated together to get the full packet content. The only will the `parsePacket` function be called.
- It is important to note that there is also a main method in *IMU_Driver/src/imu_parser.cpp* which gets the UART port file, either the default '/dev/tty1' or some a custom file passed in as an arguments like '/dev/pts/2'
- The main method also sets up configurations such as the input baud rate to 921600 using the *termios* library.
### Task 2
Description: What modifications, if any, would need to be made to the code from if the Linux host was a little-
endian processor?\
Solution:
- The IEEE-754 network byte order standard means that fields in a packet are written in big endian.
- Thus, packets that are received and passed into `parsePacket` are in big-endian order which means that a packet count of 21,323 which is 0x000534B would be received as 0x000534B while a little-endian machine would receive it as 0x4B530000 with the least significant byte first
- This standard would work for big-endian machines, but not for little-endian machines which read the bytes from lef to right.
  - To address this, I wrote some code shown below which checks to see if the machine is little-endian:
      ``` 
    int test_number = 1;
    char *test_number_ptr = (char*)&test_number;

    if (*test_number_ptr == 1) {
           little_endian = true;
    } 
    ```
    If the least significant byte was stored first, the pointer to the number would be dereferenced as the lesst significant byte, indicating a little-endian machine.
- To address this, I actually used an already written function called `ntohl` from *<arpa/inet.h* Unix library which converts the network byte order field once parsed to the host's endianess
  - But if I were to address this without `ntohl`, I would use bit fiddling to convert a big-endian field to a little-endian field. Below is an example of how I would modify the code for the X-axis Gyro Rate for instance and the same is true for the Packet count and other gyro rates:
      ```
      memcpy(&packet.X_GyroRate, &dataBuffer[*position + 8], 4);   // copy X-axis Gyro Rate from packet into struct
    uint32_t X_GyroBytes;
      memcpy(&X_GyroBytes, &packet.X_GyroRate, sizeof(uint32_t));  // copy the exact bytes into uint32_t format
      X_GyroLittleEndian = ((X_GyroRateBytes & 0xFF) << 24) |     // Move 3 bytes to left
                           ((X_GyroRateBytes & 0xFF00) << 8) |    // Move 1 byte to left
                           ((X_GyroRateBytes & 0xFF0000) >> 8) |  // Move 1 byte to right
                           ((X_GyroRateBytes & 0xFF000000) >> 24) // Move 3 bytes to right
      packet.X_GyroRate = static_cast<float>(X_GyroLittleEndian);
    ```
- This way, I am essentially swapping bytes where the 2 outer bytes (left-most and right-most bytes) and the 2 inner bytes are also swapped, effectively converting to little-endian format so that the host machine can interpret.

### Task 3
Description: Write a C++ program to execute the parsing function every 80ms and broadcast the packets once parsed. The program must be able to operate in a multi-thread environment with multiple processes running.\
Solution:
- My interpretation of this is that every 80ms, packets are both read and broadcasted.
- Thus I had a while loop in the `main` method of *Imu_Parser/src/imu_parser.cpp* which runs forever and inside of it, calls `processIMU_Frames`. After this, it calles the function `std::this_thread::sleep_for(std::chrono::milliseconds(80))`. This function essentially creates a "dead" thread which sleeps/doesn't do anything for 80ms, allowing other processes to claim the core that the process was originally executing on. This way, other processes can take the time to run before the *imu_parser
 process processes a frame and broadcasts the parsed fields.
- The broadcast fields from the parsed packets, I used sockets. I took some of the code from my robotics project; in short, it sets a broadcast address to '127.255.255.255' which means all devices/processes with the private IP address starting with *127* will receive the packet. The socket also sends to the broadcast porty *8888* meaning processes on the localhost network can receive the data by listening to port *8888*.

### Take 4
Description: Write a Python program to drive the IMU parser by sending some dummy packets, listening to the broadcasted message, and verifying whether the IMU Parser parsed the sent packet correctly.\
Solution:
- I wrote the Python script *LoopTester.py* in the IMU_Driver directory which is meant to test a full loop (sending packets and receiving packets correctly)
- The LoopTester script uses an external C++ function called `createFakePacket` in *IMU_Parser/test/fake_data_extern.cpp*. The reason why I decided to do it this way is because I already wrote a mock IMU in *mock_imu.cpp* and that C++ file calls *fake_data.cpp* to get fake packets to send. So I just used *fake_data.cpp* as an extern and allowed the Python script to call it using *ctypes* package. I love this feature of Python which was extremely useful in my robotics teams when building in Python and C++ and I decided to use it here.
- The LoopTester calls `createFakePacket`, converts it to a Python list before converting the list to a byte string, and used the *pyserial* library to send the byte string across a given port.
- The LoopTester then calls the `recvfrom` function in the *socket* package (as a server socket bound to the broadcast porty 8888 is aready created) to receive the resulting fields parsed from the *imu_parser.cpp* code.
- It uses `assert` statements to check whether the received data is equal to the expected data created in `creatFakePacket`.

## Usage
### Automated build and test script:
From the parent directory, run `./run_test.sh`
- Runs cmake build sequence to build C++ code
- Creates virtual environment for Python tester, installs required libraries
- Runs `g++ -shared -o IMU_Driver/fake_data.so -fPIC IMU_Parser/test/fake_data_extern.cpp` to build external C++ createFakePacket function for LoopTester.py to use.
- Runs `./parser_test` to execute a 5 unit tests to test the C++ parsing function
- Runs `socat -d -d pty,raw,echo=0 pty,raw,echo=0 2>&1` to open a virtual serial port and 2 PTY devices for the C++ parser and Python tester to open
- Runs C++ imu_parser and Python LoopTester.py which tests the full loop communication.

### Manual usage:
1) Make build directory and `cd` into it.
2) Build C++ code
   - `cmake ..`
   - `make`
3) From parent directory, run `g++ -shared -o IMU_Driver/fake_data.so -fPIC IMU_Parser/test/fake_data_extern.cpp` to build extern createFakePacket function as .so file for LoopTester.py
3) In one terminal, run `socat -d -d pty,raw,echo=0 pty,raw,echo=0`.
   - Will output 2 PTY file descriptors with the format '/dev/pts/X'
4) In another terminal inside the `build` directory, run `./imu_parser <port to open>`. The default port is '/dev/tty1' if no argument given, but for testing virtual hardware like in this scenario, it is recommended to provide and argument. Provide one of the PTY file descriptors as an argument like this: `./imu_parser /dev/pts/1`
5) In another terminal, inside the `IMU_Driver` directory, run `python3 LoopTester.py --port <port to open>`. The default port is '/dev/ttty1', but as mentioned above, it is recommended to use the other PTY file descriptor like this: `python3 LoopTester.py --port /dev/pts/2`




Call 'socat -d -d pty,raw,echo=0 pty,raw,echo=0' to set up a virtual UART port
- The second file is the writing port
- The first file is the reading port

Need to also build .so file for fake_data_extern.cpp:
- g++ -shared -o IMU_Driver/fake_data.so -fPIC IMU_Parser/test/fake_data_extern.cpp

To test the receiver, parser, and broadcaster:
- Inside the build/ directory, run ./test_imu {second file from from socat command}
- In another terminal, run ./TrueAnomalyAssessment {first file from socat command}
- In another terminal, run 'python3 Receiver.py' from within IMU_Driver directory

You should see the packet that was generated by 'test_imu', the exact data being received and parsed in 'TrueAnomalyAssessment', and the exact data received via socket in "Receiver.py"

Driver.py uses an extern Cpp file to generate fake data and prints it --> future iteration is to generate data, send it to UART port, and receive on socket to check if it is exactly what it expects