//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include "tracker.h"
#include "camera_handler.h"
#include "serial_com.h"

//#define DEBUG
//-----------------------------------------------------------------------------
//static void switch_buffers();
static void filter_buffer();
static void erode_buffer();
static void dilate_buffer();
static void flood_fill(size_t s_i, uint8_t value);
static void flood_buffer();
static void locate_rect_buffer();

//-----------------------------------------------------------------------------
//...
static uint8_t buffer_A[TRACKER_BUF_LEN];
uint8_t request_frame = 0;
uint8_t* tracker_buffer_A = buffer_A;
uint64_t tracker_frame_count = 0;
//...
uint8_t tracker_points_len = 0;
point_rect_t tracker_points_rect[254];
tracker_status_e tracker_status = WAIT;

static uint32_t filter_min;
static uint32_t erode;
static uint32_t erode_mul;
static uint32_t erode_div;
static uint32_t dilate;
static uint32_t flip_x;
static uint32_t flip_y;

//-----------------------------------------------------------------------------
void tracker_init(){
}
//-----------------------------------------------------------------------------
void tracker_task(void * pvParameters){
}
//-----------------------------------------------------------------------------
void tracker_load_configs(){
  filter_min = CONFIGS.getInt("trk_filter_min");
  erode = CONFIGS.getInt("trk_erode");
  erode_mul = CONFIGS.getInt("trk_erode_mul");
  erode_div = CONFIGS.getInt("trk_erode_div");
  dilate = CONFIGS.getInt("trk_dilate");
  flip_x = CONFIGS.getInt("trk_flip_x");
  flip_y = CONFIGS.getInt("trk_flip_y");
}
//-----------------------------------------------------------------------------
void tracker_process(){
  filter_buffer();
  erode_buffer();
  dilate_buffer(); 
  flood_buffer();
  locate_rect_buffer();
      
  tracker_frame_count++;
}
//-----------------------------------------------------------------------------
// Pushes camera frame buffer to tracker buffer 'A'
void tracker_push_camera_buffer(camera_fb_t *fb){
  //switch_buffers();
  for(int y = 0; y < TRACKER_HEIGHT; y++){
    for(int x = 0; x < TRACKER_WIDTH; x++){
      int fb_x = (fb->width * x) / TRACKER_WIDTH;
      int fb_y = (fb->height * y) / TRACKER_HEIGHT;
      int f_x = flip_x ? (TRACKER_WIDTH-x-1) : x;
      int f_y = flip_y ? (TRACKER_HEIGHT-y-1) : y;
      tracker_buffer_A[(f_y*TRACKER_WIDTH)+f_x] = fb->buf[(fb_y*fb->width)+fb_x];
    }
  }
}
//-----------------------------------------------------------------------------
// Apply TRACKER_FILTER_MIN filter from 'B' to 'A' buffer
static void filter_buffer(){
  //uint8_t buffer_B[TRACKER_BUF_LEN];
  uint8_t *buffer_B = (uint8_t *)malloc(TRACKER_BUF_LEN);
  for(size_t i = 0; i < TRACKER_BUF_LEN; i++){
    if(tracker_buffer_A[i] < filter_min){
      buffer_B[i] = 0x00;
    }else{
      buffer_B[i] = 0xFF;
    }
  }
  //...
  memcpy(tracker_buffer_A, buffer_B, TRACKER_BUF_LEN);
  free(buffer_B);
}
//-----------------------------------------------------------------------------
static void erode_buffer(){
  //...
  uint8_t *buffer_B = (uint8_t *)malloc(TRACKER_BUF_LEN);
  memset(buffer_B, 0, TRACKER_BUF_LEN);
  //...
  static const uint16_t area = (erode*2+1)*(erode*2+1); 
  for(size_t y = 0; y < TRACKER_HEIGHT-(erode*2); y++){
    for(size_t x = 0; x < TRACKER_WIDTH-(erode*2); x++){
      //...
      uint16_t count = 0;
      for(size_t dy = 0; dy < erode*2+1; dy++){
        for(size_t dx = 0; dx < erode*2+1; dx++){
          if(tracker_buffer_A[((y+dy)*TRACKER_WIDTH)+(x+dx)] == 0xFF){
            count ++;
          }
      }}
      if(area*erode_mul <= count*erode_div){
        buffer_B[((y+erode)*TRACKER_WIDTH)+(x+erode)] = 0xFF;
      }else{
        buffer_B[((y+erode)*TRACKER_WIDTH)+(x+erode)] = 0x0;
      }
  }}
  //...
  memcpy(tracker_buffer_A, buffer_B, TRACKER_BUF_LEN);
  free(buffer_B);
}
//-----------------------------------------------------------------------------
static void dilate_buffer(){
  //...
  uint8_t *buffer_B = (uint8_t *)malloc(TRACKER_BUF_LEN);
  memset(buffer_B, 0, TRACKER_BUF_LEN);
  //...
  for(size_t y = 0; y < TRACKER_HEIGHT-(dilate*2); y++){
    for(size_t x = 0; x < TRACKER_WIDTH-(dilate*2); x++){
      //...
      if(tracker_buffer_A[((y+dilate)*TRACKER_WIDTH)+(x+dilate)] == 0xFF){
        for(size_t dy = 0; dy < dilate*2+1; dy++){
          for(size_t dx = 0; dx < dilate*2+1; dx++){
            buffer_B[((y+dy)*TRACKER_WIDTH)+(x+dx)] = 0xFF;
        }}
      }
  }}
  //...
  memcpy(tracker_buffer_A, buffer_B, TRACKER_BUF_LEN);
  free(buffer_B);
}

//-----------------------------------------------------------------------------
static void flood_buffer(){
  uint8_t island_count = 0;
  for(size_t i = 0; i < TRACKER_BUF_LEN; i++){
    if(tracker_buffer_A[i] == 255){
      flood_fill(i, island_count+1);
      island_count++;
      if(island_count == 255){
        return;
      }
    }
  }
  tracker_points_len = island_count;
}

//-----------------------------------------------------------------------------
static void flood_fill(size_t s_i, uint8_t value) 
{ 
  uint8_t base_value = tracker_buffer_A[s_i];
  size_t check_len = (TRACKER_BUF_LEN>>2); // TRACKER_BUF_LEN / 4
  size_t *check = (size_t*)malloc(check_len*sizeof(size_t));
  check[0] = s_i;
  uint16_t check_index = 1;
  while(check_index > 0){
    if(check_index > check_len){
      return;
    }
    size_t i = check[--check_index];
    // Base cases 
    if ((i < 0) | (i > TRACKER_BUF_LEN)) {
      continue; 
    }
    if(tracker_buffer_A[i] != base_value){
      continue;
    }
    if(tracker_buffer_A[i] == 0){
      continue;
    }
    //...
    tracker_buffer_A[i] = value; 
    //...
    if(tracker_buffer_A[i-1] != value){
      check[check_index++] = i-1;
    }
    if(tracker_buffer_A[i+1] != value){
      check[check_index++] = i+1;
    }
    if(tracker_buffer_A[i-TRACKER_WIDTH] != value){
      check[check_index++] = i-TRACKER_WIDTH;
    }
    if(tracker_buffer_A[i+TRACKER_WIDTH] != value){
      check[check_index++] = i+TRACKER_WIDTH;
    }
  }
  free(check);
} 
//-----------------------------------------------------------------------------
static void locate_rect_buffer(){
  for(uint8_t i = 0; i < 254; i++){
    tracker_points_rect[i].x1 = TRACKER_WIDTH;
    tracker_points_rect[i].y1 = TRACKER_HEIGHT;
    tracker_points_rect[i].x2 = 0;
    tracker_points_rect[i].y2 = 0;
  }
  for(size_t y = 0; y < TRACKER_HEIGHT; y++){
    for(size_t x = 0; x < TRACKER_WIDTH; x++){
      size_t i = (y*TRACKER_WIDTH)+x;
      if((tracker_buffer_A[i] > 0) & (tracker_buffer_A[i] < 255)){
        uint8_t ri = tracker_buffer_A[i]-1;
        point_rect_t *rect = &tracker_points_rect[ri];
        if(x < rect->x1){
          rect->x1 = x;
        }
        if(y < rect->y1){
          rect->y1 = y;
        }
        if(x > rect->x2){
          rect->x2 = x;
        }
        if(y > rect->y2){
          rect->y2 = y;
        }
      }
    }
  }
}
