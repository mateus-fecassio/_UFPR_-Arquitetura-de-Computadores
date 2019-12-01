// ============================================================================
// ============================================================================
class cache_line_t
{
  private:

  public:
    uint64_t valid;  //verifica se a linha está suja (0 = não, 1 = sim)
    uint64_t tag;    //guarda o restante do PC
    //uint64_t data; //dado
    uint64_t lru;    //last recently used (mais velho)
};


class stride_line_t
{
  private:

  public:
    uint64_t tag;           //guarda o restante do PC
    uint64_t last_address;  //guarda o último endereço de memória associado a aquele PC
    uint64_t stride;        //guarda o deslocamento calculado
    uint64_t status;        //confiabilidade do dado (0 = inativo, 1 = treinamento, 2 = ativo)
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


		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
	    void clock_STRIDE();
	    void statistics();
};
