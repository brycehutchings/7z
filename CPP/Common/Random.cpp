// Common/Random.cpp

#include "StdAfx.h"

#include <stdlib.h>

#ifndef _WIN32
#include <time.h>
#else
#include "MyWindows.h"
#endif

#include "Random.h"

void CRandom::Init(unsigned int seed) { srand(seed); }

void CRandom::Init()
{
  Init((unsigned int)
    #ifdef _WIN32
    #ifndef UWP
    GetTickCount()
    #else
    GetTickCount64()
    #endif
    #else
    time(NULL)
    #endif
    );
}

int CRandom::Generate() const { return rand(); }
