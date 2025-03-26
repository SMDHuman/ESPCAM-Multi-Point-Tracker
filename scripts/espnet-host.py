from usb_now.scripts.usbnow import USBNow, MAC
from enum_parser import enum_parser
import os, sys
from time import sleep
from struct import pack, unpack
import threading
import pygame as pg

# Callback for received data
def recv_cb(addr: bytes, data: bytes):
    tag = int(data[0])
    # -------------------------------------------------------------------------
    if(tag == ESPNET["PACKET_REQ_JOIN"]):
        if(not MAC(addr) in peers):
            usbnow.add_peer(MAC(addr))
            _id = len(peers)+1
        else:
            _id = peers.index(MAC(addr))+1

        usbnow.send(MAC(addr), bytes([ESPNET["PACKET_RSP_JOIN"], _id]))

        if(not MAC(addr) in peers):
            peers.append(MAC(addr))
            usbnow.send(peers[_id-1], bytes([ESPNET["PACKET_REQ_CONFIG"]]))
        
    # -------------------------------------------------------------------------
    if(tag == ESPNET["PACKET_RSP_CONFIG"]):
        config = unpack("BB6B6B", data[1:])
        print(f"Received config from {MAC(addr)}: \n {config}")
    # -------------------------------------------------------------------------
    if(tag == ESPNET["PACKET_RSP_PONG"]):
        print(f"Received ping response from {MAC(addr)}")
    # -------------------------------------------------------------------------
    if(tag == ESPNET["PACKET_RSP_POINTS"]):
        point_rects.clear()
        #points = unpack("B", data[1:])[0]
        points = unpack(f"{len(data[1:])//4}I", data[1:])
        for i in range(0, len(points), 4):
            x1, y1 = points[i], points[i+1]
            x2, y2 = points[i+2], points[i+3]
            point_rects.append(pg.Rect(x1, y1, x2-x1, y2-y1))

        #print(f"Received {points} points from {MAC(addr)}")

# Parse the ESPNET enum from the header file
path = os.path.join(sys.path[0], "..", "include", "espnet_handler.h")
with open(path) as f:
    enums = enum_parser(f.read())
ESPNET = enums["ESPNET_PACKETS"]
print(enums)

# Initialize 
peers: list[MAC] = []

usbnow = USBNow("COM5")
usbnow.register_recv_cb(recv_cb)
usbnow.init()

pg.init()
win = pg.display.set_mode((240, 176))
pg.display.set_caption("ESPNet Host")

point_rects: list[pg.Rect]= []

# Main loop
running = True
while running:
    for event in pg.event.get():
        if event.type == pg.QUIT:
            running = False
    
    if(not running):break

    for rect in point_rects:
        pg.draw.rect(pg.display.get_surface(), (255, 0, 0), rect)

    for peer in peers:
        usbnow.send(peer, bytes([ESPNET["PACKET_REQ_LEDTOGGLE"]]))
        #usbnow.send(peer, bytes([ESPNET["PACKET_REQ_PING"]]))
        usbnow.send(peer, bytes([ESPNET["PACKET_REQ_POINTS"]]))
    sleep(0.03)

    pg.display.flip()
    win.fill(0)


print("Exiting...")
for peer in peers:
    usbnow.send(peer, bytes([ESPNET["PACKET_REQ_LEAVE"]]))
usbnow.deinit()