#include "BasicBlock.h"
#include "Function.h"
#include <algorithm>

extern FILE* yyout;

// insert the instruction to the front of the basicblock.
void BasicBlock::insertFront(Instruction *inst)
{
    insertBefore(inst, head->getNext());
}

// insert the instruction to the back of the basicblock.
void BasicBlock::insertBack(Instruction *inst) 
{
    insertBefore(inst, head);
}

// insert the instruction dst before src.
void BasicBlock::insertBefore(Instruction *dst, Instruction *src)
{
    // Todo 
    // zsr add 2022年12月4日21:13:03
    dst->setPrev(src->getPrev());
    src->getPrev()->setNext(dst);
    src->setPrev(dst);
    dst->setNext(src);
    dst->setParent(this);
}

// remove the instruction from intruction list. 注意是没有delete inst的.
void BasicBlock::remove(Instruction *inst)
{
    inst->getPrev()->setNext(inst->getNext());
    inst->getNext()->setPrev(inst->getPrev());
}

void BasicBlock::removeAfter(Instruction *inst)
{
    inst = inst->getNext();
    while (inst != head)
    {
        Instruction *t;
        t = inst;
        inst = inst->getNext();
        remove(t);
    }
}

void BasicBlock::output() const
{
    fprintf(yyout, "B%d:", no);

    if (!pred.empty())
    {
        fprintf(yyout, "%*c; preds = %%B%d", 32, '\t', pred[0]->getNo());
        for (auto i = pred.begin() + 1; i != pred.end(); i++)
            fprintf(yyout, ", %%B%d", (*i)->getNo());
    }
    fprintf(yyout, "\n");
    for (auto i = head->getNext(); i != head; i = i->getNext())
        i->output();
}

void BasicBlock::addSucc(BasicBlock *bb)
{
    succ.push_back(bb);
}

// remove the successor basicclock bb.
void BasicBlock::removeSucc(BasicBlock *bb)
{
    auto it = std::find(succ.begin(), succ.end(), bb);
    if(it != succ.end())
        succ.erase(it);
}

void BasicBlock::cleanSucc(){
    while(!succ.empty()){
        succ.pop_back();
    }
}

void BasicBlock::addPred(BasicBlock *bb)
{
    pred.push_back(bb);
}

// remove the predecessor basicblock bb.
void BasicBlock::removePred(BasicBlock *bb)
{
    auto it = std::find(pred.begin(), pred.end(), bb);
    if(it != pred.end())
        pred.erase(it);
}

void BasicBlock::genMachineCode(AsmBuilder* builder) 
{
    auto cur_func = builder->getFunction();
    auto cur_block = new MachineBlock(cur_func, no);
    builder->setBlock(cur_block);
    for (auto i = head->getNext(); i != head; i = i->getNext())
    {
        i->genMachineCode(builder);
    }
    cur_func->InsertBlock(cur_block);
}

BasicBlock::BasicBlock(Function *f)
{
    this->no = SymbolTable::getLabel();     //和TemporarySymbolEntry的label一起按序增加。
    f->insertBlock(this);       //注意，在创建一个BasicBlock的时候，这个BasicBlock会被push_back进f的block_list
    parent = f;
    head = new DummyInstruction();
    head->setParent(this);
}

BasicBlock::~BasicBlock()
{
    Instruction *inst;
    inst = head->getNext();
    while (inst != head)
    {
        Instruction *t;
        t = inst;
        inst = inst->getNext();
        delete t;
    }
    // std::cout<<"insok"<<std::endl;
    if(!predEmpty()){
        for(auto &bb:pred)
            bb->removeSucc(this);
    }

    // std::cout<<"predok"<<std::endl;
    if(!succEmpty())
    for(auto &bb:succ)
        bb->removePred(this);
    parent->remove(this);
}
