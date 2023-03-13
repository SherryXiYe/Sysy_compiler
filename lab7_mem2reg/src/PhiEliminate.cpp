#include "PhiEliminate.h"


void PhiEliminate::pass(){
    for(auto func = unit->begin(); func != unit->end();func++){
        eliminate(*func);
    }

}

void PhiEliminate::eliminate(Function* func){
    auto PhiBlocks = func->getPhiBlocks();
    for(auto PhiB: PhiBlocks){
        auto ins = PhiB->begin();
        while(ins != PhiB->end()){
            if(!ins->isPhi())
                break;
            auto PhiIns = dynamic_cast<PhiInstruction*>(ins);
            auto PhiPreds = PhiIns->getInBlocks();
            auto srcMap = PhiIns->getSrcMap();
            auto PhiDst = PhiIns->getOperands()[0];
            //如果srcmap仅有一个
            if(srcMap.size() == 1){
                Operand* Zero = new Operand(new ConstantSymbolEntry(PhiDst->getType(), 0));
                auto assignIns = new BinaryInstruction(BinaryInstruction::ADD, PhiDst, srcMap[PhiPreds[0]], Zero);
                PhiB->insertBefore(assignIns, ins);
                auto next_ins = ins->getNext();
                PhiB->remove(ins);
                ins = next_ins;
                continue;
            }
            //如果前驱仅有一个后继
            for(auto PredPtr=PhiPreds.begin();PredPtr!=PhiPreds.end();PredPtr++){
                if((*PredPtr)->getNumOfSucc() == 1){
                    addCopyIns((*PredPtr), PhiDst, srcMap[*PredPtr]);    
                }else{
                    BasicBlock* newBlock = new BasicBlock(func);
                    auto UBins= new UncondBrInstruction(PhiB, newBlock);
                    PhiB->removePred(*PredPtr);
                    PhiB->addPred(newBlock);
                    newBlock->addSucc(PhiB);
                    (*PredPtr)->removeSucc(PhiB);
                    //可能顺序有要求
                    (*PredPtr)->addSucc(newBlock);
                    newBlock->addPred((*PredPtr));
                    CondBrInstruction* condIns = (CondBrInstruction*)((*PredPtr)->rbegin());
                    if(condIns->getFalseBranch() == PhiB){
                        condIns->setFalseBranch(newBlock);
                    }else{
                        condIns->setTrueBranch(newBlock);                   
                    }
 
                    addCopyIns(newBlock, PhiDst, srcMap[*PredPtr]);               
                
                }
            }
            auto next_ins = ins->getNext();
            PhiB->remove(ins);
            ins = next_ins;
        }
    }
}


void PhiEliminate::addCopyIns(BasicBlock* block, Operand* dst, Operand* src){
    Operand* Zero = new Operand(new ConstantSymbolEntry(dst->getType(), 0));
    auto copyIns = new BinaryInstruction(BinaryInstruction::ADD, dst, src, Zero);
    block->insertCopyIns(copyIns);
}