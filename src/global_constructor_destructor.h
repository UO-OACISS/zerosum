/*
 * MIT License
 *
 * Copyright (c) 2023-2025 University of Oregon, Kevin Huck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Portable global constructor/destructor for GCC and MSVC.
 * I found this code here: http://www.gonwan.com/?p=8 */

#pragma once

#include <stdlib.h>
#if defined (_MSC_VER)
#if (_MSC_VER >= 1500)
/* Visual Studio 2008 and later have __pragma */
#define HAS_CONSTRUCTORS
#define DEFINE_CONSTRUCTOR(_func) \
    static void _func(void); \
    static int _func ## _wrapper(void) { _func(); return 0; } \
    __pragma(section(".CRT$XCU",read)) \
    __declspec(allocate(".CRT$XCU")) static int\
        (* _array ## _func)(void) = _func ## _wrapper;
#define DEFINE_DESTRUCTOR(_func) \
    static void _func(void); \
    static int _func ## _constructor(void) { atexit (_func); return 0; } \
    __pragma(section(".CRT$XCU",read)) \
    __declspec(allocate(".CRT$XCU")) static int\
        (* _array ## _func)(void) = _func ## _constructor;
#elif (_MSC_VER >= 1400)
/* Visual Studio 2005 */
#define HAS_CONSTRUCTORS
#pragma section(".CRT$XCU",read)
#define DEFINE_CONSTRUCTOR(_func) \
    static void _func(void); \
    static int _func ## _wrapper(void) { _func(); return 0; } \
    __declspec(allocate(".CRT$XCU")) static int\
        (* _array ## _func)(void) = _func ## _wrapper;
#define DEFINE_DESTRUCTOR(_func) \
    static void _func(void); \
    static int _func ## _constructor(void) { atexit (_func); return 0; } \
    __declspec(allocate(".CRT$XCU")) static int\
        (* _array ## _func)(void) = _func ## _constructor;
#else
/* Visual Studio 2003 and early versions should use #pragma code_seg() to
 * define pre/post-main functions. */
#error Pre/Post-main function not supported on your version of Visual Studio.
#endif
#elif (__GNUC__ > 2) || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
#define HAS_CONSTRUCTORS
#define DEFINE_CONSTRUCTOR(_func) static void \
    __attribute__((constructor)) _func (void);
#define DEFINE_DESTRUCTOR(_func) static void \
    __attribute__((destructor)) _func (void);
#else
/* not supported */
#define DEFINE_CONSTRUCTOR(_func)
#define DEFINE_DESTRUCTOR(_func)
#warning "Global constructors and destructors not defined!"
#endif

