// ============================================================================
// ============================================================================
class CACHE_line_t
{
  private:

  public:
    uint64_t tag;    //guarda o restante do PC
    uint64_t data;   //dado
    uint64_t lru;    //last recently used (mais velho)
};


class STRIDE_line_t
{
  private:

  public:
    uint64_t tag;           //guarda o restante do PC
    uint64_t last_address;
    uint64_t stride;
    //STATUS
};

class processor_t
{
    private:

    public:
      cache_line_t CL1[1024][4];
      cache_line_t CL2[1024][4];


		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
	    void clock_STRIDE();
	    void statistics();
};
