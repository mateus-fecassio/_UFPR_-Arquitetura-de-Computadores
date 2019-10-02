#include "simulator.hpp"

// =====================================================================
processor_t::processor_t() {

};

// =====================================================================
void processor_t::allocate() {
	memset (orcs_engine.processor->btb, 0, sizeof(btb_line_t)*1024*4);
	memset (orcs_engine.processor->bht, 0, sizeof(bht_line_t)*1024*4);
};

// =====================================================================
void processor_t::clock_BHT()
{

	/// Get the next instruction from the trace
	opcode_package_t new_instruction;
	if (!orcs_engine.trace_reader->trace_fetch(&new_instruction)) {
		/// If EOF
		orcs_engine.simulator_alive = false;
	}
	else //caso haja instruções
	{
			orcs_engine.processor->cycles ++;
			if (orcs_engine.processor->latency > 0)
			{
				orcs_engine.processor->latency --;
			}
			else
			{
				if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_BRANCH)
				{
					orcs_engine.processor->branches ++;

					uint64_t index = new_instruction.opcode_address & 1023;
					uint64_t tag = new_instruction.opcode_address >> 10;
					uint64_t oldest_cycle = orcs_engine.global_cycle;
					bool is_empty = 0;
					bool hit = 0;
					int emptyRow = 0;
					int emptyCol = 0;
					int lastRow = 0;
					int lastCol = 0;
					int i = index;
					int j_hit;

					orcs_engine.processor->lock = 0; //free
					for (int j = 0; j < 4; ++j)
					{
						if (orcs_engine.processor->btb[i][j].tag == tag)
						{
							hit = 1;
							j_hit = j;
						}

						else if (orcs_engine.processor->btb[i][j].tag == 0)
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
					} //end for (int j = 0; j < 4; ++j)

					// verificação do HIT ou MISS
					if (!hit)
					{
						if (is_empty)
						{
							orcs_engine.processor->btb[emptyRow][emptyCol].tag = tag;
							orcs_engine.processor->btb[emptyRow][emptyCol].lru = orcs_engine.global_cycle;
							orcs_engine.processor->btb_row_target = emptyRow;
							orcs_engine.processor->btb_col_target = emptyCol;
							is_empty = 0;
						}
						else
						{
							orcs_engine.processor->btb[lastRow][lastCol].tag = tag;
							orcs_engine.processor->btb[lastRow][lastCol].lru = orcs_engine.global_cycle;
							orcs_engine.processor->btb_row_target = lastRow;
							orcs_engine.processor->btb_col_target = lastCol;
						}
						orcs_engine.processor->btb_miss ++;
						//penalidade
						orcs_engine.processor->delay = 14;
						orcs_engine.processor->trust = 0;
					} //if (!hit)
					else //if (hit)
					{
						orcs_engine.processor->trust = orcs_engine.processor->bht[index][j_hit].counter;
						//orcs_engine.processor->trust = orcs_engine.processor->bht[index].counter;
						orcs_engine.processor->btb_row_target = index;
						orcs_engine.processor->btb_col_target = j_hit;
					}
				} //end if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_BRANCH)
				else //if new_instruction.opcode_operation != INSTRUCTION_OPERATION_BRANCH
				{
					if (!orcs_engine.processor->lock)
					{
						//verificação e atualização da BHT
						if (orcs_engine.processor->trust == 0)
						{
							if (orcs_engine.processor->btb[btb_row_target][btb_col_target].target == new_instruction.opcode_address)
							{
								orcs_engine.processor->bht[btb_row_target][btb_col_target].counter ++;
								orcs_engine.processor->bht_miss ++;
							}
							//else, conta acerto
						}
						else if (orcs_engine.processor->trust == 1)
						{
							if (orcs_engine.processor->btb[btb_row_target][btb_col_target].target == new_instruction.opcode_address)
							{
								orcs_engine.processor->bht[btb_row_target][btb_col_target].counter ++;
								orcs_engine.processor->bht_miss ++;
							}
							else
							{
								orcs_engine.processor->bht[btb_row_target][btb_col_target].counter --;
								//conta acerto
							}
						}
						else if (orcs_engine.processor->trust == 2)
						{
							if (orcs_engine.processor->btb[btb_row_target][btb_col_target].target == new_instruction.opcode_address)
							{
								orcs_engine.processor->bht[btb_row_target][btb_col_target].counter ++;
								//conta acerto
							}
							else
							{
								orcs_engine.processor->bht[btb_row_target][btb_col_target].counter --;
								orcs_engine.processor->bht_miss ++;
							}
						}
						else if (orcs_engine.processor->trust == 3)
						{
							if (orcs_engine.processor->btb[btb_row_target][btb_col_target].target != new_instruction.opcode_address)
							{
								orcs_engine.processor->bht[btb_row_target][btb_col_target].counter --;
								orcs_engine.processor->bht_miss ++;
							}
							//else, conta acerto
						}

						// atualização da BTB
						orcs_engine.processor->btb[btb_row_target][btb_col_target].target = new_instruction.opcode_address;

						orcs_engine.processor->lock = 1; //locked
					}
				}
			} //end else
	} //else caso haja instruções
};


// =====================================================================
void processor_t::statistics() {
	ORCS_PRINTF("######################################################\n");
	ORCS_PRINTF("processor_t\n");

	printf("---STATISTICS---\n");
	printf("Cycles: %ld\n", orcs_engine.processor->cycles);
	printf("Branches: %ld\n", orcs_engine.processor->branches);
	printf("BTB misses: %ld\n", orcs_engine.processor->btb_miss);
	printf("BHT misses: %ld\n", orcs_engine.processor->bht_miss);

	printf("---ACCURACY---\n");
	float accuracy;
	accuracy = (1 - (float)orcs_engine.processor->bht_miss / (float)(orcs_engine.processor->branches - orcs_engine.processor->btb_miss));
	printf("%g\n", accuracy);
};
