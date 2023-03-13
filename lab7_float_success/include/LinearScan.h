/**
 * linear scan register allocation
 */

#ifndef _LINEARSCAN_H__
#define _LINEARSCAN_H__
#include <set>
#include <map>
#include <vector>
#include <list>

class MachineUnit;
class MachineOperand;
class MachineFunction;


class LinearScan
{
private:
    struct Interval //每个vreg对应一个interval
    {
        int start;
        int end;
        bool spill; // 是否需要生成溢出代码 whether this vreg should be spilled to memory
        int disp;   // displacement in stack
        int rreg;   // the real register mapped from virtual register if the vreg is not spilled to memory
        bool isfloat;      //是否是浮点
        std::set<MachineOperand *> defs;
        std::set<MachineOperand *> uses;
    };
    MachineUnit *unit;
    MachineFunction *func;
    std::vector<int> regs;
    std::vector<int> floatRegs;
    std::map<MachineOperand *, std::set<MachineOperand *>> du_chains;
    std::vector<Interval*> intervals;       //computeLiveIntervals函数后：intervals表示还未分配寄存器的活跃区间, 其中所有的interval都按照开始位置进行递增排序
    std::vector<Interval*> active;      //active 表示当前正在占用物理寄存器的活跃区间集合, 其中所有的interval都按照结束位置进行递增排序
    static bool compareStart(Interval*a, Interval*b);
    static bool compareEnd(Interval*a, Interval*b);
    void expireOldIntervals(Interval *interval);
    void spillAtInterval(Interval *interval);
    void makeDuChains();
    void computeLiveIntervals();
    bool linearScanRegisterAllocation();
    void modifyCode();
    void genSpillCode();
public:
    LinearScan(MachineUnit *unit);
    void allocateRegisters();
};

#endif