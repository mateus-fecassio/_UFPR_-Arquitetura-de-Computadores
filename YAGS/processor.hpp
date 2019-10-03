// ============================================================================
// ============================================================================
class btb_line_t
{
  private:

  public:
    uint64_t tag;    //guarda o restante do PC
    uint64_t target; //alvo (PC inteiro da próxima instrução)
    uint64_t lru;    //last recently used (mais velho)
};

class pht_line_t
{
  private:

  public:
    int counter;
};

class cache_line_t
{
  private:

  public:
    int tag;
    int counter;
    //uint64_t lru;    //last recently used (mais velho)
};

class processor_t
{
    private:

    public:
      btb_line_t btb[1024][4];
      pht_line_t pht[1024];
      cache_line_t t_cache[512];
      cache_line_t nt_cache[512];
      unsigned int bhr;
      unsigned int mask;
      uint64_t latency;
      uint64_t pht_miss;  //conta a quantidade de miss da PHT
      uint64_t btb_miss;  //conta a quantidade de miss da BTB
      uint64_t cycles;
      uint64_t branches;
      int delay;
      bool lock; //trava utilizada para instruções que não estão imediatamentes depois de um branch
      int trust; //utilizado para verificar se vai ser taken ou not taken
      bool cache_hit; //0-miss e 1-hit
      bool chosen_cache; //0-not taken e 1-taken
      int index_cache;
      int tag_cache;
      int btb_row_target; //guarda a linha para atulizar o target do BTB do branch anterior
      int btb_col_target; //guarda a coluna para atulizar o target do BTB do branch anterior

		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
	    void clock_YAGS();
	    void statistics();
};
