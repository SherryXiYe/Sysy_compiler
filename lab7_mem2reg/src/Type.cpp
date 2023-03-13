#include "Type.h"
#include <sstream>

IntType TypeSystem::commonInt = IntType(32);
IntType TypeSystem::commonBool = IntType(1);
VoidType TypeSystem::commonVoid = VoidType();
FloatType TypeSystem::commonFloat = FloatType(32);
ConstFloatType TypeSystem::commonConstFloat = ConstFloatType(32);
ConstIntType TypeSystem::commonConstInt = ConstIntType(32);

Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::voidType = &commonVoid;
Type* TypeSystem::boolType = &commonBool;
Type* TypeSystem::floatType = &commonFloat;
Type* TypeSystem::constIntType = &commonConstInt;
Type* TypeSystem::constFloatType = &commonConstFloat;

void ArrayType::init(){
    Type* typePtr = lowerType;
    while(typePtr->isArray()){
        typePtr = ((ArrayType*)typePtr)->getLowerType();
    }
    elementType  = typePtr;
}

std::string IntType::toStr()
{
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}

std::string VoidType::toStr()
{
    return "void";
}

std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() << "(";
    for(auto i= paramsType.begin();i!=paramsType.end();i++){
        buffer << (*i)->toStr();
        if((i+1)!=paramsType.end())
            buffer<<", ";
    }
    buffer<<")";
    return buffer.str();
}

std::string PointerType::toStr()
{
    std::ostringstream buffer;
    buffer << valueType->toStr() << "*";
    return buffer.str();
}
//add by zsr 2022年11月17日15:29:40
std::string ConstIntType::toStr(){
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}

std::string ConstFloatType::toStr(){
    std::ostringstream buffer;
    buffer << "float";
    return buffer.str();
}

std::string FloatType::toStr(){
    std::ostringstream buffer;
    buffer << "float";
    return buffer.str();
}

std::string StringType::toStr(){
    std::ostringstream buffer;
    buffer << "i8*";
    return buffer.str();
}

std::string ArrayType::toStr(){
    std::ostringstream buffer;
    if(dimension != -1)
        buffer<<"["<<dimension<<" x "<<lowerType->toStr()<<"]";
    else
        buffer<<lowerType->toStr()<<"*";
    return buffer.str();
}
