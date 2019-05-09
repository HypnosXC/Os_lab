#include <kernel.h>
#include <klib.h>

int main() {
  _ioe_init();
  printf("1\n");
  _cte_init(os->trap);
  printf("2\n");
  // call sequential init code
  os->init();
  printf("3\n");
  _mpe_init(os->run); // all cores call os->run()
  return 1;
}
