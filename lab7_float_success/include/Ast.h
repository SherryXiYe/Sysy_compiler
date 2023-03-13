#ifndef __AST_H__
#define __AST_H__

#include <fstream>
#include "Operand.h"
#include <stack>
#include <algorithm>
#include <math.h>
class SymbolEntry;
class Unit;
class Function;
class BasicBlock;
class Instruction;
class IRBuilder;

void SetBinaryType(SymbolEntry*& se, Type* expr1Type, Type* expr2Type);
Type* maxType(Type* type1,Type* type2);
int getCastOp(Type* srcType, Type* castType);
Type* trueType(Type* type);

class Node
{
private:
    static int counter;
    int seq;
protected:
    std::vector<Instruction*> true_list;
    std::vector<Instruction*> false_list;
    static IRBuilder *builder;
    static std::stack<BasicBlock*> while_cond_entry;
    static std::stack<BasicBlock*> while_end_entry;
    void backPatch(std::vector<Instruction*> &list, BasicBlock*bb);
    std::vector<Instruction*> merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2);

public:
    Node();
    int getSeq() const {return seq;};
    static void setIRBuilder(IRBuilder*ib) {builder = ib;};
    virtual void output(int level) = 0;
    virtual void typeCheck() = 0;
    virtual void genCode() = 0;
    std::vector<Instruction*>& trueList() {return true_list;}
    std::vector<Instruction*>& falseList() {return false_list;}
};

class ExprNode : public Node
{
protected:
    SymbolEntry *symbolEntry;
    Operand *dst;   // The result of the subtree is stored into dst.
    // double value;
    bool isnull;
    bool isUnary;
public:
    bool isInt;
    ExprNode* next;
    ExprNode(SymbolEntry *symbolEntry) : symbolEntry(symbolEntry), next(nullptr){isnull=false;isUnary=false;};
    Operand* getOperand() {return dst;};
    void setOperand(Operand* dst){this->dst = dst;};
    bool isConst(){return symbolEntry->isConstant()
                    ||symbolEntry->getType()->isConstFloat()
                    ||symbolEntry->getType()->isConstInt();}
    SymbolEntry* getSymPtr() {return symbolEntry;};
    void setSymPtr(SymbolEntry *symbolEntry){this->symbolEntry = symbolEntry;};
    virtual double getValue(){
        if(symbolEntry->isConstant())
            return ((ConstantSymbolEntry*)symbolEntry)->getValue();
    }
    bool isNullExpr(){return isnull;}
    bool isUnaryExpr() {return isUnary;}
};

class BinaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr1, *expr2;
public:
    enum {ADD, SUB, MUL, DIV, FDIV, FMUL, FADD, FSUB, MOD, AND, OR, LESS, GREATEREQUAL, LESSEQUAL, EQUAL, NOTEQUAL, GREATER};
    BinaryExpr(SymbolEntry *se, int op, ExprNode*expr1, ExprNode*expr2) : ExprNode(se), op(op), expr1(expr1), expr2(expr2){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
    double getValue();
};

class UnaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr;
public:
    enum {UMINUS, NOT,UADD, FTOI, ITOF,BTOI,BTOF};       //应该没用UADD
    UnaryExpr(SymbolEntry *se, int op, ExprNode*expr) : ExprNode(se), op(op), expr(expr){isUnary=true;};
    void output(int level);
    void typeCheck();       //2022/12/5 xzh new 
    void genCode();
    double getValue();
};

//2022年11月6日15:24:35 add by zsr
class NullExpr : public ExprNode{
public:
    NullExpr(): ExprNode(nullptr){isnull=true;}
    void output(int level);
    void typeCheck(){};
    void genCode(){};
};

class Constant : public ExprNode
{
public:
    Constant(SymbolEntry *se) : ExprNode(se){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
    double getValue();
};

class Id : public ExprNode
{
    ExprNode* arrayIndex;
    bool isLval;
public:
    Id(SymbolEntry *se) : ExprNode(se), arrayIndex(nullptr){
        SymbolEntry* temp;
        if(se->getType()->isArray()){
            temp = new TemporarySymbolEntry(new PointerType(((ArrayType*)(se->getType()))->getLowerType()),SymbolTable::getLabel());
        }else{
            temp = new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel()); 
        }
        dst = new Operand(temp);
        isLval = false;
    };
    Id(SymbolEntry *se, ExprNode* arrayIndex) : ExprNode(se), arrayIndex(arrayIndex){
        SymbolEntry* temp = new TemporarySymbolEntry(new PointerType(((ArrayType*)(se->getType()))->getLowerType()),SymbolTable::getLabel());
        dst = new Operand(temp);
        isLval = false;
    };
    void output(int level);
    void typeCheck();
    void genCode();
    void setLval(){isLval = true;}//只设置数组的Lval，在gencode中使用
    double getValue();
};

class StmtNode : public Node
{
public:
    StmtNode* next;
    std::string id_str;
    StmtNode(): next(nullptr), id_str(""){}
    StmtNode(StmtNode* next): next(next), id_str(""){}
    StmtNode(StmtNode* next, std::string id_str): next(next), id_str(id_str){}
};

class CompoundStmt : public StmtNode
{
private:
    StmtNode *stmt;
public:
    CompoundStmt(StmtNode *stmt) : stmt(stmt) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class SeqNode : public StmtNode
{
private:
    StmtNode *stmt1, *stmt2;
public:
    SeqNode(StmtNode *stmt1, StmtNode *stmt2) : stmt1(stmt1), stmt2(stmt2){};
    void output(int level);
    void typeCheck();
    void genCode();
};


class IfStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
public:
    IfStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class IfElseStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    StmtNode *elseStmt;
public:
    IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt) : cond(cond), thenStmt(thenStmt), elseStmt(elseStmt) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

//zsr add 2022年11月17日16:36:27
class WhileStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *loopStmt;
    BasicBlock* cond_bb, *end_bb;
public:
    WhileStmt(ExprNode *cond, StmtNode *loopStmt) : cond(cond), loopStmt(loopStmt){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class BreakStmt :public StmtNode
{
public:
    BreakStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ContinueStmt :public StmtNode
{
    
public:
    ContinueStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();   
};

class ReturnStmt : public StmtNode
{
private:
    ExprNode *retValue;
public:
    ReturnStmt() : retValue(nullptr) {}; 
    ReturnStmt(ExprNode*retValue) : retValue(retValue) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class AssignStmt : public StmtNode
{
private:
    ExprNode *lval;
    ExprNode *expr;
public:
    AssignStmt(ExprNode *lval, ExprNode *expr) : lval(lval), expr(expr) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

//2022年11月4日15:11:23 add by zsr
class VarDef : public StmtNode
{
private:
    // std::string id_str;
    ExprNode* Val;
    std::vector<int> index;
public:
    // VarDef* next;
    ExprNode* getVal(){return Val;}
    void setVal(ExprNode* newVal){Val = newVal;};
    VarDef(StmtNode* next, std::string id_str): StmtNode(next, id_str), Val(nullptr){index = std::vector<int>();}
    VarDef(StmtNode* next, std::string id_str, ExprNode* Val): StmtNode(next, id_str), Val(Val){index = std::vector<int>();}
    VarDef(StmtNode* next, std::string id_str, ExprNode* Val, std::vector<int> index): StmtNode(next, id_str), Val(Val), index(index){};
    
    void output(int level);
    // std::string getStr(){return id_str;}
    void typeCheck();
    void genCode();
    std::vector<int> getIndex(){return index;};
};

class VarDeclStmt : public StmtNode
{
private:
    std::vector<SymbolEntry*>* defEntries;
    StmtNode* defHead;
    Type* type;
public:
    VarDeclStmt(std::vector<SymbolEntry*>* defEntries,StmtNode* defHead, Type* type): defEntries(defEntries), defHead(defHead),type(type){}
    void output(int level);
    void typeCheck();
    void genCode();
};

class ConstDef : public StmtNode
{
private:
    ExprNode* Val;
    std::vector<int> index;
public:
    // ConstDef* next;
    ConstDef(StmtNode* next, std::string id_str, ExprNode* Val/*Need ConstInitial val*/):   StmtNode(next, id_str), Val(Val){index = std::vector<int>();}
    ConstDef(StmtNode* next, std::string id_str, ExprNode* Val, std::vector<int> index): StmtNode(next, id_str), Val(Val), index(index){}
    void output(int level);
    void typeCheck();
    void genCode();
    ExprNode* getVal(){return Val;};
    void setVal(ExprNode* newVal){Val = newVal;};
    std::vector<int> getIndex(){return index;};
};

class ConstDeclStmt : public StmtNode
{
private:
    //应该需要一个vector来存储se
    std::vector<SymbolEntry*>* defEntries;
    StmtNode* defHead;
    Type* type;
public:
    ConstDeclStmt(std::vector<SymbolEntry*>* defEntries,StmtNode* defHead,Type* type): defEntries(defEntries),defHead(defHead),type(type){}
    void output(int level);
    // std::vector<SymbolEntry*>* getEntries(){return defEntries;}
    void typeCheck();
    void genCode();
};

class FuncFParam : public StmtNode{
private:
    Type* paraType;
    std::string nameStr;
    SymbolEntry* se;    //参数ID的SymbolEntry  2022/12/14 new xzh
    std::vector<int> arrayConstIndex;
public:
    FuncFParam(Type* paraType, std::string nameStr,SymbolEntry*se,std::vector<int> arrayConstIndex): StmtNode(nullptr),paraType(paraType), nameStr(nameStr),se(se),arrayConstIndex(arrayConstIndex){}
    void output(int level);
    void typeCheck();
    void genCode();
    std::string getName(){return nameStr;};
    SymbolEntry* getSymPtr(){return se;}
};

class FuncInvoke : public ExprNode{
private:
    std::string funcName;
    ExprNode* Rparams;
public:
    FuncInvoke(SymbolEntry* se, std::string funcName, ExprNode* Rparams): ExprNode(se), funcName(funcName), Rparams(Rparams){}
    void output(int level);
    void typeCheck();
    void genCode();
};
//
class BlankStmt : public StmtNode{
public:
    BlankStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ExprStmt : public StmtNode{
private:
    ExprNode* myExp;
public:
    ExprStmt(ExprNode* myExp): myExp(myExp){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class FunctionDef : public StmtNode
{
private:
    SymbolEntry *se;
    StmtNode *stmt;
    StmtNode *funcPDef;
public:
    FunctionDef(SymbolEntry *se, StmtNode *stmt,StmtNode* funcPDef) : se(se), stmt(stmt), funcPDef(funcPDef){};
    SymbolEntry* getSymptr(){return se;};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ArrayInitialVal : public ExprNode{
private:
    ExprNode* parent;
    std::vector<ExprNode*> children;
    ExprNode* value;
    bool isLeaf;
    bool initialized;
    int Level;
    bool checked;
public:
    ArrayInitialVal(): ExprNode(nullptr),parent(nullptr),initialized(false),isLeaf(false),value(nullptr),Level(0),checked(false){};
    ArrayInitialVal(ExprNode* val): ExprNode(nullptr),parent(nullptr),initialized(true),isLeaf(true),value(val),Level(0),checked(false){};
    
    ExprNode* getParent(){return parent;}
    std::vector<ExprNode*> getChildren(){return children;}
    void setChildren(std::vector<ExprNode*> newChildren){children.swap(newChildren);}
    void addChild(ExprNode* child){children.push_back(child);}
    void deleteChild(ExprNode* child){children.erase(std::find(children.begin(),children.end(),child));}
    void replaceChild(ExprNode* oldChild, ExprNode* newChild){std::replace(std::begin(children),std::end(children),oldChild, newChild);}
    void setValue(ExprNode* value){this->value = value;}
    void setParent(ExprNode* parent){this->parent = parent;};
    void setLeaf(){isLeaf = true;}
    void setInitialized(){initialized = true;}
    void setLevel(int level){
        Level = std::max(level, Level);
        if(parent){
            ((ArrayInitialVal*)parent)->setLevel(Level+1);
        }
    }      
    void addZeroNode(std::vector<int> arrayIndexDefine, Type* zeroType);
    int getLevel(){ return Level; }
    void setRecurseLevel(int Level);
    void output(int level);
    void genCode();
    void typeCheck();
    void adjustTreeToNormal();
    ExprNode* getValWithIndex(std::vector<int> indexVec);
    ExprNode* getVal(){return value;};
    void packSubtree(std::vector<int> arrayIndexDefine, Type* zeroType);
    void testPackTree(std::vector<int> arrayIndexDefine, Type* zeroType);
    void checkTreeType(Type* targetType);
    void setChecked(){checked=true;};
    bool isChecked(){return checked;};
    std::string toStr(Type* nowType);
    void toVector(std::vector<double>* valueVec);
};

class Ast
{
private:
    Node* root;
public:
    Ast() {root = nullptr;}
    void setRoot(Node*n) {root = n;}
    void output();
    void typeCheck();
    void genCode(Unit *unit);
};

ArrayInitialVal* zeroValTree(int level, std::vector<int> arrayIndexDefine, Type* zeroType);

#endif
