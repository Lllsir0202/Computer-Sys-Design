#include "cpu/exec.h"

make_EHelper(mov);
// Add: in pa2-1 used in exec
make_EHelper(call);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(sub);
make_EHelper(xor);
make_EHelper(ret);

//Add: in pa2-2
make_EHelper(lea);
make_EHelper(and);
make_EHelper(nop);
make_EHelper(add);
make_EHelper(adc);
make_EHelper(or);
make_EHelper(setcc);
make_EHelper(movzx);
make_EHelper(test);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);
