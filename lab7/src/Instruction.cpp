#include "Instruction.h"
#include "BasicBlock.h"
#include <iostream>
#include "Function.h"
#include "Type.h"
extern FILE* yyout;

Instruction::Instruction(unsigned instType, BasicBlock *insert_bb)
{
    prev = next = this;
    opcode = -1;
    this->instType = instType;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

Instruction::~Instruction()
{
    parent->remove(this);
}

BasicBlock *Instruction::getParent()
{
    return parent;
}

void Instruction::setParent(BasicBlock *bb)
{
    parent = bb;
}

void Instruction::setNext(Instruction *inst)
{
    next = inst;
}

void Instruction::setPrev(Instruction *inst)
{
    prev = inst;
}

Instruction *Instruction::getNext()
{
    return next;
}

Instruction *Instruction::getPrev()
{
    return prev;
}

BinaryInstruction::BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(BINARY, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);        //operands[0],dst
    operands.push_back(src1);       
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

BinaryInstruction::~BinaryInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void BinaryInstruction::output() const      //需要添加
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[0]->getType()->toStr();
    switch (opcode)//需要提前把opcode设置好，判断操作数为什么类型
    {
    case AND:
        op = "and";
        break;
    case OR:
        op = "or";
        break;
    case ADD:
        op = "add";
        break;
    case SUB:
        op = "sub";
        break;
    case MUL:
        op = "mul";
        break;
    case DIV:
        op = "sdiv";
        break;
    case FADD:
        op = "fadd";
        break;
    case FSUB:
        op = "fsub";
        break;
    case FMUL:
        op = "fmul";
        break;
    case FDIV:
        op = "fdiv";
        break;
    case MOD:
        op = "srem";
        break;
    default:
        break;
    }
    fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

CmpInstruction::CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb): Instruction(CMP, insert_bb){
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

CmpInstruction::~CmpInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void CmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
    switch (opcode)
    {
    case E:
        op = "eq";
        break;
    case NE:
        op = "ne";
        break;
    case L:
        op = "slt";
        break;
    case LE:
        op = "sle";
        break;
    case G:
        op = "sgt";
        break;
    case GE:
        op = "sge";
        break;
    default:
        op = "";
        break;
    }

    fprintf(yyout, "  %s = icmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}


FCmpInstruction::FCmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb): Instruction(CMP, insert_bb){
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

FCmpInstruction::~FCmpInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void FCmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
    switch (opcode)//以后记得加fcmp
    {
    case E:
        op = "oeq";
        break;
    case NE:
        op = "one";
        break;
    case L:
        op = "olt";
        break;
    case LE:
        op = "ole";
        break;
    case G:
        op = "ogt";
        break;
    case GE:
        op = "oge";
        break;
    default:
        op = "";
        break;
    }
    
    fprintf(yyout, "  %s = fcmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

UncondBrInstruction::UncondBrInstruction(BasicBlock *to, BasicBlock *insert_bb) : Instruction(UNCOND, insert_bb)
{
    branch = to;
}

void UncondBrInstruction::output() const
{
    fprintf(yyout, "  br label %%B%d\n", branch->getNo());
}

void UncondBrInstruction::setBranch(BasicBlock *bb)
{
    branch = bb;
}

BasicBlock *UncondBrInstruction::getBranch()
{
    return branch;
}

CondBrInstruction::CondBrInstruction(BasicBlock*true_branch, BasicBlock*false_branch, Operand *cond, BasicBlock *insert_bb) : Instruction(COND, insert_bb){
    this->true_branch = true_branch;
    this->false_branch = false_branch;
    cond->addUse(this);
    operands.push_back(cond);
}

CondBrInstruction::~CondBrInstruction()
{
    operands[0]->removeUse(this);
}

void CondBrInstruction::output() const
{
    std::string cond, type;
    cond = operands[0]->toStr();
    type = operands[0]->getType()->toStr();
    int true_label = true_branch->getNo();
    int false_label = false_branch->getNo();
    fprintf(yyout, "  br %s %s, label %%B%d, label %%B%d\n", type.c_str(), cond.c_str(), true_label, false_label);
}

void CondBrInstruction::setFalseBranch(BasicBlock *bb)
{
    false_branch = bb;
}

BasicBlock *CondBrInstruction::getFalseBranch()
{
    return false_branch;
}

void CondBrInstruction::setTrueBranch(BasicBlock *bb)
{
    true_branch = bb;
}

BasicBlock *CondBrInstruction::getTrueBranch()
{
    return true_branch;
}

RetInstruction::RetInstruction(Operand *src, BasicBlock *insert_bb) : Instruction(RET, insert_bb)
{
    if(src != nullptr)          //注意src可能为空，return;
    {
        operands.push_back(src);
        src->addUse(this);
    }
}

RetInstruction::~RetInstruction()
{
    if(!operands.empty())
        operands[0]->removeUse(this);
}

void RetInstruction::output() const
{
    if(operands.empty())
    {
        fprintf(yyout, "  ret void\n");
    }
    else
    {
        std::string ret, type;
        ret = operands[0]->toStr();
        type = operands[0]->getType()->toStr();
        fprintf(yyout, "  ret %s %s\n", type.c_str(), ret.c_str());
    }
}

AllocaInstruction::AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb) : Instruction(ALLOCA, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    point2se = nullptr;
    this->se = se;
}

AllocaInstruction::~AllocaInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)    //没用了再删
        delete operands[0];
}

void AllocaInstruction::output() const
{
    std::string dst, type;
    dst = operands[0]->toStr();
    type = se->getType()->toStr();
    if(se->getType()->isArray())
        fprintf(yyout, "  %s = alloca %s, align 16\n", dst.c_str(), type.c_str());
    else        
        fprintf(yyout, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
}

LoadInstruction::LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb) : Instruction(LOAD, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src_addr);
    dst->setDef(this);
    src_addr->addUse(this);
}

LoadInstruction::~LoadInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void LoadInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string src_type;
    std::string dst_type;
    dst_type = operands[0]->getType()->toStr();
    src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = load %s, %s %s, align 4\n", dst.c_str(), dst_type.c_str(), src_type.c_str(), src.c_str());
}

StoreInstruction::StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb) : Instruction(STORE, insert_bb)
{
    operands.push_back(dst_addr);
    operands.push_back(src);
    dst_addr->addUse(this);
    src->addUse(this);
}

StoreInstruction::~StoreInstruction()
{
    operands[0]->removeUse(this);
    operands[1]->removeUse(this);
}

void StoreInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();

    fprintf(yyout, "  store %s %s, %s %s, align 4\n", src_type.c_str(), src.c_str(), dst_type.c_str(), dst.c_str());
}


CallInstruction::CallInstruction(Operand* dst,SymbolEntry* func,std::vector<Operand*> params,BasicBlock* insert_bb)
    : Instruction(CALL, insert_bb), func(func) {
    operands.push_back(dst);
    if (dst != nullptr)
        dst->setDef(this);
    for (auto paramPtr = params.begin(); paramPtr != params.end(); paramPtr++) {
        operands.push_back(*paramPtr);
        (*paramPtr)->addUse(this);
    }
}

CallInstruction::~CallInstruction(){
    for(auto OperandPtr= operands.begin();OperandPtr != operands.end();OperandPtr++){
        if(OperandPtr== operands.begin())
            continue;
        (*OperandPtr)->removeUse(this);
    }
    if(operands[0]){
        operands[0]->setDef(nullptr);
        if (operands[0]->usersNum() == 0)
            delete operands[0]; 
    }
}

void CallInstruction::output() const {
    fprintf(yyout, "  ");
    if (operands[0] != nullptr)
        fprintf(yyout, "%s = ", operands[0]->toStr().c_str());
    FunctionType* type = (FunctionType*)(func->getType());
    fprintf(yyout, "call %s %s(", type->getRetType()->toStr().c_str(),func->toStr().c_str());
    for (int i = 1; i < operands.size(); i++) {
        if (i != 1)
            fprintf(yyout, ", ");
        fprintf(yyout, "%s %s", operands[i]->getType()->toStr().c_str(),operands[i]->toStr().c_str());
    }
    fprintf(yyout, ")\n");
}

ZextInstruction::ZextInstruction(Operand* dst,Operand* src, BasicBlock* insert_bb): Instruction(ZEXT, insert_bb) {
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

void ZextInstruction::output() const {
    Operand* dst = operands[0];
    Operand* src = operands[1];
    fprintf(yyout, "  %s = zext %s %s to i32\n", dst->toStr().c_str(),
            src->getType()->toStr().c_str(), src->toStr().c_str());
}

ZextInstruction::~ZextInstruction(){
    operands[0]->removeUse(this);
    if(operands[0]->usersNum()==0)
        delete operands[0];
    operands[1]->removeUse(this);    
}

CastInstruction::CastInstruction(unsigned opcode,Operand* src ,Operand* dst, BasicBlock* insert_bb): Instruction(CAST, insert_bb){
    this->opcode = opcode;
    operands.push_back(src);
    operands.push_back(dst);
    this->setParent(insert_bb);
    src->addUse(this);
    dst->setDef(this);
}

GepInstruction::GepInstruction(Operand* dst, Operand* arr,Operand* idx,BasicBlock* insert_bb, bool first) : Instruction(GEP, insert_bb), first(first) {
    operands.push_back(dst);
    operands.push_back(arr);
    operands.push_back(idx);
    dst->setDef(this);
    arr->addUse(this);
    idx->addUse(this);
}

void GepInstruction::output() const {
    Operand* dst = operands[0];
    Operand* arr = operands[1];
    Operand* idx = operands[2];
    std::string arrType = arr->getType()->toStr();
    if(first)
        fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, i32 %s\n",
                dst->toStr().c_str(),arrType.substr(0, arrType.size() - 1).c_str(),
                arrType.c_str(),arr->toStr().c_str(), idx->toStr().c_str());
    else
        fprintf(
            yyout, "  %s = getelementptr inbounds %s, %s %s, i64 0, i32 %s\n",
            dst->toStr().c_str(), arrType.substr(0, arrType.size() - 1).c_str(),
            arrType.c_str(), arr->toStr().c_str(), idx->toStr().c_str());
}

GepInstruction::~GepInstruction() {
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void CastInstruction::output() const{
    Operand* dst = operands[1];
    Operand* src = operands[0];
    switch (opcode)
    {
    case ITOF:
        fprintf(yyout, "  %s = sitofp i32 %s to float\n", dst->toStr().c_str(), src->toStr().c_str());
        break;
    case FTOI:
        fprintf(yyout, "  %s = fptosi float %s to i32\n", dst->toStr().c_str(), src->toStr().c_str());
        break;
    case BTOI:
        fprintf(yyout, "  %s = zext %s %s to i32\n", dst->toStr().c_str(),src->getType()->toStr().c_str(), src->toStr().c_str());
        break;
    }
}

CastInstruction::~CastInstruction(){
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum()==0)
        delete operands[0];
    operands[1]->removeUse(this);

}

MachineOperand* Instruction::genMachineOperand(Operand* ope)
{
    auto se = ope->getEntry();
    MachineOperand* mope = nullptr;
    if(se->isConstant()){
        mope = new MachineOperand(MachineOperand::IMM, dynamic_cast<ConstantSymbolEntry*>(se)->getValue());
        // std::cout<<(long long int)dynamic_cast<ConstantSymbolEntry*>(se)->getValue()<<std::endl;
    }
    else if(se->isTemporary()){
        auto idSe = dynamic_cast<TemporarySymbolEntry*>(se);
        mope = new MachineOperand(MachineOperand::VREG, idSe->getLabel());
    }
    else if(se->isVariable())
    {
        auto id_se = dynamic_cast<IdentifierSymbolEntry*>(se);
        if(id_se->isGlobal()){
            if(id_se->getType()->isFunc())
                mope = new MachineOperand(id_se->toStr().c_str());
            else{
                mope = new MachineOperand(id_se->toStr().c_str()+1);
            }
                
        }else if(id_se->isParam()){//需要判断isParam的情况，此时可分配寄存器有限
            if(id_se->getType()->isFloat()){
                //注意保留一个用来过渡的寄存器
            }else{
                if(id_se->getParamNo() < 4){
                    mope = new MachineOperand(MachineOperand::REG, id_se->getParamNo());
                }else 
                    mope = new MachineOperand(MachineOperand::REG, 33); //不会用到这个寄存器
            }
        }else
            exit(0);
    }
    return mope;
}

MachineOperand* Instruction::genMachineReg(int reg) 
{
    return new MachineOperand(MachineOperand::REG, reg);
}

MachineOperand* Instruction::genMachineVReg() 
{
    return new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
}

MachineOperand* Instruction::genMachineImm(int val) 
{
    return new MachineOperand(MachineOperand::IMM, val);
}

MachineOperand* Instruction::genMachineLabel(int block_no)
{
    std::ostringstream buf;
    buf << ".L" << block_no;
    std::string label = buf.str();
    return new MachineOperand(label);
}

void AllocaInstruction::genMachineCode(AsmBuilder* builder)
{
    /* HINT:
    * Allocate stack space for local variabel
    * Store frame offset in symbol entry */

    auto cur_func = builder->getFunction();
    int offset;
    if(point2se){// 当为函数参数时
        if(point2se->getSize()<0)
            std::cout<<"en?"<<std::endl;
        TemporarySymbolEntry* temp_se = dynamic_cast<TemporarySymbolEntry*>(point2se);    
        if(temp_se->getParamNo()>=4){   //超过四个的参数的offset在output时调整
            offset=0;
        }else{
            offset = cur_func->AllocSpace(point2se->getSize()<0?4:point2se->getSize());//todo 需要考虑数组的情况，size不一定是4
        }
        offset = cur_func->AllocSpace(point2se->getSize()<0?4:point2se->getSize());//todo 需要考虑数组的情况，size不一定是4
        (dynamic_cast<TemporarySymbolEntry*>(point2se))->setOffset(-offset);
    }
    else{
        SymbolEntry* op_se = operands[0]->getEntry();
        if(op_se->getType()->isPTR()){
            offset = cur_func->AllocSpace(
                    dynamic_cast<PointerType*>(op_se->getType())->getValueType()->getSize()); 
        }else{
            offset = cur_func->AllocSpace(op_se->getSize());
        }
        (dynamic_cast<TemporarySymbolEntry*>(op_se))->setOffset(-offset);

    }
}

void LoadInstruction::genMachineCode(AsmBuilder* builder)
{
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst = nullptr;
    // Load global operand
    //TODO： 如果是浮点数，从label加载变量
    if(operands[1]->getEntry()->isVariable()
    && dynamic_cast<IdentifierSymbolEntry*>(operands[1]->getEntry())->isGlobal())
    {
        auto dst = genMachineOperand(operands[0]);
        auto internal_reg1 = genMachineVReg();
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        auto src = genMachineOperand(operands[1]);
        // example: load r0, addr_a
        cur_inst = new LoadMInstruction(cur_block, internal_reg1, src);
        cur_block->InsertInst(cur_inst);
        // example: load r1, [r0]
        cur_inst = new LoadMInstruction(cur_block, dst, internal_reg2);
        cur_block->InsertInst(cur_inst);
    }
    // Load local operand
    else if(operands[1]->getEntry()->isTemporary()
    && operands[1]->getDef()
    && operands[1]->getDef()->isAlloc())
    {
        // example: load r1, [r0, #4]
        auto dst = genMachineOperand(operands[0]);
        // auto src1 = genMachineReg(11);//fp
        // //需要考虑偏移在大于255和小于-255的情况，此时src2无法使用立即数
        // int offset=dynamic_cast<TemporarySymbolEntry*>(operands[1]->getEntry())->getOffset();
        // auto src2 = genMachineImm(offset);
        // if(offset<-255||offset>255){
        //     auto internal_reg1 = genMachineVReg();
        //     cur_inst = new LoadMInstruction(cur_block, internal_reg1, src2);
        //     cur_block->InsertInst(cur_inst);
        //     src2=internal_reg1;
        // }
        cur_inst = new LoadMInstruction(cur_block, dst, operands[1]->getEntry());
        cur_block->InsertInst(cur_inst);
    }
    // Load operand from temporary variable
    else
    {
        // example: load r1, [r0]
        auto dst = genMachineOperand(operands[0]);
        auto src = genMachineOperand(operands[1]);
        cur_inst = new LoadMInstruction(cur_block, dst, src);
        cur_block->InsertInst(cur_inst);
    }
}

void StoreInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst = nullptr;
    auto dst = genMachineOperand(operands[0]);
    auto src = genMachineOperand(operands[1]);
    if (src->isImm()) {
        auto internal_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, internal_reg, src);
        cur_block->InsertInst(cur_inst);
        src = new MachineOperand(*internal_reg);
    }
    // store global operand
    //TODO： 如果是浮点数，通过label存
    if(operands[0]->getEntry()->isVariable()
        && dynamic_cast<IdentifierSymbolEntry*>(operands[0]->getEntry())->isGlobal())
    {
        auto internal_reg1 = genMachineVReg();
        // example: load r0, addr_a
        cur_inst = new LoadMInstruction(cur_block, internal_reg1, dst);
        cur_block->InsertInst(cur_inst);
        // example: store r1, [r0]
        cur_inst = new StoreMInstruction(cur_block, src, internal_reg1);
        cur_block->InsertInst(cur_inst);
    }
    // store local operand, 包括将函数参数存入栈中   
    else if(operands[0]->getEntry()->isTemporary() && operands[0]->getDef()
            && operands[0]->getDef()->isAlloc())
    {
        // auto dst1 = genMachineReg(11);
        // int offset = dynamic_cast<TemporarySymbolEntry*>(operands[0]->getEntry())
        //               ->getOffset();
        // auto dst2 = genMachineImm(offset);
        // if (offset > 255 || offset < -255) {
        //     auto internal_reg1 = genMachineVReg();
        //     cur_inst = new LoadMInstruction(cur_block, internal_reg1, dst2);
        //     cur_block->InsertInst(cur_inst);
        //     dst2=internal_reg1;
        // }
        auto param_se= dynamic_cast<IdentifierSymbolEntry*>(operands[1]->getEntry());
        bool isparam = false;
        if(param_se&&param_se->isParam()){
            isparam=true;
        }
        cur_inst = new StoreMInstruction(cur_block, src, operands[0]->getEntry(),isparam);
        cur_block->InsertInst(cur_inst);
    }
    // store to pointer   待改
    else if (operands[0]->getType()->isPTR()) {
        cur_inst = new StoreMInstruction(cur_block, src, dst);
        cur_block->InsertInst(cur_inst);
    }
}

void BinaryInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO:
    // complete other instructions
    auto cur_block = builder->getBlock();
    auto dst = genMachineOperand(operands[0]);
    auto src1 = genMachineOperand(operands[1]);
    auto src2 = genMachineOperand(operands[2]);
    /* HINT:
    * The source operands of ADD instruction in ir code both can be immediate num.
    * However, it's not allowed in assembly code.
    * So you need to insert LOAD/MOV instrucrion to load immediate num into register.
    * As to other instructions, such as MUL, CMP, you need to deal with this situation, too.*/
    MachineInstruction* cur_inst = nullptr;
    if(src1->isImm()){
        auto temp_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, temp_reg, src1);
        cur_block->InsertInst(cur_inst);
        src1 = temp_reg;
    }
    if(src2->isImm()){
        //考虑不合法立即数的情况
        int imm_val = src2->getVal();
        //float还没考虑
        if((opcode==MUL||opcode==DIV||opcode==MOD)||(imm_val>255||imm_val<-255)){
            auto temp_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block, temp_reg, src2);
            cur_block->InsertInst(cur_inst);
            src2 = temp_reg;
        }
    }
    switch (opcode)
    {
    case ADD:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, dst, src1, src2);
        break;
    case SUB:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, dst, src1, src2);
        break;
    case AND:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::AND, dst, src1, src2);
        break;
    case OR:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::OR,dst, src1, src2);
        break;
    case MUL:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, dst, src1, src2);
        break;
    case DIV:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, dst, src1, src2);
        break;
    case MOD:{
        auto div_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, dst, src1, src2);
        MachineOperand *dst1 = new MachineOperand(*dst);
        src1 = new MachineOperand(*src1);
        src2 = new MachineOperand(*src2);
        cur_block->InsertInst(div_inst);
        auto mul_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, dst1, dst, src2);
        cur_block->InsertInst(mul_inst);
        dst = new MachineOperand(*dst1);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, dst, src1, dst1);
        break;
    }//add by zsr 2022年12月24日15:48:16
    default:
        break;
    }
    cur_block->InsertInst(cur_inst);
}

void CmpInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO add by zsr 2022年12月24日15:50:47
    auto cur_block = builder->getBlock();
    auto src1 = genMachineOperand(operands[1]);
    auto src2 = genMachineOperand(operands[2]);
    MachineInstruction* cur_inst = nullptr;
    if (src1->isImm()) {
        auto internal_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, internal_reg, src1);
        cur_block->InsertInst(cur_inst);
        src1 = new MachineOperand(*internal_reg);
    }
    if (src2->isImm() && ((ConstantSymbolEntry*)(operands[2]->getEntry()))->getValue() > 255) {
        auto internal_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, internal_reg, src2);
        cur_block->InsertInst(cur_inst);
        src2 = new MachineOperand(*internal_reg);
    }
    cur_inst = new CmpMInstruction(cur_block, src1, src2, opcode);
    cur_block->InsertInst(cur_inst);
    builder->setCmpOpcode(opcode);
    // for(auto it = operands[0]->use_begin();it != operands[0]->use_end();it++){
    //     std::cout<<(*it)->isCond()<<std::endl;
    // }
    //如果使用比较结果的第一条指令不是跳转指令,需要给操作数分配寄存器
    if(!(*(operands[0]->use_begin()))->isCond()||operands[0]->usersNum()>1){
        // std::cout<<"x1"<<std::endl;
        auto dst_reg = genMachineOperand(operands[0]);
        auto true_op = genMachineImm(1);
        auto false_op = genMachineImm(0);
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst_reg, false_op);
        cur_block->InsertInst(cur_inst);
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst_reg, true_op, opcode);
        cur_block->InsertInst(cur_inst);    
    }
}

void UncondBrInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO add by zsr 2022年12月24日14:38:54
    auto cur_block = builder->getBlock();
    MachineOperand* dst = genMachineLabel(branch->getNo());     
    auto cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst);
    cur_block->InsertInst(cur_inst);
}

void CondBrInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO add by zsr 2022年12月24日15:39:03
    auto cur_block = builder->getBlock();
    MachineOperand* dst = genMachineLabel(true_branch->getNo());
    auto cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst, builder->getCmpOpcode());
    cur_block->InsertInst(cur_inst);
    dst = genMachineLabel(false_branch->getNo());
    cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst);
    cur_block->InsertInst(cur_inst);
}

void RetInstruction::genMachineCode(AsmBuilder* builder)
{
    // TODO
    /* HINT:
    * 1. Generate mov instruction to save return value in r0
    * 2. Restore callee saved registers and sp, fp
    * 3. Generate bx instruction */
    auto cur_block = builder->getBlock();
    //保存返回值到r0
    if (!operands.empty()) {
        auto dst = genMachineReg(0);
        auto src = genMachineOperand(operands[0]);
        auto cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, src);
        cur_block->InsertInst(cur_inst);
    }
    //恢复函数栈 在block中遍历pop指令，然后添加恢复
    // auto cur_func = builder->getFunction();
    // auto sp_reg = new MachineOperand(MachineOperand::REG, 13);//sp
    // auto size = new MachineOperand(MachineOperand::IMM, cur_func->AllocSpace(0));
    // auto recov_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, sp_reg, sp_reg, size);
    // cur_block->InsertInst(recov_inst);

    //直接pop到fp与pc寄存器中  
    auto fp_reg = new MachineOperand(MachineOperand::REG, 11);//fp
    auto pc_reg = new MachineOperand(MachineOperand::REG, 15);//pc
    auto pop_fppc = new StackMInstrcuton(nullptr, StackMInstrcuton::POP, {fp_reg, pc_reg});
    cur_block->InsertInst(pop_fppc);
}

void CastInstruction::genMachineCode(AsmBuilder* builder){
    //operand[0]:src operand[1]:dst
    switch(opcode){
        case ITOF:
            break;
        case FTOI:
            break;
        case BTOI:
            auto cur_block = builder->getBlock();
            auto dst = genMachineOperand(operands[1]);
            auto src = genMachineOperand(operands[0]);
            auto cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, src);
            cur_block->InsertInst(cur_inst);
            break;
    }
}

void GepInstruction::genMachineCode(AsmBuilder* builder){
    // 关键在于获取加载基址的偏移
    /* operands:
    * dst: Gep加载存储的基址，为temporarySE的operand
    * arr: Gep加载的数组指针操作数[3 x i32]*
    * idx: Gep加载的序号操作数，为ConstantSE\temporarySE的operand
    */
    /**另外，关于全局变量中数组存储方式，展示如下：
     * 低地址
     * > 数组基址a:
     * a[0]
     * a[1]
     * ...
     * a[n-1]
     * 高地址
     * Q：我们临时变量的设计要和他同流合污吗？
    */
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst;
    auto fp_reg = genMachineReg(11);
    auto gep_dst = genMachineOperand(operands[0]);
    auto gep_arr = genMachineOperand(operands[1]);
    auto gep_idx = genMachineOperand(operands[2]);
    int const_id = -1;
    if(operands[2]->getEntry()->isConstant()){//尝试获取idx的值，获取不到只有加载了
        const_id = ((ConstantSymbolEntry*)operands[2]->getEntry())->getValue();
    }
    Type *arr_type = (operands[1]->getEntry())->getType();
    int gep_off;
    MachineOperand* off_op;
    if (first){
        // 如果是函数参数中的指针的Gep:
        // arr_type一定是指针类型，偏移需要找指针包裹类型的size
        gep_off = dynamic_cast<PointerType *>(arr_type)->getValueType()->getSize();
    }
    else{
        // 如果是临时变量或全局变量的Gep:
        // arr_type一定是数组指针类型，偏移需要找指针包裹的数组的下一级数组size
        Type* temp_type = dynamic_cast<PointerType*>(arr_type)->getValueType();
        gep_off = dynamic_cast<ArrayType*>(temp_type)->getLowerType()->getSize();
    }
    if (const_id < 0 || const_id * gep_off > 255){ 
        // 如果这个偏移大于255，还是得算偏移
        off_op = genMachineImm(gep_off);
        auto temp_reg = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, temp_reg, off_op);
        cur_block->InsertInst(cur_inst);
        off_op = temp_reg;
        auto temp_off = genMachineVReg();
        if(gep_idx->isImm()){
            auto temp_idx = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block, temp_idx, gep_idx);
            cur_block->InsertInst(cur_inst);
            gep_idx = temp_idx;
        }
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, temp_off, off_op, gep_idx);
        cur_block->InsertInst(cur_inst);
        off_op = temp_off;
    }
    else
        off_op = genMachineImm(gep_off * const_id);
    // 接下来，需要加载原基址到寄存器，再计算偏移
    if(operands[1]->getEntry()->isTemporary()){
        int src_off = dynamic_cast<TemporarySymbolEntry*>(operands[1]->getEntry())->getOffset();
        if(src_off != -1){// 如果在函数栈中分配了地址 [fp, #src_off]
            MachineOperand* src_off_op;    
            src_off_op = genMachineImm(src_off);
            if(off_op->isImm()){// 避免两个操作数都为立即数
                if(std::abs(gep_off*const_id+src_off)>255){
                    auto temp_reg = genMachineVReg();
                    cur_inst = new LoadMInstruction(cur_block, temp_reg, src_off_op);
                    cur_block->InsertInst(cur_inst);
                    src_off_op = temp_reg;
                    auto temp_dst=genMachineVReg();
                    cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, temp_dst, src_off_op, off_op);
                    cur_block->InsertInst(cur_inst);
                    cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, gep_dst, fp_reg, temp_dst);
                    cur_block->InsertInst(cur_inst);
                }else{
                    cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, gep_dst, fp_reg, genMachineImm(gep_off*const_id+src_off));
                    cur_block->InsertInst(cur_inst);
                    return;
                }
            }else{  // 如果off_op不是立即数
                if(src_off > 255||src_off < -255){
                    auto temp_reg = genMachineVReg();
                    cur_inst = new LoadMInstruction(cur_block, temp_reg, src_off_op);
                    cur_block->InsertInst(cur_inst);
                    src_off_op = temp_reg;
                }
                auto temp_dst=genMachineVReg();
                cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, temp_dst, off_op, src_off_op);
                cur_block->InsertInst(cur_inst);
                cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, gep_dst, fp_reg, temp_dst);
                cur_block->InsertInst(cur_inst);
            }
        }else{//如果没有被分配过函数栈中的位置，可能是之前Gep计算出的地址，直接用即可
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, gep_dst, gep_arr, off_op);
            cur_block->InsertInst(cur_inst);
        }
    }else{// 应该是全局变量
        auto idSe = dynamic_cast<IdentifierSymbolEntry*>(operands[1]->getEntry());
        if(idSe ==nullptr|| !idSe->isGlobal()){
            std::cout<<"哦豁"<<std::endl;
        }
        auto src_dst = genMachineVReg();
        cur_inst = new LoadMInstruction(cur_block, src_dst, gep_arr);
        cur_block->InsertInst(cur_inst);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, gep_dst, src_dst, off_op);
        cur_block->InsertInst(cur_inst);
    }
}

void ZextInstruction::genMachineCode(AsmBuilder* builder){
    auto cur_block = builder->getBlock();
    auto dst = genMachineOperand(operands[0]);
    auto src = genMachineOperand(operands[1]);
    auto cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, src);
    cur_block->InsertInst(cur_inst);
}

void CallInstruction::genMachineCode(AsmBuilder* builder){
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst;
    for(int i=1; i< operands.size()&&i<5;i++){      //float还没考虑
        auto mv_dst = genMachineReg(i-1);
        auto mv_src = genMachineOperand(operands[i]);
        if (mv_src->isImm() && mv_src->getVal() > 255) {    //认为不合法的立即数采用ldr指令
            cur_inst = new LoadMInstruction(cur_block, mv_dst, mv_src);
        } else{
            cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV,mv_dst, mv_src);
        }
        cur_block->InsertInst(cur_inst);
    }
    for(int i= operands.size()-1; i>4; i--){    //push序号大于4的实参
        auto push_op = genMachineOperand(operands[i]);
        if(push_op->isImm()){
            auto internal_op = genMachineVReg();
            if (push_op->getVal() > 255)
                cur_inst = new LoadMInstruction(cur_block, internal_op, push_op);
            else
                cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, internal_op, push_op);
            cur_block->InsertInst(cur_inst);
            push_op = internal_op ;            
        }
        cur_inst = new StackMInstrcuton(cur_block, StackMInstrcuton::PUSH, push_op);
        cur_block->InsertInst(cur_inst);
    }
    auto func_label = new MachineOperand(func->toStr());
    cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::BL, func_label);
    cur_block->InsertInst(cur_inst);
    if (operands.size() > 5) {
        auto offset = genMachineImm((operands.size() - 5) * 4);
        auto sp_reg = new MachineOperand(MachineOperand::REG, 13);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, sp_reg, sp_reg, offset);
        cur_block->InsertInst(cur_inst);
    }
    if (operands[0]) {
        auto ret_op = genMachineOperand(operands[0]);
        auto r0 = new MachineOperand(MachineOperand::REG, 0);
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, ret_op, r0);
        cur_block->InsertInst(cur_inst);
    }
}

void FCmpInstruction::genMachineCode(AsmBuilder* builder){

}