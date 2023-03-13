%option noyywrap
%{
    /*
    * You will need to comment this line in lab5.
    */
    #define ONLY_FOR_LEX
    
    #ifdef ONLY_FOR_LEX
    #else
    #define YYSTYPE void *
    #include "parser.h"
    #endif
    #define YY_NO_UNPUT
    #define YY_NO_INPUT
    #include <string>
    #include <vector>
    
    #ifdef ONLY_FOR_LEX
    #include <ostream>
    #include <fstream>  
    #include <iostream>
    using namespace std;
    extern FILE *yyin; 
    extern FILE *yyout;
    int offsets = 0;
    

    struct symbol{
        string name;
        bool isInt=false;
        float value=0;
    };

    class symbolTable{
    public:
        symbolTable(){prevScope=NULL;}
        symbol* find(string name);
        symbolTable* prevScope;
        vector<symbol> symTable;
    };

    class scopeTree{
    public:
        scopeTree(){currentScope=new symbolTable();}
        symbol* addSymbol(symbol sym);
        void newScope();
        void prevScope();
        symbol* find(string name);
        symbolTable* currentScope;
        
    };
    scopeTree scoTree;

    symbol* symbolTable::find(string name){
        symbol* resSymbolPtr = NULL;
        for(int i = 0; i < int(symTable.size()); i++){
            if(name == symTable[i].name){
                resSymbolPtr = &symTable[i];
                break;
            }
        }
        return resSymbolPtr;
    }

    symbol* scopeTree::find(string name){
        symbol* resSymbolPtr = NULL;
        symbolTable* p = currentScope;
        while(p){
            resSymbolPtr = p->find(name);
            if(resSymbolPtr){
                return resSymbolPtr;
            }else{
                p = p->prevScope;
            }
        }
        return resSymbolPtr;
    }

    symbol* scopeTree::addSymbol(symbol sym){
        currentScope->symTable.push_back(sym);
        return &(currentScope->symTable.back());
    }

    void scopeTree::newScope(){
        symbolTable* newScope=new symbolTable();
        newScope->prevScope=currentScope;
        currentScope=newScope;
    }

    void scopeTree::prevScope(){
        currentScope = currentScope->prevScope;
    }
    
    void DEBUG_FOR_LAB4(std::string s,string l, symbol* symPtr = NULL){
        char DEBUG_INFO[200];
        if(symPtr == NULL){
            sprintf(DEBUG_INFO, "%12s%20s%12s%12s\n", s.c_str(),l.c_str() ,to_string(yylineno).c_str(), to_string(offsets).c_str());
        }
            // DEBUG_INFO =  s + "\t" + to_string(yylineno) + "\t" + to_string(offsets) + "\n";
        else{
            char temp[100];
            sprintf(temp, "0x%lx", (long unsigned int)symPtr);
            sprintf(DEBUG_INFO, "%12s%20s%12s%12s%20s\n", s.c_str(), l.c_str(), to_string(yylineno).c_str(), to_string(offsets).c_str(), temp);
            // DEBUG_INFO =  s + "\t" + to_string(yylineno) + "\t" + to_string(offsets) + "\t" +string(temp) + "\n";
        }
        fputs(DEBUG_INFO, yyout);
    }
    #endif
%}

DECIMIAL ([1-9][0-9]*|0)
OCTAL (0[0-7]+)
HEXADECIMAL (0[xX][0-9A-Fa-f]+)
FLOAT (([0-9]+(\.[0-9]+)?([Ee]([+-])?[0-9]+)?)|(\.(([0-9]*)|([Ee]([+-])?[0-9]+)))|(0[xX]([1-9A-Fa-f][0-9A-Fa-f]*)?((\.[0-9A-Fa-f]*[Pp]([+-])?[0-9]+)|([Pp]([+-])?[0-9]+))))
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
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("INT","int");
        offsets += strlen("int");
    #else
        return INT;
    #endif
}
"void" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("VOID","void");
        offsets += strlen("void");
    #else
        return VOID;
    #endif 
}
"const" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("CONST","const");
        offsets += strlen("const");
    #else
        return CONST;
    #endif 
}
"if" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("IF","if");
        offsets += strlen("if");
    #else
        return IF;
    #endif
}
"else" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ELSE","else");
        offsets += strlen("else");
    #else
        return ELSE;
    #endif
}
"return" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RETURN","return");
        offsets += strlen("return");
    #else
        return RETURN;
    #endif
}
"while" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("WHILE","while");
        offsets += strlen("while");
    #else
        return WHILE;
    #endif
}
"break" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("BREAK","break");
        offsets += strlen("break");
    #else
        return BREAK;
    #endif
}
"continue" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("CONTINUE","continue");
        offsets += strlen("continue");
    #else
        return CONTINUE;
    #endif
}
"==" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("EQUAL","==");
        offsets += strlen("==");
    #else
        return EQUAL;
    #endif
}
"!=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("NOTEQUAL","!=");
        offsets += strlen("!=");
    #else
        return NOTEQUAL;
    #endif
}
"=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ASSIGN","=");
        offsets += strlen("=");
    #else
        return ASSIGN;
    #endif
}
"<=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LESSEQUAL","<=");
        offsets += strlen("<=");
    #else
        return LESSEQUAL;
    #endif
}
">=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("GREATEREQUAL","<=");
        offsets += strlen(">=");
    #else
        return GREATEREQUAL;
    #endif
}
"<" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LESS","<");
        offsets += strlen("<");
    #else
        return LESS;
    #endif
}
">" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("GREATER",">");
        offsets += strlen(">");
    #else
        return GREATER;
    #endif
}
"+" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ADD","+");
        offsets += strlen("+");
    #else
        return ADD;
    #endif
}
"-" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SUB","-");
        offsets += strlen("-");
    #else
        return SUB;
    #endif
}
"*" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MUL","*");
        offsets += strlen("*");
    #else
        return MUL;
    #endif
}
"/" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("DIV","/");
        offsets += strlen("/");
    #else
        return DIV;
    #endif
}
"%" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MOD","%");
        offsets += strlen("%");
    #else
        return MOD;
    #endif
}
"&&" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("AND","&&");
        offsets += strlen("&&");
    #else
        return AND;
    #endif
}
"||" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("OR","||");
        offsets += strlen("||");
    #else
        return OR;
    #endif
}
"!" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("NOT","!");
        offsets += strlen("!");
    #else
        return NOT;
    #endif
}
";" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SEMICOLON",";");
        offsets += strlen(";");
    #else
        return SEMICOLON;
    #endif
}
"(" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LPAREN","(");
        offsets += strlen("(");
    #else
        return LPAREN;
    #endif
}
")" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RPAREN",")");
        offsets += strlen(")");
    #else
    return RPAREN;
    #endif
}
"{" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LBRACE","{");
        offsets += strlen("{");
        // symbolLen[symbolArrIdx] = symbolIdx + 1;
        // scope.push(++symbolArrIdx);
        // symbolIdx = 0;
        scoTree.newScope();
    #else
        return LBRACE;
    #endif
}
"}" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RBRACE","}");
        offsets += strlen("}");
        scoTree.prevScope();
        // int temp = 0;
        // if(!scope.empty()){
        //     temp = scope.top();
        //     scope.pop();
        // }
        // symbolIdx = (symbolLen[temp] - 1) >= 0 ? (symbolLen[temp] - 1): 0;
    #else
        return RBRACE;
    #endif
}
"[" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LBRACKET","[");
        offsets += strlen("[");
    #else
        return LBRACKET;
    #endif
}
"]" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RBRACKET","}");
        offsets += strlen("]");
    #else
        return RBRACKET;
    #endif
}
"," {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("COMMA",",");
        offsets += strlen(",");
    #else
        return COMMA;
    #endif
}
{DECIMIAL} {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4(string("INTEGER") , string(yytext));\
        offsets += strlen(yytext);
    #else
        return INTEGER;
    #endif
}

{OCTAL} {
    #ifdef ONLY_FOR_LEX
        int temp;
        sscanf(yytext, "%o", &temp);
        DEBUG_FOR_LAB4(string("INTEGER") , to_string(temp));
        offsets += strlen(yytext);
    #else
        return INTEGER;
    #endif
}

{HEXADECIMAL} {
    #ifdef ONLY_FOR_LEX
        int temp;
        sscanf(yytext, "%x", &temp);
        DEBUG_FOR_LAB4(string("INTEGER") , to_string(temp));
        offsets += strlen(yytext);
    #else
        return INTEGER;
    #endif
}
{FLOAT} {
    #ifdef ONLY_FOR_LEX
        float temp;
        temp = atof(yytext);
        //cout<<temp<<endl;
        DEBUG_FOR_LAB4(string("FLOAT") , to_string(temp));
        offsets += strlen(yytext);
    #else
        return FLOAT;
    #endif
}

{ID} {
    #ifdef ONLY_FOR_LEX
        string name = string(yytext);
        symbol* nowSymbol = scoTree.find(name);
        offsets += strlen(yytext);
        if(nowSymbol){
            
            DEBUG_FOR_LAB4(string("ID") , name, nowSymbol);
        }else{
            struct symbol newSymbol;
            newSymbol.name=name;
            nowSymbol=scoTree.addSymbol(newSymbol);
            DEBUG_FOR_LAB4(string("ID") , name, nowSymbol);
        }
    #else
        return ID
    #endif
}

{LINECOMMENT} {}
{BLOCKCOMMENTBEGIN} {BEGIN BLOCKCOMMENT;}
<BLOCKCOMMENT>{BLOCKCOMMENTELEMENT} {}
<BLOCKCOMMENT>{BLOCKCOMMENTEND} {BEGIN INITIAL;}
<*>{EOL} {
    yylineno++;
    offsets = 0;
}
{WHITE}


%%

#ifdef ONLY_FOR_LEX
int main(int argc, char **argv){
    if(argc != 5){
        fprintf(stderr, "Argument Not Enough");
        exit(EXIT_FAILURE);
    }

    if(!(yyin = fopen(argv[1], "r"))){
        fprintf(stderr, "No such file or directory: %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    if(!(yyout = fopen(argv[3], "w"))){
        fprintf(stderr, "No such file or directory: %s", argv[3]);
        exit(EXIT_FAILURE);
    }
    char temp[100];
    sprintf(temp,"%12s%20s%12s%12s%20s\n","token","lexeme","lineno","offset","pointer");
    fputs(temp, yyout);
    yylex();
    return 0;
}
#endif
