#include <kernel.h>
#include <klib.h>
int main() {
  _ioe_init();
  _cte_init(os->trap);
  // call sequential init code
  os->init();
  printf("3\n");
  _mpe_init(os->run); // all cores call os->run()
  return 1;
}

