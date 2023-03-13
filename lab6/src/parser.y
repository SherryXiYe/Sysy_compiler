%code top{
    #include <iostream>
    #include <assert.h>
    #include "parser.h"
    #include <cstring>
    #include <vector>
    #include "Ast.h"
    extern Ast ast;
    extern FILE *yyout;
    int Level = 0;
    std::vector<int> arrayIndexDefine;
    FunctionType* curFunc = nullptr;
    ArrayInitialVal* curInitValList = nullptr;
    std::vector<ExprNode*> ArrayInitialValVector; 
    Type* constNowType = nullptr;
    Type* varNowType = nullptr;
    std::vector<SymbolEntry*>* constEntries;
    int yylex();
    int yyerror( char const * );
}

%code requires {
    #include "Ast.h"
    #include "SymbolTable.h"
    #include "Type.h"
}

%union {
    int itype;
    float floattype;       
    char* strtype;
    StmtNode* stmttype;
    ExprNode* exprtype;
    Type* type;
}

%start Program
%token <strtype> ID STRING
%token <itype> INTEGER
%token <floattype> FLOATNUM
%token IF ELSE WHILE BREAK CONTINUE
%token INT VOID FLOAT
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA LBRACKET RBRACKET 
%token ADD SUB MUL DIV MOD OR AND LESS GREATER GREATEREQUAL LESSEQUAL EQUAL NOTEQUAL ASSIGN 
%token CONST NOT                 
%token RETURN



%type <stmttype> Stmts Stmt AssignStmt BlockStmt IfStmt WhileStmt BreakStmt ContinueStmt ReturnStmt DeclStmt FuncDef ConstDef VarDef VarDefList VarDecl ConstDecl ConstDefList FuncFParams FuncFParam ExprStmt 
%type <exprtype> Exp AddExp Cond LOrExp PrimaryExp LVal RelExp LAndExp UnaryExp Number MulExp EqExp ConstInitVal ConstInitValList ConstExp InitialValList InitVal FuncRParams ArrayIndex ArrayConstIndex
%type <type> Type

%precedence THEN
%precedence ELSE
%%
Program
    : Stmts {
        ast.setRoot($1);
    }
    ;
Stmts
    : Stmt {$$=$1;}
    | Stmts Stmt{
        $$ = new SeqNode($1, $2);
    }
    ;
Stmt                            
    : AssignStmt {$$=$1;}
    | BlockStmt {$$=$1;}
    | IfStmt {$$=$1;}
    | ReturnStmt {$$=$1;}
    | DeclStmt {$$=$1;}
    | FuncDef {$$=$1;}
    | WhileStmt {$$=$1;}
    | BreakStmt { $$=$1;}
    | ContinueStmt { $$=$1;}
    | ExprStmt {$$=$1;}
    ;
ExprStmt
    : Exp SEMICOLON {
        $$ = new ExprStmt($1);
    }
    | SEMICOLON {
        $$= new BlankStmt();
    }
    ;
AssignStmt
    :
    LVal ASSIGN Exp SEMICOLON {
        $$ = new AssignStmt($1, $3);
    }
    ;
BlockStmt
    :   LBRACE 
        {identifiers = new SymbolTable(identifiers);} 
        Stmts RBRACE 
        { 
            $$ = new CompoundStmt($3); 
            SymbolTable *top = identifiers;
            identifiers = identifiers->getPrev();
            delete top;
        }
    |
    LBRACE RBRACE{ //2022年11月6日14:45:13 add by zsr
        $$ = new CompoundStmt(nullptr);
    }
    ;
IfStmt
    : IF LPAREN Cond RPAREN Stmt %prec THEN {
        $$ = new IfStmt($3, $5);
    }
    | IF LPAREN Cond RPAREN Stmt ELSE Stmt {
        $$ = new IfElseStmt($3, $5, $7);
    }
    ;
WhileStmt
    : WHILE LPAREN Cond RPAREN Stmt {
        $$ = new WhileStmt($3, $5);
    }
    ;
BreakStmt                          
    : BREAK SEMICOLON {
        $$ = new BreakStmt();
    }
    ;
ContinueStmt
    : CONTINUE SEMICOLON {
        $$ = new ContinueStmt();
    }
    ;
ReturnStmt
    : RETURN Exp SEMICOLON {
        $$ = new ReturnStmt($2);
    }
    | RETURN SEMICOLON {
        $$ = new ReturnStmt();
    }
    ;
ConstExp
    : AddExp { $$ = $1;}
    ;
Exp
    : AddExp {$$ = $1;}
    ;
AddExp             
    : MulExp {$$ = $1;}
    |
    AddExp ADD MulExp
    {
        SymbolEntry *se = nullptr;
        Type* expr1Type=$1->getSymPtr()->getType();
        Type* expr2Type=$3->getSymPtr()->getType();
        SetBinaryType(se, expr1Type,expr2Type);
        if(se->getType()->isInt()){
            $$ = new BinaryExpr(se, BinaryExpr::ADD, $1, $3);
        }else if(se->getType()->isFloat()){
            $$ = new BinaryExpr(se, BinaryExpr::FADD, $1, $3);
        }
    }
    |
    AddExp SUB MulExp
    {
        SymbolEntry *se = nullptr;
        Type* expr1Type=$1->getSymPtr()->getType();
        Type* expr2Type=$3->getSymPtr()->getType();
        SetBinaryType(se, expr1Type,expr2Type);
        if(se->getType()->isInt()){
             $$ = new BinaryExpr(se, BinaryExpr::SUB, $1, $3);
        }else if(se->getType()->isFloat()){
            $$ = new BinaryExpr(se, BinaryExpr::FSUB, $1, $3);
        }
    }
    ;
MulExp                  
    :
    UnaryExp { $$=$1;}
    |
    MulExp MUL UnaryExp
    {
        SymbolEntry *se = nullptr;
        Type* expr1Type=$1->getSymPtr()->getType();
        Type* expr2Type=$3->getSymPtr()->getType();
        SetBinaryType(se, expr1Type,expr2Type);
        if(se->getType()->isInt()){
            $$ = new BinaryExpr(se, BinaryExpr::MUL, $1, $3);
        }else if(se->getType()->isFloat()){
            $$ = new BinaryExpr(se, BinaryExpr::FMUL, $1, $3);
        }
    }
    |
    MulExp DIV UnaryExp
    {
        SymbolEntry *se = nullptr;
        Type* expr1Type=$1->getSymPtr()->getType();
        Type* expr2Type=$3->getSymPtr()->getType();
        SetBinaryType(se, expr1Type,expr2Type);
        if(se->getType()->isInt())
            $$ = new BinaryExpr(se, BinaryExpr::DIV, $1, $3);
        else if(se->getType()->isFloat())
            $$ = new BinaryExpr(se, BinaryExpr::FDIV, $1, $3);
    }
    |
    MulExp MOD UnaryExp
    {
        SymbolEntry *se = nullptr;
        Type* expr1Type=$1->getSymPtr()->getType();
        Type* expr2Type=$3->getSymPtr()->getType();
        SetBinaryType(se, expr1Type,expr2Type);
        if(se->getType()->isInt()){
            $$ = new BinaryExpr(se, BinaryExpr::MOD, $1, $3);
        }else if(se->getType()->isFloat()){
            fprintf(stderr, "Wrong float operand type of modulus operation.");
            exit(EXIT_FAILURE);
        }
    }
    ;
UnaryExp                
    : PrimaryExp { $$=$1;}
    | ID LPAREN FuncRParams RPAREN {
        SymbolEntry *se;
        se = identifiers->lookup($1);
        if(se == nullptr)
        {
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }
        $$ = new FuncInvoke(se, $1, $3);
        
    }
    | ADD UnaryExp {
        $$ = $2; 
    }
    | SUB UnaryExp {
        SymbolEntry *se = nullptr;
        Type* exprType=$2->getSymPtr()->getType();
        if(exprType->isFunc()){
            FunctionType* exprTypeFunc=dynamic_cast<FunctionType*>(exprType);
            exprType=exprTypeFunc->getRetType();
        }else if(exprType->isArray()){         //待定
            ArrayType* exprTypeArr=dynamic_cast<ArrayType*>(exprType);            
            exprType=exprTypeArr->getElementType();
        } 
        if(exprType->isInt()){
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }else{
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        $$ = new UnaryExpr(se, UnaryExpr::UMINUS, $2);
    }
    | NOT UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::NOT, $2);
    }
    ;
PrimaryExp
    : LPAREN Exp RPAREN   { $$=$2;}       //2022/11/3 xzh new
    | LVal { $$ = $1;}
    | Number {
        $$ = $1;                  //2022/11/3 xzh new
    }
    | STRING {
        SymbolEntry* se;
        SymbolTable* tablePtr=identifiers;
        while(tablePtr->getPrev()){
            tablePtr=tablePtr->getPrev();
        }
        SymbolTable* globals = tablePtr;
        se = globals->lookup(std::string($1));
        if(se == nullptr){
            Type* type = new StringType(strlen($1));
            se = new ConstantSymbolEntry(type, std::string($1));
            globals->install(std::string($1), se);
        }
        ExprNode* expr = new Constant(se);
        $$ = expr;
    }
    ;
    
LVal                   
    : ID ArrayIndex {       //2022/11/4 xzh 改
        SymbolEntry *se;
        se = identifiers->lookup($1);
        if(se == nullptr)
        {
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }
        $$ = new Id(se,$2);
        delete []$1;
    } 
    | ID {
        SymbolEntry *se;
        se = identifiers->lookup($1);
        if(se == nullptr)
        {
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }
        $$ = new Id(se);
        delete []$1;
    }
    ;
ArrayIndex                      
    : LBRACKET Exp RBRACKET {
        $$ = $2;
    }
    | LBRACKET Exp RBRACKET ArrayIndex{
        $2->next = $4;
        $$ = $2;
    }
    ;
Number                          
    : INTEGER {        
        // std::cout<<$1<<std::endl;         
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::intType, $1);
        $$ = new Constant(se);
    }
    | FLOATNUM {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::floatType, $1);
        $$ = new Constant(se);
    }
    ;
FuncRParams                 
    : Exp {
        $$ = $1;
    }
    | Exp COMMA FuncRParams {
        $1->next = $3;
        $$ = $1;
    }
    | %empty {$$ = nullptr;}
    ;
Cond
    : LOrExp { $$ = $1;}
    ;
LOrExp
    : LAndExp {$$ = $1;}
    | LOrExp OR LAndExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::OR, $1, $3);
    }
    ;
LAndExp
    : EqExp {$$ = $1;}       //2022/11/3 xzh 改 
    | LAndExp AND EqExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::AND, $1, $3);
    }
    ;
EqExp                      
    : RelExp { $$=$1; }
    | EqExp EQUAL RelExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::EQUAL, $1, $3);
    }
    | EqExp NOTEQUAL RelExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NOTEQUAL, $1, $3);
    }
    ;
RelExp                      
    :
    AddExp {$$ = $1;}
    |
    RelExp LESS AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESS, $1, $3);
    }
    |
    RelExp GREATER AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GREATER, $1, $3);
    }
    | 
    RelExp LESSEQUAL AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESSEQUAL, $1, $3);
    }
    |
    RelExp GREATEREQUAL AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GREATEREQUAL, $1, $3);
    }
    ;

Type
    : INT {
        $$ = TypeSystem::intType;
    }
    | VOID {
        $$ = TypeSystem::voidType;
    }
    | FLOAT {
        $$ = TypeSystem::floatType;      //2022/11/3 xzh new   
    }
    ;
DeclStmt                 
    : ConstDecl { $$=$1; }
    | VarDecl  { $$=$1; }
    ;
ConstDecl
    : CONST Type {
        constEntries = new std::vector<SymbolEntry*>();
        if($2->isInt())
            constNowType = TypeSystem::constIntType;
        else // if(1->toStr() == "float")
            constNowType = TypeSystem::constFloatType;
    } ConstDefList SEMICOLON{
        $$ = new ConstDeclStmt(constEntries, $4,constNowType);
        constNowType = nullptr;
    }
    ;
ConstDefList 
    : ConstDef COMMA ConstDefList {
        $1->next = $3;
        $$ = $1;
    }
    | ConstDef {
        $$ = $1;
    }
    ;
ConstDef 
    : ID ArrayConstIndex ASSIGN ConstInitVal {      //2022年11月4日20:16:28 缺少关于ArrayConstIndex的参数
        ExprNode* indexPtr = $2;
        while(indexPtr != nullptr){
            arrayIndexDefine.push_back((int)indexPtr->getValue());
            indexPtr=indexPtr->next;
        }
        //初值树重构
        ArrayInitialVal* initVal = dynamic_cast<ArrayInitialVal*>($4);
        assert(initVal != nullptr);
        initVal->adjustTreeToNormal();
        initVal->setRecurseLevel(arrayIndexDefine.size());
        initVal->setLevel(arrayIndexDefine.size());
        Type* zeroType;
        if(constNowType->isConstInt())
            zeroType = TypeSystem::intType;
        else
            zeroType = TypeSystem::floatType;
        initVal->testPackTree(arrayIndexDefine, zeroType);
        std::string nowId($1);
        SymbolEntry* nowId_se=identifiers->lookup(nowId);
        if(nowId_se!=nullptr){
            if(((IdentifierSymbolEntry*)nowId_se)->getScope()==identifiers->getLevel()){
                fprintf(stderr, "identifier %s is redeclined\n", nowId.c_str());
                exit(EXIT_FAILURE);
            }
        }
        //记得读取ID的arrayindex
        std::vector<int> index = arrayIndexDefine;
        std::reverse(index.begin(),index.end());
        IdentifierSymbolEntry* se;
        int i=1;
        Type* typePtr= new ArrayType(constNowType, index[0]);
        Type* arrType = typePtr;
        while(i<index.size()){
            arrType = new ArrayType(typePtr, index[i]);
            i++;
            typePtr = arrType;
        }
        se = new IdentifierSymbolEntry(arrType, nowId, identifiers->getLevel());
        //设置初值树的根节点
        if(initVal){
            se->setArrayValRoot(initVal);
            SymbolEntry* val_se = new TemporarySymbolEntry(arrType, SymbolTable::getLabel());
            initVal->setSymPtr(val_se);
        }
        identifiers->install(nowId, se);
        // constEntries->push_back(se);
        
        constEntries->push_back(se);
        $$ = new ConstDef(nullptr, $1, initVal, arrayIndexDefine);
        std::vector<int>().swap(arrayIndexDefine);
    }
    | ID ASSIGN ConstInitVal {          //2022年11月5日11:53:58 add by zsr
        std::string nowId($1);
        SymbolEntry* nowId_se=identifiers->lookup(nowId);
        if(nowId_se!=nullptr){
            if(((IdentifierSymbolEntry*)nowId_se)->getScope()==identifiers->getLevel()){
                fprintf(stderr, "identifier %s is redeclined\n", nowId.c_str());
                exit(EXIT_FAILURE);
            }
        }
        IdentifierSymbolEntry* se = new IdentifierSymbolEntry(constNowType, nowId, identifiers->getLevel());
        se->setValue($3->getValue());
        identifiers->install(nowId, se);
        constEntries->push_back(se);
        $$ = new ConstDef(nullptr, $1, $3);
    }
    ;
ConstInitVal
    : ConstExp {
        $$ = $1;
    }
    | LBRACE {
        ArrayInitialVal* newInitList = new ArrayInitialVal();
        newInitList->setParent(curInitValList);
        curInitValList = newInitList;
        Level++;
    } 
    ConstInitValList RBRACE {
        ExprNode* parentArr =((ArrayInitialVal*)$3)->getParent();
        if(parentArr!=nullptr){
            curInitValList = dynamic_cast<ArrayInitialVal*>(parentArr);
        }
        $$ = $3;
        Level--;
    }
    | LBRACE RBRACE {
        ArrayInitialVal* newNode = new ArrayInitialVal();       //2022/12/16 new xzh
        newNode->setParent(curInitValList);
        // newNode->setRealLevel(-1);
        $$=newNode;
    }
    ;
ConstInitValList
    : ConstInitVal {
        ExprNode* newVal = new ArrayInitialVal($1);
        curInitValList->addChild(newVal);
        ((ArrayInitialVal*)newVal)->setParent(curInitValList);
        $$ = curInitValList;
    }
    | ConstInitVal COMMA {
        ExprNode* newVal = new ArrayInitialVal($1);
        curInitValList->addChild(newVal);
        ((ArrayInitialVal*)newVal)->setParent(curInitValList);      
    } 
    ConstInitValList {
        $$ = curInitValList;
    }
    ;
ArrayConstIndex
    :
    LBRACKET ConstExp RBRACKET
    {
        $$ = $2;
    }
    |
    LBRACKET ConstExp RBRACKET ArrayConstIndex 
    {
        $2->next = $4;
        $$ = $2;
    }
    |
    LBRACKET RBRACKET
    {
        $$ = new NullExpr();
    }
    |
    LBRACKET RBRACKET ArrayConstIndex 
    {
        $$ = new NullExpr();
        $$->next = $3;
    }
    /* | %empty { comment by zsr 2022年11月5日11:58:56

    } */
    ;
VarDecl
    : Type VarDefList SEMICOLON {
        std::vector<SymbolEntry*>* varEntries = new std::vector<SymbolEntry*>();
        StmtNode* nowDef = $2;
        while(nowDef){
            std::string nowId = nowDef->id_str;
            // std::cout<<"1"<<nowId<<std::endl;
            SymbolEntry* nowId_se=identifiers->lookup(nowId);
            if(nowId_se!=nullptr){
                if(((IdentifierSymbolEntry*)nowId_se)->getScope()==identifiers->getLevel()){
                    fprintf(stderr, "identifier %s is redeclined\n", nowId.c_str());
                    exit(EXIT_FAILURE);
                }
            }
            //记得读取ID的arrayindex
            std::vector<int> index = ((VarDef*)nowDef)->getIndex();
            std::reverse(index.begin(),index.end());
            if(index.size()==0){
                IdentifierSymbolEntry* se = new IdentifierSymbolEntry($1, nowId, identifiers->getLevel());
                identifiers->install(nowId, se);
                nowDef = nowDef->next;
                varEntries->push_back(se);
            }else {
                int i=1;
                Type* typePtr= new ArrayType($1, index[0]);
                Type* arrType = typePtr;
                while(i<index.size()){
                    arrType = new ArrayType(typePtr, index[i]);
                    i++;
                    typePtr = arrType;
                }
                IdentifierSymbolEntry* se = new IdentifierSymbolEntry(arrType, nowId, identifiers->getLevel());
                ExprNode* val = ((VarDef*)nowDef)->getVal();
                //设置初值树的根节点
                if(val){
                    //初值树重构
                    ArrayInitialVal* initVal = dynamic_cast<ArrayInitialVal*>(val);
                    assert(initVal != nullptr);
                    std::vector<int> arrayIndex = ((VarDef*)nowDef)->getIndex();
                    initVal->setLevel(arrayIndex.size());
                    initVal->adjustTreeToNormal();
                    initVal->setRecurseLevel(arrayIndex.size());
                    initVal->testPackTree(arrayIndex,$1);
                    se->setArrayValRoot(initVal);
                    SymbolEntry* val_se = new TemporarySymbolEntry(arrType, SymbolTable::getLabel());
                    initVal->setSymPtr(val_se);
                }
                identifiers->install(nowId, se);
                varEntries->push_back(se);
                nowDef = nowDef->next;
            }           
        }
        $$ = new VarDeclStmt(varEntries, $2,$1);
    }
    ;
VarDefList
    :
    VarDef 
    {
        $$ = $1;
    }
    |
    VarDef COMMA VarDefList
    { 
        $1->next = $3;
        $$ = $1;
    }
    ;
VarDef
    :
    ID ArrayConstIndex
    {
        ExprNode* indexPtr = $2;
        while(indexPtr != nullptr){
            arrayIndexDefine.push_back((int)indexPtr->getValue());
            indexPtr=indexPtr->next;
        }
        $$ = new VarDef(nullptr, $1,nullptr,arrayIndexDefine);
        std::vector<int>().swap(arrayIndexDefine);
    }
    |
    ID ArrayConstIndex ASSIGN InitVal
    {
        ExprNode* indexPtr = $2;
        while(indexPtr != nullptr){
            arrayIndexDefine.push_back((int)indexPtr->getValue());
            indexPtr=indexPtr->next;
        }
        $$ = new VarDef(nullptr, $1,$4, arrayIndexDefine);
        std::vector<int>().swap(arrayIndexDefine);
    }
    |
    ID {
        $$ = new VarDef(nullptr,$1);     //2022/11/5 xzh new 
    }
    |
    ID ASSIGN InitVal {                 //2022/11/5 xzh new 
        $$ = new VarDef(nullptr,$1,$3);
    }
    ;
InitVal
    :
    Exp{
        $$=$1;
    }
    |
    LBRACE {
        ArrayInitialVal* newInitList = new ArrayInitialVal();
        newInitList->setParent(curInitValList);
        curInitValList = newInitList;
        Level++;
    } 
     InitialValList RBRACE
    {
        ExprNode* parentArr =((ArrayInitialVal*)$3)->getParent();
        if(parentArr!=nullptr){
            curInitValList = dynamic_cast<ArrayInitialVal*>(parentArr);
        }
        $$ = $3;
        Level--;
    }
    |
    LBRACE RBRACE
    {
        ArrayInitialVal* newNode = new ArrayInitialVal();       //2022/12/16 new xzh
        newNode->setParent(curInitValList);
        // newNode->setRealLevel(-1);
        $$=newNode;
    }
    ;
InitialValList
    :
    InitVal
    {
        ExprNode* newVal = new ArrayInitialVal($1);
        curInitValList->addChild(newVal);
        ((ArrayInitialVal*)newVal)->setParent(curInitValList);
        $$ = curInitValList;
    }
    | InitVal COMMA {
        ExprNode* newVal = new ArrayInitialVal($1);
        curInitValList->addChild(newVal);
        ((ArrayInitialVal*)newVal)->setParent(curInitValList);      
    } 
    InitialValList{
        $$ = curInitValList;
    }
    ;
    
FuncDef
    :
    Type ID {
        Type *funcType;
        funcType = new FunctionType($1,{});
        SymbolEntry *se = new IdentifierSymbolEntry(funcType, $2, identifiers->getLevel());
        identifiers->install($2, se);
        identifiers = new SymbolTable(identifiers);
        curFunc = dynamic_cast<FunctionType*>(funcType);
    }
    LPAREN FuncFParams RPAREN           //2022/11/3 xzh 改
    BlockStmt
    {
        SymbolEntry *se;
        se = identifiers->lookup($2);
        assert(se != nullptr);
        $$ = new FunctionDef(se, $7, $5);
        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev();
        delete top;
        delete []$2;
    }
    ;
FuncFParams
    : FuncFParam {
        $$ = $1;
    }
    | FuncFParam COMMA FuncFParams {
        $1->next = $3;
        $$ = $1;
    }
    | %empty {$$=nullptr;}
    ;
FuncFParam
    : Type ID ArrayConstIndex {
        ExprNode* indexPtr = $3->next;//跳过了第一个
        std::vector<int> dimVec;
        while(indexPtr){
            if(indexPtr->isNullExpr()){
                fprintf(stderr, "function array formal parameter error\n");
                exit(EXIT_FAILURE);
            }else{
                dimVec.push_back(indexPtr->getValue());
                indexPtr=indexPtr->next;
            }
        }
        std::reverse(dimVec.begin(),dimVec.end());
        Type* nowType = $1;
        for(int i=0; i<dimVec.size(); i++){
            ArrayType* tempArr = new ArrayType(nowType, dimVec[i]);
            nowType = tempArr;            
        }
        Type* arrType = new ArrayType(nowType, -1);
        SymbolEntry *se = new IdentifierSymbolEntry(arrType, $2, identifiers->getLevel(),true);
        identifiers->install($2, se);
        dimVec.insert(dimVec.begin(), -1);
        $$ = new FuncFParam(arrType, $2,se,dimVec);
        curFunc->addParamsType(arrType);
    }
    |
    Type ID {
        SymbolEntry *se = new IdentifierSymbolEntry($1, $2, identifiers->getLevel(),true);
        identifiers->install($2, se);
        $$ = new FuncFParam($1, $2,se,{});
        curFunc->addParamsType($1);
        // SymbolEntry* parSe = identifiers->lookup("c");
        // if(parSe==nullptr){
        //     std::cout<<"parser_pn"<<std::endl;
        // }else {
        //     std::cout<<parSe->toStr()<<std::endl;
        // }
    }
    ;

%%

/* int yyerror(char const* message)
{
    std::cerr<<message<<std::endl;
    return -1;
} */
int yyerror(char const *s)
{
	extern int yylineno;	// defined and maintained in lex
	extern char *yytext;	// defined and maintained in lex
	char buf[1000]={0};
	strcpy(buf, yytext);
	fprintf(yyout, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
    return -1;
}
