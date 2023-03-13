#include "Ast.h"
#include "SymbolTable.h"
#include <iostream>
#include <string>
#include "Type.h"

extern FILE *yyout;
int Node::counter = 0;

Node::Node()
{
    seq = counter++;
}

void Ast::output()
{
    fprintf(yyout, "program\n");
    if(root != nullptr)
        root->output(4);
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
        fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s array dimension: ", level, ' ',
            name.c_str(), scope, type.c_str());
        this->arrayIndex->output(4);
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

// void DeclStmt::output(int level)
// {
//     fprintf(yyout, "%*cDeclStmt\n", level, ' ');
//     Id->output(level + 4);
// }

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
    fprintf(yyout, "%*cConstDef ID: %s ConstInitVal:\n", level, ' ', this->id_str.c_str()); 
    val->output(level + 4);      
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