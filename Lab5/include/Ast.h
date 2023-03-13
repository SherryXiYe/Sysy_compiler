#ifndef __AST_H__
#define __AST_H__

#include <fstream>
#include <vector>
#include <Type.h>

class SymbolEntry;

class Node
{
private:
    static int counter;
    int seq;
public:
    Node();
    int getSeq() const {return seq;};
    virtual void output(int level) = 0;
};

class ExprNode : public Node
{
protected:
    SymbolEntry *symbolEntry;
public:
    bool isInt;
    ExprNode* next;
    ExprNode(SymbolEntry *symbolEntry) : symbolEntry(symbolEntry), next(nullptr){};
};

class BinaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr1, *expr2;

public:
    enum {ADD, SUB, MUL, DIV, MOD, AND, OR, LESS, GREATER, GREATEREQUAL, LESSEQUAL, EQUAL, NOTEQUAL};
    BinaryExpr(SymbolEntry *se, int op, ExprNode*expr1, ExprNode*expr2) : ExprNode(se), op(op), expr1(expr1), expr2(expr2){};
    void output(int level);
};

class UnaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr;
public:
    enum {UMINUS, UADD, NOT};
    UnaryExpr(SymbolEntry *se, int op, ExprNode*expr) : ExprNode(se), op(op), expr(expr){};
    void output(int level);
};

class Constant : public ExprNode
{
public:
    Constant(SymbolEntry *se) : ExprNode(se){};
    void output(int level);
};

class Id : public ExprNode
{
    ExprNode* arrayIndex;
public:
    Id(SymbolEntry *se) : ExprNode(se), arrayIndex(nullptr){};
    Id(SymbolEntry *se, ExprNode* arrayIndex) : ExprNode(se), arrayIndex(arrayIndex){};
    void output(int level);
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
};

class SeqNode : public StmtNode
{
private:
    StmtNode *stmt1, *stmt2;
public:
    SeqNode(StmtNode *stmt1, StmtNode *stmt2) : stmt1(stmt1), stmt2(stmt2){};
    void output(int level);
};

// class DeclStmt : public StmtNode        
// {
// private:
//     Id *id;
// public:
//     DeclStmt(Id *id) : id(id){};
//     void output(int level);
// };

class IfStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
public:
    IfStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void output(int level);
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
};

//2022年11月3日17:20:52 add by zsr
class WhileStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *loopStmt;
public:
    WhileStmt(ExprNode *cond, StmtNode *loopStmt) : cond(cond), loopStmt(loopStmt){};
    void output(int level);
};

//2022/11/4 xzh new
class BreakStmt :public StmtNode
{
public:
    BreakStmt(){};
    void output(int level);
};
class ContinueStmt :public StmtNode
{
public:
    ContinueStmt(){};
    void output(int level);
};

class ReturnStmt : public StmtNode
{
private:
    ExprNode *retValue;
public:
    ReturnStmt() {};                //2022/11/4 xzh new
    ReturnStmt(ExprNode*retValue) : retValue(retValue) {};
    void output(int level);
};

class AssignStmt : public StmtNode
{
private:
    ExprNode *lval;
    ExprNode *expr;
public:
    AssignStmt(ExprNode *lval, ExprNode *expr) : lval(lval), expr(expr) {};
    void output(int level);
};


//2022年11月4日15:11:23 add by zsr
class VarDef : public StmtNode
{
private:
    // std::string id_str;
    ExprNode* Val;
public:
    // VarDef* next;
    VarDef(StmtNode* next, std::string id_str): StmtNode(next, id_str), Val(nullptr){}
    VarDef(StmtNode* next, std::string id_str, ExprNode* Val): StmtNode(next, id_str), Val(Val){}
    void output(int level);
    // std::string getStr(){return id_str;}
};
//2022年11月3日22:44:30 add by zsr
class VarDeclStmt : public StmtNode
{
private:
    std::vector<SymbolEntry*>* defEntries;
    StmtNode* defHead;
    Type* type;
public:
    VarDeclStmt(std::vector<SymbolEntry*>* defEntries,StmtNode* defHead, Type* type): defEntries(defEntries), defHead(defHead),type(type){}
    void output(int level);
};


class ConstDef : public StmtNode
{
private:
    ExprNode* val;
public:
    // ConstDef* next;
    ConstDef(ConstDef* next, std::string id_str, ExprNode* val/*Need ConstInitial val*/):   StmtNode(next, id_str), val(val){}
    // std::string getStr(){return id_str;}
    void output(int level);
};

// class ConstInitVal : public ExprNode{
// private:
// public:
//     ConstInitVal(SymbolEntry* se): ExprNode(se){}
// };

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
};


class FuncFParam : public StmtNode{
private:
    Type* paraType;
    std::string nameStr;
public:
    FuncFParam(Type* paraType, std::string nameStr): StmtNode(nullptr),paraType(paraType), nameStr(nameStr){}
    void output(int level);
};

class FuncInvoke : public ExprNode{
private:
    std::string funcName;
    ExprNode* Rparams;
public:
    FuncInvoke(SymbolEntry* se, std::string funcName, ExprNode* Rparams): ExprNode(se), funcName(funcName), Rparams(Rparams){}
    void output(int level);
};
//
class BlankStmt : public StmtNode{
public:
    BlankStmt(){};
    void output(int level);
};

class ExprStmt : public StmtNode{
private:
    ExprNode* myExp;
public:
    ExprStmt(ExprNode* myExp): myExp(myExp){};
    void output(int level);
};
//2022年11月6日15:24:35 add by zsr
class NullExpr : public ExprNode{
public:
    NullExpr(): ExprNode(nullptr){}
    void output(int level);
};

class FunctionDef : public StmtNode
{
private:
    SymbolEntry *se;
    StmtNode *stmt;
    StmtNode *funcPDef;
public:
    FunctionDef(SymbolEntry *se, StmtNode *stmt, StmtNode* funcPDef) : se(se), stmt(stmt), funcPDef(funcPDef){};
    void output(int level);
};

class Ast
{
private:
    Node* root;
public:
    Ast() {root = nullptr;}
    void setRoot(Node*n) {root = n;}
    void output();
};

#endif
