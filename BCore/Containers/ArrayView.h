#pragma once

template <typename T>
struct ArrayView
{
    T* data;
    size_t size;
};