#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include <list>

extern FILE* yyout;

Function::Function(Unit *u, SymbolEntry *s)
{
    u->insertFunc(this);
    entry = new BasicBlock(this);   //entry是function的block_list第一个BasicBlock（因为在创建一个BasicBlock的时候，这个BasicBlock会被push_back进f的block_list）
    sym_ptr = s;
    parent = u;
    params = std::vector<SymbolEntry*>();
}

Function::~Function()
{
    
    auto delete_list = block_list;
    for (auto &i : delete_list){
        delete i;
    }
    parent->removeFunc(this);
}

// remove the basicblock bb from its block_list.
void Function::remove(BasicBlock *bb)
{
    block_list.erase(std::find(block_list.begin(), block_list.end(), bb));
}

void Function::output() const
{
    FunctionType* funcType = dynamic_cast<FunctionType*>(sym_ptr->getType());
    Type *retType = funcType->getRetType();
    fprintf(yyout, "define %s %s(", retType->toStr().c_str(), sym_ptr->toStr().c_str());
    if(!params.empty()){
        auto paramPtr = params.begin();
        while(paramPtr != params.end()){
            fprintf(yyout, "%s %s", (*paramPtr)->getType()->toStr().c_str(), (*paramPtr)->toStr().c_str());
            paramPtr++;
            if(paramPtr != params.end())
                fprintf(yyout, ", ");
        }
    }
    fprintf(yyout, ") {\n");
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);         
    v.insert(entry);
    while (!q.empty())
    {
        auto bb = q.front();
        q.pop_front();
        bb->output();
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())       //如果没找到（因为如果没找到，find的结果就是最后一个元素的后一个，即v.end）
            {
                v.insert(*succ);
                q.push_back(*succ);
            }
        }
    }
    fprintf(yyout, "}\n");
}
