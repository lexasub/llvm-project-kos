#ifndef _CUDA_H
#define _CUDA_H

#include "ppcg.h"
#include "ppcg_options.h"

int generate_cuda(isl_ctx *ctx, struct ppcg_options *options,
                  const char *input);

__isl_give isl_printer *
print_host_user(__isl_take isl_printer *p,
                __isl_take isl_ast_print_options *print_options,
                __isl_keep isl_ast_node *node, void *user);
#endif
