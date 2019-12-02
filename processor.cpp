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
bool processor_t::check_L2(uint64_t address) {
	uint64_t index = new_instruction.read_address & 2047;
	uint64_t tag = new_instruction.read_address >> 11;
	bool cache_L2_hit = 0;

	for (int j = 0; j < 8; ++j)
	{
		if (orcs_engine.processor->CL2[index][j].tag == tag)
			cache_L2_hit = 1;
	}
	
	return cache_L2_hit;
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
			orcs_engine.processor->latency--;
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
				uint64_t index = new_instruction.read_address & 255;
				uint64_t tag = new_instruction.read_address >> 8;

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
					index = new_instruction.read_address & 2047;
					tag = new_instruction.read_address >> 11;

					for (j = 0; j < 8; ++j)
					{
						if (orcs_engine.processor->CL2[index][j].tag == tag)
							cache_L2_hit = 1;
					}
				}

				//se houver miss nas duas, chamar o STRIDE
				if (!cache_L1_hit && !cache_L2_hit)
				{
					//INÍCIO DO ALGORITMO DO STRIDE PREFETCHER
					bool cache_stride_hit = 0;
					bool cache_stride_empty = 0;
					int empty_block = 0;
					int cholast_addresssen_block = 0;
					uint64_t bigger = -1;

					for (j = 0; j < 16; ++j)
					{
						if (orcs_engine.processor->stride[j].tag == new_instruction.opcode_address)
						{
							cache_stride_hit = 1;
							index = j;
							orcs_engine.processor->stride[j].counter = 0;
						}

						//procuar por uma linha vazia
						if (orcs_engine.processor->stride[j].tag == 0)
						{
							cache_stride_empty = 1;
							empty_block = j;
						}

						//procurar por uma linha que tenha o maior contador
						if (orcs_engine.processor->stride[j].counter > bigger)
						{
							bigger = orcs_engine.processor->stride[j].counter;
							chosen_block = j;
						}
					}

					//se não houve hit na tabela do prefetcher, alocar um espaço
					if (!cache_stride_hit)
					{
						//se existe um bloco livre, colocar as informações nesse bloco
						if (cache_stride_empty)
						{
							orcs_engine.processor->stride[empty_block].tag = new_instruction.opcode_address;
							orcs_engine.processor->stride[empty_block].last_address = new_instruction.read_address;
							orcs_engine.processor->stride[empty_block].status = -1;
							orcs_engine.processor->stride[empty_block].counter = 0;

							index = empty_block;
						}

						//se não houver um bloco livre, colocar as informações no bloco mais antigo
						else
						{
							orcs_engine.processor->stride[chosen_block].tag = new_instruction.opcode_address;
							orcs_engine.processor->stride[chosen_block].last_address = new_instruction.read_address;
							orcs_engine.processor->stride[chosen_block].status = -1;
							orcs_engine.processor->stride[chosen_block].counter = 0;

							index = chosen_block;

							//aumenta os contadores dos outros blocos
							for (j = 0; j < 16; ++j)
							{
								if (j != chosen_block)
								{
									orcs_engine.processor->stride[j].counter ++;
								}
							}
						}
					} //end if (!cache_stride_hit)

					//independente de hit ou não, fazer o conjunto de instruções
					uint64_t current_address = new_instruction.read_address;
					uint64_t last_address = orcs_engine.processor->stride[index].last_address;
					uint64_t stride_size;

					//calcular o stride
					stride_size = abs(current_address - last_address);

					//verificar se esse stride bate com o que consta na tabela
					if (stride_size == orcs_engine.processor->stride[index].stride)
					{
						if (orcs_engine.processor->stride[index].stride < 2)
							orcs_engine.processor->stride[index].stride ++;
					}
					else //caso o stride não coincida, invativar esse status
					{
						orcs_engine.processor->stride[index].stride = 0;
					}

					//se o status estiver como ativo
					if (orcs_engine.processor->stride[index].stride == 2)
					{
						//verificar se a linha está na cache L2

						check_L2(new_instruction.read_address);
					}




					//orcs_engine.processor->latency += 200; //VERIFICAR
				} //end if (!cache_L1_hit && !cache_L2_hit)
			}






			else if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_MEM_STORE)
			{
				bool cache_L1_hit = 0;
				bool cache_L2_hit = 0;
				int j;

				//FAZER A BUSCA NA CACHE L1
				orcs_engine.processor->latency = 1;
				uint64_t index = new_instruction.write_address & 255;
				uint64_t tag = new_instruction.write_address >> 8;

				for (j = 0; j < 4; ++j)
				{
					if (orcs_engine.processor->CL1[index][j].tag == tag)
					{
						cache_L1_hit = 1;
						orcs_engine.processor->CL1[index][j].valid = 0;
					}
				}

				//se não houve hit na cache L1
				if (!cache_L1_hit)
				{
					//FAZER A BUSCA NA CACHE L2
					orcs_engine.processor->latency += 4;
					index = new_instruction.write_address & 2047;
					tag = new_instruction.write_address >> 11;

					for (j = 0; j < 8; ++j)
					{
						if (orcs_engine.processor->CL2[index][j].tag == tag)
						{
							cache_L2_hit = 1;
							orcs_engine.processor->CL2[index][j].valid = 0;
						}
					}
				}

				//se houver miss nas duas, penalidade de mais 200 ciclos para a escrita em memória
				if (!cache_L1_hit && !cache_L2_hit)
				{
					orcs_engine.processor->latency += 200;
				}
			} //end if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_MEM_STORE)
		} //end if (orcs_engine.processor->latency < 0)
	} //else caso haja instruções
};


// =====================================================================
void processor_t::statistics() {
	ORCS_PRINTF("######################################################\n");
	ORCS_PRINTF("processor_t\n");

	printf("---STATISTICS---\n");

	printf("---ACCURACY---\n");

};
