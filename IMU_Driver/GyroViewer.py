import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import threading
import socket
import struct
import time

gyroData = [0.0, 0.0, 0.0]
def receive_gyro_data():
    global gyroData
    PACKET_SIZE = 20
    packet = []

    BROADCAST_PORT = 8888
    PACKET_FORMAT = 'I 3f'  # format specifier used later to unpack messages received from broadcast

    fd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # socket to receive broadcast from imu_parser
    fd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    fd.bind(('', BROADCAST_PORT))  # bind to port 8888 which is the port that the imu_parser is sending to
    fd.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    while True:
        data = sock.recv(16)  # assuming 4 floats = 16 bytes
        if not data:
            break
        packet = struct.unpack(PACKET_FORMAT, data)
        packetCount = packet[0]
        X_GyroRate = packet[1]
        Y_GyroRate = packet[2]
        Z_GyroRate = packet[3]

        gyroData = [X_GyroRate, Y_GyroRate, Z_GyroRate]

    sock.close()

receiver_thread = threading.Thread(target=receive_gyro_data, daemon=True)
receiver_thread.start()

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.set_xlim(-1, 1)
ax.set_ylim(-1, 1)
ax.set_zlim(-1, 1)
arrow = ax.quiver(0, 0, 0, 0, 0, 0, length=0.5, normalize=True)

# Update function
def update_plot():
    global arrow
    arrow.remove()

    X, Y, Z = gyroData
    arrow = ax.quiver(0, 0, 0, X, Y, Z, length=0.5, normalize=True)

    fig.canvas.draw_idle()
    fig.canvas.flush_events()
    fig.canvas.manager.window.after(50, update_plot)  # update every 50 ms

# Start updating
fig.canvas.manager.window.after(50, update_plot)
plt.show()