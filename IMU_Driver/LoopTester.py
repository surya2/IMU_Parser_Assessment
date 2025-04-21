import ctypes
import os
import sys
import serial
import time
import struct
import socket

def main():
    PACKET_SIZE = 20
    packet = []

    BROADCAST_PORT = 8888
    PACKET_FORMAT = 'I 3f'

    fd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    fd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    fd.bind(('', BROADCAST_PORT))
    fd.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    UART_PORT = "/dev/tty1"
    TEST_MODE = 0  # 0 - Random Packet Generator; 1 - Test Case File
    n = len(sys.argv)
    for i in range(1, n):
        if sys.argv[i] == "--port" and i+1 <= n:
            UART_PORT = sys.argv[i+1]
            i+=1
        elif i+1 > n:
            raise Exception("Did not give an argument for --port flag")
        if sys.argv[i] == "--test-mode" and i+1 <= n:
            try:
                TEST_MODE = int(sys.argv[i+1])
            except ValueError:
                raise Exception("Provided incorrect type for TEST_MODE, must be integer where 0 is for Random Packet Generator-based test and 1 is for Test Case File-based test")
        elif i+1 > n:
            raise Exception("Did not give an argument for --test-mode flag")

    script_dir = os.path.dirname(os.path.abspath(__file__))
    fake_data_lib_path = os.path.join(script_dir, "fake_data.so")

    fake_data_lib = ctypes.CDLL(fake_data_lib_path)
    fake_data_lib.createFakePacket.argtypes = [ctypes.c_uint32]
    fake_data_lib.createFakePacket.restype = ctypes.POINTER(ctypes.c_uint8)
    fake_data_lib.freePacket.argtypes = [ctypes.POINTER(ctypes.c_uint8)]

    count = 0
    uart_serial = serial.Serial(UART_PORT, baudrate=921600)
    while True:
        if TEST_MODE == 0:
            data_ptr = fake_data_lib.createFakePacket(count)
            packet = [data_ptr[i] for i in range(PACKET_SIZE)]
            fake_data_lib.freePacket(data_ptr)

        packet_bytes = bytes(packet)
        packetContExp = struct.unpack('>I', packet_bytes[4:8])[0]
        X_GyroRateExp = struct.unpack('>f', packet_bytes[8:12])[0]
        Y_GyroRateExp = struct.unpack('>f', packet_bytes[12:16])[0]
        Z_GyroRateExp = struct.unpack('>f', packet_bytes[16:20])[0]
        print(f"Sending to IMU Parser - Packet Count: {packetContExp}, X Gyro: {X_GyroRateExp}, Y Gyro: {Y_GyroRateExp}, Z Gyro: {Z_GyroRateExp}")
        uart_serial.write(packet_bytes)
        uart_serial.flush()

        data, addr = fd.recvfrom(1024)
        try:
            packet = struct.unpack(PACKET_FORMAT, data)
            packetCount = packet[0]
            X_GyroRate = packet[1]
            Y_GyroRate = packet[2]
            Z_GyroRate = packet[3]
            print(f"Received from IMU Parser - Packet Count: {packetCount}, X Gyro: {X_GyroRate}, Y Gyro: {Y_GyroRate}, Z Gyro: {Z_GyroRate}")
            assert packetCount == packetContExp
            assert X_GyroRate == X_GyroRateExp
            assert Y_GyroRate == Y_GyroRateExp
            assert Z_GyroRate == Z_GyroRateExp
        except struct.error as e:
            raise Exception(f"Couldnt unpack data: {e}")

        count+=1
        time.sleep(1)

if __name__ == "__main__":
    main()




