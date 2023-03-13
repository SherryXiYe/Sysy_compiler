#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>
#include <string>

class Type
{
private:
    int kind;
protected:
    enum {INT, VOID, FUNC, FLOAT, CONST_INT, ARRAY, CONST_FLOAT, String}; //2022年11月3日15:51:22 zsr add
public:
    Type(int kind) : kind(kind) {};
    virtual ~Type() {};
    virtual std::string toStr() = 0;
    bool isInt() const {return kind == INT;};
    bool isVoid() const {return kind == VOID;};
    bool isFunc() const {return kind == FUNC;};
    //2022年11月3日15:51:03 zsr add
    bool isConstInt() const {return kind == CONST_INT;};
    bool isFloat() const {return kind == FLOAT;};
    bool isArray() const {return kind == ARRAY;};
    bool isConstFloat() const {return kind == CONST_FLOAT;};
};

class IntType : public Type
{
private:
    int size;
public:
    IntType(int size) : Type(Type::INT), size(size){};
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
public:
    FunctionType(Type* returnType, std::vector<Type*> paramsType) : 
    Type(Type::FUNC), returnType(returnType), paramsType(paramsType){};
    std::string toStr();
};
//2022年11月3日15:47:32 zsr add
class ArrayType : public Type
{
private:
    Type *elementType;
    std::vector<int> dimensions;
public:
    ArrayType(Type* elementType, std::vector<int> dimensions) : Type(Type::ARRAY){};
    std::string toStr();
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
    std::string toStr();
};

//2022年11月5日22:16:24 add by zsr
class StringType : public Type
{
private:

public:
    StringType() : Type(Type::String){}
    std::string toStr();
};

//2022年11月3日16:16:55 modified by zsr
class TypeSystem
{
private:
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
    // static Type *functionType;
    // static Type *arrayType;
};

#endif
