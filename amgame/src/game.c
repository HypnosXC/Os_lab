#include <game.h>
#define FPS 60
//uint32_t printf(char *tmf,...);
void init_screen();
void splash();
void read_key();
typedef struct _vec {
 int x;
 int y; 
}vec;
typedef struct _body{
  int x;
  int y;
}body;
uint32_t uptime();
typedef struct _snake{
    body block;
    struct _snake* next;
}snake;
char pre[100];
char* itoa(uint32_t ax)	{
    for(int i=0;i<10;i++)
		pre[i]='s';	
	int j=0,t=ax;
	while(t)j++,t/=10;
	pre[j+1]='\n';
	for(int i=j;i>=0;i--){
		pre[j]=ax%10+'0';
		ax/=10;
	}
	return pre;
}
snake one_snake;/*
void move(snake a)	{
	a.x+=b
}*/
int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
  splash();
  while (1) {
    read_key();
	for(int i=1;i<=10000000;i++)	{
		for(int j=1;j<=1000;j++)
		if(i>=10000000&&j>=1000)
			puts(itoa(uptime()));
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
