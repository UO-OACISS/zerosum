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