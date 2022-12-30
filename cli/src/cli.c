#include <stdio.h>
#include <string.h>

#include "binary_format.h"
#include "executor.h"

static void print_help()
{
    printf("Help menu:\n");
    printf("./litenvm --version - to see the version of the VM\n");
    printf("./litenvm --help - to see the help menu\n");
    printf("./litenvm --print <lvm-file> - to print information about the program such as constant pool and instruction stream\n");
    printf("./litenvm <lvm-file> - to run the program stored inside the lvm file\n");
}

static void print_version()
{
    printf("LitenVM VERSION %s\n", LITENVM_VERSION);
}

static FILE *open_file(FILE *file, const char *filename)
{
    file = fopen(filename, "rb");

    if (!file)
    {
        printf("Could not open the file: %s\n", filename);
    }

    return file;
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--version") == 0)
    {
        print_version();
    }
    else if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        print_help();
    }
    else if (argc == 3 && strcmp(argv[1], "--print") == 0)
    {
        FILE *file;

        if (open_file(file, argv[2]))
        {
            ConstantPool *constpool = binform_read_constantpool(file);
            InstructionStream *inststream = binform_read_instructions(file);
            binform_print(constpool, inststream);
        }
    }
    else if (argc == 2)
    {
        FILE *file;

        if (open_file(file, argv[1]))
        {
            ConstantPool *constpool = binform_read_constantpool(file);
            InstructionStream *inststream = binform_read_instructions(file);
            Executor *executor = executor_new(constpool, inststream);
            executor_step_all(executor);
        }
    }
    else
    {
        print_help();
    }

    return 0;
}