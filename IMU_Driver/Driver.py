import ctypes
import os

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    fake_data_lib_path = os.path.join(script_dir, "fake_data_extern.so")
    
    fake_data_lib = ctypes.CDLL(fake_data_lib_path)
    fake_data_lib.createFakePacket.argtypes = []
    fake_data_lib.createFakePacket.restype = ctypes.POINTER(ctypes.c_uint8)
    fake_data_lib.freePacket.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
    
    data_ptr = fake_data_lib.createFakePacket()
    packet = [data_ptr[i] for i in range(20)]
    fake_data_lib.freePacket(data_ptr)
    print(packet)

if __name__ == "__main__":
    main()




