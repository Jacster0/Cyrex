#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

//The following code is redundant when NOMINMAX is defined
//However I will keep it as some sort of backup in case some sorcerer 
//decides to delete the NOMINMAX macro 
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
