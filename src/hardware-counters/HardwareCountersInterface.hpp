/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2020 Barcelona Supercomputing Center (BSC)
*/

#ifndef HARDWARE_COUNTERS_INTERFACE_HPP
#define HARDWARE_COUNTERS_INTERFACE_HPP


class CPUHardwareCountersInterface;
class Task;
class TaskHardwareCountersInterface;
class ThreadHardwareCountersInterface;

class HardwareCountersInterface {

public:

	virtual ~HardwareCountersInterface()
	{
	}

	//! \brief Get the number of enabled and available counters of the backend
	virtual size_t getNumEnabledCounters() const = 0;

	//! \brief Initialize hardware counter structures for a new thread
	//!
	//! \param[out] threadCounters The hardware counter structures to initialize
	virtual void threadInitialized(ThreadHardwareCountersInterface *threadCounters) = 0;

	//! \brief Destroy the hardware counter structures of a thread
	//!
	//! \param[out] threadCounters The hardware counter structures to initialize
	virtual void threadShutdown(ThreadHardwareCountersInterface *threadCounters) = 0;

	//! \brief Reinitialize all hardware counter structures for a task
	//!
	//! \param[out] taskCounters The hardware counter structure to reinitialize
	virtual void taskReinitialized(TaskHardwareCountersInterface *taskCounters) = 0;

	//! \brief Read hardware counters for a task
	//!
	//! \param[out] threadCounters The hardware counter structures of the thread executing the task
	//! \param[out] taskCounters The hardware counter structure of the task to start
	virtual void readTaskCounters(
		ThreadHardwareCountersInterface *threadCounters,
		TaskHardwareCountersInterface *taskCounters
	) = 0;

	//! \brief Accumulate hardware counters for a CPU
	//!
	//! \param[out] cpuCounters The hardware counter structures of the CPU
	//! \param[out] threadCounters The hardware counter structures of the thread
	virtual void readCPUCounters(
		CPUHardwareCountersInterface *cpuCounters,
		ThreadHardwareCountersInterface *threadCounters
	) = 0;

	//! \brief An optional function that displays statistics of the backend
	virtual void displayStatistics() const
	{
	}

};

#endif // HARDWARE_COUNTERS_INTERFACE_HPP
