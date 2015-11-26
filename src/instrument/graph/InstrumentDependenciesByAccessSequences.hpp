#ifndef INSTRUMENT_GRAPH_DEPENDENCIES_BY_ACCESS_SEQUENCE_HPP
#define INSTRUMENT_GRAPH_DEPENDENCIES_BY_ACCESS_SEQUENCE_HPP


#include "InstrumentDataAccessId.hpp"
#include "InstrumentDataAccessSequenceId.hpp"

#include "../InstrumentDependenciesByAccessSequences.hpp"


namespace Instrument {
	data_access_sequence_id_t registerAccessSequence();
	
	data_access_id_t addedDataAccessInSequence(data_access_sequence_id_t dataAccessSequenceId, DataAccessType accessType, bool satisfied, task_id_t originatorTaskId);
	
	void upgradedDataAccessInSequence(data_access_sequence_id_t dataAccessSequenceId, data_access_id_t dataAccessId, DataAccessType previousAccessType, DataAccessType newAccessType, bool becomesUnsatisfied, task_id_t originatorTaskId);
	
	void dataAccessBecomesSatisfied(data_access_sequence_id_t dataAccessSequenceId, data_access_id_t dataAccessId, task_id_t triggererTaskId, task_id_t targetTaskId);
	
	void removedDataAccessFromSequence(data_access_sequence_id_t dataAccessSequenceId, data_access_id_t dataAccessId, task_id_t triggererTaskId);
}


#endif // INSTRUMENT_GRAPH_DEPENDENCIES_BY_ACCESS_SEQUENCE_HPP
