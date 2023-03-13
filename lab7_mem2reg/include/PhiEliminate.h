#ifndef __PHI_ELIMINATE_H__
#define __PHI_ELIMINATE_H__
#include "Unit.h"

class PhiEliminate{
private:
    Unit* unit;
    void eliminate(Function* func);
public:
    PhiEliminate(Unit* unit):unit(unit){};
    void addCopyIns(BasicBlock*, Operand* dst, Operand* src);
    void pass();
};


#endif