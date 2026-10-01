#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include <variant>
#include <string>
#include <memory>

#include "Texture.h"

namespace engine::graphics {

enum class ShaderScalarType {
    Bool,
    Int,
    UInt,
    Float,
    Double,
};

struct ShaderType;

struct ShaderVoidType {
    ShaderVoidType();
    ShaderVoidType(const ShaderVoidType& other);
    ShaderVoidType& operator=(const ShaderVoidType& other);
    bool operator==(const ShaderVoidType& other) const;
};

struct ShaderVectorType {
    ShaderScalarType base_type;
    uint32_t dimension;

    ShaderVectorType(ShaderScalarType base_type, uint32_t dimension);
    ShaderVectorType(const ShaderVectorType& other);
    ShaderVectorType& operator=(const ShaderVectorType& other);
    bool operator==(const ShaderVectorType& other) const;
};

struct ShaderMatrixType {
    ShaderScalarType base_type;
    uint32_t rows;
    uint32_t cols;

    ShaderMatrixType(ShaderScalarType base_type, uint32_t rows, uint32_t cols);
    ShaderMatrixType(const ShaderMatrixType& other);
    ShaderMatrixType& operator=(const ShaderMatrixType& other);
    bool operator==(const ShaderMatrixType& other) const;
};

struct ShaderImageType {
    using SampledType = std::variant<ShaderScalarType, ShaderVoidType>;
    SampledType sampled_type;
    TextureDimension dimension;
    bool arrayed;
    bool multisampled;

    ShaderImageType(
        SampledType sampled_type,
        TextureDimension dimension,
        bool arrayed,
        bool multisampled
    );
    ShaderImageType(const ShaderImageType& other);
    ShaderImageType& operator=(const ShaderImageType& other);
    bool operator==(const ShaderImageType& other) const;
};

struct ShaderSampledImageType {
    ShaderImageType image_type;

    ShaderSampledImageType(ShaderImageType image_type);
    ShaderSampledImageType(const ShaderSampledImageType& other);
    ShaderSampledImageType& operator=(const ShaderSampledImageType& other);
    bool operator==(const ShaderSampledImageType& other) const;
};

struct ShaderArrayType {
    std::unique_ptr<ShaderType> base_type;
    uint32_t dimension;

    ShaderArrayType(ShaderType base_type, uint32_t dimension);
    ShaderArrayType(const ShaderArrayType& other);
    ShaderArrayType& operator=(const ShaderArrayType& other);
    bool operator==(const ShaderArrayType& other) const;
};

struct ShaderStructType {
    struct Member {
        std::string name;
        std::unique_ptr<ShaderType> type;
    };

    std::string name;
    std::vector<Member> members;

    ShaderStructType(std::vector<ShaderType> member_types);
    ShaderStructType(const ShaderStructType& other);
    ShaderStructType& operator=(const ShaderStructType& other);
    bool operator==(const ShaderStructType& other) const;
};

struct ShaderPointerType {
    std::unique_ptr<ShaderType> base_type;

    ShaderPointerType(ShaderType base_type);
    ShaderPointerType(const ShaderPointerType& other);
    ShaderPointerType& operator=(const ShaderPointerType& other);
    bool operator==(const ShaderPointerType& other) const;
};

struct ShaderFunctionType {
    std::unique_ptr<ShaderType> return_type;
    std::vector<std::unique_ptr<ShaderType>> parameters;

    ShaderFunctionType(ShaderType return_type, std::vector<ShaderType> parameter_types);
    ShaderFunctionType(const ShaderFunctionType& other);
    ShaderFunctionType& operator=(const ShaderFunctionType& other);
    bool operator==(const ShaderFunctionType& other) const;
};

struct ShaderType {
    using Type = std::variant<
        ShaderScalarType,
        ShaderVoidType,
        ShaderVectorType,
        ShaderMatrixType,
        ShaderImageType,
        ShaderSampledImageType,
        ShaderArrayType,
        ShaderStructType,
        ShaderPointerType,
        ShaderFunctionType
    >;
    Type type;

    ShaderType(Type type);
    ShaderType(const ShaderType& other);
    ShaderType& operator=(const ShaderType& other);
    bool operator==(const ShaderType& other) const;
};

std::string to_string(const ShaderScalarType& type);
std::string to_string(const ShaderVoidType& type);
std::string to_string(const ShaderVectorType& type);
std::string to_string(const ShaderMatrixType& type);
std::string to_string(const ShaderImageType& type);
std::string to_string(const ShaderSampledImageType& type);
std::string to_string(const ShaderArrayType& type);
std::string to_string(const ShaderStructType& type);
std::string to_string(const ShaderPointerType& type);
std::string to_string(const ShaderFunctionType& type);
std::string to_string(const ShaderType& type);
    
}