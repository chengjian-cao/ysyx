// #include <stdio.h>
// #include <stdlib.h>
// #include <assert.h>
// #include <nvboard.h>
// #include "Vtop.h"
// #include "verilated.h"
// #include "verilated_vcd_c.h"

// int main(int argc, char** argv) {
//   VerilatedContext* contextp = new VerilatedContext;
//   contextp->commandArgs(argc, argv);
//   Vtop* top = new Vtop{contextp};
//   contextp->traceEverOn(true);
//   VerilatedVcdC* tfp = new VerilatedVcdC;
//   top->trace(tfp, 0); 
//   tfp->open("dump.vcd");
//   while (true) {
//     int a = rand() & 1;
//     int b = rand() & 1;
//     top->a = a;
//     top->b = b;
//     top->eval();
//     printf("a = %d, b = %d, f = %d\n", a, b, top->f);
//     assert(top->f == (a ^ b));
//     contextp->timeInc(1);
//     tfp->dump(contextp->time());
//   }
//   tfp->close();
//   delete top;
//   delete contextp;
//   return 0;
// }

#include <nvboard.h>
#include <Vtop.h>

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME* top);

// static void single_cycle() {
//   dut.clk = 0; dut.eval();
//   dut.clk = 1; dut.eval();
// }

// static void reset(int n) {
//   dut.rst = 1;
//   while (n -- > 0) single_cycle();
//   dut.rst = 0;
// }

// int main() {
//   nvboard_bind_all_pins(&dut);
//   nvboard_init();

//   // reset(10);

//   while(1) {
//     nvboard_update();
//     dut.eval();
//     // single_cycle();
//   }
// }
void single_cycle() {
  dut.clk = 0; dut.eval();
  dut.clk = 1; dut.eval();
}

void reset(int n) {
  dut.rst = 1;
  while (n -- > 0) single_cycle();
  dut.rst = 0;
}


int main() {
  nvboard_bind_all_pins(&dut);
  nvboard_init();

  reset(10);

  while(1) {
    nvboard_update();    
    single_cycle();
  }
}