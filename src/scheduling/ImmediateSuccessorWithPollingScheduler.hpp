#ifndef IMMEDIATE_SUCCESSOR_WITH_POLLING_SCHEDULER_HPP
#define IMMEDIATE_SUCCESSOR_WITH_POLLING_SCHEDULER_HPP


#include <atomic>
#include <deque>
#include <vector>

#include "SchedulerInterface.hpp"
#include "lowlevel/TicketSpinLock.hpp"
#include "executors/threads/CPU.hpp"


class Task;


class ImmediateSuccessorWithPollingScheduler: public SchedulerInterface {
	typedef TicketSpinLock<> spinlock_t;
	
	spinlock_t _globalLock;
	
	std::deque<Task *> _readyTasks;
	std::deque<Task *> _unblockedTasks;
	
	std::deque<CPU *> _idleCPUs;
	
	std::atomic<polling_slot_t *> _pollingSlot;
	
	
	inline CPU *getIdleCPU();
	inline Task *getReplacementTask(CPU *hardwarePlace);
	inline void cpuBecomesIdle(CPU *cpu);
	
public:
	ImmediateSuccessorWithPollingScheduler();
	~ImmediateSuccessorWithPollingScheduler();
	
	ComputePlace *addReadyTask(Task *task, ComputePlace *hardwarePlace, ReadyTaskHint hint);
	
	void taskGetsUnblocked(Task *unblockedTask, ComputePlace *hardwarePlace);
	
	Task *getReadyTask(ComputePlace *hardwarePlace, Task *currentTask = nullptr);
	
	ComputePlace *getIdleComputePlace(bool force=false);
	
	void disableComputePlace(ComputePlace *hardwarePlace);
	
	bool requestPolling(ComputePlace *hardwarePlace, polling_slot_t *pollingSlot);
	bool releasePolling(ComputePlace *hardwarePlace, polling_slot_t *pollingSlot);

    void createReadyQueues(std::size_t nodes);
};


#endif // IMMEDIATE_SUCCESSOR_WITH_POLLING_SCHEDULER_HPP

