#include <algorithm>
#include "LinearScan.h"
#include "MachineCode.h"
#include "LiveVariableAnalysis.h"

LinearScan::LinearScan(MachineUnit *unit)
{
    this->unit = unit;
    for (int i = 4; i < 11; i++)    //r4~r10，可保存局部变量
        regs.push_back(i);
}

void LinearScan::allocateRegisters()
{
    for (auto &f : unit->getFuncs())
    {
        // regs.clear();
        // for (int i = 4; i < 11; i++)
        //     regs.push_back(i);
        func = f;
        bool success;
        success = false;
        while (!success)        // repeat until all vregs can be mapped
        {
            computeLiveIntervals();
            success = linearScanRegisterAllocation();
            if (success){        // all vregs can be mapped to real regs
                modifyCode();
            }else{                // spill vregs that can't be mapped to real regs
                genSpillCode();
            }
        }
    }
}

void LinearScan::makeDuChains()
{
    LiveVariableAnalysis lva;
    lva.pass(func);
    du_chains.clear();
    int i = 0;
    std::map<MachineOperand, std::set<MachineOperand *>> liveVar;
    for (auto &bb : func->getBlocks())
    {
        liveVar.clear();
        for (auto &t : bb->getLiveOut())
            liveVar[*t].insert(t);
        int no;
        no = i = bb->getInsts().size() + i;
        for (auto inst = bb->getInsts().rbegin(); inst != bb->getInsts().rend(); inst++)
        {
            (*inst)->setNo(no--);
            for (auto &def : (*inst)->getDef())
            {
                if (def->isVReg())
                {
                    auto &uses = liveVar[*def];
                    du_chains[def].insert(uses.begin(), uses.end());
                    auto &kill = lva.getAllUses()[*def];
                    std::set<MachineOperand *> res;
                    set_difference(uses.begin(), uses.end(), kill.begin(), kill.end(), inserter(res, res.end()));
                    liveVar[*def] = res;
                }
            }
            for (auto &use : (*inst)->getUse())
            {
                if (use->isVReg())
                    liveVar[*use].insert(use);
            }
        }
    }
}

void LinearScan::computeLiveIntervals()
{
    makeDuChains();
    intervals.clear();
    for (auto &du_chain : du_chains)
    {
        int t = -1;
        for (auto &use : du_chain.second)
            t = std::max(t, use->getParent()->getNo());
        Interval *interval = new Interval({du_chain.first->getParent()->getNo(), t, false, 0, 0, {du_chain.first}, du_chain.second});
        intervals.push_back(interval);
    }
    for (auto& interval : intervals) {
        auto uses = interval->uses;
        auto begin = interval->start;
        auto end = interval->end;
        for (auto block : func->getBlocks()) {
            auto liveIn = block->getLiveIn();
            auto liveOut = block->getLiveOut();
            bool in = false;
            bool out = false;
            for (auto use : uses)
                if (liveIn.count(use)) {
                    in = true;
                    break;
                }
            for (auto use : uses)
                if (liveOut.count(use)) {
                    out = true;
                    break;
                }
            if (in && out) {
                begin = std::min(begin, (*(block->begin()))->getNo());
                end = std::max(end, (*(block->rbegin()))->getNo());
            } else if (!in && out) {
                for (auto i : block->getInsts())
                    if (i->getDef().size() > 0 &&
                        i->getDef()[0] == *(uses.begin())) {
                        begin = std::min(begin, i->getNo());
                        break;
                    }
                end = std::max(end, (*(block->rbegin()))->getNo());
            } else if (in && !out) {
                begin = std::min(begin, (*(block->begin()))->getNo());
                int temp = 0;
                for (auto use : uses)
                    if (use->getParent()->getBlock() == block)
                        temp = std::max(temp, use->getParent()->getNo());
                end = std::max(temp, end);
            }
        }
        interval->start = begin;
        interval->end = end;
    }
    bool change;
    change = true;
    while (change)
    {
        change = false;
        std::vector<Interval *> t(intervals.begin(), intervals.end());
        for (size_t i = 0; i < t.size(); i++)
            for (size_t j = i + 1; j < t.size(); j++)
            {
                Interval *w1 = t[i];
                Interval *w2 = t[j];
                if (**w1->defs.begin() == **w2->defs.begin())
                {
                    std::set<MachineOperand *> temp;
                    set_intersection(w1->uses.begin(), w1->uses.end(), w2->uses.begin(), w2->uses.end(), inserter(temp, temp.end()));
                    if (!temp.empty())
                    {
                        change = true;
                        w1->defs.insert(w2->defs.begin(), w2->defs.end());
                        w1->uses.insert(w2->uses.begin(), w2->uses.end());
                        // w1->start = std::min(w1->start, w2->start);
                        // w1->end = std::max(w1->end, w2->end);
                        auto w1Min = std::min(w1->start, w1->end);
                        auto w1Max = std::max(w1->start, w1->end);
                        auto w2Min = std::min(w2->start, w2->end);
                        auto w2Max = std::max(w2->start, w2->end);
                        w1->start = std::min(w1Min, w2Min);
                        w1->end = std::max(w1Max, w2Max);
                        auto it = std::find(intervals.begin(), intervals.end(), w2);
                        if (it != intervals.end())
                            intervals.erase(it);
                    }
                }
            }
    }
    sort(intervals.begin(), intervals.end(), compareStart);
}

bool LinearScan::linearScanRegisterAllocation()
{
    // Todo
//      active ←{}
//          foreach live interval i, in order of increasing start point
//              ExpireOldIntervals(i)
//              if length(active) = R then
//              SpillAtInterval(i)
//          else
//              register[i] ← a register removed from pool of free registers
//              add i to active, sorted by increasing end point
    regs.clear();
    for (int i = 4; i < 11; i++)
        regs.push_back(i);

    bool noSpill = true;
    active.clear();
    for (auto &interval : intervals){
        expireOldIntervals(interval);
        if(this->regs.empty()){//可用寄存器为空
            spillAtInterval(interval);
            noSpill=false;
        }else{
            interval->rreg = regs[0];
            regs.erase(regs.begin());
            active.push_back(interval);
            sort(active.begin(), active.end(), compareEnd);
        }
    }
    return noSpill;
}

void LinearScan::modifyCode()
{
    for (auto &interval : intervals)
    {
        func->addSavedRegs(interval->rreg);
        for (auto def : interval->defs)
            def->setReg(interval->rreg);
        for (auto use : interval->uses)
            use->setReg(interval->rreg);
    }
}

void LinearScan::genSpillCode()
{
    for(auto &interval:intervals)
    {
        if(!interval->spill)
            continue;
        // TODO
        /* HINT:
         * The vreg should be spilled to memory.
         * 1. insert ldr inst before the use of vreg
         * 2. insert str inst after the def of vreg
         */ 
        interval->disp = func->AllocSpace(4);
        auto offset = new MachineOperand(MachineOperand::IMM, -interval->disp);
        auto fp_reg = new MachineOperand(MachineOperand::REG, 11);
        for(auto use : interval->uses){
            auto use_inst = use->getParent();
            MachineInstruction* load_inst;
            if(interval->disp > 255){
                MachineOperand* op_offset = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                auto offset_load = new LoadMInstruction(use_inst->getBlock(), op_offset, offset);
                use_inst->getBlock()->insertBefore(use_inst, offset_load);
                load_inst = new LoadMInstruction(use_inst->getBlock(), use, fp_reg, op_offset);            
            }else{
                load_inst = new LoadMInstruction(use_inst->getBlock(), use, fp_reg, offset);
            }
            use_inst->getBlock()->insertBefore(use_inst, load_inst);
        }
        for(auto def : interval->defs){
            auto def_inst = def->getParent();
            MachineInstruction* store_inst;
            if(interval->disp > 255){
                MachineOperand* op_offset = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                auto offset_load = new LoadMInstruction(def_inst->getBlock(), op_offset, offset);
                def_inst->getBlock()->insertAfter(def_inst, offset_load);
                store_inst = new StoreMInstruction(def_inst->getBlock(), def, fp_reg, op_offset);   
                def_inst->getBlock()->insertAfter(offset_load, store_inst);         
            }else{
                store_inst = new StoreMInstruction(def_inst->getBlock(), def, fp_reg, offset);
                def_inst->getBlock()->insertAfter(def_inst, store_inst);
            }
        }
    }
}

void LinearScan::expireOldIntervals(Interval *interval)
{
    // Todo
    //foreach interval j in active, in order of increasing end point
        //if endpoint[j] ≥ startpoint[i] then
            //return
        //remove j from active
        //add register[j] to pool of free registers
    for(auto it = active.begin();it != active.end();){
        if((*it)->end >= interval->start)
            return; 
        regs.push_back((*it)->rreg);
        sort(regs.begin(),regs.end());  
        it = active.erase(it);  
    }
}
void LinearScan::spillAtInterval(Interval *interval)
{
    // Todo

    //spill ← last interval in active
    //if endpoint[spill] > endpoint[i] then
        //register[i] ← register[spill]
        //location[spill] ← new stack location
        //remove spill from active      这一步和实验指导书不一样
        //add i to active, sorted by increasing end point
    //else
        //location[i] ← new stack location 
    auto spill=active.end()-1;
    if((*spill)->end > interval->end){
        interval->rreg = (*spill)->rreg;
        (*spill)->spill = true;
        active.push_back(interval);
        sort(active.begin(),active.end(),compareEnd);
    }else{
        interval->spill=true;
    }
}

bool LinearScan::compareStart(Interval *a, Interval *b)
{
    return a->start < b->start;
}

bool LinearScan::compareEnd(Interval *a, Interval *b){
    return a->end < b->end;
} 