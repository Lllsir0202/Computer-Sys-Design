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

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);
