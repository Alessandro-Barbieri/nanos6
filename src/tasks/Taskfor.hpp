/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#ifndef TASKFOR_HPP
#define TASKFOR_HPP

#include <cmath>

#include "tasks/Task.hpp"
#include "tasks/TaskImplementation.hpp"

class Taskfor : public Task {
public:
	typedef nanos6_loop_bounds_t bounds_t;

private:
	// Source
	Padded<std::atomic<int>> _currentChunk;
	// Source
	Padded<std::atomic<size_t>> _remainingIterations;
	// Source and collaborator
	bounds_t _bounds;
	// Collaborator
	size_t _completedIterations;
	// Collaborator
	int _myChunk;

public:
	// Methods for both source and collaborator taskfors
	inline Taskfor(
		void *argsBlock, size_t argsBlockSize,
		nanos6_task_info_t *taskInfo,
		nanos6_task_invocation_info_t *taskInvokationInfo,
		Task *parent,
		Instrument::task_id_t instrumentationTaskId,
		size_t flags,
		bool runnable = false
	)
		: Task(argsBlock, argsBlockSize, taskInfo, taskInvokationInfo, parent, instrumentationTaskId, flags, nullptr, nullptr, 0),
		  _bounds(), _completedIterations(0), _myChunk(-1)
	{
		assert(isFinal());
		setRunnable(runnable);
		_currentChunk.store(0, std::memory_order_relaxed);
		_remainingIterations.store(0, std::memory_order_relaxed);
	}

	inline void setRunnable(bool runnableValue)
	{
		_flags[Task::non_runnable_flag] = !runnableValue;
	}

	inline size_t getIterationCount()
	{
		return (_bounds.upper_bound - _bounds.lower_bound);
	}

	// Methods for source taskfors
	inline void initialize(size_t lowerBound, size_t upperBound, size_t chunksize)
	{
		assert(!isRunnable());

		_bounds.lower_bound = lowerBound;
		_bounds.upper_bound = upperBound;
		_bounds.chunksize = chunksize;

		size_t maxCollaborators = CPUManager::getNumCPUsPerTaskforGroup();
		assert(maxCollaborators > 0);

		size_t totalIterations = getIterationCount();
		_remainingIterations.store(totalIterations, std::memory_order_relaxed);

		if (_bounds.chunksize == 0) {
			// Just distribute iterations over collaborators if no hint.
			_bounds.chunksize = std::max(totalIterations / maxCollaborators, (size_t) 1);
		}
		else {
			// Distribute iterations over collaborators respecting the "alignment".
			size_t newChunksize = std::max(totalIterations / maxCollaborators, _bounds.chunksize);
			size_t alignedChunksize = ((newChunksize-1)|(_bounds.chunksize-1))+1;
			if (std::ceil((double)totalIterations / (double)alignedChunksize) < maxCollaborators) {
				alignedChunksize = std::max(alignedChunksize - _bounds.chunksize, _bounds.chunksize);
			}
			assert(alignedChunksize % _bounds.chunksize == 0);
			_bounds.chunksize = alignedChunksize;
		}

		assert(_currentChunk == 0);
		_currentChunk.store(std::ceil((double) totalIterations/(double) _bounds.chunksize), std::memory_order_relaxed);
	}

	inline bounds_t const &getBounds() const
	{
		assert(!isRunnable());
		return _bounds;
	}

	inline void notifyCollaboratorHasStarted()
	{
		assert(!isRunnable());

		increaseRemovalBlockingCount();
	}

	inline bool notifyCollaboratorHasFinished()
	{
		assert(!isRunnable());

		return decreaseRemovalBlockingCount();
	}

	inline void markAsScheduled()
	{
		assert(!isRunnable());

		increaseRemovalBlockingCount();
	}

	inline bool removedFromScheduler()
	{
		assert(!isRunnable());

		return decreaseRemovalBlockingCount();
	}

	inline bool decrementRemainingIterations(size_t amount)
	{
		assert(!isRunnable());
		size_t remaining = _remainingIterations.fetch_sub(amount, std::memory_order_relaxed) - amount;
		return (remaining == 0);
	}

	inline int getNextChunk()
	{
		assert(!isRunnable());
		int myChunk = _currentChunk.fetch_sub(1, std::memory_order_relaxed)-1;
		return myChunk;
	}

	// Methods for collaborator taskfors
	inline void reinitialize(
		void *argsBlock, size_t argsBlockSize,
		nanos6_task_info_t *taskInfo,
		nanos6_task_invocation_info_t *taskInvokationInfo,
		Task *parent,
		Instrument::task_id_t instrumentationTaskId,
		size_t flags,
		bool runnable = false
	)
	{
		assert(isRunnable());
		Task::reinitialize(argsBlock, argsBlockSize, taskInfo, taskInvokationInfo, parent, instrumentationTaskId, flags);
		_bounds.lower_bound = 0;
		_bounds.upper_bound = 0;
		_bounds.grainsize = 0;
		_bounds.chunksize = 0;
		_completedIterations = 0;
		setRunnable(runnable);
	}

	inline void body(
		__attribute__((unused)) void *deviceEnvironment,
		__attribute__((unused)) nanos6_address_translation_entry_t *translationTable = nullptr
	) {
		assert(hasCode());
		assert(isRunnable());
		assert(_thread != nullptr);
		assert(deviceEnvironment == nullptr);

		Task *parent = getParent();
		assert(parent != nullptr);
		assert(parent->isTaskfor());

		run(*((Taskfor *)parent));
	}

	inline bounds_t &getBounds()
	{
		assert(isRunnable());
		return _bounds;
	}

	inline void setChunk(int chunk)
	{
		assert(isRunnable());
		_myChunk = chunk;
	}

	inline int getMyChunk()
	{
		assert(isRunnable());
		return _myChunk;
	}

	inline void computeChunkBounds(bounds_t &collaboratorBounds)
	{
		assert(isRunnable());
		int myChunk = _myChunk;
		assert(myChunk >= 0);

		const Taskfor *source = (Taskfor *) getParent();
		bounds_t const &bounds = source->getBounds();
		size_t totalIterations = bounds.upper_bound - bounds.lower_bound;
		size_t totalChunks = std::ceil((double) totalIterations / (double) bounds.chunksize);
		assert(totalChunks > 0);
		myChunk = totalChunks - (myChunk+1);

		collaboratorBounds.lower_bound = bounds.lower_bound+(myChunk*bounds.chunksize);
		size_t myIterations = std::min(bounds.chunksize, bounds.upper_bound - collaboratorBounds.lower_bound);
		assert(myIterations > 0 && myIterations <= bounds.chunksize);
		collaboratorBounds.upper_bound = collaboratorBounds.lower_bound + myIterations;
	}

	inline size_t getCompletedIterations()
	{
		assert(isRunnable());
		assert(getParent()->isTaskfor());
		return _completedIterations;
	}

	inline bool hasFirstChunk()
	{
		assert(isRunnable());
		return (_bounds.lower_bound == 0);
	}

	inline bool hasLastChunk()
	{
		assert(isRunnable());
		const Taskfor *source = (Taskfor *) getParent();
		return (_bounds.upper_bound == source->getBounds().upper_bound);
	}

private:
	void run(Taskfor &source);
};

#endif // TASKFOR_HPP
