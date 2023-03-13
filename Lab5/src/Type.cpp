#include "Type.h"
#include <sstream>
//2022年11月3日16:53:15 modified by zsr
IntType TypeSystem::commonInt = IntType(4);
VoidType TypeSystem::commonVoid = VoidType();
FloatType TypeSystem::commonFloat = FloatType(4);
ConstFloatType TypeSystem::commonConstFloat = ConstFloatType(4);
ConstIntType TypeSystem::commonConstInt = ConstIntType(4);

Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::voidType = &commonVoid;
Type* TypeSystem::floatType = &commonFloat;
Type* TypeSystem::constIntType = &commonConstInt;
Type* TypeSystem::constFloatType = &commonConstFloat;

std::string IntType::toStr()
{
    return "int";
}

std::string VoidType::toStr()
{
    return "void";
}

std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() << "()";
    return buffer.str();
}

std::string ConstIntType::toStr(){
    return "const int";
}

std::string ConstFloatType::toStr(){
    return "const float";
}

std::string FloatType::toStr(){
    return "float";
}

std::string StringType::toStr(){
    return "string";
}

std::string ArrayType::toStr(){
    std::ostringstream buffer;
    buffer << this->elementType->toStr();
    for (auto it = this->dimensions.begin(); it != dimensions.end(); it++)
    {
        buffer << "[" << *it << "]";
    }
    return buffer.str();
}