from usb_now.scripts.usbnow import USBNow, MAC
from enum_parser import enum_parser
import os, sys, time
from time import sleep
from struct import pack, unpack
import threading
import pygame as pg
from multiprocessing import freeze_support

def new_peer(mac: MAC):
    global peers_points_surf, peers
    peers_fps.append(pg.time.Clock())
    peers.append(mac)
    peers_points_surf.append(pg.Surface((240, 176)))

# Callback for received data
def recv_cb(addr: bytes, data: bytes):
    if(len(data) == 0): return
    tag = int(data[0])
    # -------------------------------------------------------------------------
    if(tag == ESPNET["PACKET_REQ_JOIN"]):
        if(not MAC(addr) in peers):
            usbnow.add_peer(MAC(addr))
            _id = len(peers)+1
        else:
            _id = peers.index(MAC(addr))+1
        print(f"Received join request from {MAC(addr)}")
        usbnow.send(MAC(addr), bytes([ESPNET["PACKET_RSP_JOIN"], _id]))

        if(not MAC(addr) in peers):
            new_peer(MAC(addr))
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
        lenght = data[1]
        #print(lenght, data, len(data))
        if(lenght*4*4 != len(data)-2): return
        points = unpack(f"{lenght*4}I", data[2:])
        _id = peers.index(MAC(addr))
        peers_points_surf[_id].fill(0)
        for i in range(0, len(points), 4):
            x1, y1 = points[i], points[i+1]
            x2, y2 = points[i+2], points[i+3]
            rect = pg.Rect(x1, y1, x2-x1, y2-y1)
            pg.draw.rect(peers_points_surf[_id], (255, 0, 0), rect)
        peers_fps[_id].tick()
        #print(f"Received {points} points from {MAC(addr)}")

# Parse the ESPNET enum from the header file
path = os.path.join(sys.path[0], "..", "include", "espnet_handler.h")
with open(path) as f:
    enums = enum_parser(f.read())
ESPNET = enums["ESPNET_PACKETS"]
print(enums)

# Initialize 

peers: list[MAC] = []
peers_fps: list[pg.time.Clock] = []
usbnow = USBNow("COM11", timeout=0.25)
usbnow.receive_thread.start()
usbnow.register_recv_cb(recv_cb)
err = usbnow.init()
if(err):
    print("USBNow has error: ", err)
    sys.exit(1)
print("USBNow initialized")

pg.init()
win = pg.display.set_mode((240*2, 176*2))
pg.display.set_caption("ESPNet Host")



point_rects: list[pg.Rect]= []
peers_points_surf: list[pg.Surface] = []

# Main loop
running = True
while running:
    for event in pg.event.get():
        if event.type == pg.QUIT:
            running = False
    
    if(not running):break

    for i, peer in enumerate(peers):
        win.blit(peers_points_surf[i], (240*(i%2), 176*(i//2)))
        pg.draw.rect(win, (255, 255, 255), pg.Rect(240*(i%2), 176*(i//2), 240, 176), 1)

    for peer in peers:
        start_time = time.time()
        res = usbnow.send(peer, bytes([ESPNET["PACKET_REQ_LEDTOGGLE"]]))
        if(res):
            print(f"Error sending to {peer}: {res}")
        #print(f"Time taken to send: {time.time()-start_time}")
        #usbnow.send(peer, bytes([ESPNET["PACKET_REQ_PING"]]))
        usbnow.send(peer, bytes([ESPNET["PACKET_REQ_POINTS"]]))
        fps = peers_fps[peers.index(peer)].get_fps()
        font_surf = pg.font.SysFont("Arial", 12).render(f"{fps:.2f} FPS", True, (255, 255, 255))
        win.blit(font_surf, (240*(peers.index(peer)%2)+3, 176*(peers.index(peer)//2)+176-12-3))
    sleep(0.03)
    
    pg.display.flip()
    win.fill(0)


print("Exiting...")
for peer in peers:
    usbnow.send(peer, bytes([ESPNET["PACKET_REQ_LEAVE"]]))
usbnow.deinit()