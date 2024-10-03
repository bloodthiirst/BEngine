#pragma once

/// @brief Color struct with each channel taking 32 bits (4 bytes) , giving 16 bytes total
struct Color
{
    /// @brief red channel , from 0 to 1
    float r;
    
    /// @brief green channel , from 0 to 1
    float g;
    
    /// @brief blue channel , from 0 to 1
    float b;
    
    /// @brief alpha channel , from 0 to 1
    float a;
};