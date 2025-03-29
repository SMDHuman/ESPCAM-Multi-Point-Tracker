import serial
import time, os, sys, struct
from enum_parser import enum_parser
from pyslip import SLIP

checksum = 0
def slip_send_byte(serial: serial.Serial, data: int):
    global checksum
    checksum += data + 1
    slip_send(serial, data)

def slip_send_bytes(serial: serial.Serial, data: bytes|list[int]):
    global checksum
    for byte in data:
        checksum += byte + 1
        slip_send(serial, byte)

def slip_send(serial: serial.Serial, byte: int):
    if(byte == SLIP.END):
        serial.write(bytes([SLIP.ESC, SLIP.ESC_END]))
    elif(byte == SLIP.ESC):
        serial.write(bytes([SLIP.ESC, SLIP.ESC_ESC]))
    else:
        serial.write(bytes([byte]))

def slip_end(serial: serial.Serial):
    global checksum
    checksum %= 2**32
    slip_send(serial, checksum & 0xFF)
    slip_send(serial, (checksum >> 8) & 0xFF)
    slip_send(serial, (checksum >> 16) & 0xFF)
    slip_send(serial, (checksum >> 24) & 0xFF)
    checksum = 0
    serial.write(bytes([SLIP.END]))


last_fcount = [0]*255
last_fcount_time = [0]*255
def handle_response(packet: bytes):
    global last_fcount, last_fcount_time
    tag = packet[0]
    data = packet[1:]
    if tag == RSP["RSP_PONG"]:
        print("Ping response received")
    elif tag == RSP["RSP_FCOUNT"]:
        frm = data[0]
        fcount = struct.unpack("Q", data[1:])[0]
        #print(f"Fcount from '{data[0]}': ", fcount)
        print(f"FPS from {data[0]}: {(fcount - last_fcount[frm])/(time.time() - last_fcount_time[frm]):.2f}")
        last_fcount[frm] = fcount
        last_fcount_time[frm] = time.time()
    elif tag == RSP["RSP_PEERCOUNT"]:
        peer_count = data[0]
        print(f"Peer count: {peer_count}")
    elif tag == RSP["RSP_PEERLIST"]:
        peer_count = len(data) // 7
        peer_list = []
        for i in range(peer_count):
            peer_info = struct.unpack("B6B", data[i*7:(i+1)*7])
            peer_list.append(peer_info)
        print(f"Peer list: {peer_list}")
    else:
        print("Unknown response received: ", hex(tag))
        print("Data: ", data.hex())

# import the enums from the header file
path = os.path.join(sys.path[0], "..", "include", "command_handler.h")
with open(path) as f:
    enums = enum_parser(f.read())
CMD = enums["CMD_TYPE_E"]
RSP = enums["RSP_TYPE_E"]

slip = SLIP()

esp = serial.Serial("COM3", 115200, timeout=1)
esp.flush()

last_ping = 0
esp_read_start = 0
while(1):
    esp_read_start = time.time()
    while(esp.in_waiting > 0 and time.time() - esp_read_start < 1):
        byte = esp.read(1)[0]
        #print("Byte: ", byte)
        slip.push(byte)
        while(slip.in_wait() > 0):
            packate = slip.get()
            handle_response(packate)
            slip.reset_buffer()
        time.sleep(0.001)

    if(time.time() - last_ping > 1):
        # Send a ping command to the ESP32
        #slip_send_bytes(esp, [CMD["CMD_RQ_POINTS"], 0])
        #slip_end(esp)
        #slip_send_bytes(esp, [CMD["CMD_RQ_PEERLIST"]])
        #slip_end(esp)
        if(int(time.time())%2 == 0):
            slip_send_bytes(esp, [CMD["CMD_RQ_FCOUNT"], 1])
            slip_end(esp)
        else:
            slip_send_bytes(esp, [CMD["CMD_RQ_FCOUNT"], 0])
            slip_end(esp)
        last_ping = time.time()
    
    time.sleep(0.001)