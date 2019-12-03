// ============================================================================
// ============================================================================
class cache_line_t
{
  private:

  public:
    int clean;                //verifica se a linha está limpa (0 = não(suja), 1 = sim)
    uint64_t tag;             //guarda o restante do endereço de memória
    //uint64_t data;          //dado
    uint64_t ready_cycle;     //guarda o ciclo de clock que aquele dado estará pronto
    uint64_t lru;             //last recently used (mais velho)

    int load_prefetch;        //usado para diferenciar se a linha foi fruto de um prefetch (0 = NÃO, 1 = SIM)
};


class stride_line_t
{
  private:

  public:
    uint64_t tag;           //guarda o restante do PC
    uint64_t last_address;  //guarda o último endereço de memória associado a aquele PC
    uint64_t stride;        //guarda o deslocamento calculado
    uint64_t confidence;    //confiabilidade do dado (0 = inativo, 1 = treinamento, 2 = ativo)
    uint64_t counter;       //usado para a necessidade de substituição na tabela do stride prefetcher
};

class processor_t
{
    private:

    public:
      cache_line_t CL1[256][4];
      cache_line_t CL2[2048][8];
      stride_line_t stride[16];
      uint64_t latency;
      uint64_t cycles;
      uint64_t correctPF;       //contador de número de prefetches feitos e usados
      uint64_t incorrectPF;     //contador de número de prefetches feitos e que sofreram eviction (e não foram utilizados)

      uint64_t L1_hit;
      uint64_t L1_access;

      uint64_t L2_hit;
      uint64_t L2_access;

      uint64_t total_prefetches;

      //classes para a construção do histograma
      uint64_t class1;      //<200
      uint64_t class2;      //-199 a -150
      uint64_t class3;      //-149 a -100
      uint64_t class4;      //-99 a -50
      uint64_t class5;      //-49 a -1
      uint64_t class6;      //0
      uint64_t class7;      //1 a 49
      uint64_t class8;      //50 a 99
      uint64_t class9;      //100 a 149
      uint64_t class10;     //150 a 199
      uint64_t class11;     //>200

		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
      void determine_range (uint64_t a, uint64_t b);
	    void clock_STRIDE();
	    void statistics();
      bool check_L2(uint64_t address);    //usada para checar se esse endereço está a cache L2. retorna 0 se não encontrou o dado e 1 se encontrou
      bool is_cacheL2_empty(uint64_t address); //0 NÃO, 1 SIM
      void put_cache_L1(int parameter, uint64_t address, uint64_t cycle);
      void put_cache_L2(int j, uint64_t address, int clean, int load, uint64_t ready_cycle, uint64_t lru);
      int free_cacheL1_block(uint64_t address); //retorna a coluna que você pode colocar o dado na cache L1
      int free_cacheL2_block(uint64_t address);

};
