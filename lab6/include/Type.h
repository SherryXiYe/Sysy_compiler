#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>
#include <string>
#include <iostream>
class Type
{
private:
    int kind;
protected:
    enum {INT,VOID, FUNC, PTR, FLOAT, CONST_INT, ARRAY, CONST_FLOAT, STRING};
public:
    Type(int kind) : kind(kind) {};
    virtual ~Type() {};
    virtual std::string toStr() = 0;
    bool isInt() const {return kind == INT||kind == CONST_INT;};
    bool isVoid() const {return kind == VOID;};
    bool isFunc() const {return kind == FUNC;};
    bool isPTR() const {return kind == PTR;};
    //2022年11月17日15:22:40 zsr add
    bool isConstInt() const {return kind == CONST_INT;};
    bool isFloat() const {return kind == FLOAT|| kind == CONST_FLOAT;};
    bool isArray() const {return kind == ARRAY;};
    bool isConstFloat() const {return kind == CONST_FLOAT;};
    bool isStr() const {return kind == STRING;}
};

class IntType : public Type
{
private:
    int size;
public:
    IntType(int size) : Type(Type::INT), size(size){};
    bool isBool() const {return size==1;};      //2022/12/6 xzh new
    std::string toStr();
};

class VoidType : public Type
{
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
};

class FunctionType : public Type
{
private:
    Type *returnType;
    std::vector<Type*> paramsType;
    bool havereturn;
public:
    FunctionType(Type* returnType, std::vector<Type*> paramsType) : 
    Type(Type::FUNC), returnType(returnType), paramsType(paramsType){
    if(returnType->isVoid())
        havereturn = true;
    else
        havereturn = false;
    }
    Type* getRetType() {return returnType;};
    void addParamsType(Type* type){paramsType.push_back(type);}
    void setReturn(){havereturn = true;};
    bool haveReturn(){return havereturn;};
    std::vector<Type*> getParams(){return paramsType;}
    std::string toStr();
};

class PointerType : public Type
{
private:
    Type *valueType;
public:
    PointerType(Type* valueType) : Type(Type::PTR) {this->valueType = valueType;};
    Type* getValueType(){return valueType;}
    std::string toStr();
};

class ArrayType : public Type
{
private:
    Type *lowerType;
    Type *elementType;
    int dimension;//修改为 int类型 多维数组嵌套type
public:
    ArrayType(Type* lowerType, int dimension): Type(Type::ARRAY){
        this->lowerType = lowerType;
        this->dimension = dimension;
        this->init();};
    void init();
    Type* getElementType(){return elementType;};//获取元素type
    Type* getLowerType(){return lowerType;} //获取下一级数组类型（如果有）}
    std::string toStr();
    int getDimension(){return dimension;}
};

class FloatType : public Type
{
private:
    int size;
public:
    FloatType(int size) : Type(Type::FLOAT), size(size){};
    std::string toStr();
};

class ConstFloatType : public Type
{
private:
    int size;
public:
    ConstFloatType(int size) : Type(Type::CONST_FLOAT),size(size){};
    std::string toStr();
};

class ConstIntType : public Type
{
private:
    int size;
public:
    ConstIntType(int size) : Type(Type::CONST_INT), size(size){};
    bool isBool() const {return size==1;};      //2022/12/6 xzh new
    std::string toStr();
};

//2022年11月5日22:16:24 add by zsr
class StringType : public Type
{
private:
    int length;
public:
    StringType() : Type(Type::STRING),length(0){;}
    StringType(int len): Type(Type::STRING),length(len){;}
    std::string toStr();
};

class TypeSystem
{
private:
    static IntType commonBool;
    static IntType commonInt;
    static VoidType commonVoid;
    static FloatType commonFloat;
    static ConstIntType commonConstInt;
    static ConstFloatType commonConstFloat;
public:
    static Type *intType;
    static Type *voidType;
    static Type *floatType;
    static Type *constFloatType;
    static Type *constIntType;
    static Type *boolType;
};

#endif
