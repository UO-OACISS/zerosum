/*
 * MIT License
 *
 * Copyright (c) 2023 University of Oregon, Kevin Huck
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

#pragma once
#ifdef PERFSTUBS_USE_TIMERS
#include "perfstubs_api/timer.h"
#else
#define PERFSTUBS_INITIALIZE()
#define PERFSTUBS_FINALIZE()
#define PERFSTUBS_PAUSE_MEASUREMENT()
#define PERFSTUBS_RESUME_MEASUREMENT()
#define PERFSTUBS_REGISTER_THREAD()
#define PERFSTUBS_DUMP_DATA()
#define PERFSTUBS_TIMER_START(_timer, _timer_name)
#define PERFSTUBS_TIMER_STOP(_timer_name)
#define PERFSTUBS_START_STRING(_timer_name)
#define PERFSTUBS_STOP_STRING(_timer_name)
#define PERFSTUBS_STOP_CURRENT()
#define PERFSTUBS_SET_PARAMETER(_parameter, _value)
#define PERFSTUBS_DYNAMIC_PHASE_START(_phase_prefix, _iteration_index)
#define PERFSTUBS_DYNAMIC_PHASE_STOP(_phase_prefix, _iteration_index)
#define PERFSTUBS_TIMER_START_FUNC(_timer)
#define PERFSTUBS_TIMER_STOP_FUNC(_timer)
#define PERFSTUBS_SAMPLE_COUNTER(_name, _value)
#define PERFSTUBS_METADATA(_name, _value)
#define PERFSTUBS_SCOPED_TIMER(__name)
#define PERFSTUBS_SCOPED_TIMER_FUNC()
#endif