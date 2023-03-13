%option noyywrap
%{
    #define YY_NO_UNPUT
    #define YY_NO_INPUT

    #include "parser.h"
    #include <ostream>
    #include <fstream>
    using namespace std;

    extern FILE *yyin; 
    extern FILE *yyout;
    extern bool dump_tokens;

    void DEBUG_FOR_LAB4(std::string s){
        std::string DEBUG_INFO = "[DEBUG LAB4]: \t" + s + "\n";
        fputs(DEBUG_INFO.c_str(), yyout);
    }
%}

DECIMIAL ([1-9][0-9]*|0)
OCTAL (0[0-7]+)
HEXADECIMAL (0[xX][0-9A-Fa-f]+)
FLOATNUM (([0-9]+(\.[0-9]+)?([Ee]([+-])?[0-9]+)?)|(\.(([0-9]*)|([Ee]([+-])?[0-9]+)))|(0[xX]([1-9A-Fa-f][0-9A-Fa-f]*)?((\.[0-9A-Fa-f]*[Pp]([+-])?[0-9]+)|([Pp]([+-])?[0-9]+))))
ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]
BLOCKCOMMENTBEGIN \/\*
BLOCKCOMMENTELEMENT .
BLOCKCOMMENTEND \*\/
%x BLOCKCOMMENT
LINECOMMENT \/\/[^\n]*

%%

"int" {
    /*
    * Questions: 
    *   Q1: Why we need to return INT in further labs?
    *   Q2: What is "INT" actually?
    */
    if(dump_tokens)
        DEBUG_FOR_LAB4("INT\tint");
    return INT;
}
"float" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("FLOAT\tfloat");
    return FLOAT;
}
"void" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("VOID\tvoid");
    return VOID;
}
"const" {                   //new  2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("CONST\tconst");
    return CONST;
};
"if" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("IF\tif");
    return IF;
};
"else" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ELSE\telse");
    return ELSE;
};
"return" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RETURN\treturn");
    return RETURN;
}
"while" {           //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("WHILE\twhile");
    return WHILE;
};
"break" {           //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("BREAK\tbreak");
    return BREAK;
};
"continue" {           //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("CONTINUE\tcontinue");
    return CONTINUE;
};

"==" {                  //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("EQUAL\t==");
    return EQUAL;
}
"!=" {                  //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("NOTEQUAL\t!=");
    return NOTEQUAL;
}
"=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ASSIGN\t=");
    return ASSIGN;
}
"<=" {                  //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("LESSEQUAL\t<=");
    return LESSEQUAL;
}
">=" {                  //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("GREATEREQUAL\t>=");
    return GREATEREQUAL;
}
"<" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LESS\t<");
    return LESS;
}
">" {                  //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("GREATER\t>=");
    return GREATER;
}
"+" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ADD\t+");
    return ADD;
}
"-" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("SUB\t-");
    return SUB;
}
"*" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("MUL\t*");
    return MUL;
}
"/" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("DIV\t/");
    return DIV;
}
"%" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("MOD\t%");
    return MOD;
}
"&&" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("AND\t&&");
    return AND;
}
"||" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("OR\t||");
    return OR;
}
"!" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("NOT\t!");
    return NOT;
}
";" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("SEMICOLON\t;");
    return SEMICOLON;
}
"(" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LPAREN\t(");
    return LPAREN;
}
")" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RPAREN\t)");
    return RPAREN;
}
"{" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LBRACE\t{");
    return LBRACE;
}
"}" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RBRACE\t}");
    return RBRACE;
}
"[" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("LBRACKET\t[");
    return LBRACKET;
}
"]" {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("RBRACKET\t]");
    return RBRACKET;
}
"," {                   //new 2022/11/1
    if(dump_tokens)
        DEBUG_FOR_LAB4("COMMA\t,");
    return COMMA;
}

"putint" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paraTypes;
    paraTypes.push_back(TypeSystem::intType);
    Type* funcType = new FunctionType(TypeSystem::voidType, paraTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}
"getint" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paraTypes;
    Type* funcType = new FunctionType(TypeSystem::intType, paraTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}
"putch" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paraTypes;
    paraTypes.push_back(TypeSystem::intType);
    Type* funcType = new FunctionType(TypeSystem::voidType, paraTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}
"getch" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paraTypes;   //wu
    Type* funcType = new FunctionType(TypeSystem::intType, paraTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}

"putarray" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    //应该有两个参数类型：第一个为int a, 第二个是int型的数组,其dimension为(a)
    std::vector<Type*> paraTypes;
    paraTypes.push_back(TypeSystem::intType);
    std::vector<int> dim(1);
    ArrayType* nowArray = new ArrayType(TypeSystem::intType, dim);
    paraTypes.push_back(nowArray);
    Type* funcType = new FunctionType(TypeSystem::voidType, paraTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}

"getarray" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paraTypes;
    //只有一个参数类型： 一个int array 
    std::vector<int> dim(1);
    ArrayType* nowArray = new ArrayType(TypeSystem::intType, dim);
    paraTypes.push_back(nowArray);
    Type* funcType = new FunctionType(TypeSystem::intType, paraTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}

"putf" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    StringType* strType = new StringType();
    paramTypes.push_back(strType);
    Type* funcType = new FunctionType(TypeSystem::voidType, paramTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}
"putfloat" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paraTypes;
    paraTypes.push_back(TypeSystem::floatType);
    Type* funcType = new FunctionType(TypeSystem::voidType, paraTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}
"getfloat" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paraTypes;
    Type* funcType = new FunctionType(TypeSystem::floatType, paraTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}
"starttime" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;  //无参数
    Type* funcType = new FunctionType(TypeSystem::voidType, paramTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}

"stoptime" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    Type* funcType = new FunctionType(TypeSystem::voidType, paramTypes);
    SymbolTable* scope = identifiers;
    while(scope->getPrev())
        scope = scope->getPrev();//回到global作用域
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, scope->getLevel());
    scope->install(yytext, se);
    return ID;
}

{DECIMIAL} {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    yylval.itype = atoi(yytext);
    return INTEGER;
}

{OCTAL} {                   //new 2022/11/1
    int temp;
    sscanf(yytext, "%o", &temp);
    yylval.itype=temp;
    if(dump_tokens)
        DEBUG_FOR_LAB4(string("OCTAL\t").append(to_string(temp)));
        //offsets += strlen(yytext);
    return INTEGER;
}

{HEXADECIMAL} {             //new 2022/11/1
    int temp;
    sscanf(yytext, "%x", &temp);
    yylval.itype=temp;
    if(dump_tokens)
        DEBUG_FOR_LAB4(string("HEXADECIMAL\t").append(to_string(temp)));
        //offsets += strlen(yytext);
    return INTEGER;
}
{FLOATNUM} {                   //new 2022/11/1
    float temp;
    temp = atof(yytext);
    yylval.floattype=temp;
    if(dump_tokens)
        DEBUG_FOR_LAB4(string("FLOAT\t").append(to_string(temp)));
        //offsets += strlen(yytext);
    return FLOATNUM;
}

{ID} {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    return ID;
}

{LINECOMMENT} {}
{BLOCKCOMMENTBEGIN} {BEGIN BLOCKCOMMENT;}
<BLOCKCOMMENT>{BLOCKCOMMENTELEMENT} {}
<BLOCKCOMMENT>{BLOCKCOMMENTEND} {BEGIN INITIAL;}
<*>{EOL} {
    yylineno++;
    // offsets = 0;
}
{WHITE}
%%
