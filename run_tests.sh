BUILD_DIR="build"
PYTHON_VENV_DIR="IMU_Driver/venv"

echo "Building C++ code into executables"
if [ ! -d "$DIR" ]; then
  mkdir -p "$DIR"
fi

cd "$BUILD_DIR"
cmake ..
make

cd ..

echo "Setting up Python venv to install necessary Python packages"
if [ ! -d "$PYTHON_VENV_DIR" ]; then
  echo "Creating Python virtual env..."
  python3 -m venv "$PYTHON_VENV_DIR"
  source "$PYTHON_VENV_DIR/bin/activate"
  cd IMU_Driver
  pip install -r requirements.txt
  cd ..
else
  echo "Activating Python env"
  source "$PYTHON_VENV_DIR/bin/activate"
fi

g++ -shared -o IMU_Driver/fake_data.so -fPIC IMU_Parser/test/fake_data_extern.cpp  # Python tester script uses an external C++ function which must be built first

cd "$BUILD_DIR"

### Run unit test for parse function
echo "Running unit test for parser function..."
./parser_test
echo "Unit test for parser completed successfully!"

# Set up virtual serial port - writing and reading to /dev/tty1 by 2 processes is not allowed on my computer because /dev/tty1 is a console specific port
# 'socat -d -d pty,raw,echo=0 pty,raw,echo=0 2>&1' command opens a single serial port but connects two PTY devices or virtual file descriptors to simulate a real hardware link
#PORTS=$(socat -d -d pty,raw,echo=0 pty,raw,echo=0 2>&1 | grep -o "/dev/pts/[0-9]*")  <-- doesn't work because it block rest of processes
SOCAT_LOG="/tmp/socatOutput.txt"
socat -d -d pty,raw,echo=0 pty,raw,echo=0 2>&1 | tee "$SOCAT_LOG" &
SOCAT_PID=$!
sleep 1
PORTS=$(grep -o "/dev/pts/[0-9]*" /tmp/socatOutput.txt)
VPORT1=$(echo "$PORTS" | sed -n 1p)
VPORT2=$(echo "$PORTS" | sed -n 2p)

### Run system test - loop test with Python LoopTester script
echo "Running system test for full loop communication with parsing and broadcasting..."
./imu_parser "$VPORT1" &
PARSER_PID=$!

python3 ../IMU_Driver/LoopTester.py --port "$VPORT2" &
TESTER_PID=$!

sleep 20 # Let the Python driver test the parser for a good 20 seconds before they halt.

kill PARSER_PID TESTER_PID SOCAT_PID
wait $IMU_PID $PY_PID SOCAT_PID 2>/dev/null

echo "Tests complete!"



