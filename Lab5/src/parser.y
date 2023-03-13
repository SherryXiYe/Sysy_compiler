%code top{
    #include <iostream>
    #include <assert.h>
    #include "parser.h"
    #include <cstring>
    extern Ast ast;
    extern FILE *yyout;
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
%token <strtype> ID 
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
        //if($1.)
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::ADD, $1, $3);
    }
    |
    AddExp SUB MulExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::SUB, $1, $3);
    }
    ;
MulExp                  
    :
    UnaryExp { $$=$1;}
    |
    MulExp MUL UnaryExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MUL, $1, $3);
    }
    |
    MulExp DIV UnaryExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::DIV, $1, $3);
    }
    |
    MulExp MOD UnaryExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MOD, $1, $3);
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
        $$ = new FuncInvoke(se,std::string($1), $3);
        
    }
    | ADD UnaryExp {
        $$ = $2; 
    }
    | SUB UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::UMINUS, $2);
    }
    | NOT UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::NOT, $2);
    }
    ;
PrimaryExp
    : LPAREN Exp RPAREN   { $$=$2;}       //2022/11/3 xzh new
    | LVal { $$ = $1;}
    | Number {
        $$ = $1;                  //2022/11/3 xzh new
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
    | LBRACKET RBRACKET
    {
        $$ = new NullExpr();
    }
    |
    LBRACKET RBRACKET ArrayIndex 
    {
        $$ = new NullExpr();
        $$->next = $3;
    }
    /* | %empty { $$=nullptr;}                 *///comment by zsr 2022年11月5日12:00:22
    ;
Number                          
    : INTEGER {                 
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
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::OR, $1, $3);
    }
    ;
LAndExp
    : EqExp {$$ = $1;}       //2022/11/3 xzh 改 
    | LAndExp AND EqExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::AND, $1, $3);
    }
    ;
EqExp                      
    : RelExp { $$=$1; }
    | EqExp EQUAL RelExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::EQUAL, $1, $3);
    }
    | EqExp NOTEQUAL RelExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NOTEQUAL, $1, $3);
    }
    ;
RelExp                      
    :
    AddExp {$$ = $1;}
    |
    RelExp LESS AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESS, $1, $3);
    }
    |
    RelExp GREATER AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GREATER, $1, $3);
    }
    | 
    RelExp LESSEQUAL AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESSEQUAL, $1, $3);
    }
    |
    RelExp GREATEREQUAL AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
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
    : CONST Type ConstDefList SEMICOLON{
        std::vector<SymbolEntry*>* constEntries = new std::vector<SymbolEntry*>();
        StmtNode* nowDef = $3;
        Type* nowType;
        if($2->toStr() == "int")
            nowType = TypeSystem::constIntType;
        else // if(1->toStr() == "float")
            nowType = TypeSystem::constFloatType;
        // std::cout<<nowType->toStr()<<std::endl;
        while(nowDef){
            std::string nowId = nowDef->id_str;
            IdentifierSymbolEntry* se = new IdentifierSymbolEntry(nowType, nowId, identifiers->getLevel());      //第一个参数type有点问题 //zsr 2022年11月5日11:36:21 后修改
            identifiers->install(nowId, se);
            nowDef = nowDef->next;
            constEntries->push_back(se);
        }
        $$ = new ConstDeclStmt(constEntries, $3,nowType);
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
        $$ = new ConstDef(nullptr, $1, $4);
    }
    | ID ASSIGN ConstInitVal {          //2022年11月5日11:53:58 add by zsr
        $$ = new ConstDef(nullptr, $1, $3);
    }
    ;
ConstInitVal
    : ConstExp {
        $$ = $1;
    }
    | LBRACE ConstInitValList RBRACE {
        $$ = $2;
    }
    | LBRACE RBRACE {
        //wait to add
    }
    ;
ConstInitValList
    : ConstInitVal {
        $$ = $1;
    }
    | ConstInitVal COMMA ConstInitValList {
        $1->next = $3;
        $$ = $1;
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
            IdentifierSymbolEntry* se = new IdentifierSymbolEntry($1, nowId, identifiers->getLevel());
            identifiers->install(nowId, se);
            nowDef = nowDef->next;
            varEntries->push_back(se);
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
        $$ = new VarDef(nullptr, $1);
    }
    |
    ID ArrayConstIndex ASSIGN InitVal
    {
        $$ = new VarDef(nullptr, $1,$4);
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
    Exp
    {
        $$ = $1;
    }
    |
    LBRACE InitialValList RBRACE
    {
        $$ = $2;
    }
    |
    LBRACE RBRACE
    {
        
    }
    ;
InitialValList
    :
    InitVal
    {
        $$ = $1;
    }
    | InitVal COMMA InitialValList {
        $1->next = $3;
        $$ = $1;
    }
    /* |
    ExpList COMMA Exp
    {
        $1->next = $3;
        $$ = $1;
    } */
    ;
    /* Type ID SEMICOLON {
        SymbolEntry *se;
        se = new IdentifierSymbolEntry($1, $2, identifiers->getLevel());
        identifiers->install($2, se);
        $$ = new DeclStmt(new Id(se));
        delete []$2;
    }
    ; */
FuncDef
    :
    Type ID {
        Type *funcType;
        funcType = new FunctionType($1,{});
        SymbolEntry *se = new IdentifierSymbolEntry(funcType, $2, identifiers->getLevel());
        identifiers->install($2, se);
        identifiers = new SymbolTable(identifiers);
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
        SymbolEntry *se = new IdentifierSymbolEntry($1, $2, identifiers->getLevel());
        identifiers->install($2, se);
        $$ = new FuncFParam($1, $2);
    }
    |
    Type ID {
        SymbolEntry *se = new IdentifierSymbolEntry($1, $2, identifiers->getLevel());
        identifiers->install($2, se);
        $$ = new FuncFParam($1, $2);
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