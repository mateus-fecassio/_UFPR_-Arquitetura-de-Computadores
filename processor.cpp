#include "simulator.hpp"

// =====================================================================
processor_t::processor_t() {

};

// =====================================================================
void processor_t::allocate() {
	memset (orcs_engine.processor->CL1, 0, sizeof(cache_line_t)*256*4);
	memset (orcs_engine.processor->CL2, 0, sizeof(cache_line_t)*2048*8);
	memset (orcs_engine.processor->stride, 0, sizeof(stride_line_t)*16);
};

// =====================================================================
void processor_t::clock_STRIDE()
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
			if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD)
			{
				bool cache_L1_hit = 0;
				bool cache_L2_hit = 0;
				int j;

				//FAZER A BUSCA NA CACHE L1
				orcs_engine.processor->latency = 1;
				uint64_t index = new_instruction.opcode_address & 255;
				uint64_t tag = new_instruction.opcode_address >> 8;

				for (j = 0; j < 4; ++j)
				{
					if (orcs_engine.processor->CL1[index][j].tag == tag)
						cache_L1_hit = 1;
				}

				//se não houve hit na cache L1
				if (!cache_L1_hit)
				{
					//FAZER A BUSCA NA CACHE L2
					orcs_engine.processor->latency += 4;
					index = new_instruction.opcode_address & 2047;
					tag = new_instruction.opcode_address >> 11;

					for (j = 0; j < 8; ++j)
					{
						if (orcs_engine.processor->CL2[index][j].tag == tag)
							cache_L2_hit = 1;
					}
				}

				//se houver miss nas duas, chamar o STRIDE
				if (!cache_L1_hit && !cache_L2_hit)
				{
					//fazer alguma coisa aqui no stride prefetcher
					orcs_engine.processor->latency += 200; //VERIFICAR
				}
			}






			else if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_MEM_STORE)
			{
				//FAZER A BUSCA NA CACHE L1

				//FAZER A BUSCA NA CACHE L2

				//se houver miss nas duas, chamar o STRIDE
			}
		} //end if (orcs_engine.processor->latency > 0)

	} //else caso haja instruções
};


// =====================================================================
void processor_t::statistics() {
	ORCS_PRINTF("######################################################\n");
	ORCS_PRINTF("processor_t\n");

	printf("---STATISTICS---\n");

	printf("---ACCURACY---\n");

};
