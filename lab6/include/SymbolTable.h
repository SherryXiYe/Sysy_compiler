#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include <string>
#include <map>
#include "Type.h"

class Operand;
class ArrayInitialVal;
class SymbolEntry
{
private:
    int kind;
protected:
    enum {CONSTANT, VARIABLE, TEMPORARY};
    Type *type;

public:
    SymbolEntry(Type *type, int kind);
    virtual ~SymbolEntry() {};
    bool isConstant() const {return kind == CONSTANT;};
    bool isTemporary() const {return kind == TEMPORARY;};
    bool isVariable() const {return kind == VARIABLE;};
    Type* getType() {return type;};
    void setType(Type *type) {this->type = type;};
    virtual std::string toStr() = 0;
    // You can add any function you need here.
};


/*  
    Symbol entry for literal constant. Example:

    int a = 1;

    Compiler should create constant symbol entry for literal constant '1'.
*/
class ConstantSymbolEntry : public SymbolEntry
{
private:
    // int value;
    // float f_value;
    double value;
    std::string strValue;
public:
    ConstantSymbolEntry(Type *type, double value);     //2022/12/6 xzh 把int value改成flaot value
    ConstantSymbolEntry(Type *type, std::string strValue);
    virtual ~ConstantSymbolEntry() {};
    float getValue() const {return value;};   //2022/12/6 xzh 把int 改成flaot
    std::string toStr();
    // You can add any function you need here.
};


/* 
    Symbol entry for identifier. Example:

    int a;
    int b;
    void f(int c)
    {
        int d;
        {
            int e;
        }
    }

    Compiler should create identifier symbol entries for variables a, b, c, d and e:

    | variable | scope    |
    | a        | GLOBAL   |
    | b        | GLOBAL   |
    | c        | PARAM    |
    | d        | LOCAL    |
    | e        | LOCAL +1 |
*/
class IdentifierSymbolEntry : public SymbolEntry
{
private:
    enum {GLOBAL, PARAM, LOCAL};
    std::string name;
    int scope;
    Operand *addr;  // The address of the identifier.
    // You can add any field you need here.
    bool isparam;
    bool isSysy;
    int label;
    float value;
    ArrayInitialVal* valRoot;
public:
    IdentifierSymbolEntry(Type *type, std::string name, int scope, bool isparam);  
    IdentifierSymbolEntry(Type *type, std::string name, int scope); 
    virtual ~IdentifierSymbolEntry() {};
    std::string toStr();
    void setValue(float value){this->value = value;}
    float getValue(){return value;}
    bool isGlobal() const {return scope == GLOBAL;};
    bool isParam() const {return isparam;};
    void setParam(){isparam = true;};
    bool isSysyFunc() const {return isSysy;};
    void setSysyFunc(){isSysy = true;}    
    bool isLocal() const {return scope >= LOCAL;};
    int getScope() const {return scope;};
    void setAddr(Operand *addr) {this->addr = addr;};
    void setArrayValRoot(ArrayInitialVal* valRoot){this->valRoot = valRoot;};
    ArrayInitialVal* getArrayValRoot(){return valRoot;}
    Operand* getAddr() {return addr;};
    // You can add any function you need here.
};


/* 
    Symbol entry for temporary variable created by compiler. Example:

    int a;
    a = 1 + 2 + 3;

    The compiler would generate intermediate code like:

    t1 = 1 + 2
    t2 = t1 + 3
    a = t2

    So compiler should create temporary symbol entries for t1 and t2:

    | temporary variable | label |
    | t1                 | 1     |
    | t2                 | 2     |
*/
class TemporarySymbolEntry : public SymbolEntry
{
private:
    int label;
    float value;
public:
    TemporarySymbolEntry(Type *type, int label);
    virtual ~TemporarySymbolEntry() {};
    std::string toStr();
    int getLabel() const {return label;};   //注意这个是retrun label ，而SymolTable的是r
    // You can add any function you need here.
};

// symbol table managing identifier symbol entries
class SymbolTable
{
private:
    std::map<std::string, SymbolEntry*> symbolTable;
    SymbolTable *prev;
    int level;          //这个是作用域
    static int counter;
public:
    SymbolTable();
    SymbolTable(SymbolTable *prev);
    void install(std::string name, SymbolEntry* entry);
    SymbolEntry* lookup(std::string name);
    SymbolTable* getPrev() {return prev;};
    int getLevel() {return level;};
    static int getLabel() {return counter++;};
};

extern SymbolTable *identifiers;
extern SymbolTable *globals;

#endif
