#pragma once
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "Ws2_32.lib")

#include "targetver.h"

/// Windows 헤더 파일:
#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#include <windows.h>
#include <winperf.h>

/// C 런타임 헤더 파일입니다.
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

/// ATL / MFC 헤더 파일:
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // 일부 CString 생성자는 명시적으로 선언됩니다.
#include <atlbase.h>
#include <atlimage.h>
#include <atlstr.h>

// 수학 상수 선언
#define _USE_MATH_DEFINES
#include <math.h>

// 표준 라이브러리
#include <memory>
#include <vector>
#include <list>
#include <stack>
#include <set>
#include <map>
#include <queue>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <chrono>

using namespace std;
