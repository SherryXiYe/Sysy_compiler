#include "Ast.h"
#include "SymbolTable.h"
#include "Unit.h"
#include <sstream>
#include "Instruction.h"
#include "IRBuilder.h"
#include <string>
#include "Type.h"

extern FILE *yyout;
extern bool check_no_err;
int Node::counter = 0;
IRBuilder* Node::builder = nullptr;
std::stack<BasicBlock*> Node::while_cond_entry = std::stack<BasicBlock*>();
std::stack<BasicBlock*> Node::while_end_entry = std::stack<BasicBlock*>();
static Type* checkFuncType = nullptr;
FunctionType* currentFuncType = nullptr;
int notCount=0;


Node::Node()
{
    seq = counter++;
}

void Node::backPatch(std::vector<Instruction*> &list, BasicBlock*bb)
{
    for(auto &inst:list)
    {
        if(inst->isCond())
            dynamic_cast<CondBrInstruction*>(inst)->setTrueBranch(bb);
        else if(inst->isUncond())
            dynamic_cast<UncondBrInstruction*>(inst)->setBranch(bb);
    }
}

std::vector<Instruction*> Node::merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2)
{
    std::vector<Instruction*> res(list1);
    res.insert(res.end(), list2.begin(), list2.end());
    return res;
}

void Ast::genCode(Unit *unit)
{
    IRBuilder *builder = new IRBuilder(unit);
    Node::setIRBuilder(builder);
    root->genCode();
}

void FunctionDef::genCode()
{
    Unit *unit = builder->getUnit();
    Function *func = new Function(unit, se);
    BasicBlock *entry = func->getEntry();
    // set the insert point to the entry basicblock of this function.
    builder->setInsertBB(entry);
    std::vector<SymbolEntry*> params;
    if(funcPDef != nullptr){
        FuncFParam* defPtr = dynamic_cast<FuncFParam*>(funcPDef);
        while(defPtr != nullptr){
            SymbolEntry* parSe = defPtr->getSymPtr();//参数的sbey
            params.push_back(parSe);
            Operand* idAddr = new Operand(parSe);

            SymbolEntry* alloc_se = new TemporarySymbolEntry(parSe->getType(), SymbolTable::getLabel());
            ((TemporarySymbolEntry*)alloc_se)->setParamNo(((IdentifierSymbolEntry*)parSe)->getParamNo());
            Operand* alloc_dst_addr = new Operand(alloc_se);
            auto alloc_inst = new AllocaInstruction(alloc_dst_addr, alloc_se);
            entry->insertFront(alloc_inst);
            
            IdentifierSymbolEntry* identParSe = dynamic_cast<IdentifierSymbolEntry*>(parSe);

            TemporarySymbolEntry* addr_se_copy = dynamic_cast<TemporarySymbolEntry*>(alloc_se);
            SymbolEntry* addrPtr = new TemporarySymbolEntry(*addr_se_copy);
            addrPtr->setType(new PointerType(parSe->getType()));
            Operand* opr=new Operand(addrPtr);
            identParSe->setAddr(opr);
            opr->setDef(alloc_inst);

            Operand* store_dst_addr = new Operand(addrPtr);
            alloc_inst->setPoint2Se(addrPtr);
            store_dst_addr->setDef(alloc_inst);
            new StoreInstruction(store_dst_addr, idAddr, entry);
            defPtr = (FuncFParam*)defPtr->next;
        }
    }
    func->setParams(params);
    stmt->genCode();

    /**
     * Construct control flow graph. You need do set successors and predecessors for each basic block.
     * Todo
    */
    FunctionType* funcType = dynamic_cast<FunctionType*>(func->getSymPtr()->getType());
    for(auto it = func->begin();it != func->end(); it++){
        Instruction* i = (*it)->begin();
        Instruction* last_ins = (*it)->rbegin();
        while (i != last_ins) {
            if (i->isCond() || i->isUncond()) {
                (*it)->remove(i);
            }
            i = i->getNext();
        }
        if((*it)->empty()){
            if(it!=func->begin())
                continue;
            else{
                Type* midType = func->getSymPtr()->getType();
                FunctionType* funcType = dynamic_cast<FunctionType*>(midType);
                if(funcType->getRetType()->isVoid())
                    new RetInstruction(nullptr, *it);
                else if(funcType->getRetType()->isInt())
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)), *it);
                else if(funcType->getRetType()->isFloat())
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::floatType, 0)), *it);  
            }
        }

        if(last_ins->isCond()){
            CondBrInstruction* cond_br = dynamic_cast<CondBrInstruction*>(last_ins);
            BasicBlock* insertBb = last_ins->getParent();
            BasicBlock* trueBb = cond_br->getTrueBranch();
            BasicBlock* falseBb = cond_br->getFalseBranch();
            if(trueBb->empty()){
                if(funcType->getRetType()->isVoid())
                    new RetInstruction(nullptr, trueBb);
                else if(funcType->getRetType()->isInt())
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)), trueBb);
                else if(funcType->getRetType()->isFloat())
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::floatType, 0)), trueBb);                    
            }
            if(falseBb->empty()){
                if(funcType->getRetType()->isVoid())
                    new RetInstruction(nullptr, falseBb);
                else if(funcType->getRetType()->isInt())
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)), falseBb);
                else if(funcType->getRetType()->isFloat())
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::floatType, 0)), falseBb);    
            }
            trueBb->addPred(insertBb);
            falseBb->addPred(insertBb);
            insertBb->addSucc(trueBb);
            insertBb->addSucc(falseBb);
        }else if(last_ins->isUncond()){
            UncondBrInstruction* uncon_br = dynamic_cast<UncondBrInstruction*>(last_ins);
            BasicBlock* insertBb = last_ins->getParent();
            BasicBlock* destBb = uncon_br->getBranch();
            if(destBb->empty()){
                if(funcType->getRetType()->isVoid())
                    new RetInstruction(nullptr, destBb);
                else if(funcType->getRetType()->isInt())
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)), destBb);
                else if(funcType->getRetType()->isFloat())
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::floatType, 0)), destBb);                    
            }
            insertBb->addSucc(destBb);
            destBb->addPred(insertBb);
        }else if(!last_ins->isRet()){
            if(funcType->getRetType()->isVoid())
                new RetInstruction(nullptr, *it);
        }
    }
    for(auto it = func->begin();it != func->end(); it++){
        if((*it)->empty()){
            func->remove(*it);
        }
        Instruction* last_ins = (*it)->rbegin();
        if(last_ins->isRet()){
            if (!(*it)->succEmpty())
            {
                for (auto i = (*it)->succ_begin() + 1; i != (*it)->succ_end(); i++)
                    (*i)->removePred(*it);
            }
            (*it)->cleanSucc();
        }
        for(auto insPtr = (*it)->begin();insPtr!=last_ins;insPtr = insPtr->getNext()){
            if(insPtr->isRet()&&insPtr!=last_ins){
                (*it)->removeAfter(insPtr);
                if (!(*it)->succEmpty())
                {
                    for (auto i = (*it)->succ_begin() + 1; i != (*it)->succ_end(); i++)
                        (*i)->removePred(*it);
                }
                (*it)->cleanSucc();
                break;
            }
        }
    }
}

void BinaryExpr::genCode()
{
    BasicBlock *bb = builder->getInsertBB();
    Function *func = bb->getParent();
    if (op == AND)
    {
        BasicBlock *trueBB = new BasicBlock(func);  // if the result of lhs is true, jump to the trueBB.
        expr1->genCode();
        backPatch(expr1->trueList(), trueBB);
        builder->setInsertBB(trueBB);               // set the insert point to the trueBB so that intructions generated by expr2 will be inserted into it.
        expr2->genCode();
        true_list = expr2->trueList();
        false_list = merge(expr1->falseList(), expr2->falseList());
    }
    else if(op == OR)
    {
        // Todo
        BasicBlock *nextBB = new BasicBlock(func);
        expr1->genCode();
        backPatch(expr1->falseList(), nextBB);
        builder->setInsertBB(nextBB);
        expr2->genCode();
        true_list = merge(expr1->trueList(),expr2->trueList());
        false_list = expr2->falseList();

    }
    else if(op >= LESS && op <= GREATER)
    {
        // Todo
        expr1->genCode();
        expr2->genCode();
        Operand* op1 = expr1->getOperand();
        Operand* op2 = expr2->getOperand();
        int cmpType;
        Type* op1Type = trueType(op1->getType());
        Type* op2Type = trueType(op2->getType());
        Type* maxT = maxType(op1Type,op2Type);
        if(maxT->isFloat()){
            switch (op) {
                case LESS:
                    cmpType = FCmpInstruction::L;
                    break;
                case LESSEQUAL:
                    cmpType = FCmpInstruction::LE;
                    break;
                case GREATER:
                    cmpType = FCmpInstruction::G;
                    break;
                case GREATEREQUAL:
                    cmpType = FCmpInstruction::GE;
                    break;
                case EQUAL:
                    cmpType = FCmpInstruction::E;
                    break;
                case NOTEQUAL:
                    cmpType = FCmpInstruction::NE;
                    break;
            }
            new FCmpInstruction(cmpType, dst, op1, op2, bb);
        }else{
            switch (op) {
                case LESS:
                    cmpType = CmpInstruction::L;
                    break;
                case LESSEQUAL:
                    cmpType = CmpInstruction::LE;
                    break;
                case GREATER:
                    cmpType = CmpInstruction::G;
                    break;
                case GREATEREQUAL:
                    cmpType = CmpInstruction::GE;
                    break;
                case EQUAL:
                    cmpType = CmpInstruction::E;
                    break;
                case NOTEQUAL:
                    cmpType = CmpInstruction::NE;
                    break;
            }
            new CmpInstruction(cmpType, dst, op1, op2, bb);
        }

        BasicBlock *true_bb, *false_bb, *mid_bb;
        true_bb = new BasicBlock(func);
        false_bb = new BasicBlock(func);
        mid_bb = new BasicBlock(func);

        true_list.push_back(new CondBrInstruction(true_bb, mid_bb, dst, bb));
        false_list.push_back(new UncondBrInstruction(false_bb, mid_bb));        
    }
    else if(op >= ADD && op <= MOD) // add by zsr 2022年12月5日20:27:16
    {
        expr1->genCode();
        expr2->genCode();
        Operand *src1 = expr1->getOperand();
        Operand *src2 = expr2->getOperand();
        int opcode;
        switch (op)
        {
        case ADD:
            opcode = BinaryInstruction::ADD;
            break;
        case SUB:
            opcode = BinaryInstruction::SUB;
            break;
        case MUL:
            opcode = BinaryInstruction::MUL;
            break;
        case DIV:
            opcode = BinaryInstruction::DIV;
            break;
        case FDIV:
            opcode = BinaryInstruction::FDIV;
            break;
        case FMUL:
            opcode = BinaryInstruction::FMUL;
            break;
        case FADD:
            opcode = BinaryInstruction::FADD;
            break;
        case FSUB:
            opcode = BinaryInstruction::FSUB;
            break;
        case MOD:
            opcode = BinaryInstruction::MOD;
            break;
        }
        new BinaryInstruction(opcode, dst, src1, src2, bb);
    }
}

void Constant::genCode()
{
    // we don't need to generate code.
}

void Id::genCode()
{
    Type* idType = getSymPtr()->getType();
    BasicBlock *bb = builder->getInsertBB();
    Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getAddr();
    if(!idType->isArray()){
        new LoadInstruction(dst, addr, bb);
    }else{
        if (arrayIndex!=nullptr) {
            Type* lowerType = ((ArrayType*)(idType))->getLowerType();
            // std::cout<<type->toStr()<<std::endl;
            Type* currentType = idType;
            Operand* currentSrc = addr;
            Operand* currentDst = dst;
            ExprNode* index = arrayIndex;
            bool flag = false;
            bool isPointer = false;
            while (true) {
                // 如果是函数参数中的指针类型
                if (((ArrayType*)currentType)->getDimension() == -1) {
                    Operand* newDst = new Operand(new TemporarySymbolEntry(new PointerType(lowerType), SymbolTable::getLabel()));
                    currentSrc = newDst;
                    new LoadInstruction(newDst, addr, bb);
                    flag = true;
                }
                if (!index) {
                    Operand* newDst = new Operand(new TemporarySymbolEntry(new PointerType(lowerType), SymbolTable::getLabel()));
                    Operand* index = new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
                    new GepInstruction(newDst, currentSrc, index, bb);
                    currentDst = newDst;
                    isPointer = true;
                    break;
                }
                
                index->genCode();
                new GepInstruction(currentDst, currentSrc, index->getOperand(), bb,flag);
                if (flag)
                    flag = false;
                if (!lowerType->isArray())
                    break;
                lowerType = ((ArrayType*)lowerType)->getLowerType();
                currentType = ((ArrayType*)currentType)->getLowerType();
                currentSrc = currentDst;
                currentDst = new Operand(new TemporarySymbolEntry(new PointerType(lowerType), SymbolTable::getLabel()));
                index = index->next;
            }
            dst = currentDst;
            // 如果是右值还需要一条load
            if (!isLval&&!isPointer) {
                Operand* newDst = new Operand(new TemporarySymbolEntry(((ArrayType*)idType)->getElementType(), SymbolTable::getLabel()));
                new LoadInstruction(newDst, dst, bb);
                dst = newDst;
            }
        } else {
            if (((ArrayType*)(idType))->getDimension() == -1) {
                Operand* newDst = new Operand(new TemporarySymbolEntry(new PointerType(((ArrayType*)(idType))->getLowerType()),SymbolTable::getLabel()));
                new LoadInstruction(newDst, addr, bb);
                dst = newDst;
            } else {
                Operand* index = new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
                new GepInstruction(dst, addr, index, bb);
            }
        }
    } 
}

void IfStmt::genCode()
{
    Function *func;
    BasicBlock *then_bb, *end_bb;

    func = builder->getInsertBB()->getParent();
    then_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    cond->genCode();
    backPatch(cond->trueList(), then_bb);
    backPatch(cond->falseList(), end_bb);

    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    then_bb = builder->getInsertBB();   //因为生成thenStmt结点中间代码的过程中可能改变指令的插入点，因此更新插入插入点
    new UncondBrInstruction(end_bb, then_bb);

    builder->setInsertBB(end_bb);
}

void IfElseStmt::genCode()
{
    // Todo
    Function *func = builder->getInsertBB()->getParent();
    BasicBlock *then_bb, *else_bb, *end_bb;

    then_bb = new BasicBlock(func);
    else_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    cond->genCode();
    backPatch(cond->trueList(), then_bb);
    backPatch(cond->falseList(), else_bb);

    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);

    builder->setInsertBB(else_bb);
    elseStmt->genCode();
    else_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, else_bb);

    builder->setInsertBB(end_bb);
}

void CompoundStmt::genCode()
{
    // Todo
    if(stmt != nullptr){
        stmt->genCode();
    }
}

void SeqNode::genCode()
{
    // Todo
    stmt1->genCode();
    stmt2->genCode();
}

//add by zsr 2022年12月5日22:47:55
void VarDeclStmt::genCode(){
    VarDef* varDefPtr = nullptr; 
    if(defHead != nullptr){
        varDefPtr = dynamic_cast<VarDef*>(defHead);
    }

    for(auto it = defEntries->begin();it != defEntries->end();it++){
        IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(*it);
        if(se->isGlobal())
        {
            Operand *addr;
            SymbolEntry *addr_se;
            addr_se = new IdentifierSymbolEntry(*se);
            addr_se->setType(new PointerType(se->getType()));
            addr = new Operand(addr_se);
            se->setAddr(addr);
            builder->getUnit()->insertGlob(se);
        }
        else if(se->isLocal())
        {
            Function *func = builder->getInsertBB()->getParent();
            BasicBlock *entry = func->getEntry();
            Instruction *alloca;
            Operand *addr;
            SymbolEntry *addr_se;
            Type *type;
            type = new PointerType(se->getType());
            addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
            addr = new Operand(addr_se);
            alloca = new AllocaInstruction(addr, se);                   // allocate space for local id in function stack.
            entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.
            se->setAddr(addr);                                        // set the addr operand in symbol entry so that we can use it in subsequent code generation.
            if (varDefPtr == nullptr){
                fprintf(stderr, "varDefPtr == nullptr\n");
                return;
            }
            if(!se->getType()->isArray()){
                ExprNode* val = varDefPtr->getVal();
                if(val != nullptr){
                    val->genCode();
                    new StoreInstruction(addr, val->getOperand(), builder->getInsertBB());
                }
            }else{
                ExprNode* val = varDefPtr->getVal();
                if(val == nullptr){
                    varDefPtr = dynamic_cast<VarDef*>(varDefPtr->next);
                    continue;
                }
                //wait to add
                std::vector<int> indexVec = varDefPtr->getIndex();
                ArrayInitialVal* valTreeRoot = dynamic_cast<ArrayInitialVal*>(val);
                Type* idType = se->getType();
                BasicBlock *bb = builder->getInsertBB();
                Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(se)->getAddr();
                int currentNum = 0;
                int limit = 1;               
                for(int i=0;i<indexVec.size();i++){
                    limit*=indexVec[i];
                }
                while(currentNum < limit){
                    Type* lowerType = ((ArrayType*)(idType))->getLowerType();
                    SymbolEntry* temp = new TemporarySymbolEntry(
                        new PointerType(((ArrayType*)(se->getType()))->getLowerType()),
                        SymbolTable::getLabel());
                    Operand *dst = new Operand(temp);
                    // std::cout<<type->toStr()<<std::endl;
                    Type* currentType = idType;
                    Operand* currentSrc = addr;
                    Operand* currentDst = dst;
                    std::vector<int> findIndexVec;
                    int dimPtr = 0;
                    int indexPtr= currentNum;
                    while (dimPtr < indexVec.size()) {
                        int nowIndex= indexPtr;
                        for(int i= dimPtr+1; i<indexVec.size();i++){
                            nowIndex /= indexVec[i];
                        }
                        findIndexVec.push_back(nowIndex);
                        int tempJi = 1;
                        for(int i= dimPtr+1; i<indexVec.size();i++){
                            tempJi *= indexVec[i];
                        }
                        indexPtr %= tempJi;
                        new GepInstruction(currentDst, currentSrc, new Operand(new ConstantSymbolEntry(TypeSystem::intType, nowIndex)), bb, false);
                        if (!lowerType->isArray())
                            break;
                        lowerType = ((ArrayType*)lowerType)->getLowerType();
                        currentType = ((ArrayType*)currentType)->getLowerType();
                        currentSrc = currentDst;
                        currentDst = new Operand(new TemporarySymbolEntry(new PointerType(lowerType), SymbolTable::getLabel()));
                        dimPtr++;
                    }
                    dst = currentDst;
                    ExprNode* initVal = valTreeRoot->getValWithIndex(findIndexVec);
                    initVal->genCode();
                    new StoreInstruction(dst, initVal->getOperand(), bb);
                    currentNum++;
                }
            }
        }
        varDefPtr = dynamic_cast<VarDef*>(varDefPtr->next);
    }
}

void ReturnStmt::genCode()
{
    // Todo
    BasicBlock *bb = builder->getInsertBB();
    if(retValue != nullptr){
        retValue->genCode();
        new RetInstruction(retValue->getOperand(), bb);
    }else{
        new RetInstruction(nullptr, bb);
    }
}

void AssignStmt::genCode()
{
    BasicBlock *bb = builder->getInsertBB();
    expr->genCode();
    Operand *addr;
    if (!lval->getSymPtr()->getType()->isArray())
        addr = dynamic_cast<IdentifierSymbolEntry*>(lval->getSymPtr())->getAddr();
    else {
        ((Id*)lval)->setLval();
        lval->genCode();
        addr = lval->getOperand();
    }
    Operand *src = expr->getOperand();
    /***
     * We haven't implemented array yet, the lval can only be ID. So we just store the result of the `expr` to the addr of the id.
     * If you want to implement array, you have to caculate the address first and then store the result into it.
     */
    new StoreInstruction(addr, src, bb);
}

//2022/12/6 xzh new  待定
void UnaryExpr::genCode()
{
    if(op==NOT){
        notCount++;
    }
    expr->genCode();
    BasicBlock *bb = builder->getInsertBB();
    Function *func = bb->getParent();
    Type* exprType=expr->getSymPtr()->getType();
    exprType=trueType(exprType);
    if(op==UMINUS){
        Operand *src2=expr->getOperand();
        if(exprType->isInt()){
            Operand *src1=new Operand(new ConstantSymbolEntry(TypeSystem::intType,0));
            if(exprType->isConstInt()){
                ConstIntType* exprTypeInt=dynamic_cast<ConstIntType*>(exprType);
                if(exprTypeInt->isBool()){      //bool到int的转换
                    Operand* zextDest = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
                    new ZextInstruction(zextDest, src2, bb);
                    src2=zextDest;
                }
            }else{
                IntType* exprTypeInt=dynamic_cast<IntType*>(exprType);
                if(exprTypeInt->isBool()){      //bool到int的转换
                    Operand* zextDest = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
                    new ZextInstruction(zextDest, src2, bb);
                    src2=zextDest;
                }
            }
            dst = new Operand(new TemporarySymbolEntry(this->getSymPtr()->getType(),SymbolTable::getLabel()));
            new BinaryInstruction(BinaryInstruction::SUB,dst,src1,src2,bb);
        }else if(exprType->isFloat()){      //还没区分constFloat和float
            Operand *src1=new Operand(new ConstantSymbolEntry(TypeSystem::floatType,0));
            dst = new Operand(new TemporarySymbolEntry(this->getSymPtr()->getType(),SymbolTable::getLabel()));
            new BinaryInstruction(BinaryInstruction::FSUB,dst,src1,src2,bb);
        }
    }else if(op==NOT){
        Operand *src1=expr->getOperand();
        notCount--;
        if(exprType->isInt()){      //应该不需要判断是不是bool，可以同样操作
            Operand* src2=new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
            dst = new Operand(new TemporarySymbolEntry(this->getSymPtr()->getType(),SymbolTable::getLabel()));
            new CmpInstruction(CmpInstruction::E, dst, src1,src2, bb);
        }else if(exprType->isFloat()){
            Operand* src2=new Operand(new ConstantSymbolEntry(TypeSystem::floatType, 0));
            dst = new Operand(new TemporarySymbolEntry(this->getSymPtr()->getType(),SymbolTable::getLabel()));
            new FCmpInstruction(FCmpInstruction::E, dst, src1,src2, bb);
        }
        if(notCount==0){
            BasicBlock *true_bb, *false_bb, *mid_bb;
            true_bb = new BasicBlock(func);
            false_bb = new BasicBlock(func);
            mid_bb = new BasicBlock(func);

            true_list.push_back(new CondBrInstruction(true_bb, mid_bb, dst, bb));
            false_list.push_back(new UncondBrInstruction(false_bb, mid_bb));  
        }
    }else if(op==FTOI){
        Operand *src= expr->getOperand();
        dst = new Operand(new TemporarySymbolEntry(this->getSymPtr()->getType(),SymbolTable::getLabel()));
        new CastInstruction(CastInstruction::FTOI, src, dst, bb);
    }else if(op==ITOF){
        
        Operand *src= expr->getOperand();
        dst = new Operand(new TemporarySymbolEntry(TypeSystem::floatType,SymbolTable::getLabel()));
        new CastInstruction(CastInstruction::ITOF, src, dst, bb);
    }else if(op==BTOI){
        Operand *src= expr->getOperand();
        // if(src==nullptr)
        //     std::cout<<"src null"<<std::endl;
        // if(dst==nullptr)
        //     std::cout<<"dst null"<<std::endl;
        dst = new Operand(new TemporarySymbolEntry(this->getSymPtr()->getType(),SymbolTable::getLabel()));
        new CastInstruction(CastInstruction::BTOI, src, dst, bb);   
    }
}
//2022/12/6 xzh new  待定
void FuncInvoke::genCode(){
    if (((IdentifierSymbolEntry*)getSymPtr())->isSysyFunc()) {
        builder->getUnit()->insertDeclare(getSymPtr());
    }
    std::vector<Operand*> params;
    ExprNode* paramPtr = Rparams;
    while(paramPtr != nullptr){
        paramPtr->genCode();
        params.push_back(paramPtr->getOperand());
        paramPtr = paramPtr->next;
    }
    Type* midtype = getSymPtr()->getType();

    FunctionType* funcType = dynamic_cast<FunctionType*>(midtype);
    Type* retType = funcType->getRetType();

    if(retType->isVoid()){
        new CallInstruction(nullptr, getSymPtr(), params, builder->getInsertBB());
    }else {
        SymbolEntry* trueDst = new TemporarySymbolEntry(retType, SymbolTable::getLabel());
        Operand* retOp = new Operand(trueDst);
        
        new CallInstruction(retOp, getSymPtr(), params, builder->getInsertBB()); 
        setSymPtr(trueDst);
        setOperand(retOp);
    }
}
//2022/12/10 xzh new
void WhileStmt::genCode(){
    Function *func = builder->getInsertBB()->getParent();
    BasicBlock *cond_bb, *loopStmt_bb, *end_bb;
    cond_bb = new BasicBlock(func);
    loopStmt_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);
    
    this->cond_bb=cond_bb;
    this->end_bb=end_bb;
    while_cond_entry.push(cond_bb);     //将当前的条件语句块push进while_cond_entry（栈），便于continue语句获取cond_bb  
    while_end_entry.push(end_bb);    //将当前的结束语句块push进while_end_entry（栈），便于break语句获取end_bb     
    
    BasicBlock* bb=builder->getInsertBB();
    new UncondBrInstruction(cond_bb,bb);    //将跳转到条件语句块的无条件跳转指令插入当前块

    builder->setInsertBB(cond_bb);
    cond->genCode();
    backPatch(cond->trueList(), loopStmt_bb);
    backPatch(cond->falseList(), end_bb);

    builder->setInsertBB(loopStmt_bb);
    loopStmt->genCode();
    loopStmt_bb = builder->getInsertBB();
    new UncondBrInstruction(cond_bb, loopStmt_bb);  //在循环体的块的末尾加一个无条件跳转语句到条件语句块

    builder->setInsertBB(end_bb);

    while_cond_entry.pop();
    while_end_entry.pop();
}
//2022/12/10 xzh new
void ContinueStmt::genCode(){
    BasicBlock* bb=builder->getInsertBB();
    Function* func = builder->getInsertBB()->getParent();
    if(while_cond_entry.size()!=0){
        BasicBlock* while_cond_bb=while_cond_entry.top();
        new UncondBrInstruction(while_cond_bb,bb);

        BasicBlock* continue_next_bb = new BasicBlock(func);
        builder->setInsertBB(continue_next_bb);
    }else{
        fprintf(stderr, "while_cond_entry.size()==0. ContinueStmt position error\n");
        return;
    }
}
//2022/12/10 xzh new
void BreakStmt::genCode(){
    BasicBlock* bb=builder->getInsertBB();
    Function* func = builder->getInsertBB()->getParent();
    if(while_end_entry.size()!=0){
        BasicBlock* while_end_bb=while_end_entry.top();
        new UncondBrInstruction(while_end_bb,bb);

        BasicBlock* break_next_bb = new BasicBlock(func);
        builder->setInsertBB(break_next_bb);
    }else{
        fprintf(stderr, "while_end_entry.size()==0. BreakStmt position error\n");
        return;
    }
}

void ExprStmt::genCode(){
    if(myExp != nullptr)
        myExp->genCode();
}

void BlankStmt::genCode(){
    //nothing to do
}

void FuncFParam::genCode(){
    //nothing to do
}

void VarDef::genCode(){
    
}

void ConstDeclStmt::genCode(){
    ConstDef* varDefPtr = nullptr; 
    if(defHead != nullptr){
        varDefPtr = dynamic_cast<ConstDef*>(defHead);
    }
    for(auto it = defEntries->begin();it != defEntries->end();it++){
        IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(*it);
        if(se->isGlobal()){
            // std::cout<<se->toStr()<<" ast_val:"<<se->getValue()<<std::endl;
            Operand *addr;
            SymbolEntry *addr_se;
            addr_se = new IdentifierSymbolEntry(*se);
            addr_se->setType(new PointerType(se->getType()));
            addr = new Operand(addr_se);
            se->setAddr(addr);
            builder->getUnit()->insertGlob(se);
        }
        else if(se->isLocal())
        {
            Function *func = builder->getInsertBB()->getParent();
            BasicBlock *entry = func->getEntry();
            Instruction *alloca;
            Operand *addr;
            SymbolEntry *addr_se;
            Type *type;
            type = new PointerType(se->getType());
            addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
            addr = new Operand(addr_se);
            alloca = new AllocaInstruction(addr, se);                   // allocate space for local id in function stack.
            entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.
            se->setAddr(addr);                                        // set the addr operand in symbol entry so that we can use it in subsequent code generation.
            if (varDefPtr == nullptr){
                fprintf(stderr, "varDefPtr == nullptr\n");
                return;
            }
            ExprNode* val = varDefPtr->getVal();
            if(val != nullptr){
                if(!se->getType()->isArray()){
                    val->genCode();
                    new StoreInstruction(addr, val->getOperand(), builder->getInsertBB());
                }else{
                    ExprNode* val = varDefPtr->getVal();
                    //wait to add
                    std::vector<int> indexVec = varDefPtr->getIndex();
                    ArrayInitialVal* valTreeRoot = dynamic_cast<ArrayInitialVal*>(val);
                    Type* idType = se->getType();
                    BasicBlock *bb = builder->getInsertBB();
                    Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(se)->getAddr();
                    int currentNum = 0;
                    int limit = 1;               
                    for(int i=0;i<indexVec.size();i++){
                        limit*=indexVec[i];
                    }
                    while(currentNum < limit){
                        Type* lowerType = ((ArrayType*)(idType))->getLowerType();
                        SymbolEntry* temp = new TemporarySymbolEntry(
                            new PointerType(((ArrayType*)(se->getType()))->getLowerType()),
                            SymbolTable::getLabel());
                        Operand *dst = new Operand(temp);
                        // std::cout<<type->toStr()<<std::endl;
                        Type* currentType = idType;
                        Operand* currentSrc = addr;
                        Operand* currentDst = dst;
                        std::vector<int> findIndexVec;
                        int dimPtr = 0;
                        int indexPtr= currentNum;
                        while (dimPtr < indexVec.size()) {
                            int nowIndex= indexPtr;
                            for(int i= dimPtr+1; i<indexVec.size();i++){
                                nowIndex /= indexVec[i];
                            }
                            findIndexVec.push_back(nowIndex);
                            int tempJi = 1;
                            for(int i= dimPtr+1; i<indexVec.size();i++){
                                tempJi *= indexVec[i];
                            }
                            indexPtr %= tempJi;
                            new GepInstruction(currentDst, currentSrc, new Operand(new ConstantSymbolEntry(TypeSystem::intType, nowIndex)), bb, false);
                            if (!lowerType->isArray())
                                break;
                            lowerType = ((ArrayType*)lowerType)->getLowerType();
                            currentType = ((ArrayType*)currentType)->getLowerType();
                            currentSrc = currentDst;
                            currentDst = new Operand(new TemporarySymbolEntry(new PointerType(lowerType), SymbolTable::getLabel()));
                            dimPtr++;
                        }
                        dst = currentDst;
                        ExprNode* initVal = valTreeRoot->getValWithIndex(findIndexVec);
                        initVal->genCode();
                        new StoreInstruction(dst, initVal->getOperand(), bb);
                        currentNum++;
                    }            
                }
            }
        }
        varDefPtr = dynamic_cast<ConstDef*>(varDefPtr->next);
    }
}

void ConstDef::genCode(){

}

void ArrayInitialVal::genCode(){

}


void Ast::typeCheck()
{
    if(root != nullptr)
        root->typeCheck();
}

void FunctionDef::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
    if(funcPDef!=nullptr)
        funcPDef->typeCheck();
    FunctionType* funcType=dynamic_cast<FunctionType*>(getSymptr()->getType());
    currentFuncType = funcType;
    checkFuncType=funcType->getRetType();
    if(stmt!= nullptr){
        stmt->typeCheck();   
    }
    if(!funcType->haveReturn()&&!funcType->getRetType()->isVoid()){
        fprintf(stderr, "function %s doesn't have a return stmt", getSymptr()->toStr().c_str());
        check_no_err = false;
        exit(EXIT_FAILURE);
    }
}

// 2022/12/11 xzh new  待定
void BinaryExpr::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
    expr1->typeCheck();
    expr2->typeCheck();
    Type* expr1Type=expr1->getSymPtr()->getType();
    Type* expr2Type=expr2->getSymPtr()->getType();
    expr1Type=trueType(expr1Type);
    expr2Type=trueType(expr2Type);
    if(expr1Type->isVoid()||expr2Type->isVoid()){       //返回值为 void 的函数调用结果参与了某表达式计算
        fprintf(stderr, "The result of a function call with a return value of void participates in the calculation of an expression\n ");
        check_no_err=false;
        return;
    }
    if ( (ADD<=op &&op<=FSUB)||(op>=LESS && op<=GREATER)){
        Type* maxT=maxType(expr1Type,expr2Type);
        if(expr1Type->toStr().find(maxT->toStr()) == -1){       //expr1Type!=maxT
            SymbolEntry *castSe = new TemporarySymbolEntry(maxT, SymbolTable::getLabel());
            int castOp=getCastOp(expr1Type,maxT);
            ExprNode* castNode;
            if(castOp==UnaryExpr::BTOF){
                SymbolEntry *btoiCastSe = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                ExprNode* btoiCastNode=new UnaryExpr(btoiCastSe,UnaryExpr::BTOI,expr1);
                castNode=new UnaryExpr(castSe,UnaryExpr::ITOF,btoiCastNode);
            }else{
                castNode=new UnaryExpr(castSe,castOp,expr1);
            }
            expr1=castNode;
        }
        if(expr2Type->toStr().find(maxT->toStr()) == -1){
            SymbolEntry *castSe = new TemporarySymbolEntry(maxT, SymbolTable::getLabel());
            int castOp=getCastOp(expr2Type,maxT);
            ExprNode* castNode;
            if(castOp==UnaryExpr::BTOF){
                SymbolEntry *btoiCastSe = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                ExprNode* btoiCastNode=new UnaryExpr(btoiCastSe,UnaryExpr::BTOI,expr2);
                castNode=new UnaryExpr(castSe,UnaryExpr::ITOF,btoiCastNode);
            }else{
                castNode=new UnaryExpr(castSe,castOp,expr2);
            }
            expr2=castNode;

        }

    }else if(op==MOD){
        if(expr1Type->isInt()&&expr2Type->isInt()){
            if(!(expr1Type->isConstInt()||expr2Type->isConstInt())){
                IntType* intType1=dynamic_cast<IntType*>(expr1Type);
                IntType* intType2=dynamic_cast<IntType*>(expr2Type);
                if(intType1->isBool()||intType2->isBool()){
                    fprintf(stderr, "Wrong operand type of modulus operation. The types of the two operands are %s and %s \n",expr1Type->toStr().c_str(), expr2Type->toStr().c_str());
                    check_no_err=false;
                    return;
                }
            }
        }else{
            fprintf(stderr, "Wrong operand type of modulus operation. The types of the two operands are %s and %s \n",expr1Type->toStr().c_str(), expr2Type->toStr().c_str());
            check_no_err=false;
            return;
        }
    }else if(op==AND||op==OR){
        if(expr1Type->isInt()){
            if(expr1Type->isConstInt()){
                    SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::intType, 0);
                    Constant* zero = new Constant(zeroSe);
                    SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
                    ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,expr1,zero);
                    expr1=castNode;
                    std::cout<< "Conditional expression: Implicit conversion from int to bool in symbol: "<<expr1->getSymPtr()->toStr()<<std::endl;
            }else{
                IntType* exprTypeInt=dynamic_cast<IntType*>(expr1Type);
                if(!exprTypeInt->isBool()){
                    SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::intType, 0);
                    Constant* zero = new Constant(zeroSe);
                    SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
                    ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,expr1,zero);
                    expr1=castNode;
                    std::cout<< "Conditional expression: Implicit conversion from int to bool in symbol: "<<expr1->getSymPtr()->toStr()<<std::endl;
                }
            }
        }else if(expr1Type->isFloat()){     
            SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::floatType, 0);
            Constant* zero = new Constant(zeroSe);
            SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,expr1,zero);
            expr1=castNode;
            std::cout<<"Conditional expression: Implicit conversion from float to bool in symbol: "<<expr1->getSymPtr()->toStr()<<std::endl;
        }
        if(expr2Type->isInt()){
            if(expr2Type->isConstInt()){
                SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::intType, 0);
                Constant* zero = new Constant(zeroSe);
                SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
                ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,expr2,zero);
                expr2=castNode;
                std::cout<< "Conditional expression: Implicit conversion from int to bool in symbol: "<<expr2->getSymPtr()->toStr()<<std::endl;
            }else{
                IntType* exprTypeInt=dynamic_cast<IntType*>(expr2Type);
                if(!exprTypeInt->isBool()){
                    SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::intType, 0);
                    Constant* zero = new Constant(zeroSe);
                    SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
                    ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,expr2,zero);
                    expr2=castNode;
                    std::cout<< "Conditional expression: Implicit conversion from int to bool in symbol: "<<expr2->getSymPtr()->toStr()<<std::endl;
                } 
            }
        }else if(expr2Type->isFloat()){
            SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::floatType, 0);
            Constant* zero = new Constant(zeroSe);
            SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,expr2,zero);
            expr2=castNode;
            std::cout<<"Conditional expression: Implicit conversion from float to bool in symbol: "<<expr2->getSymPtr()->toStr()<<std::endl;
        }
    }
    if(expr1->isConst()&&expr2->isConst()){
        float val;
        float expr1Value = expr1->getSymPtr()->isConstant() ? 
                ((ConstantSymbolEntry*)(expr1->getSymPtr()))->getValue() : 
                ((IdentifierSymbolEntry*)(expr1->getSymPtr()))->getValue();
        float expr2Value = expr2->getSymPtr()->isConstant() ? 
                ((ConstantSymbolEntry*)(expr2->getSymPtr()))->getValue() : 
                ((IdentifierSymbolEntry*)(expr2->getSymPtr()))->getValue();
        switch(op)
        {
            case ADD:
                val = expr1Value + expr2Value;
                break;
            case SUB:
                val = expr1Value - expr2Value;
                break;
            case MUL:
                val = expr1Value * expr2Value;
                break;
            case DIV:
                val = expr1Value / expr2Value;
                break;
            case FADD:
                val = expr1Value + expr2Value;
                break;
            case FSUB:
                val = expr1Value - expr2Value;
                break;
            case FMUL:
                val = expr1Value * expr2Value;
                break;
            case FDIV:
                val = expr1Value / expr2Value;
                break;
            case MOD:
                val = (int)expr1Value % (int)expr2Value;
                break;
            case AND:
                val = expr1Value && expr2Value;
                break;
            case OR:
                val = expr1Value || expr2Value;
                break;
            case LESS:
                val = expr1Value < expr2Value;
                break;
            case GREATEREQUAL:
                val = expr1Value >= expr2Value;
                break;
            case LESSEQUAL:
                val = expr1Value <= expr2Value;
                break;
            case EQUAL:
                val = expr1Value == expr2Value;
                break;
            case NOTEQUAL:
                val = expr1Value != expr2Value;
                break;
            case GREATER:
                val = expr1Value > expr2Value;
                break;
        }
        SymbolEntry* val_se = new ConstantSymbolEntry(getSymPtr()->getType(),val);
        setSymPtr(val_se);
    }
}

void Constant::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
}

void Id::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
}

//这里要检查一下条件是否为bool并进行隐式转化(插入一个ne的binarynode) 还没写完
void IfStmt::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
    cond->typeCheck();
    Type* condType=cond->getSymPtr()->getType();
    condType=trueType(condType);
    if(condType->isVoid()){       //返回值为 void 的函数调用结果参与了某表达式计算
        fprintf(stderr, "IfStmt: The result of a function call with a return value of void.\n ");
        check_no_err=false;
        return;
    }   
    if(condType->isConstInt())
        condType = TypeSystem::intType;
    else if(condType->isConstFloat())
        condType = TypeSystem::floatType;
    if(condType->isInt()){
        IntType* exprTypeInt=dynamic_cast<IntType*>(condType);
        if(!exprTypeInt->isBool()){
            SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::intType, 0);
            Constant* zero = new Constant(zeroSe);
            SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,cond,zero);
            cond=castNode;
            std::cout<< "IfStmt: Implicit conversion from int to bool in symbol: "<<cond->getSymPtr()->toStr()<<std::endl;
        } 
    }else if(condType->isFloat()){
        SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::floatType, 0);
        Constant* zero = new Constant(zeroSe);
        SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,cond,zero);
        cond=castNode;
        std::cout<<"IfStmt: Implicit conversion from float to bool in symbol: "<<cond->getSymPtr()->toStr()<<std::endl;
    }
    thenStmt->typeCheck();
}

void IfElseStmt::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
    cond->typeCheck();
    Type* condType=cond->getSymPtr()->getType();
    condType=trueType(condType); 
    if(condType->isVoid()){       //返回值为 void 的函数调用结果参与了某表达式计算
        fprintf(stderr, "IfElseStmt: The result of a function call with a return value of void.\n ");
        check_no_err=false;
        return;
    }   
    if(condType->isConstInt())
        condType = TypeSystem::intType;
    else if(condType->isConstFloat())
        condType = TypeSystem::floatType;
    if(condType->isInt()){
        IntType* exprTypeInt=dynamic_cast<IntType*>(condType);
        if(!exprTypeInt->isBool()){
            SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::intType, 0);
            Constant* zero = new Constant(zeroSe);
            SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,cond,zero);
            cond=castNode;
            std::cout<< "IfElseStmt: Implicit conversion from int to bool in symbol: "<<cond->getSymPtr()->toStr()<<std::endl;
        } 
    }else if(condType->isFloat()){
        SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::floatType, 0);
        Constant* zero = new Constant(zeroSe);
        SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,cond,zero);
        cond=castNode;
        std::cout<< "IfElseStmt: Implicit conversion from float to bool in symbol: "<<cond->getSymPtr()->toStr()<<std::endl;
    }
    thenStmt->typeCheck();
    elseStmt->typeCheck();
}

void CompoundStmt::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
    if(stmt!=nullptr)
        stmt->typeCheck();
}

void SeqNode::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
    if(stmt1 != nullptr)
        stmt1->typeCheck();
    if(stmt2 != nullptr)
        stmt2->typeCheck();
}


void ReturnStmt::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
    if(checkFuncType == nullptr){
        fprintf(stderr, "Return Stmt should write in Function\n");
        check_no_err = false;
        return;
    }

    Type* funcType = checkFuncType;
    Type* retType;
    if(retValue != nullptr){
        retValue->typeCheck();
        retType = retValue->getSymPtr()->getType();
        // if(retType->isFunc()){
        //     FunctionType* funcType = dynamic_cast<FunctionType*>(retType);
        //     retType = funcType->getRetType();
        // }else if(retType->isArray()){
        //     ArrayType* arrType = dynamic_cast<ArrayType*>(retType);
        //     retType = arrType->getElementType();
        // }
        retType = trueType(retType);
    }else{
       retType = TypeSystem::voidType; 
    }
    if(retType->isConstInt())
        retType = TypeSystem::intType;
    else if(retType->isConstFloat())
        retType = TypeSystem::floatType;
    if (retType->toStr() != funcType->toStr()){
        if(funcType->isVoid()||retType->isVoid()){
            check_no_err = false;
            fprintf(stderr, "Return Type Wrong! Target return Type: %s Now Return Type: %s\n",
                    funcType->toStr().c_str(),
                    retType->toStr().c_str());
        }else{
            SymbolEntry *castSe = new TemporarySymbolEntry(funcType, SymbolTable::getLabel());
            int castOp=getCastOp(retType,funcType);
            ExprNode* castNode;
            if(castOp==UnaryExpr::BTOF){
                SymbolEntry *btoiCastSe = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                ExprNode* btoiCastNode=new UnaryExpr(btoiCastSe,UnaryExpr::BTOI,retValue);
                castNode=new UnaryExpr(castSe,UnaryExpr::ITOF,btoiCastNode);
            }else{
                castNode=new UnaryExpr(castSe,castOp,retValue);
            }
            retValue=castNode;
        }
    }else{
        currentFuncType->setReturn();
    }
    return;
}

void AssignStmt::typeCheck()
{
    // Todo
    if(!check_no_err)
        return;
    Type* lValType = lval->getSymPtr()->getType();
    lValType = trueType(lValType);
    if(lValType->isConstFloat()||lValType->isConstInt()){
        fprintf(stderr, "const value can't assign\n");
        check_no_err = false;
    }
    if(expr==nullptr){
        fprintf(stderr, "assign expr is null?");
        exit(EXIT_FAILURE);
    }
    expr->typeCheck();
    Type* rValType = expr->getSymPtr()->getType();
    rValType = trueType(rValType);
    // std::cout<<lValType->toStr()<<","<<rValType->toStr()<<std::endl;
    if ((lValType->isInt()&&rValType->isFloat())||(lValType->isFloat()&&rValType->isInt())){
        // std::cout<<"ne"<<std::endl;
        SymbolEntry *castSe = new TemporarySymbolEntry(lValType, SymbolTable::getLabel());
        int castOp = getCastOp(rValType, lValType);
        ExprNode *castNode;
        if (castOp == UnaryExpr::BTOF){
            SymbolEntry *btoiCastSe = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
            ExprNode *btoiCastNode = new UnaryExpr(btoiCastSe, UnaryExpr::BTOI, expr);
            castNode = new UnaryExpr(castSe, UnaryExpr::ITOF, btoiCastNode);
        }
        else{
            castNode = new UnaryExpr(castSe, castOp, expr);
        }
        expr = castNode;
    }
}



void ContinueStmt::typeCheck(){
    if(!check_no_err)
        return;
        //nothing to do
}

void BreakStmt::typeCheck(){
    if(!check_no_err)
        return;
        //nothing to do
}

void WhileStmt::typeCheck(){
    if(!check_no_err)
        return;
    cond->typeCheck();
    Type* condType=cond->getSymPtr()->getType();
    condType=trueType(condType); 
    if(condType->isVoid()){       //返回值为 void 的函数调用结果参与了某表达式计算
        fprintf(stderr, "WhileStmt: The result of a function call with a return value of void.\n ");
        check_no_err=false;
        return;
    }
    if(condType->isConstInt())
        condType = TypeSystem::intType;
    else if(condType->isConstFloat())
        condType = TypeSystem::floatType;
    if(condType->isInt()){
        IntType* exprTypeInt=dynamic_cast<IntType*>(condType);
        if(!exprTypeInt->isBool()){
            SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::intType, 0);
            Constant* zero = new Constant(zeroSe);
            SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,cond,zero);
            cond=castNode;
            std::cout<< "WhileStmt: Implicit conversion from int to bool in symbol: "<<cond->getSymPtr()->toStr()<<std::endl;
        } 
    }else if(condType->isFloat()){
        SymbolEntry *zeroSe = new ConstantSymbolEntry(TypeSystem::floatType, 0);
        Constant* zero = new Constant(zeroSe);
        SymbolEntry *castSe = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        ExprNode* castNode=new BinaryExpr(castSe,BinaryExpr::NOTEQUAL,cond,zero);
        cond=castNode;
        std::cout<<"WhileStmt: Implicit conversion from float to bool in symbol: "<<cond->getSymPtr()->toStr()<<std::endl;
    }
    loopStmt->typeCheck();
}

void VarDef::typeCheck(){
    if(!check_no_err)
        return;
}

void VarDeclStmt::typeCheck(){
    if(!check_no_err)
        return;
    VarDef* varDefPtr = nullptr; 
    if(defHead != nullptr){
        varDefPtr = dynamic_cast<VarDef*>(defHead);
    }else{
        fprintf(stderr, "defHead is null!\n");
        check_no_err = false;
        return;
    }
    for(auto it = defEntries->begin();it != defEntries->end();it++){
        IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(*it);            
        Type* seType = se->getType();
        ExprNode* val = varDefPtr->getVal();
        if(val != nullptr)
            val->typeCheck();
        if(seType->isArray()){
            //数组初始化的typecheck
            if(val){
                ArrayInitialVal* valRoot = dynamic_cast<ArrayInitialVal*>(val);
                valRoot->checkTreeType(type);
            }
        }
        if(val != nullptr){
            if(val->getSymPtr()==nullptr)   //应该不会进入
                return;
            Type* valType = val->getSymPtr()->getType();
            valType = trueType(valType);
            seType = trueType(seType);
            if(valType->toStr().find(seType->toStr()) == -1){
                if(val->getSymPtr()->isConstant()){
                    ConstantSymbolEntry* constSymPtr = dynamic_cast<ConstantSymbolEntry*>(val->getSymPtr());
                    if(seType->isInt()){
                        if(valType->isInt()){
                            //Exp不可解析为RelExp，因此不存在此路径
                            std::cout<<"sth wrong?1"<<std::endl;
                        }else if(valType->isFloat()){
                            //在constantSymbolentry的output里进行了隐式转换
                            std::cout<<"warning: implicit conversion"
                                <<" from \'float\' to \'int\' changes value from "
                                <<constSymPtr->getValue()<<" to "
                                <<(int)constSymPtr->getValue()<<std::endl;
                            ExprNode* newVal = new Constant(new ConstantSymbolEntry(TypeSystem::intType, (int)constSymPtr->getValue()));
                            varDefPtr->setVal(newVal);
                        }
                    }else if(seType->isFloat()){
                        std::cout<<"warning: implicit conversion"
                                <<" from \'int\' to \'float\' changes value from "
                                <<constSymPtr->getValue()<<" to "
                                <<(double)constSymPtr->getValue()<<std::endl;
                        ExprNode* newVal = new Constant(new ConstantSymbolEntry(TypeSystem::floatType, (double)constSymPtr->getValue()));
                        varDefPtr->setVal(newVal);
                    }
                }else {//如果不是constant 无法使用隐式的转换
                    if(seType->isInt()){
                        if(valType->isInt()){
                            //Exp不可解析为RelExp，因此不存在此路径
                            std::cout<<"sth wrong?2"<<std::endl;
                        }else if(valType->isFloat()){
                            TemporarySymbolEntry* tempSe = new TemporarySymbolEntry(TypeSystem::intType,SymbolTable::getLabel());
                            ExprNode* castNode = new UnaryExpr(tempSe, UnaryExpr::FTOI, val);
                            varDefPtr->setVal(castNode);
                            std::cout<<"warning: implicit cast from float to int in symbol: "<<val->getSymPtr()->toStr()<<std::endl;
                        }
                    }else if(seType->isFloat()){
                        TemporarySymbolEntry* tempSe = new TemporarySymbolEntry(TypeSystem::floatType,SymbolTable::getLabel());
                        ExprNode* castNode = new UnaryExpr(tempSe, UnaryExpr::ITOF, val);
                        varDefPtr->setVal(castNode);
                        std::cout<<"warning: implicit cast from int to float in symbol: "<<val->getSymPtr()->toStr()<<std::endl;
                    }   
                }
            }
        }
        if(se->isGlobal()){
            if(!se->getType()->isArray()){
                checkFuncType = nullptr;
                ExprNode* valNode = varDefPtr->getVal();
                if(valNode == nullptr){
                    se->setValue(0);
                    varDefPtr = (VarDef*)(varDefPtr->next);
                    continue;
                }
                // if(!valNode->isConst()){
                //     fprintf(stderr, "initial val should be constant\n");
                //     exit(EXIT_FAILURE);
                // }有错也是他有错
                SymbolEntry* valSe = valNode->getSymPtr();
                float globValue = valSe->isConstant() ? 
                    ((ConstantSymbolEntry*)(valSe))->getValue() : 
                    ((IdentifierSymbolEntry*)(valSe))->getValue();
                if(se->getType()->isInt())
                    globValue = (int)globValue;
                se->setValue(globValue);
            }
            checkFuncType = nullptr;
        }
        varDefPtr = (VarDef*)(varDefPtr->next);
    }
}

void ConstDef::typeCheck(){
    if(!check_no_err)
        return;
        //nothing to do
}

void ConstDeclStmt::typeCheck(){
    if(!check_no_err)
        return;
    ConstDef* varDefPtr = nullptr; 
    if(defHead != nullptr){
        varDefPtr = dynamic_cast<ConstDef*>(defHead);
    }else{
        fprintf(stderr, "defHead is null!\n");
        check_no_err = false;
        return;
    }
    for(auto it = defEntries->begin();it != defEntries->end();it++){
        IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(*it);        
        Type* seType = se->getType();
        ExprNode* val = varDefPtr->getVal();
        if(val)
            val->typeCheck();
        if(seType->isArray()){
            //数组初始化的typecheck
            if(val){
                ArrayInitialVal* valRoot = dynamic_cast<ArrayInitialVal*>(val);
                Type* checkType=type;
                if(type->isConstInt())
                    checkType = TypeSystem::intType;
                else if(type->isConstFloat())
                    checkType = TypeSystem::floatType;
                valRoot->checkTreeType(checkType);
            }
        }
        if(val != nullptr){
            Type* valType = val->getSymPtr()->getType();
            valType=trueType(valType);
            seType=trueType(seType);
            if(seType->toStr().find(valType->toStr()) == -1){
                if(val->getSymPtr()->isConstant()){
                    ConstantSymbolEntry* constSymPtr = dynamic_cast<ConstantSymbolEntry*>(val->getSymPtr());
                    if(seType->isInt()){
                        if(valType->isInt()){
                            //Exp不可解析为RelExp，因此不存在此路径
                            std::cout<<"sth wrong?3"<<std::endl;
                        }else if(valType->isFloat()){
                            //在constantSymbolentry的output里进行了隐式转换
                            std::cout<<"warning: implicit conversion"
                                <<" from \'float\' to \'int\' changes value from "
                                <<constSymPtr->getValue()<<" to "
                                <<(int)constSymPtr->getValue()<<std::endl;
                            ExprNode* newVal = new Constant(new ConstantSymbolEntry(TypeSystem::intType, (int)constSymPtr->getValue()));
                            varDefPtr->setVal(newVal);
                        }
                    }else if(seType->isFloat()){
                        std::cout<<"warning: implicit conversion"
                                <<" from \'int\' to \'float\' changes value from "
                                <<constSymPtr->getValue()<<" to "
                                <<(int)constSymPtr->getValue()<<std::endl;
                        ExprNode* newVal = new Constant(new ConstantSymbolEntry(TypeSystem::floatType, (double)constSymPtr->getValue()));
                        varDefPtr->setVal(newVal);
                    }
                }else {//如果不是constant 无法使用隐式的转换
                    if(seType->isInt()){
                        if(valType->isInt()){
                            //Exp不可解析为RelExp，因此不存在此路径
                            std::cout<<"sth wrong?4"<<std::endl;
                        }else if(valType->isFloat()){
                            TemporarySymbolEntry* tempSe = new TemporarySymbolEntry(TypeSystem::intType,SymbolTable::getLabel());
                            ExprNode* castNode = new UnaryExpr(tempSe, UnaryExpr::FTOI, val);
                            varDefPtr->setVal(castNode);
                            std::cout<<"warning: implicit cast from float to int in symbol: "<<val->getSymPtr()->toStr()<<std::endl;
                        }
                    }else if(seType->isFloat()){
                        TemporarySymbolEntry* tempSe = new TemporarySymbolEntry(TypeSystem::floatType,SymbolTable::getLabel());
                        ExprNode* castNode = new UnaryExpr(tempSe, UnaryExpr::ITOF, val);
                        varDefPtr->setVal(castNode);
                        std::cout<<"warning: implicit cast from int to float in symbol: "<<val->getSymPtr()->toStr()<<std::endl;
                    }   
                }
            }
        }
        if(se->isGlobal()){
            checkFuncType = nullptr;
            if(!se->getType()->isArray()){
                ExprNode* valNode = varDefPtr->getVal();
                // if(valNode == nullptr){
                //     se->setValue(0);
                //     varDefPtr = (ConstDef*)(varDefPtr->next);
                //     continue;
                // }
                // if(!valNode->isConst()){
                //     fprintf(stderr, "initial val should be constant\n");
                //     exit(EXIT_FAILURE);
                // }
                SymbolEntry* valSe = valNode->getSymPtr();
                float globValue;
                if(valNode->isConst()){
                    globValue = valSe->isConstant() ? 
                            ((ConstantSymbolEntry*)(valSe))->getValue() : 
                            ((IdentifierSymbolEntry*)(valSe))->getValue();
                    if(se->getType()->isInt())
                        globValue = (int)globValue;
                        se->setValue(globValue);
                }
            }//数组初始化typecheck还没
            
        }
        varDefPtr = (ConstDef*)(varDefPtr->next);
    }

}

void FuncFParam::typeCheck(){
    if(!check_no_err)
        return;
}

void FuncInvoke::typeCheck(){
    if(!check_no_err)
        return;
    ExprNode* paramPtr = Rparams;
    Type* midtype = getSymPtr()->getType();
    FunctionType* funcType = dynamic_cast<FunctionType*>(midtype);
    std::vector<Type*> paramtypes = funcType->getParams();
    auto fparamPtr = paramtypes.begin();
    ExprNode* prev = nullptr;
    int RparamNum = 0;
    while (paramPtr != nullptr)
    {
        RparamNum++;
        paramPtr->typeCheck();
        Type* expr1Type = paramPtr->getSymPtr()->getType();  //实参类型
        Type* expr2Type = *fparamPtr;   //形参类型
        expr1Type=trueType(expr1Type);
        expr2Type=trueType(expr2Type);
        if(expr1Type->isVoid()||expr2Type->isVoid()){       //返回值为 void 的函数调用结果参与了某表达式计算
            fprintf(stderr, "The result of a function call with a return value of void participates in the calculation of an expression\n ");
            check_no_err=false;
            return;
        }
        if(expr1Type->isConstInt())
            expr1Type=TypeSystem::intType;
        else if(expr1Type->isConstFloat())
            expr1Type=TypeSystem::floatType;
        if(expr1Type!=expr2Type){
            SymbolEntry* syptr = paramPtr->getSymPtr();
            if(syptr->isConstant()){
                ConstantSymbolEntry* constSymPtr = dynamic_cast<ConstantSymbolEntry*>(syptr);
                if(expr1Type->isInt()){
                    if(expr2Type->isInt()){
                        //Exp不可解析为RelExp，因此不存在此路径
                        std::cout<<"sth wrong?5"<<std::endl;
                    }else if(expr2Type->isFloat()){
                        //在constantSymbolentry的output里进行了隐式转换
                        std::cout<<"warning: implicit conversion"
                                <<" from \'int\' to \'float\' changes value from "
                                <<constSymPtr->getValue()<<" to "
                                <<(double)constSymPtr->getValue()<<std::endl;
                        ExprNode* newConst = new Constant(new ConstantSymbolEntry(TypeSystem::floatType, (double)constSymPtr->getValue()));
                        if(prev!=nullptr){
                            prev->next = newConst;
                        }else{
                            Rparams = newConst;
                        }
                        newConst->next = paramPtr->next;
                        paramPtr = newConst;
                    }
                }else if(expr1Type->isFloat()){
                    std::cout<<"warning: implicit conversion"
                            <<" from \'float\' to \'int\' changes value from "
                            <<constSymPtr->getValue()<<" to "
                            <<(int)constSymPtr->getValue()<<std::endl;
                    ExprNode* newConst = new Constant(new ConstantSymbolEntry(TypeSystem::intType, (int)constSymPtr->getValue()));
                    if(prev!=nullptr)
                        prev->next = newConst;
                    else{
                        Rparams = newConst;
                    }
                    newConst->next = paramPtr->next;
                    paramPtr = newConst;
                }               
            }else{
                SymbolEntry *castSe = new TemporarySymbolEntry(expr2Type, SymbolTable::getLabel());
                int castOp = getCastOp(expr1Type, expr2Type);
                ExprNode *castNode;
                switch (castOp)
                {
                case UnaryExpr::ITOF:
                    std::cout<<"warning: implicit cast from int to float in symbol: "<<paramPtr->getSymPtr()->toStr()<<std::endl;
                    break;
                case UnaryExpr::FTOI:
                    std::cout<<"warning: implicit cast from float to int in symbol: "<<paramPtr->getSymPtr()->toStr()<<std::endl;
                    break;
                case UnaryExpr::BTOI:
                    std::cout<<"warning: implicit cast from bool to int in symbol: "<<paramPtr->getSymPtr()->toStr()<<std::endl;
                    break;
                case UnaryExpr::BTOF:
                    std::cout<<"warning: implicit cast from bool to float in symbol: "<<paramPtr->getSymPtr()->toStr()<<std::endl;
                    break;
                }
                if (castOp == UnaryExpr::BTOF){
                    SymbolEntry *btoiCastSe = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                    ExprNode *btoiCastNode = new UnaryExpr(btoiCastSe, UnaryExpr::BTOI, paramPtr);
                    castNode = new UnaryExpr(castSe, UnaryExpr::ITOF, btoiCastNode);
                }
                else{
                    castNode = new UnaryExpr(castSe, castOp, paramPtr);
                }
                if(prev!=nullptr){
                    prev->next = castNode;
                }else{
                    Rparams = castNode;
                }
                castNode->next = paramPtr->next;
                paramPtr=castNode;
                
            }
        }
        fparamPtr++;
        prev = paramPtr;
        paramPtr = paramPtr->next;
    }
    if(RparamNum > paramtypes.size()){
        check_no_err = false;
        fprintf(stderr, "too many Rparams in funcinvoke: %s \n", funcName.c_str());
        // exit(EXIT_FAILURE);
        return;
    }else if(RparamNum < paramtypes.size()){
        check_no_err = false;
        fprintf(stderr, "too few Rparams in funcinvoke: %s \n", funcName.c_str());
        // exit(EXIT_FAILURE);
        return;
    }    
}


void BlankStmt::typeCheck(){
    if(!check_no_err)
        return;    
    //nothing to do
}

void ExprStmt::typeCheck(){
    if(!check_no_err)
        return;   
    myExp->typeCheck();
}

void UnaryExpr::typeCheck(){
    if(!check_no_err)
        return;
    Type* exprType = expr->getSymPtr()->getType();
    if(exprType->isFunc()){
        FunctionType* exprTypeFunc=dynamic_cast<FunctionType*>(exprType);
        if(exprTypeFunc->getRetType()->isVoid()){       //返回值为 void 的函数调用结果参与了某表达式计算
             fprintf(stderr, "UnaryExpr: The result of a function call with a return value of void participates in the calculation of an expression\n ");
             check_no_err=false;
             return;
        }
    }
    if(expr->isConst()){
        float val = expr->getSymPtr()->isConstant() ? 
                ((ConstantSymbolEntry*)(expr->getSymPtr()))->getValue() : 
                ((IdentifierSymbolEntry*)(expr->getSymPtr()))->getValue();
        if(op==UMINUS){
            val=-val;
        }else if(op==NOT){
            val=!val;
        }
        SymbolEntry* val_se = new ConstantSymbolEntry(getSymPtr()->getType(),val);
        setSymPtr(val_se);
        // setOperand(new Operand(val_se));
    }
}

void ArrayInitialVal::typeCheck(){
    if(!check_no_err)
        return;
}   

double BinaryExpr::getValue(){
    // std::cout<<expr1->getValue()<<":"<<expr2->getValue()<<std::endl;
    double a,b;
    
    a=expr1->getValue();
    b=expr2->getValue();
    // std::cout<<a<<","<<b<<std::endl;
    switch(op)
    {
        case ADD:
            return a+b;
        case SUB:
            return a-b;       
        case MUL:
            return a*b;
        case DIV:
            return a/b;
        case FADD:
            return a+b;
        case FSUB:
            return a-b;
        case FMUL:
            return a*b;
        case FDIV:
            return a/b;
        case MOD:
            return (int)a % (int)b;
        case AND:
            return a && b;
        case OR:
            return a || b;
        case LESS:
            return a < b;
        case GREATEREQUAL:
            return a >= b;
        case LESSEQUAL:
            return a <= b;
        case EQUAL:
            return a == b;
        case NOTEQUAL:
            return a != b;
        case GREATER:
            return a > b;
    }
}

double UnaryExpr::getValue(){
    double val = expr->getValue();
    if(op == UMINUS){
        val = -val;
    }else if(op == NOT){
        val = !val;
    }
    return val;
}

double Id::getValue(){
    IdentifierSymbolEntry* idSe = dynamic_cast<IdentifierSymbolEntry*>(getSymPtr());
    if(idSe->getType()->isConstFloat()||idSe->getType()->isConstInt()){
        return idSe->getValue();
    }else{
        fprintf(stderr, "Unconst identifier can't be used as const!");
        exit(EXIT_FAILURE) ;
        return -999;  
    }
}


double Constant::getValue(){
    if(getSymPtr()->isConstant()){
        ConstantSymbolEntry* ConSe = dynamic_cast<ConstantSymbolEntry*>(getSymPtr());
        return ConSe->getValue();
    }else{
        fprintf(stderr,"Constant Symbolentry doesn't have constant value!");
        exit(EXIT_FAILURE);
        return -999;
    }
}

void BinaryExpr::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case ADD:
            op_str = "add";
            break;
        case SUB:
            op_str = "sub";
            break;
        case AND:
            op_str = "and";
            break;
        case OR:
            op_str = "or";
            break;
        case LESS:
            op_str = "less";
            break;
        case LESSEQUAL:
            op_str = "lessequal";
            break;
        case GREATER:
            op_str = "greater";
            break;
        case GREATEREQUAL:
            op_str = "greaterequal";
            break;
        case MUL:
            op_str = "mul";
            break;
        case DIV:
            op_str = "div";
            break;
        case FDIV:
            op_str = "fdiv";
            break;
        case FMUL:
            op_str = "fmul";
            break;
        case FADD:
            op_str = "fadd";
            break;
        case FSUB:
            op_str = "fsub";
            break;
        case MOD:
            op_str = "mod";
            break;
        case EQUAL:
            op_str = "equal";
            break;
        case NOTEQUAL:
            op_str = "notequal";
            break;
    }
    fprintf(yyout, "%*cBinaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr1->output(level + 4);
    expr2->output(level + 4);
}

void Ast::output()
{
    fprintf(yyout, "program\n");
    if(root != nullptr)
        root->output(4);
}

//2022年11月3日20:40:13 zsr add
void UnaryExpr::output(int level){
    std::string op_str;
    switch (op)
    {
    case UMINUS:
        op_str = "uminus";
        break;
    case UADD:
        op_str = "uadd";
        break;
    case NOT:
        op_str = "not";
        break;
    case BTOI:
        op_str = "Zext";
        break;
    case ITOF:
        op_str = "ITOF";
        break;
    case FTOI:
        op_str = "FTOI";
        break;
    }
    fprintf(yyout, "%*cUnaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr->output(level + 4);
}

void Constant::output(int level)
{
    std::string type, value;
    type = symbolEntry->getType()->toStr();
    value = symbolEntry->toStr();
    fprintf(yyout, "%*cIntegerLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
}

void Id::output(int level)
{
    std::string name, type;
    int scope;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
    if(arrayIndex == nullptr)
        fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\n", level, ' ',
            name.c_str(), scope, type.c_str());
    else
    {
        fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s \n", level, ' ',
            name.c_str(), scope, type.c_str());
        ExprNode* ptr = arrayIndex;
        while(ptr){
            fprintf(yyout, "%*cArrayIndex: ",level+4, ' ');ptr->output(0);
            ptr= ptr->next;
        }
    }
}

void CompoundStmt::output(int level)
{
    fprintf(yyout, "%*cCompoundStmt\n", level, ' ');
    if (stmt)
        stmt->output(level + 4);
}

void SeqNode::output(int level)
{
    // fprintf(yyout, "%*cSequence\n", level, ' ');
    stmt1->output(level);
    stmt2->output(level);
}

void IfStmt::output(int level)
{
    fprintf(yyout, "%*cIfStmt\n", level, ' ');
    fprintf(yyout, "%*cCondExpr\n", level + 4, ' ');
    cond->output(level + 8);
    fprintf(yyout, "%*cThenStmt\n", level + 4, ' ');
    thenStmt->output(level + 8);
}

void IfElseStmt::output(int level)
{
    fprintf(yyout, "%*cIfElseStmt\n", level, ' ');
    fprintf(yyout, "%*cCondExpr\n", level + 4, ' ');
    cond->output(level + 8);
    fprintf(yyout, "%*cThenStmt\n", level + 4, ' ');
    thenStmt->output(level + 8);
    fprintf(yyout, "%*cElseStmt\n", level + 4, ' ');
    elseStmt->output(level + 8);
}

void ReturnStmt::output(int level)
{
    fprintf(yyout, "%*cReturnStmt\n", level, ' ');
    if(retValue != nullptr)
        retValue->output(level + 4);
}

void AssignStmt::output(int level)
{
    fprintf(yyout, "%*cAssignStmt\n", level, ' ');
    lval->output(level + 4);
    expr->output(level + 4);
}

void FunctionDef::output(int level)
{
    std::string name, type;
    name = se->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "%*cFunctionDefine function name: %s, type: %s FuncParams:\n", level, ' ', 
            name.c_str(), type.c_str());
    if (funcPDef)
    {
        funcPDef->output(level + 4);
    }else{
        fprintf(yyout, "%*cno params\n", level+4, ' ');
    }
    fprintf(yyout, "%*cFunctionBody:\n", level, ' ');
    stmt->output(level + 4);
}

//2022年11月3日17:22:26 add by zsr
void WhileStmt::output(int level)
{
    fprintf(yyout, "%*cWhileStmt\n", level, ' ');
    fprintf(yyout, "%*cCondExpr\n", level + 4, ' ');
    cond->output(level + 8);
    fprintf(yyout, "%*cLoopStmt\n", level + 4, ' ');
    loopStmt->output(level + 8);
}

void BreakStmt::output(int level)
{
    fprintf(yyout, "%*cBreakStmt\n", level, ' ');
}

void ContinueStmt::output(int level)
{
    fprintf(yyout, "%*cContinueStmt\n", level, ' ');
}

//2022年11月4日15:02:42 add by zsr
void ConstDef::output(int level){
    // TODO: wait to add
    fprintf(yyout, "%*cConstDef %s",  level, ' ', this->id_str.c_str());
    if(index.size()!=0){
        fprintf(yyout, "  ArrayDimension:");
        for(int i=0; i<index.size(); i++){
            fprintf(yyout,"[%d]", index[i]);
        }
    }
    if(Val)
    {
        fprintf(yyout, "  initVal:\n");
        Val->output(level+4);
    }
    else
        fprintf(yyout, "  no initVal\n");
    if (next)
    {
        next->output(level);
    }
}

void ConstDeclStmt::output(int level){
    fprintf(yyout, "%*cConstDeclStmt Type: %s\n", level, ' ', type->toStr().c_str());
    this->defHead->output(level + 4);
}

void VarDef::output(int level){
    fprintf(yyout, "%*cVarDef %s",  level, ' ', this->id_str.c_str());
    if(index.size()!=0){
        fprintf(yyout, "  ArrayDimension:");
        for(int i=0; i<index.size(); i++){
            fprintf(yyout,"[%d]", index[i]);
        }
    }
    if(Val)
    {
        fprintf(yyout, "  initVal:\n");
        Val->output(level+4);
    }
    else
        fprintf(yyout, "  no initVal\n");
    if (next)
    {
        next->output(level);
    }
}

void VarDeclStmt::output(int level){
    fprintf(yyout, "%*cVarDeclStmt Type: %s\n", level, ' ', type->toStr().c_str());
    this->defHead->output(level + 4);
}

void FuncFParam::output(int level){
    if(this->nameStr == "")
        fprintf(yyout, "%*cFuncFParam %s\n", level, ' ', this->paraType->toStr().c_str());
    else
        fprintf(yyout, "%*cFuncFParam %s %s\n",  level, ' ', this->paraType->toStr().c_str(), this->nameStr.c_str());      
    if (next)
    {
        next->output(level);
    }
}

void FuncInvoke::output(int level){
    fprintf(yyout, "%*cFuncInvoke FuncName: %s FuncRParams:\n",  level, ' ', this->funcName.c_str());
    if(this->Rparams)
    {
        ExprNode* p=this->Rparams;
        while (p)
        {
            p->output(level + 4);
            p=p->next;
        }
    }
    else
        fprintf(yyout, "%*cNo RParams\n",level+4, ' ');
}

void BlankStmt::output(int level){
    fprintf(yyout, "%*cBlankStmt \n",  level, ' ');
}

void ExprStmt::output(int level){
    fprintf(yyout, "%*cExprStmt \n",  level, ' ');
    this->myExp->output(level + 4);
}

void NullExpr::output(int level){
    fprintf(yyout, "%*cNullExpr \n",  level, ' ');
}

void ArrayInitialVal::output(int level){
    if(value){
        fprintf(yyout, "%*cvalue Level: %d\n", level, ' ',Level);
        value->output(level);
    }else{
        fprintf(yyout, "%*cArrayInitialVal Level: %d\n", level, ' ',Level);
        for(auto it = children.begin();it != children.end(); it++){
            (*it)->output(level +4);
        }
    }
}

void ArrayInitialVal::adjustTreeToNormal(){
    if(value){
        ArrayInitialVal* val = dynamic_cast<ArrayInitialVal*>(value);
        if(val){
            isLeaf = false;
            initialized = false;
            val->adjustTreeToNormal();
            ArrayInitialVal* parentTure = dynamic_cast<ArrayInitialVal*>(parent);
            parentTure->replaceChild(this, val);
            val->setParent(parentTure);
        }else{
            setLevel(0);
        }
    }
    if(children.size()>0){
        for(int i=0; i<children.size(); i++){
            ArrayInitialVal* trueChild = dynamic_cast<ArrayInitialVal*>(children[i]);
            if(trueChild == nullptr){
                fprintf(yyout, "adjusterror!");
            }
            else{
                trueChild->adjustTreeToNormal();
            }
        }
    }
}

void ArrayInitialVal::addZeroNode(std::vector<int> arrayIndexDefine,Type*zeroType){
    if(Level>0){
        for(int i=0;i<children.size();i++){
            ArrayInitialVal* nowAddVal = dynamic_cast<ArrayInitialVal*>(children[i]);
            // nowAddVal->setLevel
            nowAddVal->addZeroNode(arrayIndexDefine,zeroType);
        }
        int childLimit = arrayIndexDefine[arrayIndexDefine.size()-Level];
        int addZeroNum=childLimit-children.size();
        if(addZeroNum>0){
            for(int i=0;i<addZeroNum;i++){
                ArrayInitialVal* zeroNode = zeroValTree(Level-1,arrayIndexDefine,zeroType);        //type?
                this->addChild(zeroNode);
                zeroNode->setParent(this);
            }
        }
    }
}

ExprNode* ArrayInitialVal::getValWithIndex(std::vector<int> indexVec){
    ArrayInitialVal* valPtr = this;
    for (size_t i = 0; i < indexVec.size(); i++)
    {
        valPtr = ((ArrayInitialVal*)(valPtr->getChildren().at(indexVec[i])));
    }
    return valPtr->getVal();
}

void ArrayInitialVal::packSubtree(std::vector<int> arrayIndexDefine,Type*zeroType){
    std::vector<ExprNode *> resetVec = children;
    int minLevel = 0;
    int limit;
    while (minLevel < arrayIndexDefine.size()-1){
        std::vector<ExprNode *> midVec;
        limit = arrayIndexDefine[arrayIndexDefine.size() - minLevel - 1];
        for (int i = 0; i < resetVec.size(); i++){
            ArrayInitialVal *nowCheckVal = dynamic_cast<ArrayInitialVal *>(resetVec[i]);
            if (nowCheckVal->getLevel() == minLevel){
                ArrayInitialVal *parentNode = new ArrayInitialVal();
                parentNode->setLevel(minLevel + 1);
                int j;
                for (j = i; j < i + limit; j++){
                    if (j < resetVec.size()){
                        ArrayInitialVal *continueVal = dynamic_cast<ArrayInitialVal *>(resetVec[j]);
                        if (continueVal->getLevel() == minLevel){
                            std::vector<int> subIndex(arrayIndexDefine.end()-minLevel,arrayIndexDefine.end());
                            continueVal->testPackTree(subIndex,zeroType);
                            continueVal->setChecked();

                            parentNode->addChild(continueVal);
                            continueVal->setParent(parentNode);
                        }else{
                            fprintf(stderr, "Array initialization error.\n");
                            exit(EXIT_FAILURE);
                        }
                    }else{
                        parentNode->addZeroNode(arrayIndexDefine,zeroType);
                    }
                }
                i = j - 1;
                midVec.push_back(parentNode);
            }else{
                midVec.push_back(nowCheckVal);
            }
        }
        resetVec.swap(midVec);
        minLevel++;
    }
    children=resetVec;
    this->addZeroNode(arrayIndexDefine,zeroType);
}
void ArrayInitialVal::testPackTree(std::vector<int> arrayIndexDefine,Type*zeroType){
    bool needPack = false;
    if (children.size() > 0){
        for (int i = 0; i < children.size(); i++){
            ArrayInitialVal *childVal = dynamic_cast<ArrayInitialVal *>(children[i]);
            if (childVal->getLevel() < Level - 1){
                needPack = true;
            }else if((childVal->getLevel()==Level-1)&&(!childVal->isChecked())){
                std::vector<int> subIndex(arrayIndexDefine.end()-Level+1,arrayIndexDefine.end());
                childVal->testPackTree(subIndex,zeroType);
            }
        }
    }else if(children.size()==0&&Level>0){
        this->addZeroNode(arrayIndexDefine,zeroType);
    }
    if (needPack){
        this->packSubtree(arrayIndexDefine,zeroType);
    }else{
        addZeroNode(arrayIndexDefine,zeroType);
    }
}

void ArrayInitialVal::setRecurseLevel(int level){
    if(Level == 0){
        if(children.size()==0&&!value)//判断是空节点
            Level = level;
        else if(value)
            return;
    }
    Level = level;
    for(int i=0;i< children.size();i++){
        ((ArrayInitialVal*)children[i])->setRecurseLevel(level-1);
    }
}

std::string ArrayInitialVal::toStr(Type* nowType){
    std::stringstream buffer;
    if(children.size()==0){
        buffer<<value->getValue();
        return buffer.str();
    }
    ArrayType* type = dynamic_cast<ArrayType*>(nowType);
    Type* lowerType = type->getLowerType();
    
    buffer<<"[";
    for(int i=0;i<children.size();i++){
        buffer<<lowerType->toStr()<<" "<<((ArrayInitialVal*)children[i])->toStr(lowerType);
        if(i!=children.size()-1)
            buffer<<", ";
    }
    buffer<<"]";
    return buffer.str();
}

void ArrayInitialVal::checkTreeType(Type* targetType){
    if(Level == 0){
        //开始check
        Type* valType = value->getSymPtr()->getType();
        valType = trueType(valType);
        if(valType->isConstInt())
            valType=TypeSystem::intType;
        else if(valType->isConstFloat())
            valType=TypeSystem::floatType;
        if(valType != targetType){
            if(value->getSymPtr()->isConstant()){
                ConstantSymbolEntry *constSymPtr = dynamic_cast<ConstantSymbolEntry *>(value->getSymPtr());
                if (targetType->isInt()){
                    if (valType->isInt()){
                        // Exp不可解析为RelExp，因此不存在此路径
                        std::cout << "sth wrong?1" << std::endl;
                    }else if (valType->isFloat()){
                       // 在constantSymbolentry的output里进行了隐式转换
                        std::cout << "warning: implicit conversion"
                                << " from \'float\' to \'int\' changes value from "
                                << constSymPtr->getValue() << " to "
                                << (int)constSymPtr->getValue() << std::endl;
                        ExprNode *newVal = new Constant(new ConstantSymbolEntry(TypeSystem::intType, (int)constSymPtr->getValue()));
                        value = newVal;
                    }
                }
                else if (targetType->isFloat()){
                    std::cout << "warning: implicit conversion"
                            << " from \'int\' to \'float\' changes value from "
                            << constSymPtr->getValue() << " to "
                            << (double)constSymPtr->getValue() << std::endl;
                    ExprNode *newVal = new Constant(new ConstantSymbolEntry(TypeSystem::floatType, (double)constSymPtr->getValue()));
                    value = newVal;
                }
            }else{
                if(valType != targetType){
                    SymbolEntry *castSe = new TemporarySymbolEntry(valType, SymbolTable::getLabel());
                    int castOp = getCastOp(valType,targetType);
                    switch (castOp)
                    {
                    case UnaryExpr::ITOF:
                        std::cout<<"warning: implicit cast from int to float in symbol: "<<value->getSymPtr()->toStr()<<std::endl;
                        break;
                    case UnaryExpr::FTOI:
                        std::cout<<"warning: implicit cast from float to int in symbol: "<<value->getSymPtr()->toStr()<<std::endl;
                        break;
                    case UnaryExpr::BTOI:
                        std::cout<<"warning: implicit cast from bool to int in symbol: "<<value->getSymPtr()->toStr()<<std::endl;
                        break;
                    case UnaryExpr::BTOF:
                        std::cout<<"warning: implicit cast from bool to float in symbol: "<<value->getSymPtr()->toStr()<<std::endl;
                        break;
                    }
                    ExprNode *castNode;
                    if (castOp == UnaryExpr::BTOF){
                        SymbolEntry *btoiCastSe = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                        ExprNode *btoiCastNode = new UnaryExpr(btoiCastSe, UnaryExpr::BTOI, value);
                        castNode = new UnaryExpr(castSe, UnaryExpr::ITOF, btoiCastNode);
                    }
                    else{
                        castNode = new UnaryExpr(castSe, castOp, value);
                    }
                    value = castNode;
                }
            }
        }
    }else{
        for(int i=0;i<children.size();i++){
            ((ArrayInitialVal*)children[i])->checkTreeType(targetType);
        }
    }
}

void ArrayInitialVal::toVector(std::vector<double>* valueVec){
    if(Level==0&&value){
        double val=value->getValue();
        valueVec->push_back(val);
    }else{
        for(int i=0;i<children.size();i++){
            ArrayInitialVal *childVal = dynamic_cast<ArrayInitialVal *>(children[i]);
            childVal->toVector(valueVec);
        }
    }
}

void SetBinaryType(SymbolEntry*& se, Type* expr1Type, Type* expr2Type){
    expr1Type=trueType(expr1Type);
    expr2Type=trueType(expr2Type);
    if(expr1Type->isInt() && expr2Type->isInt()){
        se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
    }else{
        se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
    }
}

Type* maxType(Type* tempType,Type* type2){
    if(tempType->isFloat()||type2->isFloat()){     //还没管constfloat
        return TypeSystem::floatType;
    }else{      
        if(tempType->isConstInt()||type2->isConstInt()){
            return TypeSystem::intType;
        }else{
            IntType* intType1=dynamic_cast<IntType*>(tempType);
            IntType* intType2=dynamic_cast<IntType*>(type2);
            if(intType1->isBool()&&intType2->isBool()){
                return TypeSystem::boolType;
            }
            return TypeSystem::intType;
        }
    }
}

int getCastOp(Type* srcType, Type* castType){
    if(srcType->isInt()){
        if(srcType->isConstInt()){
            return UnaryExpr::ITOF;
        }
        IntType* intType=dynamic_cast<IntType*>(srcType);
        if(intType->isBool()){
            if(castType->isInt()){
                return UnaryExpr::BTOI;     
            }else if(castType->isFloat()){
                return UnaryExpr::BTOF;
            }
        }else{
            if(castType->isFloat()){
                return UnaryExpr::ITOF;
            }
        }
    }else if(srcType->isFloat()){
        if(castType->isInt()){
            return UnaryExpr::FTOI;
        }
    }
    return -1;
}

ArrayInitialVal* zeroValTree(int level, std::vector<int> arrayIndexDefine,Type*zeroType){
    if(level==0){
        SymbolEntry *se = new ConstantSymbolEntry(zeroType, 0);
        ExprNode* zeroVal = new Constant(se);
        ArrayInitialVal* zeroNode = new ArrayInitialVal(zeroVal); 
        return zeroNode;
    }else{
        ArrayInitialVal* zeroTreeRoot = new ArrayInitialVal();
        zeroTreeRoot->setLevel(level);
        int childNum = arrayIndexDefine[arrayIndexDefine.size()-level];
        for(int i=0;i<childNum;i++){
            ArrayInitialVal* childNode = zeroValTree(level-1,arrayIndexDefine,zeroType);
            zeroTreeRoot->addChild(childNode);
            childNode->setParent(zeroTreeRoot);
        }
        return zeroTreeRoot;
    }
}

Type* trueType(Type* exprType){
    if(exprType->isPTR()){
        PointerType* exprTypePtr=dynamic_cast<PointerType*>(exprType);
        exprType=exprTypePtr->getValueType();
    }
    if(exprType->isFunc()){
        FunctionType* exprTypeFunc=dynamic_cast<FunctionType*>(exprType);
        exprType=exprTypeFunc->getRetType();
    }else if(exprType->isArray()){         //待定
        ArrayType* exprTypeArr=dynamic_cast<ArrayType*>(exprType);
        exprType=exprTypeArr->getElementType();
    }
    // if(exprType->isConstFloat()){
    //     exprType =TypeSystem::floatType;        //可能导致后续有问题
    // }else if(exprType->isConstInt()){
    //     exprType =TypeSystem::intType;
    // }
    return exprType;   
}
