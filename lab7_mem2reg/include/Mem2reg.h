#ifndef __MEM2REG_H__
#define __MEM2REG_H__

#include "Unit.h"

struct RenamePassData
{
    RenamePassData(BasicBlock *B, BasicBlock *P, std::vector<Operand*> V, std::vector<BasicBlock*> L)
      : BB(B), Pred(P), Values(std::move(V)), Locations(std::move(L)) {}
    BasicBlock *BB;
    BasicBlock *Pred;
    std::vector<Operand*> Values;
    std::vector<BasicBlock*> Locations;
};
class Mem2reg
{
private:
    Unit* unit;
    std::vector<AllocaInstruction*> allocaVec;//储存alloca语句
    std::map<Operand*, std::stack<Operand*>> reachingDef;//栈顶存储operand当前达到的变量名
    Operand* NewName(std::map<Operand*, int>& counter, Operand* def);
    void InsertPhiIns(Function* func);
    void renameFunc(Function* func);
    void renameB(BasicBlock* block);
    /*llvm设计*/
    std::map<PhiInstruction*, int> Phi2AllocaMap;
    void renameLLVM(BasicBlock* block, 
        BasicBlock* pred, 
        std::vector<RenamePassData>& workList,
        std::vector<BasicBlock*>& IncomingLoac,
        std::vector<Operand*>& IncomingOp
        );
    std::set<BasicBlock*> Visted;//记录pass过程中访问过的block
    /*end*/
public:
    Mem2reg(Unit* unit):unit(unit){};
    void pass();
};







#endif