#include "Mem2reg.h"

// #define LLVM 1


void Mem2reg::pass(){
    for (auto func = unit->begin(); func != unit->end(); func++)
    {
        // std::cout<<"x0"<<std::endl;
        (*func)->createDfsTree();
        // std::cout<<"x1"<<std::endl;
        (*func)->computeSemiDom();
        // std::cout<<"x2"<<std::endl;
        (*func)->semi_NCA();
        // std::cout<<"x3"<<std::endl;
        (*func)->computeDomFrontier();
        // std::cout<<"x4"<<std::endl;
        InsertPhiIns(*func);
        // std::cout<<"x5"<<std::endl;
        renameFunc((*func));
        // std::cout<<"x6"<<std::endl;
    }
    
}

void Mem2reg::InsertPhiIns(Function* func){
    //step1: 获取所有变量(alloca声明),且保证其未进行地址运算
    /**It is not used in a volatile instruction.
     * It is loaded or stored directly, i.e, its address is not taken.
    */
    func->getPhiBlocks().clear();
    std::vector<AllocaInstruction*>().swap(allocaVec);
    auto entry = func->getEntry();
    for(auto it = entry->begin();it!=entry->end();it = it->getNext()){
        if(!it->isAlloc())
            break;
        auto alloca = dynamic_cast<AllocaInstruction*>(it);
        if(!alloca->getSe()->getType()->isArray()&&!alloca->getPoint2Se())//确保alloca promptable
            allocaVec.push_back(alloca);
    }
    //step2: 剪枝优化,好像有助于idf计算(先不写了)
    auto allocaPtr = allocaVec.begin();
    int allocaNo = 0;
    while (allocaPtr != allocaVec.end()){
        auto cur_alloc = *allocaPtr;
        func->getEntry()->remove(cur_alloc);
        //删去当前alloca
        auto block = cur_alloc->getParent();
        auto alloca_op = cur_alloc->getOperands()[0];
        //优化1：删去无user的alloca
        if(alloca_op->usersNum() == 0){
            block->remove(cur_alloc);
            cur_alloc->clear();
            allocaVec.erase(allocaPtr);
            continue;
        }
        //线性扫描，获取使用该alloca_op的block_list和定值该alloca_op的block_list
        std::set<BasicBlock*> storeBlocks;//即定值blocks
        std::vector<BasicBlock*> loadBlocks;//即useblocks
        Instruction* OnlyStore = nullptr;
        while(alloca_op->use_begin()!= alloca_op->use_end()){
            auto User = (*alloca_op->use_begin());
            if(User->isLoad()){
                loadBlocks.push_back(User->getParent());
            }else if(User->isStore()){
                storeBlocks.insert(User->getParent());
                OnlyStore = User;
            }
            // User->getParent()->remove(User);
            alloca_op->removeUse(User);
        }
        //优化2：如果只有一个store，替换支配的users为该def的值
        //todo
        //优化3：如果读写均在一个块中，替换load的右值为store的右值
        //todo

        //placing PHInode
        //https://blog.csdn.net/dashuniuniu/article/details/103275708
        std::vector<BasicBlock*> workList;
        std::set<BasicBlock*> inWorkList; //记录已经插入的blocks
        std::set<BasicBlock*> phiBlocks;
        for(auto def : storeBlocks){
            workList.push_back(def);
            inWorkList.insert(def);
            while(!workList.empty()){
                auto cur_block = workList[0];
                workList.erase(workList.begin());
                for(auto domfBlock : cur_block->dominFrontierVec){
                    // if(func->getPhiBlocks().find(domfBlock) == func->getPhiBlocks().end()){
                    if(phiBlocks.find(domfBlock) == phiBlocks.end()){
                        Type* phi_type = dynamic_cast<PointerType*>(alloca_op->getType())->getValueType();
                        auto phi_op = new Operand(new TemporarySymbolEntry(phi_type, SymbolTable::getLabel()));
                        domfBlock->insertPhiNode(phi_op);
                        Phi2AllocaMap[(PhiInstruction*)(domfBlock->begin())] = allocaNo;
                        phiBlocks.insert(domfBlock);
                        if(inWorkList.find(domfBlock) == inWorkList.end()){
                            inWorkList.insert(domfBlock);
                            workList.push_back(domfBlock);
                        }
                    }
                }
            }
        }
        func->addPhiBlocks(phiBlocks);
        allocaPtr++;
        allocaNo++;
    }
}

    


void Mem2reg::renameFunc(Function* func){
    //给op加入一个标志位，初始化
    //也不用 判断其def为alloca即可
    std::vector<RenamePassData> workList;
    std::vector<Operand*> valueVec;
    // valueVec.resize(allocaVec.size());
    std::vector<BasicBlock*> locVec;
    locVec.resize(allocaVec.size());
    valueVec.resize(allocaVec.size());
    // for(int i = 0; i != valueVec.size(); i++){
    //     Type* opType = dynamic_cast<PointerType*>(allocaVec[i]->getDefOp()->getType())->getValueType();
    //     valueVec[i] = new Operand(new ConstantSymbolEntry(opType, 0));
    // }
    Visted.clear();
    workList.emplace_back(func->getEntry(), nullptr, std::move(valueVec), std::move(locVec));
    do{
        auto passData = std::move(workList.back());
        workList.pop_back();
        renameLLVM(
            passData.BB,
            passData.Pred,
            workList,
            passData.Locations,
            passData.Values
        );
    }while (!workList.empty());
}

// 变量重命名。整个过程是一个 DFS，为了减小内存开销所以用迭代的方式做
// 建立一个 map 记录每个 alloca 当前对应的值。所有 alloca 在函数入口的值都初始化为 UndefValue
// 用迭代 DFS 的方式遍历基本块，基本块信息存在结构体 RenamePassData 中，内部包含了一个数组 Values[]（即 IncomingVals[]）记录当前基本块末尾某个 alloca 对应的 Value（一次迭代只填入一个前驱流过来的值）
// While (worklist != NULL)
// 标记当前基本块已经处理过，防止重复处理
// 遍历当前块中第 4 步添加的 \varphiφ（程序里原来可能也有 \varphiφ，不能在这里处理）：
// 找到 \varphiφ 对应的 alloca L
// 为 \varphiφ 添加前驱块 Pred 到当前块的边（有几条边就要添加几次）：Phi.add(IncomingVals[L], Pred)
// 设置 IncomingVals[L] = Phi
// 如果当前基本块没有重复访问过，则对于基本块内的每条指令
// 如果当前指令是 load，找到对应的 alloca L，将用到 load 结果的地方都替换成 IncomingVals[L]
// 如果当前指令是 store，找到对应的 alloca L，更新版本 IncomingVals[L] = V 并删除这条 store
// 将没有访问过的后继基本块加入 worklist

void Mem2reg::renameLLVM(
        BasicBlock* block,
        BasicBlock* pred, 
        std::vector<RenamePassData>& workList,
        std::vector<BasicBlock*>& IncomingLoc,
        std::vector<Operand*>& IncomingOp){
NextIter:
    if(block->begin()->isPhi()){
        //todo：计算边数？为啥计算
        //maybe不用算
        auto ins = block->begin();
        PhiInstruction* PhiI = dynamic_cast<PhiInstruction*>(ins);
        do{
            int allocaNo = Phi2AllocaMap[PhiI];
            //update当前Phi的loc
            //还没懂这一步
            //不用管，dbg用（目前看来
            //update当前Phi的src
            if(IncomingOp[allocaNo]){
                PhiI->addIncoming(IncomingOp[allocaNo], pred);
            }
            //update 当前alloca对应变量的Incoming
            IncomingOp[allocaNo] = PhiI->getOperands()[0];
            //get下一个PhiNode
            ins = ins->getNext();
            PhiI = dynamic_cast<PhiInstruction*>(ins);
            if(!PhiI)
                break;
            
        }while(1);
        //终止条件待商榷????todo
        //终止条件使用到了之前需要计算的边数，但不知何用
        //好像是排除原生phi
    }
    
    if(!Visted.insert(block).second)//插入失败 返回end，second为null（<-原理好像是错的 但实践正确
        return;
    //遍历处理store\load
    for(auto ins = block->begin(); ins != block->end();/*no iter++*/){
        if(auto LI = dynamic_cast<LoadInstruction*>(ins)){
            // 好像想得有点问题
            // load指令的替换: 应该是把使用dst（指针）指向的那个量给修改值为load的src
            // dst = load i32, i32* src
            auto pointerOp = LI->getOperands()[1];
            auto AI = dynamic_cast<AllocaInstruction*>(pointerOp->getDef());
            if(!AI){
                ins = ins->getNext();
                continue;
            }
            auto AIpointer = std::find(allocaVec.begin(),allocaVec.end(),AI);
            if(AIpointer == allocaVec.end()){//可能是数组的Alloca
                ins = ins->getNext();
                continue;
            }
            Operand* nowOp = IncomingOp[(AIpointer - allocaVec.begin())];
            // if(nowOp == nullptr)//什么时候会这样 应该不会
            //     gandiansha();
            
            // If the load was marked as nonnull we don't want to lose
            // that information when we erase this Load. So we preserve
            // it with an assume.          

            //替换所有LI的def的use的值为nowOp
            auto defOp = LI->getDefOp();
            // for(auto use = defOp->getUse().begin(); use!=defOp->getUse().end();use++){
            //     if((*use)){
            //         (*use)->replaceUse(defOp, nowOp);
            //         std::cout<<"not nuill"<<std::endl;
            //     }
                    
            // }
            while (defOp->use_begin() != defOp->use_end()) {
                auto u = *(defOp->use_begin());
                u->replaceUse(defOp, nowOp);
                
            }
            auto ins_next = ins->getNext();
            block->remove(ins);
            ins = ins_next;
        }else if(auto SI = dynamic_cast<StoreInstruction*>(ins)){
            //store i32 src, i32* dst
            auto pointerOp = SI->getOperands()[0];
            auto AI = dynamic_cast<AllocaInstruction*>(pointerOp->getDef());
            if(!AI){
                ins = ins->getNext();
                continue;
            }
            auto AIpointer = std::find(allocaVec.begin(),allocaVec.end(),AI);
            if(AIpointer == allocaVec.end()){//可能是数组的Alloca
                ins = ins->getNext();
                continue;
            }
            int allocaNo = (AIpointer - allocaVec.begin());
            
            //更新incomingOp
            IncomingOp[allocaNo] = SI->getOperands()[1];
            // std::cout<<IncomingOp[allocaNo]<<std::endl;
            //没看懂Loc的更新，先这样
            //Loc为dbg的位置，其实我们不需要
            IncomingLoc[allocaNo] = block;
            auto ins_next = ins->getNext();
            block->remove(ins);
            ins = ins_next;
        }else{
            // Operand* def = ins->getDefOp();
            // if(def && std::find(IncomingOp.begin(),IncomingOp.end(),def)!=IncomingOp.end()){
            //     std::cout<<"x0"<<std::endl;
            //     int defNo = std::find(IncomingOp.begin(),IncomingOp.end(),def) - IncomingOp.begin();
            //     Operand* newOp = new Operand(new TemporarySymbolEntry(def->getType(), SymbolTable::getLabel()));
            //     IncomingOp[defNo]=newOp;
            //     ins->replaceDef(newOp);
            // }
            // if(!ins->isPhi()){
            //      std::cout<<"x1"<<std::endl;
            //     bool isFirst = true;
            //     for (auto opr : ins->getOperands()){
            //         if(isFirst && ins->getDefOp()){
            //             isFirst = false;
            //             continue;
            //         }
            //         if (std::find(IncomingOp.begin(),IncomingOp.end(),def)!=IncomingOp.end()){
            //             std::cout<<"x2"<<std::endl;
            //             int defNo = std::find(IncomingOp.begin(),IncomingOp.end(),def) - IncomingOp.begin();
            //             ins->replaceUse(opr, IncomingOp[defNo]);
            //         }
            //     }
            // }
            ins = ins->getNext();
        }
    }
    auto I = block->succ_begin(), E = block->succ_end();
    if(I == E)
        return;
    
    //dfs
    std::set<BasicBlock*> VisitedSuccs;
    VisitedSuccs.insert(*I);
    pred = block;
    block = *I;
    ++I;

    for (; I != E; ++I)
        if (VisitedSuccs.insert(*I).second)
            workList.emplace_back(*I, pred, IncomingOp, IncomingLoc);
    goto NextIter;
}



void Mem2reg::renameB(BasicBlock* block){
    std::map<Operand*, int> counter;
    for(auto it = block->begin(); it!= block->end(); it++){
        auto def = it->getDefOp();
        if(it->isStore()||it->isLoad()){
            block->remove(it);
            it--;
            continue;
        }
        if(def!=nullptr && reachingDef.find(def) != reachingDef.end()){
            counter[def]++;
            Operand* newOp = new Operand(new TemporarySymbolEntry(def->getType(), SymbolTable::getLabel()));
            reachingDef[def].push(newOp);
            it->replaceDef(newOp);
        }
        if(!it->isPhi()){
            
        }
    }
}

