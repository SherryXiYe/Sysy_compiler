#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "Operand.h"
#include "AsmBuilder.h"
#include <vector>
#include <map>
#include <sstream>

class BasicBlock;

class Instruction
{
public:
    Instruction(unsigned instType, BasicBlock *insert_bb = nullptr);
    virtual ~Instruction();
    BasicBlock *getParent();
    bool isUncond() const {return instType == UNCOND;};
    bool isCond() const {return instType == COND;};
    bool isAlloc() const {return instType == ALLOCA;};
    bool isRet() const {return instType == RET;};
    bool isStore() const {return instType == STORE;};
    bool isLoad() const {return instType == LOAD;};
    bool isPhi() const {return instType == PHI;};
    void setParent(BasicBlock *);
    void setNext(Instruction *);
    void setPrev(Instruction *);
    Instruction *getNext();
    Instruction *getPrev();
    virtual void output() const = 0;
    MachineOperand* genMachineOperand(Operand*,bool isfp = false);
    MachineOperand* genMachineReg(int reg, bool isfp =false);
    MachineOperand* genMachineVReg(bool isfp = false);
    MachineOperand* genMachineImm(double val, bool isfp = false);
    MachineOperand* genMachineLabel(int block_no, bool isfp= false);
    virtual void genMachineCode(AsmBuilder*) = 0;
    void replaceDef(Operand* newDef){
        operands[0]->setDef(nullptr);
        operands[0] = newDef;
        newDef->setDef(this);
    }
    std::vector<Operand*> getOperands(){return operands;}
    virtual Operand* getDefOp(){return nullptr;}
    virtual void replaceUse(Operand* oldUse, Operand* newUse);
protected:
    unsigned instType;
    unsigned opcode;
    Instruction *prev;
    Instruction *next;
    BasicBlock *parent;
    std::vector<Operand*> operands;
    enum {BINARY, COND, UNCOND, RET, LOAD, STORE, CMP, ALLOCA, CALL/*函数调用*/, ZEXT /*无符号扩展*/,GEP/*获取元素指针*/,CAST/*浮点整型转换*/,PHI};
// GEP使用:https://blog.csdn.net/woiyyn/article/details/118670736
};

// meaningless instruction, used as the head node of the instruction list.
class DummyInstruction : public Instruction
{
public:
    DummyInstruction() : Instruction(-1, nullptr) {};
    void output() const {};
    void genMachineCode(AsmBuilder*) {};
};

class AllocaInstruction : public Instruction
{
public:
    AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb = nullptr);
    ~AllocaInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    void setPoint2Se(SymbolEntry* se){point2se = se;};
    void clear();
    SymbolEntry* getPoint2Se(){return point2se;};
    SymbolEntry* getSe(){return se;}
    Operand* getDefOp(){return operands[0];}
private:
    SymbolEntry *se;        //注意！!与符号表的连接在这
    SymbolEntry *point2se;
};

class LoadInstruction : public Instruction
{
public:
    LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb = nullptr);
    ~LoadInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    Operand* getDefOp(){return operands[0];}
};

class StoreInstruction : public Instruction
{
public:
    StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb = nullptr);
    ~StoreInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
};

class BinaryInstruction : public Instruction
{
public:
    BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~BinaryInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    Operand* getDefOp(){return operands[0];}
    enum {AND, OR, SUB, ADD, MUL, DIV, FSUB, FADD, FMUL, FDIV, MOD};
};

class CmpInstruction : public Instruction
{
public:
    CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~CmpInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    Operand* getDefOp(){return operands[0];}
    enum {E, NE, L, GE, G, LE};
};

class FCmpInstruction : public Instruction
{
public:
    FCmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~FCmpInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    Operand* getDefOp(){return operands[0];}
    enum {E, NE, L, GE, G, LE};
};

// unconditional branch
class UncondBrInstruction : public Instruction
{
public:
    UncondBrInstruction(BasicBlock*, BasicBlock *insert_bb = nullptr);
    void output() const;
    void setBranch(BasicBlock *);
    BasicBlock *getBranch();
    void genMachineCode(AsmBuilder*);
protected:
    BasicBlock *branch;
};

// conditional branch
class CondBrInstruction : public Instruction
{
public:
    CondBrInstruction(BasicBlock*true_branch, BasicBlock*false_branch, Operand *cond, BasicBlock *insert_bb = nullptr);
    ~CondBrInstruction();
    void output() const;
    void setTrueBranch(BasicBlock*);
    BasicBlock* getTrueBranch();
    void setFalseBranch(BasicBlock*);
    BasicBlock* getFalseBranch();
    void genMachineCode(AsmBuilder*);
protected:
    BasicBlock* true_branch;
    BasicBlock* false_branch;
};

class RetInstruction : public Instruction
{
public:
    RetInstruction(Operand *src, BasicBlock *insert_bb = nullptr);
    ~RetInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
};

// **the same as former comment
class CallInstruction : public Instruction {
private:
    SymbolEntry *func;
public:
    CallInstruction(Operand *dst, SymbolEntry *func, std::vector<Operand *> params, BasicBlock *insert_bb = nullptr);
    ~CallInstruction();
    void output() const;
    void genMachineCode(AsmBuilder *);
    Operand* getDefOp(){return operands[0];}
};
// same
class ZextInstruction : public Instruction {
public:
    ZextInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb = nullptr);
    ~ZextInstruction();
    void output() const;
    void genMachineCode(AsmBuilder *);
    Operand* getDefOp(){return operands[0];}
};
// same zsr add 2022年12月5日18:56:47
class GepInstruction : public Instruction {
private:
    bool first;
public:
    GepInstruction(Operand *dst, Operand *arr, Operand *idx, BasicBlock *insert_bb = nullptr, bool first = false);
    ~GepInstruction();
    void output() const;
    void genMachineCode(AsmBuilder *);
    Operand* getDefOp(){return operands[0];}
};

class CastInstruction : public Instruction {
public:
    CastInstruction(unsigned opcode, Operand *src, Operand *dst, BasicBlock *insert_bb = nullptr);
    void output() const;
    ~CastInstruction();
    enum{FTOI,ITOF,BTOI};
    void genMachineCode(AsmBuilder *);
    Operand* getDefOp(){return operands[0];}
};

class PhiInstruction : public Instruction {
private:
    Operand* dst;
    std::vector<BasicBlock*> inBlocks;
    std::map<BasicBlock*, Operand*> srcs;
    Operand* getDefOp(){return operands[0];}
public:
    PhiInstruction(Operand* dst, BasicBlock* insert_bb = nullptr);
    ~PhiInstruction();
    std::vector<BasicBlock*> getInBlocks(){return inBlocks;};
    void output() const;
    void addIncoming(Operand*, BasicBlock*);
    void genMachineCode(AsmBuilder* s){}
    std::map<BasicBlock*, Operand*> getSrcMap(){return srcs;};
};

#endif
