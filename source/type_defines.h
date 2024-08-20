#pragma once
#ifdef UPROPERTY

#include "CoreMinimal.h"
#include "MathUtil.h"

#define STRING FString
#define LENGTH(x) x.Len()
#define SUBSTRING(x,p,n) x.Mid(p,n)
#define EMPTY(x) x.IsEmpty()
#define ATOI(x) FCString::Atoi(*x)
#define ATOD(x) FCString::Atod(*x)

#define ARRAY TArray
#define SIZE(x) x.Num()
#define ADD(x, y) x.Add(y)
#define VECTOR2D FVector2D

#define MAX FMath::Max
#define PI TMathUtilConstants<double>::Pi
#else
#define _USE_MATH_DEFINES

#include <algorithm>
#include <cstdlib>
#include <math.h>
#include <string>
#include <vector>
#include "math/Vector.hpp"

#define STRING std::string
#define LENGTH(x) x.length()
#define SUBSTRING(x,p,n) x.substr(p,n)
#define EMPTY(x) x.empty()
#define ATOI(x) atoi(x.c_str())
#define ATOD(x) atof(x.c_str())

#define ARRAY std::vector
#define SIZE(x) x.size()
#define ADD(x, y) x.push_back(y)
#define VECTOR2D Vector2D<double>

#define MAX std::max
#define PI M_PI

#endif

template<typename T>
T GetRangeMappedValue(T value, T min, T max) {
	return (max - value) / (max - min);
}