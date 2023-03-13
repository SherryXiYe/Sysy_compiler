#include "MachineCode.h"
#include "Ast.h"
#include <sstream>
extern FILE* yyout;
int pool_no = 0;

MachineOperand::MachineOperand(int tp, double val, bool isfp)
{
    this->type = tp;
    this->isfp = isfp;
    if(tp == MachineOperand::IMM)
        this->val = val;
    else if(tp == MachineOperand::REG){     //暂定
        this->reg_no = val;
    }else{
        this->reg_no = val;
    }
}

MachineOperand::MachineOperand(std::string label, bool isfp)
{
    this->type = MachineOperand::LABEL;
    this->label = label;
    this->isfp = isfp;
}

bool MachineOperand::operator==(const MachineOperand&a) const
{
    if (this->type != a.type)
        return false;
    if (this->isfp != a.isfp)
        return false;       //2023年1月1日15:14:33 add
    if (this->type == IMM)
        return this->val == a.val;
    else if(this->type == LABEL)
        return this->label == a.label;
    return this->reg_no == a.reg_no;
}

bool MachineOperand::operator<(const MachineOperand&a) const
{
    if(this->type == a.type)
    {
        if(this->type == IMM)
            return this->val < a.val;
        else if(this->type == LABEL)
            return this->label <a.label;
        return this->reg_no < a.reg_no;
    }
    return this->type < a.type;
    //不会进入后面吧？？
    // if (this->type != a.type)
    //     return false;
    // if (this->type == IMM)
    //     return this->val == a.val;
    // return this->reg_no == a.reg_no;
}

void MachineOperand::PrintReg()
{
    //添加reg_no判断，如果编号为16- 则为浮点寄存器，则输出为s..
    if(reg_no > 15){
        int true_no = reg_no - 16;
        fprintf(yyout, "s%d", true_no);
        //todo: 可能要使用FPSCR
    }else{
        switch (reg_no)
        {
        case 11:
            fprintf(yyout, "fp");
            break;
        case 13:
            fprintf(yyout, "sp");
            break;
        case 14:
            fprintf(yyout, "lr");
            break;
        case 15:
            fprintf(yyout, "pc");
            break;
        default:
            fprintf(yyout, "r%d", reg_no);
            break;
        }
    }
}

void MachineOperand::output() 
{
    /* HINT：print operand
    * Example:
    * immediate num 1 -> print #1;
    * register 1 -> print r1;
    * lable addr_a -> print addr_a; */
    std::stringstream buffer;
    switch (this->type)
    {
    case IMM:
        if(isfp){
            float fval = this->val;
            fprintf(yyout, "#%u", reinterpret_cast<uint32_t&>(fval));
        }
        else
            fprintf(yyout, "#%lld", (long long int)this->val);
        break;
    case VREG:
        fprintf(yyout, "v%d", this->reg_no);
        break;
    case REG:
        PrintReg();
        break;
    case LABEL:
        if (this->label.substr(0, 2) == ".L")
            fprintf(yyout, "%s", this->label.c_str());
        else if(this->label.substr(0, 1) == "@"){
            fprintf(yyout, "%s", this->label.substr(1, label.size()-1).c_str());
        }else
            fprintf(yyout, "addr_%s%d", this->label.c_str(), pool_no);
    default:
        break;
    }
}

void MachineInstruction::PrintCond()
{
    // TODO add by xzh
    switch (cond)
    {
        case LT:
            fprintf(yyout, "lt");
            break;
        case EQ:
            fprintf(yyout, "eq");
            break;
        case NE:
            fprintf(yyout, "ne");
            break;
        case LE:
            fprintf(yyout, "le");
            break;
        case GT:
            fprintf(yyout, "gt");
            break;
        case GE:
            fprintf(yyout, "ge");
            break;
        default:
            break;
    }
}

BinaryMInstruction::BinaryMInstruction(
    MachineBlock* p, int op, 
    MachineOperand* dst, MachineOperand* src1, MachineOperand* src2, 
    int cond)
{
    this->parent = p;
    this->type = MachineInstruction::BINARY;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    dst->setParent(this);
    src1->setParent(this);
    src2->setParent(this);
}

void BinaryMInstruction::output() 
{
    // TODO:  2022/12/24 add by xzh，还要考虑FADD等
    // Complete other instructions
    switch (this->op)
    {
        case BinaryMInstruction::ADD:
            fprintf(yyout, "\tadd");
            this->PrintCond();
            fprintf(yyout, " ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::SUB:
            fprintf(yyout, "\tsub");
            this->PrintCond();
            fprintf(yyout, " ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::AND:
            fprintf(yyout, "\tand");
            this->PrintCond();
            fprintf(yyout, " ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::OR:
            fprintf(yyout, "\torr");
            this->PrintCond();
            fprintf(yyout, " ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::MUL:
            fprintf(yyout, "\tmul");
            this->PrintCond();
            fprintf(yyout, " ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::DIV:
            fprintf(yyout, "\tsdiv");
            this->PrintCond();
            fprintf(yyout, " ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::VADD:
            fprintf(yyout, "\tvadd");
            this->PrintCond();
            fprintf(yyout, ".f32 ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::VSUB:
            fprintf(yyout, "\tvsub");
            this->PrintCond();
            fprintf(yyout, ".f32 ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::VMUL:
            fprintf(yyout, "\tvmul");
            this->PrintCond();
            fprintf(yyout, ".f32 ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        case BinaryMInstruction::VDIV:
            fprintf(yyout, "\tvdiv");
            this->PrintCond();
            fprintf(yyout, ".f32 ");
            this->def_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[0]->output();
            fprintf(yyout, ", ");
            this->use_list[1]->output();
            fprintf(yyout, "\n");
            break;
        default:
            break;
    }
}

LoadMInstruction::LoadMInstruction(MachineBlock* p,
    MachineOperand* dst, MachineOperand* src1, MachineOperand* src2,
    int cond, bool vldr)
{
    this->se = nullptr;
    this->parent = p;
    this->type = MachineInstruction::LOAD;
    this->op = -1;
    this->cond = cond;              //cond默认为MachineInstruction::NONE
    this->vldr = vldr;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    if (src2)
        this->use_list.push_back(src2);
    dst->setParent(this);
    src1->setParent(this);
    if (src2)
        src2->setParent(this);
}

LoadMInstruction::LoadMInstruction(MachineBlock* p,
                    MachineOperand* dst, SymbolEntry* se, int cond, bool vldr)
{
    this->parent = p;
    this->type = MachineInstruction::LOAD;    
    this->op = -1;
    this->cond = cond;
    this->se = se;
    this->vldr = vldr;
    this->def_list.push_back(dst);
    dst->setParent(this);
}
                    
void LoadMInstruction::output()
{
    if (se){
        int offset = dynamic_cast<TemporarySymbolEntry *>(se)->getOffset();
        auto offset_op = new MachineOperand(MachineOperand::IMM, offset);
        if(offset<-255||offset>255){
            auto internal_reg = new MachineOperand(MachineOperand::REG, 3);
            auto cur_inst = new LoadMInstruction(nullptr, internal_reg, offset_op);
            cur_inst->output();
            offset_op = internal_reg;
        }
        auto fp_reg = new MachineOperand(MachineOperand::REG, 11);
        if(vldr){
            MachineOperand* load_pos_reg = new MachineOperand(MachineOperand::REG, 3);
            auto load_pos_inst = new BinaryMInstruction(nullptr, BinaryMInstruction::ADD, load_pos_reg, fp_reg, offset_op);
            load_pos_inst->output();
            use_list.push_back(load_pos_reg);
        }else{
            use_list.push_back(fp_reg);
            use_list.push_back(offset_op);
        }
    }
    if(vldr)
        fprintf(yyout, "\tvldr.32 ");
    else
        fprintf(yyout, "\tldr ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");

    // Load immediate num, eg: ldr r1, =8
    
    if(this->use_list[0]->isImm())
    {   
        
        if(!use_list[0]->isfp)
            fprintf(yyout, "=%lld\n", (long long int)this->use_list[0]->getVal());
        else{
            float val = use_list[0]->getVal();
            fprintf(yyout, "=%u\n", reinterpret_cast<uint32_t&>(val));
        }
        return;
    }

    // Load address
    if(this->use_list[0]->isReg()||this->use_list[0]->isVReg())
        fprintf(yyout, "[");

    this->use_list[0]->output();
    if( this->use_list.size() > 1 )
    {
        fprintf(yyout, ", ");
        this->use_list[1]->output();
    }

    if(this->use_list[0]->isReg()||this->use_list[0]->isVReg())
        fprintf(yyout, "]");
    fprintf(yyout, "\n");
}

StoreMInstruction::StoreMInstruction(MachineBlock* p,
    MachineOperand* src1, MachineOperand* src2, MachineOperand* src3,
    int cond, bool vstr) 
{
    // TODO add by zsr 2022年12月25日15:53:29
    this->se = nullptr;
    this->parent = p;
    this->type = MachineInstruction::STORE;
    this->op = -1;
    this->cond = cond;
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    this->isParam = false;
    this->vstr = vstr;
    if (src3){
        this->use_list.push_back(src3);
        src3->setParent(this);
    }
    src1->setParent(this);
    src2->setParent(this); 
}
//调整r0,r1,r2,r3偏移顺序
StoreMInstruction::StoreMInstruction(MachineBlock* p, 
        MachineOperand* src1, SymbolEntry* se, bool isParam,int cond, bool vstr)
{
    this->parent = p;
    this->type = MachineInstruction::STORE;    
    this->op = -1;
    this->cond = cond;
    this->se = se;
    this->isParam = isParam;
    this->vstr = vstr;
    this->use_list.push_back(src1);
    if(src1)
        src1->setParent(this);
}

void StoreMInstruction::output()
{
    // TODO add by zsr 2022年12月25日15:53:16
    if(use_list[0]->getReg() == -1){       //reg为-1的说明这是超出4个的参数，不输出
        return;
    }
    if(se){
        int offset=dynamic_cast<TemporarySymbolEntry*>(se)->getOffset();
        auto offset_op = new MachineOperand(MachineOperand::IMM, offset);
        if(offset<-255||offset>255){
            MachineOperand* internal_reg;
            if(isParam)
                internal_reg = new MachineOperand(MachineOperand::REG, 4);
            else
                internal_reg = new MachineOperand(MachineOperand::REG, 3);
            auto cur_inst = new LoadMInstruction(nullptr, internal_reg, offset_op);
            cur_inst->output();
            offset_op = internal_reg;
        }
        auto fp_reg = new MachineOperand(MachineOperand::REG, 11);
        if(vstr){
            MachineOperand* store_pos_reg;
            if(isParam)
                store_pos_reg = new MachineOperand(MachineOperand::REG, 4);
            else
                store_pos_reg = new MachineOperand(MachineOperand::REG, 3);
            auto store_pos_inst = new BinaryMInstruction(nullptr, BinaryMInstruction::ADD, store_pos_reg, fp_reg, offset_op);
            store_pos_inst->output();
            use_list.push_back(store_pos_reg);
        }else{
            use_list.push_back(fp_reg);
            use_list.push_back(offset_op);
        } 
    }
    if(vstr)
        fprintf(yyout, "\tvstr.32 ");
    else
        fprintf(yyout, "\tstr ");
    use_list[0]->output();
    fprintf(yyout, ", ");
    bool isReg = use_list[1]->isReg() || use_list[1]->isVReg();
    if (isReg)
        fprintf(yyout, "[");
    use_list[1]->output();
    if (use_list.size() > 2) {
        fprintf(yyout, ", ");
        use_list[2]->output();
    }
    if (isReg)
        fprintf(yyout, "]");
    fprintf(yyout, "\n");
}

MovMInstruction::MovMInstruction(MachineBlock* p, int op, 
    MachineOperand* dst, MachineOperand* src,
    int cond)
{
    // TODO add by zsr 2022年12月25日15:53:08
    this->parent = p;
    this->type = MachineInstruction::MOV;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src);
    dst->setParent(this);
    src->setParent(this);
}

void MovMInstruction::output() 
{
    // TODO add by zsr 2022年12月25日15:55:19
    if(op == MovMInstruction::MOV)
        fprintf(yyout, "\tmov");
    else if(op == MovMInstruction::VMOV)
        fprintf(yyout, "\tvmov");
    PrintCond();
    fprintf(yyout, " ");
    def_list[0]->output();
    fprintf(yyout, ", ");
    use_list[0]->output();
    fprintf(yyout, "\n");
}

BranchMInstruction::BranchMInstruction(MachineBlock* p, int op, 
    MachineOperand* dst, 
    int cond)
{
    // TODO 2022/12/24 add by xzh 
    this->parent = p;
    this->type = MachineInstruction::BRANCH;
    this->op = op;
    this->cond = cond;             
    this->use_list.push_back(dst);
    dst->setParent(this);
}

void BranchMInstruction::output()
{
    // TODO 2022/12/24 add by xzh 
    switch (op) {
        case B:
            fprintf(yyout, "\tb");
            PrintCond();
            fprintf(yyout, " ");
            this->use_list[0]->output();
            fprintf(yyout, "\n");
            break;
        case BX:
            fprintf(yyout, "\tbx");
            PrintCond();
            fprintf(yyout, " ");
            this->use_list[0]->output();
            fprintf(yyout, "\n");
            break;
        case BL:
            fprintf(yyout, "\tbl");
            PrintCond();
            fprintf(yyout, " ");
            this->use_list[0]->output();
            fprintf(yyout, "\n");
            break;
    }
}

CmpMInstruction::CmpMInstruction(MachineBlock* p, 
    MachineOperand* src1, MachineOperand* src2,
    int cond, bool vcmp)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::CMP;
    this->op = -1;
    this->cond = cond;
    this->vcmp = vcmp;
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    src1->setParent(this);
    src2->setParent(this);
}

void CmpMInstruction::output()
{
    // TODO
    // Jsut for reg alloca test
    // delete it after test
    if(vcmp)
        fprintf(yyout, "\tvcmp.f32 ");
    else
        fprintf(yyout, "\tcmp ");
    use_list[0]->output();
    fprintf(yyout, ", ");
    use_list[1]->output();
    fprintf(yyout, "\n");
    if(vcmp)
        fprintf(yyout, "\tvmrs APSR_nzcv, FPSCR\n");
}

StackMInstrcuton::StackMInstrcuton(MachineBlock* p, int op, 
    MachineOperand* src,
    int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::STACK;
    this->op = op;
    this->cond = cond;
    src->setParent(this);
    use_list.push_back(src);
}

StackMInstrcuton::StackMInstrcuton(MachineBlock* p, int op, 
    std::vector<MachineOperand* >srcs,
    int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::STACK;
    this->op = op;
    this->cond = cond;
    for(auto src: srcs){
        use_list.push_back(src);
        src->setParent(this);
    }
}

void StackMInstrcuton::output()
{
    // TODO
    switch (op)
    {
    case POP:
        fprintf(yyout, "\tpop ");
        break;
    case PUSH:
        fprintf(yyout, "\tpush ");
        break;
    case VPOP:
        fprintf(yyout, "\tvpop ");
        break;
    case VPUSH:
        fprintf(yyout, "\tvpush ");
        break;
    default:
        break;
    }
    fprintf(yyout, "{");
    for(int i=0;i<use_list.size();i++){
        use_list[i]->output();
        if(i!=use_list.size()-1)
            fprintf(yyout, ", ");
    }
    fprintf(yyout, "}\n");
}

VcvtMInstruction::VcvtMInstruction(MachineBlock* p, int op, MachineOperand* dst, MachineOperand* src, int cond){
    this->parent = p;
    this->type = MachineInstruction::CVT;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    dst->setParent(this);
    this->use_list.push_back(src);
    src->setParent(this);
}

void VcvtMInstruction::output(){
    fprintf(yyout, "\tvcvt");
    PrintCond();
    if(op == VcvtMInstruction::FTOI){
        fprintf(yyout, ".s32.f32 ");
    }else if(op == VcvtMInstruction::ITOF){
        fprintf(yyout, ".f32.s32 ");
    }
    def_list[0]->output();
    fprintf(yyout, ", ");
    use_list[0]->output();
    fprintf(yyout, "\n");
}

MachineFunction::MachineFunction(MachineUnit* p, SymbolEntry* sym_ptr) 
{ 
    this->parent = p; 
    this->sym_ptr = sym_ptr; 
    this->stack_size = 0;
};

void MachineBlock::output()
{   
    int inst_count = 0;
    if(inst_list.empty())
        return;
    fprintf(yyout, ".L%d:\n", this->no);
    for(auto &inst : inst_list){
        if(inst->isStack()){
            auto stk_inst = dynamic_cast<StackMInstrcuton*>(inst);
            if(stk_inst->getOp() == StackMInstrcuton::POP){
                //恢复函数栈
                auto sp_reg = new MachineOperand(MachineOperand::REG, 13);//sp
                auto size = new MachineOperand(MachineOperand::IMM, parent->AllocSpace(0));
                if(parent->AllocSpace(0)>255){
                    auto temp_reg = new MachineOperand(MachineOperand::REG, 3);
                    auto load_inst = new LoadMInstruction(nullptr, temp_reg, size);
                    load_inst->output();
                    size = temp_reg;
                }
                auto recov_inst = new BinaryMInstruction(nullptr, BinaryMInstruction::ADD, sp_reg, sp_reg, size);
                recov_inst->output();
                //pop savedRegs
                auto saved_sregs = parent->getSavedSRegs();
                if(saved_sregs.size()>0){
                    auto pop_sregs = new StackMInstrcuton(nullptr, StackMInstrcuton::VPOP, saved_sregs);
                    pop_sregs->output();
                }
                auto saved_regs = parent->getSavedRegs();
                if(saved_regs.size()>0){
                    auto pop_regs = new StackMInstrcuton(nullptr, StackMInstrcuton::POP, saved_regs);
                    pop_regs->output();
                }

            }
        }
        inst_count++;
        if(inst_count % 512 == 0){// 生成文字池，防止无法寻址
            fprintf(yyout, "\tb .B%d\n", pool_no);
            fprintf(yyout, ".LTORG\n");
            parent->getUnit()->PrintGlobalAddr();
            fprintf(yyout, ".B%d:\n", pool_no++);
        }
        inst->output();
    }
}

std::vector<MachineOperand*> MachineFunction::getSavedRegs() {
    std::vector<MachineOperand*> regs;
    MachineOperand* reg;
    for(auto regI : saved_regs){
        reg = new MachineOperand(MachineOperand::REG, regI);
        regs.push_back(reg);
    }
    return regs;
}

std::vector<MachineOperand*> MachineFunction::getSavedSRegs() {
    std::vector<MachineOperand*> regs;
    MachineOperand* reg;
    for(auto regI : saved_sregs){
        reg = new MachineOperand(MachineOperand::REG, regI,true);
        regs.push_back(reg);
    }
    return regs;
}

void MachineFunction::output()
{
    auto func_name = this->sym_ptr->toStr().substr(1, this->sym_ptr->toStr().size()-1);
    fprintf(yyout, "\t.global %s\n", func_name.c_str());
    fprintf(yyout, "\t.type %s , %%function\n", func_name.c_str());
    fprintf(yyout, "%s:\n", func_name.c_str());
    // TODO
    /* Hint:
    *  1. Save fp
    *  2. fp = sp
    *  3. Save callee saved register
    *  4. Allocate stack space for local variable */
    auto fp_reg = new MachineOperand(MachineOperand::REG, 11);
    auto lr_reg = new MachineOperand(MachineOperand::REG, 14);
    auto push_fp = new StackMInstrcuton(nullptr, StackMInstrcuton::PUSH, {fp_reg, lr_reg});
    push_fp->output();
    if(func_name == "main"){
        saved_regs.clear();
        // saved_sregs.clear();
    }
    if(saved_regs.size()!=0){
        auto push_regs = new StackMInstrcuton(nullptr, StackMInstrcuton::PUSH, getSavedRegs());
        push_regs->output();
    }
    if(saved_sregs.size()!=0){  //s4我们没用，不用无条件保存
        auto push_sregs = new StackMInstrcuton(nullptr, StackMInstrcuton::VPUSH, getSavedSRegs());
        push_sregs->output();
    }
    auto sp_reg = new MachineOperand(MachineOperand::REG, 13);
    auto new_fp = new MovMInstruction(nullptr,MovMInstruction::MOV,fp_reg,sp_reg);
    new_fp->output();
    int paramNum = ((FunctionType*)dynamic_cast<IdentifierSymbolEntry*>(sym_ptr)->getType())->getParams().size();
    bool isSpill = paramNum > 4;
    int spillOffset = (saved_regs.size()+saved_sregs.size() + 2)*4;
    int offset = AllocSpace(0);
    
    MachineOperand* offset_op;
    if(offset > 255){
        offset_op = new MachineOperand(MachineOperand::REG, 4);
        auto load_imm = new LoadMInstruction(nullptr, offset_op, new MachineOperand(MachineOperand::IMM, offset));
        load_imm->output();
    }else{
        offset_op = new MachineOperand(MachineOperand::IMM, offset);
    }
    auto alloc_stack = new BinaryMInstruction(nullptr, BinaryMInstruction::SUB ,sp_reg, sp_reg, offset_op);
    alloc_stack->output();
    int StoreCount = 0;
    // 遍历块，修改offset
    for(auto iter : block_list){
        for(auto &inst : iter->getInsts()){
            if(StoreCount>paramNum){
                break;
            }
            if(inst->isStore()){
                StoreCount++;
                if(StoreCount>paramNum){
                    break;
                }
                StoreMInstruction* st_inst = dynamic_cast<StoreMInstruction*>(inst);
                TemporarySymbolEntry* se = dynamic_cast<TemporarySymbolEntry*>(st_inst->getSymPtr());
                if(se == nullptr){
                    std::cout<<"jilo"<<std::endl;//有全局变量再删
                    std::cout<<st_inst->getSymPtr()<<std::endl;
                }else{
                    // 之前的想法
                    // 1.如果是小于4参数，不用改
                    // 2.如果是大于4的参数，调为caller push进的位置
                    /* 当前栈情况
                        *   高地址
                        *   大于4的参数
                        *   lr
                        *   fp
                        *   saved_regs
                    *fp->
                        *   临时变量
                        *   大于4的参数
                        *   小于4的参数
                    *sp->
                        *   低地址
                    */
                    // 不需要改临时变量，只需要改大于4参数的offset为正确位置，然后上移小于4的参数位置即可

                    int param_off = paramNum * 4;
                    if(isSpill){
                        int se_off = -se->getOffset();
                        if(se->getParamNo() >= 4){
                            se->setOffset(spillOffset + (se->getParamNo()-4)*4);
                        }
                    }
                }
            }
        }
    }
    
    // Traverse all the block in block_list to print assembly code.
    int inst_count = 0;
    for(auto iter : block_list){
        inst_count += iter->getInsts().size();
        iter->output();
        if(inst_count > 200){// 生成文字池，防止无法寻址
            fprintf(yyout, "\tb .B%d\n", pool_no);
            fprintf(yyout, ".LTORG\n");
            getUnit()->PrintGlobalAddr();
            fprintf(yyout, ".B%d:\n", pool_no++);
            inst_count = 0;
        }
    }
        
    fprintf(yyout, "\n");
}

void MachineUnit::PrintGlobalDecl()
{
    // TODO:
    // You need to print global variable/const declarition code;
    if(!global_list.empty())
        fprintf(yyout, "\t.data\n");
    for (auto se : global_list)
    {
        auto idSe = dynamic_cast<IdentifierSymbolEntry *>(se);
        // if(idSe->getType()->isConstFloat()||idSe->getType()->isConstInt()){
        //     fprintf(yyout, "\t.section rodata\n");
        // }
        fprintf(yyout, "\t.global %s\n", idSe->toStr().c_str() + 1);
        fprintf(yyout, "\t.align 4\n");
        fprintf(yyout, "\t.size %s, %d\n", idSe->toStr().c_str() + 1, idSe->getSize());
        fprintf(yyout, "%s:\n", idSe->toStr().c_str() + 1);
        if (idSe->getType()->isInt())
            fprintf(yyout, "\t.word %lld\n", (long long int)idSe->getValue());
        else if (idSe->getType()->isFloat()){
            float fval = idSe->getValue();
            fprintf(yyout, "\t.word %u\n", reinterpret_cast<uint32_t&>(fval));
        }
        else if (idSe->getType()->isArray()){
            auto arrType = dynamic_cast<ArrayType *>(idSe->getType());
            if (idSe->getArrayValRoot()){
                auto valVec = new std::vector<double>();
                idSe->getArrayValRoot()->toVector(valVec);
                if (arrType->getElementType()->isInt())
                    for (auto val : *valVec){
                        fprintf(yyout, "\t.word %lld\n", (long long int)val);
                    }
                else if (arrType->getElementType()->isFloat())
                    for (auto val : *valVec){
                        float fval = val;
                        fprintf(yyout, "\t.word %u\n", reinterpret_cast<uint32_t&>(fval));
                    }
            }else{
                int arr_size = arrType->getSize();
                if (arrType->getElementType()->isInt())
                    for (int i = 0; i < arr_size; i++){
                        fprintf(yyout, "\t.word 0\n");
                    }
                else if (arrType->getElementType()->isFloat())
                    for (int i = 0; i < arr_size; i++){
                        float fval = 0;
                        fprintf(yyout, "\t.word 0\n");
                    }
            }
        }
    }
}

void MachineUnit::output()
{
    // TODO
    /* Hint:
    * 1. You need to print global variable/const declarition code;
    * 2. Traverse all the function in func_list to print assembly code;
    * 3. Don't forget print bridge label at the end of assembly code!! */
    fprintf(yyout, "\t.arch armv8-a\n");
    fprintf(yyout, "\t.fpu vfpv3-d16\n");
    fprintf(yyout, "\t.arch_extension crc\n");
    fprintf(yyout, "\t.arm\n");
    PrintGlobalDecl();
    fprintf(yyout, "\t.text\n");
    for(auto iter : func_list){
        iter->output();
    }
    //最后这里，Don't forget print bridge label at the end of assembly code!
    PrintGlobalAddr();
}

void MachineUnit::PrintGlobalAddr(){
    for(auto& idSe : global_list){
        fprintf(yyout, "addr_%s%d:\n", idSe->toStr().c_str()+1, pool_no);
        fprintf(yyout, "\t.word %s\n", idSe->toStr().c_str()+1);
    }
}