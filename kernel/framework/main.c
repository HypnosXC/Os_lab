#include <kernel.h>
#include <klib.h>
device_t *dev_lookup(const char *name);
int main() {
  _ioe_init();
  _cte_init(os->trap);
  // call sequential init code
  os->init();
  printf("3\n");
  _mpe_init(os->run); // all cores call os->run()
  return 1;
}
void echo_task(void *name) {
	device_t *tty= dev_lookup(name);
	while(1) {
		char line[128],text[128];
		sprintf(text,"(%s) $ ", name);
		tty->ops->write(tty,0,text,sizeof(text));
		int nread = tty->ops->read(tty,0,line,sizeof(line));
		line[nread-1]='\0';
		sprintf(text,"Echo: %s,\n",line);
		tty->ops->write(tty,0,text,strlen(text));
	}
}
