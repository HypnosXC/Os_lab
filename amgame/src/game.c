#include <game.h>
#define FPS 60
//uint32_t printf(char *tmf,...);
struct time_flow{
	int last_FPS;
	int now_FPS;
	int upda;
}TI;
typedef struct _vec {
 int x;
 int y; 
}vec;
typedef struct _body{
  int x;
  int y;
}body;
typedef struct _snake{
    body block;
    struct _snake* next;
}snake;
snake one_snake;
char pre[100];
uint32_t rand();
uint32_t srand(int seed);
uint32_t uptime();
void init_time();
void time_update();
void init_screen();
void draw_blo(int x,int y,int color);
void splash();
void read_key();
void init_time(){
	TI.last_FPS=uptime()*FPS/1000;
	TI.now_FPS=TI.last_FPS;
	TI.upda=0;
}
void time_update()	{
	TI.now_FPS=uptime()*FPS/1000;
	if(TI.now_FPS!=TI.last_FPS)
		TI.upda=1,TI.last_FPS=TI.now_FPS;
	else
		TI.upda=0;
}
int update_enable()	{
	return TI.upda;
}

char* itoa(uint32_t ax)	{
    ax/=1000;
	for(int i=0;i<100;i++)
		pre[i]=0;	
	int j=0,t=ax;
	while(t)j++,t/=10;
	j--;
	pre[j+1]='\n';
	for(int i=j;i>=0;i--){
		pre[i]=ax%10+'0';
		ax/=10;
	}
	return pre;
}
int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
  splash();
  init_time();
  while (1) {
    read_key();
	time_update();
	puts(itoa(TI.now_FPS));
	int i=0;
	if(update_enable()) {
		i++;
		i%=30;
		draw_blo(20,i,0);
	}
  }
  return 0;
}
uint32_t uptime() {
  _DEV_TIMER_UPTIME_t uptime;
  _io_read(_DEV_TIMER, _DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
  return uptime.lo;
}
void read_key() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  #define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
  }
}

int w, h;

void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: allocated on stack
  _DEV_VIDEO_FBCTL_t event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTL, &event, sizeof(event));
}

void splash() {
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_rect(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
}
void draw_blo(int x,int y,int color)	{
	draw_rect(x*SIDE,y*SIDE,SIDE,SIDE,color);
}
