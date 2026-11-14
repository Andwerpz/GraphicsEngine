#pragma once

#define NON_COPYABLE(ClassName)                             \
    ClassName(const ClassName&) = delete;                   \
    ClassName& operator=(const ClassName&) = delete;

#define NON_MOVABLE(ClassName)                              \
    ClassName(ClassName&&) = delete;                        \
    ClassName& operator=(ClassName&&) = delete;

#define NON_COPYABLE_NON_MOVABLE(ClassName)                 \
    NON_COPYABLE(ClassName)                                 \
    NON_MOVABLE(ClassName)

