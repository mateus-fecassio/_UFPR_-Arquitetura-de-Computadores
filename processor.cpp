#include "simulator.hpp"

// =====================================================================
processor_t::processor_t() {

};

// =====================================================================
void processor_t::allocate() {
	memset (orcs_engine.processor->btb, 0, sizeof(uint64_t)*2*1024*4);
};

// =====================================================================
void processor_t::clock() {

	/// Get the next instruction from the trace
	opcode_package_t new_instruction;
	if (!orcs_engine.trace_reader->trace_fetch(&new_instruction)) {
		/// If EOF
		orcs_engine.simulator_alive = false;
	}
	else //caso haja instruções
	{
			if (orcs_engine.processor->latency > 0)
			{
				orcs_engine.processor->latency --;
			}
			else
			{
				if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_BRANCH)
				{
					uint64_t index = new_instruction.opcode_address & 1023;
					uint64_t oldest_cycle = orcs_engine.global_cycle;
					bool is_empty = 0;
					bool hit = 0;
					int emptyRow = 0;
					int emptyCol = 0;
					int lastRow = 0;
					int lastCol = 0;
					int i = index;

					for (int j = 0; j < 4; ++j)
					{
						if (orcs_engine.processor->btb[i][j].pc == index)
						{
							hit = 1;
							//printf("teve hitttttt\n");
						}

						else if (orcs_engine.processor->btb[i][j].pc == 0)
						{
							is_empty = 1;
							emptyRow = i;
							emptyCol = j;
						}

						else if (orcs_engine.processor->btb[i][j].lru < oldest_cycle)
						{
							oldest_cycle = orcs_engine.processor->btb[i][j].lru;
							lastRow = i;
							lastCol = j;
						}
					} //end for colunas

					// verificação do HIT ou MISS
					if (!hit)
					{
						if (is_empty)
						{
							orcs_engine.processor->btb[emptyRow][emptyCol].pc = index;
							orcs_engine.processor->btb[emptyRow][emptyCol].lru = orcs_engine.global_cycle;
						}
						else
						{
							orcs_engine.processor->btb[lastRow][lastCol].pc = index;
							orcs_engine.processor->btb[lastRow][lastCol].lru = orcs_engine.global_cycle;
						}
						//penalidade
						orcs_engine.processor->delay = 14;
					} //if (!hit)
				} //end if if (new_instruction.opcode_operation ...
			} //end else
	} //else caso haja instruções
};

// =====================================================================
void processor_t::statistics() {
	ORCS_PRINTF("######################################################\n");
	ORCS_PRINTF("processor_t\n");

};
