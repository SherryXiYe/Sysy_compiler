#include "Unit.h"
#include <sstream>
union HEXDOUBLE {
    double num;
    unsigned char bnum[8];
};
void Unit::insertFunc(Function *f)
{
    func_list.push_back(f);
}

void Unit::insertDeclare(SymbolEntry *se)
{
    bool insert = true;
    for(auto i = declare_list.begin();i != declare_list.end();i++){
        if((*i)->toStr() == se->toStr())
            insert = false;
    }
    if(insert)
        declare_list.push_back(se);
}

void Unit::removeFunc(Function *func)
{
    auto it = std::find(func_list.begin(), func_list.end(), func);
    if(it != func_list.end())
        func_list.erase(it);
}

void Unit::output() const
{
    
    for(auto & se : global_list){    
        if(se->getType()->isInt())
            fprintf(yyout, "%s = global %s %d, align 4\n", se->toStr().c_str(),se->getType()->toStr().c_str(),(int)((IdentifierSymbolEntry*)se)->getValue());
        else if(se->getType()->isFloat()){
            std::stringstream buffer;
            double val = ((IdentifierSymbolEntry*)se)->getValue();
            union HEXDOUBLE out_val;
            out_val.num = val;
            char sBuffer[40];
            buffer<<"0x";
            for (int i = 0; i < 8; ++i) //大端模式顺着来0-8，小端模式逆着来8-0
            {
                if(out_val.bnum[8 - i - 1] >= 0x10){
                    sprintf(sBuffer, "%x", out_val.bnum[8 - i - 1]);
                    buffer<<sBuffer;
                }
                else{
                    sprintf(sBuffer, "0%x", out_val.bnum[8 - i - 1]);
                    buffer<<sBuffer;
                }
            }
            fprintf(yyout, "%s = global %s %s, align 4\n", se->toStr().c_str(),se->getType()->toStr().c_str(),buffer.str().c_str());
        }else if(se->getType()->isArray()){
            std::stringstream buffer;
            IdentifierSymbolEntry* idSe = dynamic_cast<IdentifierSymbolEntry*>(se);
            if(idSe->getArrayValRoot()){
                fprintf(yyout, "%s = global %s %s, align 16\n", 
                se->toStr().c_str(),
                idSe->getType()->toStr().c_str(),
                idSe->getArrayValRoot()->toStr(idSe->getType()).c_str());
            }else{
                fprintf(yyout, "%s = global %s zeroinitializer, align 16\n", 
                se->toStr().c_str(),
                idSe->getType()->toStr().c_str())
                ;
            }
        }
    }
    for (auto &func : func_list)
        func->output();
    for (auto se : declare_list) {
        FunctionType* type = (FunctionType*)(se->getType());
        std::string str = type->toStr();
        std::string name = str.substr(0, str.find('('));
        std::string param = str.substr(str.find('('));
        fprintf(yyout, "declare %s %s%s\n", type->getRetType()->toStr().c_str(),
                se->toStr().c_str(), param.c_str());
    }
}

void Unit::genMachineCode(MachineUnit* munit) 
{
    AsmBuilder* builder = new AsmBuilder();
    for(int i=0; i<global_list.size();i++){
        auto idSe = global_list[i];
        munit->addGlobalList(idSe);
    }
    builder->setUnit(munit);
    for (auto &func : func_list)
        func->genMachineCode(builder);
}

Unit::~Unit()
{
    std::set<std::string> v;
    try{
        while(!func_list.empty()){
            // outputList();
            auto func = func_list.back();
            func_list.pop_back();
            delete func;
        }
    }catch(std::bad_alloc& ba){
        std::cout<<ba.what()<<std::endl;
    }

}

void Unit::insertGlob(SymbolEntry *se){
    global_list.push_back(se);
}
