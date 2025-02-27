from serial import Serial
from PIL import Image
import pygame as pg
import pygame.camera as pgcam
import numpy as np
import time
import math
from random import randint 

# Constants
camera_size = (240, 240)
erode_size = 5
erode_ratio = 3/5
dilate_size = 10
# Initialize things
pg.init()
win = pg.display.set_mode(list(np.multiply(camera_size, (3, 2))))
clock = pg.time.Clock()

pgcam.init()
print("Cameras:", pgcam.list_cameras())
#cam = pgcam.Camera()
#cam.start()

cam_surf = pg.Surface(camera_size)
filter_surf = pg.Surface(camera_size)
erode_surf = pg.Surface(camera_size)
dilate_surf = pg.Surface(camera_size)
islands_surf = pg.Surface(camera_size)
tracker_surf = pg.Surface(camera_size)

tracking_pos = [0, 0]
tracking_rect = -1

is_colors = [pg.Color(randint(0, 255), randint(0, 255), randint(0, 255)) for i in range(255)]
mouse_on_tracker = [0, 0]
font = pg.font.SysFont("arial", 20, 1)

# Functions 
def fill(surface: pg.Surface, position: int, fill_color : any):
    fill_color = surface.map_rgb(fill_color)  # Convert the color to mapped integer value.
    surf_array = pg.surfarray.pixels2d(surface)  # Create an array from the surface.
    current_color = surf_array[position]  # Get the mapped integer color value.

    # 'frontier' is a list where we put the pixels that's we haven't checked. Imagine that we first check one pixel and 
    # then expand like rings on the water. 'frontier' are the pixels on the edge of the pool of pixels we have checked.
    #
    # During each loop we get the position of a pixel. If that pixel contains the same color as the ones we've checked
    # we paint it with our 'fill_color' and put all its neighbours into the 'frontier' list. If not, we check the next
    # one in our list, until it's empty.

    frontier = [position]
    while len(frontier) > 0:
        x, y = frontier.pop()
        try:  # Add a try-except block in case the position is outside the surface.
            if surf_array[x, y] != current_color:
                continue
        except IndexError:
            continue
        surf_array[x, y] = fill_color
        # Then we append the neighbours of the pixel in the current position to our 'frontier' list.
        frontier.append((x + 1, y))  # Right.
        frontier.append((x - 1, y))  # Left.
        frontier.append((x, y + 1))  # Down.
        frontier.append((x, y - 1))  # Up.

    pg.surfarray.blit_array(surface, surf_array)

while(True):
    # Events
    mouse_down = 0
    for event in pg.event.get():
        if(event.type == pg.QUIT):
            pg.quit()
            quit()
        if(event.type == pg.MOUSEBUTTONDOWN):
            mouse_on_tracker = np.subtract(event.pos, np.multiply(camera_size, (2, 1)))
            mouse_down = event.button
    #--------------------------------------------------------------------------
    # Camera resizing and cropping
    #img = cam.get_image()
    img = pg.image.load("room.jpg")
    crop_by = min(img.get_size())
    w = int(camera_size[0]*img.get_width()/crop_by)
    h = int(camera_size[1]*img.get_height()/crop_by)
    img = pg.transform.scale(img, (w, h))
    img = pg.transform.grayscale(img)

    # Camera surface Ready
    cam_surf.blit(img, ((camera_size[0]-w)/2, (camera_size[1]-h)/2))

    #--------------------------------------------------------------------------
    # Apply filter
    for y in range(camera_size[1]):
        for x in range(camera_size[0]):
            r, g, b, a = cam_surf.get_at((x, y))
            gc = (r+g+b)/3
            f = 220 < gc < 255
            color = ["black", "white"][f]
            filter_surf.set_at((x, y), color)

    #--------------------------------------------------------------------------
    # Erode and dilate
    erode_surf.fill(0)
    # Erode
    for y in range(camera_size[1]-erode_size):
        for x in range(camera_size[0]-erode_size):
            if(filter_surf.get_at((x+(erode_size//2), y+(erode_size//2)))[0] > 128):
                sum_area = 0
                for ay in range(erode_size):
                    for ax in range(erode_size):
                        gc = filter_surf.get_at((x+ax, y+ay))[0]
                        sum_area += gc
                if(sum_area < ((erode_size**2)*erode_ratio)*255):
                    erode_surf.set_at((x+(erode_size//2), y+(erode_size//2)), "black")
                else:
                    erode_surf.set_at((x+(erode_size//2), y+(erode_size//2)), "white")
    # Dilate
    dilate_surf.fill(0)
    for y in range(camera_size[1]-dilate_size):
        for x in range(camera_size[0]-dilate_size):
            color = erode_surf.get_at((x+(dilate_size//2), y+(dilate_size//2)))
            if(color[0] >= 128):
                for ay in range(dilate_size):
                    for ax in range(dilate_size):
                        dilate_surf.set_at((x+ax, y+ay), color)

    #--------------------------------------------------------------------------
    # Fload and colorize islands
    islands_surf.blit(dilate_surf, (0, 0))
    color_index = 0
    for y in range(camera_size[1]):
        for x in range(camera_size[0]):
            color = islands_surf.get_at((x, y))
            if(color == pg.Color("white")):
                fill(islands_surf, (x, y), is_colors[color_index])
                color_index += 1

    #--------------------------------------------------------------------------
    # Record islands size and position
    islands_rect = {}
    for y in range(camera_size[1]):
        for x in range(camera_size[0]):
            color = islands_surf.get_at((x, y))
            if(color != pg.Color("black")):
                color_index = is_colors.index(color)
                if(color_index not in islands_rect):
                    islands_rect[color_index] = [x, y, x, y]
                else:
                    if(x < islands_rect[color_index][0]):
                        islands_rect[color_index][0] = x
                    if(y < islands_rect[color_index][1]):
                        islands_rect[color_index][1] = y
                    if(x > islands_rect[color_index][2]):
                        islands_rect[color_index][2] = x
                    if(y > islands_rect[color_index][3]):
                        islands_rect[color_index][3] = y
                

    #--------------------------------------------------------------------------
    # Tracker
    tracker_surf.blit(islands_surf, (0, 0))
    # Draw islands rect
    for i in islands_rect:
        x1, y1, x2, y2 = islands_rect[i]
        pg.draw.rect(tracker_surf, "red", (x1, y1, x2-x1, y2-y1), 1)
    #Select rect to track
    x, y = mouse_on_tracker
    if(mouse_down == 1 and 0 <= x < camera_size[0] and 0 <= y < camera_size[1]):
        tracking_rect = -1
        for i in islands_rect:
            x1, y1, x2, y2 = islands_rect[i]
            if(x1 < x < x2 and y1 < y < y2):
                tracking_rect = i
        if(tracking_rect != -1):
            x1, y1, x2, y2 = islands_rect[tracking_rect]
            tracking_pos = [int((x1+x2)/2), int((y1+y2)/2)]
    # Find nearest island rect
    if(tracking_rect != -1):
        nearest = camera_size[0] + camera_size[1]
        tracking_rect = -1
        for i in islands_rect:
            x1, y1, x2, y2 = islands_rect[i]
            cx, cy = int((x1+x2)/2), int((y1+y2)/2)
            tx, ty = tracking_pos
            dist = abs(cx-tx) + abs(cy-ty)
            if(dist < nearest):
                nearest = dist
                tracking_rect = i
    # Get position of tracked rect
    if(tracking_rect != -1):
        x1, y1, x2, y2 = islands_rect[tracking_rect]
        tracking_pos = [int((x1+x2)/2), int((y1+y2)/2)]
    # Draw highlight tracked islands rect
    if(tracking_rect != -1):
        x1, y1, x2, y2 = islands_rect[tracking_rect]
        pg.draw.rect(tracker_surf, "yellow", (x1, y1, x2-x1, y2-y1), 3)
        pg.draw.line(tracker_surf, "orange", (tracking_pos[0], 0), (tracking_pos[0], camera_size[1]), 1)
        pg.draw.line(tracker_surf, "orange", (0, tracking_pos[1]), (camera_size[0], tracking_pos[1]), 1)

    #...
    win.blit(cam_surf, list(np.multiply(camera_size, (0, 0))))
    win.blit(filter_surf, list(np.multiply(camera_size, (1, 0))))
    win.blit(erode_surf, list(np.multiply(camera_size, (2, 0))))
    win.blit(dilate_surf, list(np.multiply(camera_size, (0, 1))))
    win.blit(islands_surf, list(np.multiply(camera_size, (1, 1))))
    win.blit(tracker_surf, list(np.multiply(camera_size, (2, 1))))
    for i in range(6):
        text_surf = font.render(f'{i+1}/6', 1, "red")
        x, y = i%3, i//3
        win.blit(text_surf, list(np.multiply(camera_size, (x, y))))
    print(islands_rect)
    print(tracking_rect, tracking_pos)
    print(f"FPS: {clock.get_fps()}")
    #...
    pg.display.update()
    clock.tick(30)
