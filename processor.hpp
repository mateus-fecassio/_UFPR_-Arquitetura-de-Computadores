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

class bht_line_t
{
  private:

  public:
    int counter;
};

class processor_t
{
    private:

    public:
      btb_line_t btb[1024][4];
      bht_line_t bht[1024];
      uint64_t latency;
      uint64_t btb_miss;  //conta a quantidade de miss da BTB
      uint64_t bht_miss;  //conta a quantidade de miss da BHT
      uint64_t cycles;
      uint64_t branches;
      int delay;
      bool lock; //trava utilizada para instruções que não estão imediatamentes depois de um branch
      int trust; //utilizado para verificar se vai ser taken ou not taken
      int btb_row_target; //guarda a linha para atulizar o target do BTB do branch anterior
      int btb_col_target; //guarda a coluna para atulizar o target do BTB do branch anterior

		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
	    void clock_BHT();
	    void statistics();
};
