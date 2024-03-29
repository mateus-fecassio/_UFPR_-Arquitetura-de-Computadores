#include "simulator.hpp"

orcs_engine_t orcs_engine;

// =============================================================================
static void display_use() {
    ORCS_PRINTF("**** OrCS - Ordinary Computer Simulator ****\n\n");
    ORCS_PRINTF("Please provide -t <trace_file_basename>");
};

// =============================================================================
static void process_argv(int argc, char **argv) {

    // Name, {no_argument, required_argument and optional_argument}, flag, value
    static struct option long_options[] = {
        {"help",        no_argument, 0, 'h'},
        {"trace",       required_argument, 0, 't'},
        {NULL,          0, NULL, 0}
    };

    // Count number of traces
    int opt;
    int option_index = 0;
    while ((opt = getopt_long_only(argc, argv, "h:t:",
                 long_options, &option_index)) != -1) {
        switch (opt) {
        case 0:
            printf ("Option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;

        case 'h':
            display_use();
            break;

        case 't':
            orcs_engine.arg_trace_file_name = optarg;
            break;
        case '?':
            break;

        default:
            ORCS_PRINTF(">> getopt returned character code 0%o ??\n", opt);
        }
    }

    if (optind < argc) {
        ORCS_PRINTF("Non-option ARGV-elements: ");
        while (optind < argc)
            ORCS_PRINTF("%s ", argv[optind++]);
        ORCS_PRINTF("\n");
    }


    if (orcs_engine.arg_trace_file_name == NULL) {
        ORCS_PRINTF("Trace file not defined.\n");
        display_use();
    }

};


// =============================================================================
int main(int argc, char **argv) {
    process_argv(argc, argv);

    /// Call all the allocate's
    orcs_engine.allocate();
    orcs_engine.trace_reader->allocate(orcs_engine.arg_trace_file_name);
    orcs_engine.processor->allocate();

    orcs_engine.simulator_alive = true;

    /// Start CLOCK for all the components
    while (orcs_engine.simulator_alive) {
        orcs_engine.processor->clock_YAGS();
        orcs_engine.global_cycle++;
    }

    // IMPRESSÃO DA BTB NA FINALIZAÇÃO DO PROCESSAMENTO DO TRAÇO
        /*
        for (int i = 0; i < 1024; ++i)
        {
          for (int j = 0; j < 4; ++j)
          {
            printf("TAG = %ld, LRU = %ld | ", orcs_engine.processor->btb[i][j].tag, orcs_engine.processor->btb[i][j].lru);
          }
          printf("\n");
        }
        */

    // IMPRESSÃO DA PHT NA FINALIZAÇÃO DO PROCESSAMENTO DO TRAÇO
        /*
        for (int i = 0; i < 1024; ++i)
        {
            printf("  %d  ", orcs_engine.processor->pht[i].counter);
        }
        */

    // IMPRESSÃO DA T_CACHE NA FINALIZAÇÃO DO PROCESSAMENTO DO TRAÇO [VETOR]
        /*
        printf("T_CACHE:\n");
        for (int i = 0; i < 512; ++i)
        {
            printf("  TAG = %d    COUNTER = %d  \n", orcs_engine.processor->t_cache[i].tag, orcs_engine.processor->t_cache[i].counter);
        }
        printf("\n\n\n\n");
        */

    // IMPRESSÃO DA BHT NA FINALIZAÇÃO DO PROCESSAMENTO DO TRAÇO [VETOR]
        /*
        printf("NT_CACHE:\n");
        for (int i = 0; i < 512; ++i)
        {
            printf("  TAG = %d    COUNTER = %d  \n", orcs_engine.processor->nt_cache[i].tag, orcs_engine.processor->nt_cache[i].counter);
        }
        printf("\n\n\n\n");
        */
	ORCS_PRINTF("End of Simulation\n")
	orcs_engine.trace_reader->statistics();
  orcs_engine.processor->statistics();

  return(EXIT_SUCCESS);
};
