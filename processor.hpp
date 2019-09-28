// ============================================================================
// ============================================================================
class btb_line_t
{
  private:

  public:
    uint64_t pc;
    uint64_t lru;
};

class processor_t
{
    private:

    public:
      btb_line_t btb[1024][4];
      uint64_t latency;
      int delay;

		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
	    void allocate();
	    void clock();
	    void statistics();
};
