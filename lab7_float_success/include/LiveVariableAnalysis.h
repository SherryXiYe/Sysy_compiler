#ifndef __LIVE_VARIABLE_ANALYSIS_H__
#define __LIVE_VARIABLE_ANALYSIS_H__

#include <set>
#include <map>

class MachineFunction;
class MachineUnit;
class MachineOperand;
class MachineBlock;
class LiveVariableAnalysis
{
private:
    std::map<MachineOperand, std::set<MachineOperand *>> all_uses;
    //chatgpt:每个操作数被使用的位置，key值为任意操作数
    std::map<MachineBlock *, std::set<MachineOperand *>> def, use;
    //每个块中定值的操作数(def)和使用的操作数(use),用法：def[block]/use[block]
    void computeUsePos(MachineFunction *);
    void computeDefUse(MachineFunction *);
    void iterate(MachineFunction *);

public:
    void pass(MachineUnit *unit);
    void pass(MachineFunction *func);
    std::map<MachineOperand, std::set<MachineOperand *>> &getAllUses() { return all_uses; };
};

#endif