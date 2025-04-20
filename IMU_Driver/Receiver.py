import socket
import struct

BROADCAST_PORT = 8888
PACKET_FORMAT = 'I 3f'

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', BROADCAST_PORT))
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

while True:
    data, addr = sock.recvfrom(1024)

    print(f"Received data from {addr}: {data}")

    try:
        packet = struct.unpack(PACKET_FORMAT, data)
        packetCount = packet[0]
        X_GyroRate = packet[1]
        Y_GyroRate = packet[2]
        Z_GyroRate = packet[3]

        print(f"Packet Count: {packetCount}")
        print(f"X Gyro Rate: {X_GyroRate}")
        print(f"Y Gyro Rate: {Y_GyroRate}")
        print(f"Z Gyro Rate: {Z_GyroRate}")

    except struct.error as e:
        print(f"Failed to unpack data: {e}")
