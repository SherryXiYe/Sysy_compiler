#include "LiveVariableAnalysis.h"
#include "MachineCode.h"
#include <algorithm>

void LiveVariableAnalysis::pass(MachineUnit *unit)
{
    for (auto &func : unit->getFuncs())
    {
        computeUsePos(func);
        computeDefUse(func);
        iterate(func);
    }
}

void LiveVariableAnalysis::pass(MachineFunction *func)
{
    computeUsePos(func);
    computeDefUse(func);
    iterate(func);
}

void LiveVariableAnalysis::computeDefUse(MachineFunction *func)
{
    for (auto &block : func->getBlocks())
    {
        for (auto inst = block->getInsts().begin(); inst != block->getInsts().end(); inst++)
        {
            auto user = (*inst)->getUse();
            std::set<MachineOperand *> temp(user.begin(), user.end());
            //temp:当前指令所使用的操作数i            
            //temp对def[block]做差集运算，将结果插入use[block]中
            //这是因为，use[B]是：值可能在B中先于任何对它们的定值被使用的变量的集合（所以要减去当前已有的def[B]）
            set_difference(temp.begin(), temp.end(),
                       def[block].begin(), def[block].end(), inserter(use[block], use[block].end()));
            //def[B]是：在B中的定值先于任何对它们的使用的变量的集合。
            auto defs = (*inst)->getDef();
            for (auto &d : defs)
                def[block].insert(all_uses[*d].begin(), all_uses[*d].end());
        }
    }
}

void LiveVariableAnalysis::iterate(MachineFunction *func)
{
    for (auto &block : func->getBlocks())
        block->getLiveIn().clear();
    bool change;
    change = true;
    while (change)
    {
        change = false;
        for (auto &block : func->getBlocks())
        {
            block->getLiveOut().clear();
            auto old = block->getLiveIn();
            // OUT[B]=(S是B的一个后继)IN[S]的并集
            for (auto &succ : block->getSuccs())
                block->getLiveOut().insert(succ->getLiveIn().begin(), succ->getLiveIn().end());
            // IN[B] = use[B]和(OUT[B]-def[B])的并集
            block->getLiveIn() = use[block];
            set_difference(block->getLiveOut().begin(), block->getLiveOut().end(),
                           def[block].begin(), def[block].end(), inserter(block->getLiveIn(), block->getLiveIn().end()));
            if (old != block->getLiveIn())
                change = true;
        }
    }
}

void LiveVariableAnalysis::computeUsePos(MachineFunction *func)
{
    for (auto &block : func->getBlocks())
    {
        for (auto &inst : block->getInsts())
        {
            auto uses = inst->getUse();
            for (auto &use : uses){
                    all_uses[*use].insert(use);
            }
                
        }
    }
}
