#ifndef COLOR_H
#define COLOR_H

#include"stdlib.h"

//TODO: Color needs rework.
struct Color
{
public:
    float* data;
    Color(float r, float g, float b, float a = 1.0f);
    Color();
};

#endif