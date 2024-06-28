#include <cstdio>

#include "pipeline_db_builder.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <material_file1.json> [<material_file2.json> ...] -o output_file\n",argv[0]);

        return 1;
    }

    PipelineDBConstructor db_builder;

    const char* output_file = "mat_out.bin";

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];

        if (strcmp(arg, "-o") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "invalid usage: -o <out_file_here>\n");
                return -1;
            }
            output_file = argv[i];
            continue;
        }

        db_builder.compile_material_file(arg);
    }

    db_builder.dump_to_file(output_file);

    return 0;
}