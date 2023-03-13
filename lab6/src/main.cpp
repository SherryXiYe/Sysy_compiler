#include <iostream>
#include <string.h>
#include <unistd.h>
#include "Ast.h"
#include "Unit.h"
using namespace std;

Ast ast;
Unit unit;
extern FILE *yyin;
extern FILE *yyout;

int yyparse();

char outfile[256] = "a.out";
bool dump_tokens;
bool dump_ast;
bool dump_ir;
extern bool check_no_err = true;

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "iato:")) != -1)
    {
        switch (opt)
        {
        case 'o':
            strcpy(outfile, optarg);
            break;
        case 'a':
            dump_ast = true;
            break;
        case 't':
            dump_tokens = true;
            break;
        case 'i':
            dump_ir = true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-o outfile] infile\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }
    if (optind >= argc)
    {
        fprintf(stderr, "no input file\n");
        exit(EXIT_FAILURE);
    }
    if (!(yyin = fopen(argv[optind], "r")))
    {
        fprintf(stderr, "%s: No such file or directory\nno input file\n", argv[optind]);
        exit(EXIT_FAILURE);
    }
    if (!(yyout = fopen(outfile, "w")))
    {
        fprintf(stderr, "%s: fail to open output file\n", outfile);
        exit(EXIT_FAILURE);
    }
    // std::cout<<11<<std::endl;
    yyparse();
        // std::cout<<22<<std::endl;
    if(dump_ast)
        ast.output();
    // ast.output();
            // std::cout<<0<<std::endl;

    ast.typeCheck();
        // std::cout<<33<<std::endl;

    // ast.output();

    if(check_no_err){
        ast.genCode(&unit);
        if(dump_ir)
            unit.output();
    }
    else
        fprintf(yyout, "fail to generate code\n");
    // // std::cout<<44<<std::endl;
    return 0;
}
