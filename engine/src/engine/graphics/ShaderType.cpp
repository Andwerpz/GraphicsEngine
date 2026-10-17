#include <engine/graphics/ShaderType.h>
#include <engine/util/error.h>

#include <memory>
#include <cassert>
#include <functional>

#include <spirv_reflect.h>

namespace engine::graphics {

// -- CONSTRUCTOR --
ShaderVoidType::ShaderVoidType() {
    // do nothing
}

ShaderVectorType::ShaderVectorType(ShaderScalarType _base_type, uint32_t _dimension) {
    base_type = _base_type;
    dimension = _dimension;
}

ShaderMatrixType::ShaderMatrixType(ShaderScalarType _base_type, uint32_t _rows, uint32_t _cols) {
    base_type = _base_type;
    rows = _rows;
    cols = _cols;
}

ShaderImageType::ShaderImageType(
    SampledType _sampled_type,
    TextureDimension _dimension,
    bool _arrayed,
    bool _multisampled
) {
    sampled_type = _sampled_type;
    dimension = _dimension;
    arrayed = _arrayed;
    multisampled = _multisampled;
}

ShaderSampledImageType::ShaderSampledImageType(ShaderImageType _image_type) :
    image_type(_image_type)
{
    // do nothing
}

ShaderArrayType::ShaderArrayType(ShaderType _base_type, uint32_t _dimension) :
    base_type(std::make_unique<ShaderType>(_base_type))
{
    dimension = _dimension;
}

ShaderStructType::ShaderStructType(std::vector<ShaderType> member_types) {
    name = "";
    for(const ShaderType& member_type : member_types) {
        members.emplace_back((Member) {"", std::make_unique<ShaderType>(member_type)});
    }
}

ShaderPointerType::ShaderPointerType(ShaderType _base_type) :
    base_type(std::make_unique<ShaderType>(_base_type)) 
{
    // do nothing
}

ShaderFunctionType::ShaderFunctionType(ShaderType _return_type, std::vector<ShaderType> parameter_types) :
    return_type(std::make_unique<ShaderType>(_return_type))
{
    for(const ShaderType& parameter_type : parameter_types) {
        parameters.emplace_back(std::make_unique<ShaderType>(parameter_type));
    }
}

ShaderType::ShaderType(Type _type) : type(_type) {
    // do nothing
}

// -- COPY CONSTRUCTOR --
ShaderVoidType::ShaderVoidType(const ShaderVoidType& other) {
    // do nothing
}

ShaderVectorType::ShaderVectorType(const ShaderVectorType& other) {
    base_type = other.base_type;
    dimension = other.dimension;
}

ShaderMatrixType::ShaderMatrixType(const ShaderMatrixType& other) {
    base_type = other.base_type;
    rows = other.rows;
    cols = other.cols;
}

ShaderImageType::ShaderImageType(const ShaderImageType& other) {
    sampled_type = other.sampled_type;
    dimension = other.dimension;
    arrayed = other.arrayed;
    multisampled = other.multisampled;
}

ShaderSampledImageType::ShaderSampledImageType(const ShaderSampledImageType& other) :
    image_type(other.image_type)
{
    // do nothing
}

ShaderArrayType::ShaderArrayType(const ShaderArrayType& other) :
    base_type(std::make_unique<ShaderType>(*other.base_type)) 
{
    dimension = other.dimension;
}

ShaderStructType::ShaderStructType(const ShaderStructType& other) {
    name = other.name;
    for(const Member& member : other.members) {
        members.emplace_back((Member) {
            member.name,
            std::make_unique<ShaderType>(*member.type)
        });
    }
}

ShaderPointerType::ShaderPointerType(const ShaderPointerType& other) :
    base_type(std::make_unique<ShaderType>(*other.base_type))
{
    // do nothing
}

ShaderFunctionType::ShaderFunctionType(const ShaderFunctionType& other) {
    return_type = std::make_unique<ShaderType>(*other.return_type);
    for(const std::unique_ptr<ShaderType>& parameter : other.parameters) {
        parameters.emplace_back(std::make_unique<ShaderType>(*parameter));
    }
}

ShaderType::ShaderType(const ShaderType& other) : type(other.type)
{
    // do nothing
}

// -- ASSIGN OPERATOR --
ShaderVoidType& ShaderVoidType::operator=(const ShaderVoidType& other) {
    return *this;
}

ShaderVectorType& ShaderVectorType::operator=(const ShaderVectorType& other) {
    base_type = other.base_type;
    dimension = other.dimension;
    return *this;
}

ShaderMatrixType& ShaderMatrixType::operator=(const ShaderMatrixType& other) {
    base_type = other.base_type;
    rows = other.rows;
    cols = other.cols;
    return *this;
}

ShaderImageType& ShaderImageType::operator=(const ShaderImageType& other) {
    sampled_type = other.sampled_type;
    dimension = other.dimension;
    arrayed = other.arrayed;
    multisampled = other.multisampled;
    return *this;
}

ShaderSampledImageType& ShaderSampledImageType::operator=(const ShaderSampledImageType& other) {
    image_type = other.image_type;
    return *this;
}

ShaderArrayType& ShaderArrayType::operator=(const ShaderArrayType& other) {
    if(this == &other) return *this;
    base_type = std::make_unique<ShaderType>(*other.base_type);
    dimension = other.dimension;
    return *this;
}

ShaderStructType& ShaderStructType::operator=(const ShaderStructType& other) {
    if(this == &other) return *this;
    name = other.name;
    members.clear();
    members.reserve(other.members.size());
    for(const auto& other_member : other.members) {
        Member member;
        member.name = other_member.name;
        member.type = std::make_unique<ShaderType>(*other_member.type);
        members.push_back(std::move(member));
    }
    return *this;
}

ShaderPointerType& ShaderPointerType::operator=(const ShaderPointerType& other) {
    if(this == &other) return *this;
    base_type = std::make_unique<ShaderType>(*other.base_type);
    return *this;
}

ShaderFunctionType& ShaderFunctionType::operator=(const ShaderFunctionType& other) {
    if(this == &other) return *this;
    return_type = std::make_unique<ShaderType>(*other.return_type);
    parameters.clear();
    parameters.reserve(other.parameters.size());
    for(const auto& other_parameter : other.parameters) {
        parameters.push_back(std::make_unique<ShaderType>(*other_parameter));
    }
    return *this;
}

ShaderType& ShaderType::operator=(const ShaderType& other) {
    if(this == &other) return *this;
    type = other.type;
    return *this;
}

// -- EQUALS OPERATOR --
bool ShaderVoidType::operator==(const ShaderVoidType& other) const {
    return true;
}

bool ShaderVectorType::operator==(const ShaderVectorType& other) const {
    return base_type == other.base_type && dimension == other.dimension;
}

bool ShaderMatrixType::operator==(const ShaderMatrixType& other) const {
    return base_type == other.base_type && rows == other.rows && cols == other.cols;
}

bool ShaderImageType::operator==(const ShaderImageType& other) const {
    assert(false);
}

bool ShaderSampledImageType::operator==(const ShaderSampledImageType& other) const {
    return image_type == other.image_type;
}

bool ShaderArrayType::operator==(const ShaderArrayType& other) const {
    return *base_type == *other.base_type && dimension == other.dimension;
}

bool ShaderStructType::operator==(const ShaderStructType& other) const {
    if(name != other.name) return false;
    if(members.size() != other.members.size()) return false;
    for(int i = 0; i < members.size(); i++) {
        if(members[i].name != other.members[i].name) return false;
        if(!(*members[i].type == *other.members[i].type)) return false;
    }
    return true;
}

bool ShaderPointerType::operator==(const ShaderPointerType& other) const {
    return *base_type == *other.base_type;
}

bool ShaderFunctionType::operator==(const ShaderFunctionType& other) const {
    if(*return_type != *other.return_type) return false;
    if(parameters.size() != other.parameters.size()) return false;
    for(int i = 0; i < parameters.size(); i++) {
        if(!(*parameters[i] == *other.parameters[i])) return false;
    }
    return true;
}

bool ShaderType::operator==(const ShaderType& other) const {
    return type == other.type;
}

// -- TO STRING --
std::string to_string(const ShaderScalarType& type) {
    switch(type) {
        case ShaderScalarType::Bool:
            return "bool";
        case ShaderScalarType::Int:
            return "int";
        case ShaderScalarType::UInt:
            return "uint";
        case ShaderScalarType::Float:
            return "float";
        case ShaderScalarType::Double:
            return "double";
        default:
            assert(false);
    }
}

std::string to_string(const ShaderVoidType& type) {
    return "void";
}

std::string to_string(const ShaderVectorType& type) {
    return "Vector<" + to_string(type.base_type) + ", " + std::to_string(type.dimension) + ">";
}

std::string to_string(const ShaderMatrixType& type) {
    return "Matrix<" + to_string(type.base_type) + ", " + std::to_string(type.cols) + ", " + std::to_string(type.rows) + ">";
}

std::string to_string(const ShaderImageType& type) {
    return "TODO";
}

std::string to_string(const ShaderSampledImageType& type) {
    return "Sampler<" + to_string(type.image_type) + ">";
}

std::string to_string(const ShaderArrayType& type) {
    return to_string(*type.base_type) + "[" + std::to_string(type.dimension) + "]";
}

std::string to_string(const ShaderStructType& type) {
    return type.name;
}

std::string to_string(const ShaderPointerType& type) {
    return to_string(*type.base_type) + "*";
}

std::string to_string(const ShaderFunctionType& type) {
    std::string ret = "";
    ret += to_string(*type.return_type);
    ret += "(";
    for(int i = 0; i < type.parameters.size(); i++) {
        ret += to_string(*type.parameters[i]);
        if(i + 1 < type.parameters.size()) ret += ", ";
    }
    ret += ")";
    return ret;
}

std::string to_string(const ShaderType& type) {
    return std::visit(
        [](const auto& value) {return to_string(value);},
        type.type
    );
}

}