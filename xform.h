#ifndef LIBOT_XFORM_H
#define LIBOT_XFORM_H

#include <stdlib.h>
#include <string.h>
#include "ot.h"
#include "utf8.h"

typedef struct ot_xform_pair {
    ot_op* op1_prime;
    ot_op* op2_prime;
} ot_xform_pair;

ot_xform_pair ot_xform(ot_op* op1, ot_op* op2);

#endif
