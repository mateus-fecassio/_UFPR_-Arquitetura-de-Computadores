#include "simulator.hpp"

// =====================================================================
processor_t::processor_t() {

};

// =====================================================================
void processor_t::allocate() {
	memset (orcs_engine.processor->CL1, 0, sizeof(cache_line_t)*256*4);
	memset (orcs_engine.processor->CL2, 0, sizeof(cache_line_t)*2048*8);
	memset (orcs_engine.processor->stride, 0, sizeof(stride_line_t)*16);

	orcs_engine.processor->latency = 0;
	orcs_engine.processor->cycles = 0;
	orcs_engine.processor->correctPF = 0;
	orcs_engine.processor->incorrectPF = 0;
	orcs_engine.processor->L1_hit = 0;
	orcs_engine.processor->L1_access = 0;
	orcs_engine.processor->L2_hit = 0;
	orcs_engine.processor->L2_access = 0;
	orcs_engine.processor->total_prefetches = 0;
	orcs_engine.processor->class1 = 0;
	orcs_engine.processor->class2 = 0;
	orcs_engine.processor->class3 = 0;
	orcs_engine.processor->class4 = 0;
	orcs_engine.processor->class5 = 0;
	orcs_engine.processor->class6 = 0;
	orcs_engine.processor->class7 = 0;
	orcs_engine.processor->class8 = 0;
	orcs_engine.processor->class9 = 0;
	orcs_engine.processor->class10 = 0;
	orcs_engine.processor->class11 = 0;

};

// =====================================================================
bool processor_t::check_L2(uint64_t address) {
	uint64_t index = address & 2047;
	uint64_t tag = address >> 11;
	bool cache_L2_hit = 0;

	for (int j = 0; j < 8; ++j)
	{
		if (orcs_engine.processor->CL2[index][j].tag == tag)
			cache_L2_hit = 1;
	}

	return cache_L2_hit;
};

// =====================================================================
bool processor_t::is_cacheL2_empty(uint64_t address) {
	uint64_t index = address & 2047;
	bool cache_L2_empty = 0;

	for (int j = 0; j < 8; ++j)
	{
		if (orcs_engine.processor->CL2[index][j].tag == 0)
			cache_L2_empty = 1;
	}

	return cache_L2_empty;
};
// =====================================================================
void processor_t::put_cache_L2(int j, uint64_t address, int clean, int load, uint64_t ready_cycle, uint64_t lru) {
	uint64_t index = address & 2047;
	uint64_t tag = address >> 11;


	//requisição de escrita do bloco escolhido na DRAM
	//se esse campo ainda estiver setado como 1, significa que foi feito o prefetch dele, porém não foi utilizado
	if (orcs_engine.processor->CL2[index][j].load_prefetch == 1)
		orcs_engine.processor->incorrectPF++;


	//ESCRITA DO BLOCO DA CACHE L1 NA CACHE L2
	orcs_engine.processor->CL2[index][j].clean = clean;
	orcs_engine.processor->CL2[index][j].load_prefetch = load;
	orcs_engine.processor->CL2[index][j].tag = tag;
	orcs_engine.processor->CL1[index][j].ready_cycle = ready_cycle;
	orcs_engine.processor->CL1[index][j].lru = lru;
};

// =====================================================================
int processor_t::free_cacheL2_block(uint64_t address) {
	uint64_t index = address & 2047;
	uint64_t oldest_cycle = orcs_engine.global_cycle;
	bool is_empty = 0;
	int j, chosen_column, empty_column;

	for (j = 0; j < 8; ++j)
	{
		if (orcs_engine.processor->CL2[index][j].tag == 0)
		{
			is_empty = 1;
			empty_column = j;
		}

		if (orcs_engine.processor->CL2[index][j].lru < oldest_cycle)
		{
			oldest_cycle = orcs_engine.processor->CL1[index][j].lru;
			chosen_column = j;
		}
	}

	if (is_empty)
		return empty_column;
	else
		return chosen_column;
};

// =====================================================================
int processor_t::free_cacheL1_block(uint64_t address) {
	uint64_t index = address & 255;
	uint64_t oldest_cycle = orcs_engine.global_cycle;
	int j, chosen_column;

	for (j = 0; j < 4; ++j)
	{
		if (orcs_engine.processor->CL1[index][j].lru < oldest_cycle)
		{
			oldest_cycle = orcs_engine.processor->CL1[index][j].lru;
			chosen_column = j;
		}
	}

	j = free_cacheL2_block(address);

	int clean = orcs_engine.processor->CL1[index][chosen_column].clean;
	int load = orcs_engine.processor->CL1[index][chosen_column].load_prefetch;
	uint64_t ready_cycle = orcs_engine.processor->CL1[index][chosen_column].ready_cycle;
	uint64_t lru = orcs_engine.processor->CL1[index][chosen_column].lru;


	put_cache_L2(j, address, clean, load, ready_cycle, lru);

	return chosen_column;
};

// =====================================================================
void processor_t::put_cache_L1(int parameter, uint64_t address, uint64_t cycle) {
	uint64_t index = address & 255;
	uint64_t tag = address >> 8;
	bool empty = 0;
	int column = 0;
	int j;

	//procura por um espaço livre para colocar o dado
	for (j = 0; j < 4; ++j)
	{
		if (orcs_engine.processor->CL1[index][j].tag == 0)
		{
			empty = 1;
			column = j;
		}
	}

	//se NÃO tiver espaço na L1 para colocar o dado
	if (!empty)
	{
		//libero um espaço na cache L1 para colocar o dado
		column = free_cacheL1_block(address);
	}

	//ESCRITA DO BLOCO NA CACHE
	if (parameter == 1) //FRUTO DE UMA LEITURA
	{
		orcs_engine.processor->CL1[index][column].clean = 1;
		orcs_engine.processor->CL1[index][column].load_prefetch = 1;
	}
	else
	{
		orcs_engine.processor->CL1[index][column].clean = 0;
		orcs_engine.processor->CL1[index][column].load_prefetch = 0;
	}

	orcs_engine.processor->CL1[index][column].tag = tag;
	orcs_engine.processor->CL1[index][column].ready_cycle = cycle;
	orcs_engine.processor->CL1[index][column].lru = orcs_engine.global_cycle;
};

// =====================================================================
void processor_t::determine_range(uint64_t a, uint64_t b){
	uint64_t diff = abs(a - b);

	if (a < b)
	{
		if (diff > 200)
			orcs_engine.processor->class1++;
		else if (diff > 150 && diff < 199)
			orcs_engine.processor->class2++;
		else if (diff > 100 && diff < 149)
			orcs_engine.processor->class3++;
		else if (diff > 50 && diff < 99)
			orcs_engine.processor->class4++;
		else if (diff > 1 && diff < 49)
			orcs_engine.processor->class5++;

	}
	else if (a > b)
	{
		if (diff > 1 && diff < 49)
			orcs_engine.processor->class7++;
		else if (diff > 50 && diff < 99)
			orcs_engine.processor->class8++;
		else if (diff > 100 && diff < 149)
			orcs_engine.processor->class9++;
		else if (diff > 150 && diff < 199)
			orcs_engine.processor->class10++;
		else if (diff > 200)
			orcs_engine.processor->class11++;
	}
	else if (a == b)
	{
		orcs_engine.processor->class6++;
	}
};
// =====================================================================
void processor_t::clock_STRIDE()
{
	if (orcs_engine.processor->latency > 0)
	{
		orcs_engine.processor->latency--
	}
	else
	{
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
				uint64_t contador;

				//penalidade
				for(contador = orcs_engine.processor->latency; contador != 0; --contador)

				orcs_engine.processor->latency = 0;
			}

			if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD)
			{
					bool cache_L1_hit = 0;
					bool cache_L2_hit = 0;
					int j;

					//FAZER A BUSCA NA CACHE L1
					orcs_engine.processor->latency = 1;
					orcs_engine.processor->L1_access++;
					uint64_t index = new_instruction.read_address & 255;
					uint64_t tag = new_instruction.read_address >> 8;

					for (j = 0; j < 4; ++j)
					{
						if (orcs_engine.processor->CL1[index][j].tag == tag)
						{
							uint64_t a = orcs_engine.processor->CL1[index][j].ready_cycle;
							uint64_t b = orcs_engine.global_cycle;

							if (orcs_engine.processor->CL1[index][j].load_prefetch == 1)
							{
								orcs_engine.processor->CL1[index][j].load_prefetch = 0;
								orcs_engine.processor->correctPF++;
								determine_range(a, b);
							}

							if (b >= a) //significa que o dado já está na cache e pode ser usado
							{
								cache_L1_hit = 1;
								orcs_engine.processor->L1_hit++;
							}
						}
					}

					//se não houve hit na cache L1
					if (!cache_L1_hit)
					{
						//FAZER A BUSCA NA CACHE L2
						orcs_engine.processor->latency += 4;
						orcs_engine.processor->L2_access++;
						index = new_instruction.read_address & 2047;
						tag = new_instruction.read_address >> 11;

						for (j = 0; j < 8; ++j)
						{
							if (orcs_engine.processor->CL2[index][j].tag == tag)
							{
								if (orcs_engine.processor->CL2[index][j].tag == tag)
								{
									uint64_t a = orcs_engine.processor->CL2[index][j].ready_cycle;
									uint64_t b = orcs_engine.global_cycle;

									if (orcs_engine.processor->CL2[index][j].load_prefetch == 1)
									{
										orcs_engine.processor->CL2[index][j].load_prefetch = 0;
										orcs_engine.processor->correctPF++;
										determine_range(a, b);
									}

									if (b >= a) //significa que o dado já está na cache e pode ser usado
									{
										cache_L2_hit = 1;
										orcs_engine.processor->L2_hit++;
									}
								}
							}
						}
					}

					//se houver miss nas duas, chamar o STRIDE
					if (!cache_L1_hit && !cache_L2_hit)
					{
						//INÍCIO DO ALGORITMO DO STRIDE PREFETCHER
						bool cache_stride_hit = 0;
						bool cache_stride_empty = 0;
						int empty_block = 0;
						int chosen_block = 0;
						uint64_t bigger = -1;

						for (j = 0; j < 16; ++j)
						{
							if (orcs_engine.processor->stride[j].tag == new_instruction.opcode_address)
							{
								cache_stride_hit = 1;
								index = j;
								orcs_engine.processor->stride[j].counter = 0;
							}

							//procurar por uma linha vazia
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
								orcs_engine.processor->stride[empty_block].confidence = 0;
								orcs_engine.processor->stride[empty_block].counter = 0;

								index = empty_block;
							}

							//se não houver um bloco livre, colocar as informações no bloco mais antigo
							else
							{
								orcs_engine.processor->stride[chosen_block].tag = new_instruction.opcode_address;
								orcs_engine.processor->stride[chosen_block].last_address = new_instruction.read_address;
								orcs_engine.processor->stride[chosen_block].confidence = 0;
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
							if (orcs_engine.processor->stride[index].confidence < 2)
								orcs_engine.processor->stride[index].confidence ++;
						}
						else //caso o stride não coincida, inativar a confiança
						{
							orcs_engine.processor->stride[index].confidence = 0;
							orcs_engine.processor->stride[index].stride = stride_size;
						}

						orcs_engine.processor->stride[index].last_address = current_address;

						//se o confidence estiver como ativo
						if (orcs_engine.processor->stride[index].confidence == 2)
						{
							//verificar se o dado do 4 x stride está presente na cache L2
							uint64_t address;
							address = orcs_engine.processor->stride[index].last_address + (4 * orcs_engine.processor->stride[index].stride);

							//se não encontrou esse dado na cache
							if (!check_L2(address))
							{
								//faz a requisição para colocar esse endereço na cache L1, passando o parâmetro 1(leitura), endereço de memória e quando esse dado estará disponível
								put_cache_L1(1, address, orcs_engine.global_cycle+205);
								orcs_engine.processor->total_prefetches++;
							}
						}
						orcs_engine.processor->latency += 200;
					} //end if (!cache_L1_hit && !cache_L2_hit)
				}

	//----------------------------------------------------------------------------------------------------

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
							orcs_engine.processor->CL1[index][j].clean = 0;
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
								orcs_engine.processor->CL2[index][j].clean = 0;
							}
						}
					}

					//se houver miss nas duas, penalidade de mais 200 ciclos para a escrita em memória
					if (!cache_L1_hit && !cache_L2_hit)
					{
						orcs_engine.processor->latency += 200;

						//COLOCAR A LINHA NA CACHE L1, passando o parâmetro 2(escrita), o endereço para a escrita e quando esse dado estará disponível
						put_cache_L1(2, new_instruction.write_address, orcs_engine.global_cycle+205);
					}
			} //end if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_MEM_STORE)
		}
	}
};


// =====================================================================
void processor_t::statistics() {
	ORCS_PRINTF("######################################################\n");
	ORCS_PRINTF("processor_t\n");

	printf("---STATISTICS---\n");
	printf("Cycles: %ld\n", orcs_engine.processor->cycles);
	printf("Prefetches: %ld\n", orcs_engine.processor->total_prefetches);
	printf("L1 Cache Hit: %ld\n", orcs_engine.processor->L1_hit);
	printf("L1 Cache Hit Ratio: %g\n", (float)orcs_engine.processor->L1_hit / (float)orcs_engine.processor->L1_access);
	printf("L2 Cache Hit: %ld\n", orcs_engine.processor->L2_hit);
	printf("L2 Cache Hit Ratio: %g\n", (float)orcs_engine.processor->L2_hit / (float)orcs_engine.processor->L2_access);
	printf("Prefetches CORRETOS: %ld\n", orcs_engine.processor->correctPF);
	printf("Prefetches CORRETOS: %ld\n", orcs_engine.processor->incorrectPF);
	printf("Taxa de acertos nos prefetches feitos: %g\n\n\n", (float)orcs_engine.processor->correctPF / (float)orcs_engine.processor->total_prefetches);

	printf("HISTOGRAMA (em número)\n");
	printf("< -200: %ld\n", orcs_engine.processor->class1);
	printf("-199 a -150: %ld\n", orcs_engine.processor->class2);
	printf("-149 a -100: %ld\n", orcs_engine.processor->class3);
	printf("-99 a -50: %ld\n", orcs_engine.processor->class4);
	printf("-49 a -1: %ld\n", orcs_engine.processor->class5);
	printf("0: %ld\n", orcs_engine.processor->class6);
	printf("1 a 49: %ld\n", orcs_engine.processor->class7);
	printf("50 a 99: %ld\n", orcs_engine.processor->class8);
	printf("100 a 149: %ld\n", orcs_engine.processor->class9);
	printf("150 a 199: %ld\n", orcs_engine.processor->class10);
	printf("> 200: %ld\n\n", orcs_engine.processor->class11);

	printf("HISTOGRAMA (em taxa)\n");
	printf("< -200: %g\n", (float)orcs_engine.processor->class1 / (float)orcs_engine.processor->correctPF);
	printf("-199 a -150: %g\n", (float)orcs_engine.processor->class2 / (float)orcs_engine.processor->correctPF);
	printf("-149 a -100: %g\n", (float)orcs_engine.processor->class3 / (float)orcs_engine.processor->correctPF);
	printf("-99 a -50: %g\n", (float)orcs_engine.processor->class4 / (float)orcs_engine.processor->correctPF);
	printf("-49 a -1: %g\n", (float)orcs_engine.processor->class5 / (float)orcs_engine.processor->correctPF);
	printf("0: %g\n", (float)orcs_engine.processor->class6 / (float)orcs_engine.processor->correctPF);
	printf("1 a 49: %g\n", (float)orcs_engine.processor->class7 / (float)orcs_engine.processor->correctPF);
	printf("50 a 99: %g\n", (float)orcs_engine.processor->class8 / (float)orcs_engine.processor->correctPF);
	printf("100 a 149: %g\n", (float)orcs_engine.processor->class9 / (float)orcs_engine.processor->correctPF);
	printf("150 a 199: %g\n", (float)orcs_engine.processor->class10 / (float)orcs_engine.processor->correctPF);
	printf("> 200: %g\n", (float)orcs_engine.processor->class11 / (float)orcs_engine.processor->correctPF);
};
