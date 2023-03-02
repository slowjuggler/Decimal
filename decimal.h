#ifndef DECIMAL_H_
#define DECIMAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 96
#define VMAX_SIZE 256
#define MAX_POW 28
#define SIZE 4

#define SUCCESS 0
#define FALSE 0
#define TRUE 1
#define BIGVAL 1
#define ERROR 1
#define SMALVAL 2
#define DIVNUL 3
#define PRECISION 6

#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4
#define MOD 5
#define LES 6
#define EQL 7
#define NEQ 8
#define LEQ 9
#define GRT 10
#define GEQ 11
#define FLR 12
#define RND 13
#define TRN 14
#define NGT 15
#define DEC_INT 16
#define DEC_FLT 17
#define INT_DEC 18
#define FLT_DEC 19

#define bit_read(val, bit) (((val) >> (bit)) & 0x01)
#define bit_set(val, bit) ((val) |= ((uint32_t)0x01 << (bit)))
#define bit_zero(val, bit) ((val) &= ~((uint32_t)0x01 << (bit)))
#define get_sign(val) (((val) >> 31) & 0x01)

typedef struct {
     uint32_t bits[SIZE];
} decimal;

typedef union {
    decimal dec;
    uint64_t u64[SIZE];
} v256;

struct data {
    char string[MAX_POW + 2];
    uint8_t opcode;
    int ival;
    uint32_t ret;
    float fval;
    decimal val_1;
    decimal val_2;
    decimal res;
};

struct methods {
    int (*arithmetic)(decimal, decimal, decimal*);
    int (*compare)(decimal, decimal);
    int (*transform_to_str)(char*, decimal);
    int (*flt)(decimal, decimal*);
    int (*transformers)(struct data*, struct methods*);
};

int dec_add(decimal, decimal, decimal*);
int dec_sub(decimal, decimal, decimal*);
int dec_mod(decimal, decimal, decimal*);
int dec_div(decimal, decimal, decimal*);
int dec_mul(decimal, decimal, decimal*);
int is_less(decimal, decimal);
int is_less_or_equal(decimal, decimal);
int is_greater(decimal, decimal);
int is_greater_or_equal(decimal, decimal);
int is_equal(decimal, decimal);
int is_not_equal(decimal, decimal);
int dec_floor(decimal, decimal*);
int dec_round(decimal, decimal*);
int dec_truncate(decimal, decimal*);
int dec_negate(decimal, decimal*);
int from_int_to_decimal(int, decimal*);
int from_decimal_to_int(decimal, int*);
int from_float_to_decimal(float, decimal*);
int from_decimal_to_float(decimal, float*);
void insert_pow(decimal*, uint32_t);
void opcode_parser(char*, struct data*);
int transformers(struct data*, struct methods*);
void get_res(struct data*, struct methods*);
int dtoa(char*, decimal);
void proc_fill(struct data*);
void decimal_fill(decimal*, char*);
void vnorm96(v256*, int*);
int get_pow(decimal);
int norm(v256*, v256*);
void vbzero(v256*);
int vgeq(v256, v256);
int vl(v256, v256);

inline __attribute__((always_inline)) v256 sub(v256, v256);
inline __attribute__((always_inline)) v256 add(v256, v256);
inline __attribute__((always_inline)) v256 vdiv(v256*, v256, int*);
inline __attribute__((always_inline)) v256 lmul(v256, v256);
inline __attribute__((always_inline)) v256 vdiv10(v256, uint64_t*);
inline __attribute__((always_inline)) void dec_bzero(decimal*);
inline __attribute__((always_inline)) int is_zero(decimal);
inline __attribute__((always_inline)) int is_one(decimal);
inline __attribute__((always_inline)) void vshl(v256*, int);
inline __attribute__((always_inline)) void vshr(v256*, int);
inline __attribute__((always_inline)) void vmult_10(v256*);
inline __attribute__((always_inline)) int vget_pos(v256, uint32_t);
inline __attribute__((always_inline)) int vget_ten(v256);
inline __attribute__((always_inline)) int vget_pow(v256);
inline __attribute__((always_inline)) int equal(v256*, v256*);
inline __attribute__((always_inline)) int testz(v256*);

inline void vshl(v256* vval, int shift) {
    v256 tmp = *vval;
    __asm("xorl %%ecx, %%ecx\n\t"
          "movl %[shift], %%ecx\n\t"
          "movq %[v0], %%r8\n\t"
          "movq %[v1], %%r9\n\t"
          "movq %[v2], %%r10\n\t"
          "movq %[v3], %%r11\n\t"
          "movq $0x8000000000000000, %%rbx\n\t"
          "1:\n\t"
          "shlq $1, %%r11\n\t"
          "testq %%rbx, %%r10\n\t"
          "jz 2f\n\t"
          "orq $1, %%r11\n\t"
          "2:\n\t"
          "shlq $1, %%r10\n\t"
          "testq %%rbx, %%r9\n\t"
          "jz 3f\n\t"
          "orq $1, %%r10\n\t"
          "3:\n\t"
          "shlq $1, %%r9\n\t"
          "testq %%rbx, %%r8\n\t"
          "jz 4f\n\t"
          "orq $1, %%r9\n\t"
          "4:\n\t"
          "shlq $1, %%r8\n\t"
          "loop 1b\n\t"
          "movq %%r8, %[a]\n\t"
          "movq %%r9, %[b]\n\t"
          "movq %%r10, %[c]\n\t"
          "movq %%r11, %[d]\n\t"
          : [a] "=m" (tmp.u64[0]), [b] "=m" (tmp.u64[1]),
            [c] "=m" (tmp.u64[2]), [d] "=m" (tmp.u64[3])
          : [v0] "m" (tmp.u64[0]), [v1] "m" (tmp.u64[1]),
            [v2] "m" (tmp.u64[2]), [v3] "m" (tmp.u64[3]),
            [shift] "m" (shift));
    *vval = tmp;
}

inline void vshr(v256* vval, int shift) {
    v256 tmp = *vval;
    __asm("xorl %%ecx, %%ecx\n\t"
          "movl %[shift], %%ecx\n\t"
          "movq %[v0], %%r8\n\t"
          "movq %[v1], %%r9\n\t"
          "movq %[v2], %%r10\n\t"
          "movq %[v3], %%r11\n\t"
          "movq $0x1, %%rbx\n\t"
          "movq $0x8000000000000000, %%rdx\n\t"
          "1:\n\t"
          "shrq $1, %%r8\n\t"
          "testq %%rbx, %%r9\n\t"
          "jz 2f\n\t"
          "orq %%rdx, %%r8\n\t"
          "2:\n\t"
          "shrq $1, %%r9\n\t"
          "testq %%rbx, %%r10\n\t"
          "jz 3f\n\t"
          "orq %%rdx, %%r9\n\t"
          "3:\n\t"
          "shrq $1, %%r10\n\t"
          "testq %%rbx, %%r11\n\t"
          "jz 4f\n\t"
          "orq %%rdx, %%r10\n\t"
          "4:\n\t"
          "shrq $1, %%r11\n\t"
          "loop 1b\n\t"
          "movq %%r8, %[a]\n\t"
          "movq %%r9, %[b]\n\t"
          "movq %%r10, %[c]\n\t"
          "movq %%r11, %[d]\n\t"
          : [a] "=m" (tmp.u64[0]), [b] "=m" (tmp.u64[1]),
            [c] "=m" (tmp.u64[2]), [d] "=m" (tmp.u64[3])
          : [v0] "m" (tmp.u64[0]), [v1] "m" (tmp.u64[1]),
            [v2] "m" (tmp.u64[2]), [v3] "m" (tmp.u64[3]),
            [shift] "m" (shift));
    *vval = tmp;
}

inline v256 add(v256 vval_1, v256 vval_2)
{
    v256 vres = {0};
    __asm("movq %[v10], %%rdx\n\t"
          "addq %[v20], %%rdx\n\t"
          "movq %%rdx, %[a]\n\t"
          "movq %[v11], %%rdx\n\t"
          "adcq %[v21], %%rdx\n\t"
          "movq %%rdx, %[b]\n\t"
          "movq %%rdx, %%rbx\n\t"
          "movq %[v12], %%rdx\n\t"
          "adcq %[v22], %%rdx\n\t"
          "movq %%rdx, %[c]\n\t"
          : [a] "=m" (vres.u64[0]), [b] "=m" (vres.u64[1]), [c] "=m" (vres.u64[2])
          : [v10] "m" (vval_1.u64[0]), [v20] "m" (vval_2.u64[0]),
            [v11] "m" (vval_1.u64[1]), [v21] "m" (vval_2.u64[1]),
            [v12] "m" (vval_1.u64[2]), [v22] "m" (vval_2.u64[2]));
    return vres;
}

inline v256 sub(v256 vval_1, v256 vval_2)
{
    v256 vres = {0};
    __asm("movq %[v10], %%rbx\n\t"
          "movq %[v20], %%rdx\n\t"
          "notq %%rdx\n\t"
          "addq %%rdx, %%rbx\n\t"
          "movq %%rbx, %[a]\n\t"
          "movq %[v11], %%rbx\n\t"
          "movq %[v21], %%rdx\n\t"
          "notq %%rdx\n\t"
          "adcq %%rdx, %%rbx\n\t"
          "movq %%rbx, %[b]\n\t"
          "movq %[v12], %%rbx\n\t"
          "movq %[v22], %%rdx\n\t"
          "notq %%rdx\n\t"
          "adcq %%rdx, %%rbx\n\t"
          "movq %%rbx, %[c]\n\t"
          "addq $1, %[a]\n\t"
          "adcq $0, %[b]\n\t"
          "adcq $0, %[c]\n\t"
          : [a] "=m" (vres.u64[0]), [b] "=m" (vres.u64[1]),
            [c] "=m" (vres.u64[2])
          : [v10] "m" (vval_1.u64[0]), [v20] "m" (vval_2.u64[0]),
            [v11] "m" (vval_1.u64[1]), [v21] "m" (vval_2.u64[1]),
            [v12] "m" (vval_1.u64[2]), [v22] "m" (vval_2.u64[2]));
    return vres;
}

inline v256 lmul(v256 vval_1, v256 vval_2)
{
    v256 vres = {0};
    __asm("movq %[v10], %%rax\n\t"
          "movq %[v20], %%rbx\n\t"
          "mulq %%rbx\n\t"
          "movq %%rax, %[a]\n\t"
          "movq %%rdx, %%r8\n\t"
          "movq %[v11], %%rax\n\t"
          "mulq %%rbx\n\t"
          "addq %%rax, %%r8\n\t"
          "adcq $0, %%rdx\n\t"
          "movq %%rdx, %%r9\n\t"
          "movq %[v21], %%rbx\n\t"
          "movq %[v10], %%rax\n\t"
          "mulq %%rbx\n\t"
          "addq %%rax, %%r8\n\t"
          "adcq $0, %%rdx\n\t"
          "addq %%rdx, %%r9\n\t"
          "movq %[v11], %%rax\n\t"
          "mulq %%rbx\n\t"
          "addq %%rax, %%r9\n\t"
          "adcq $0, %%rdx\n\t"
          "movq %%rdx, %[d]\n\t"
          "movq %%r9, %[c]\n\t"
          "movq %%r8, %[b]\n\t"
          : [a] "=m" (vres.u64[0]), [b] "=m" (vres.u64[1]),
            [c] "=m" (vres.u64[2]), [d] "=m" (vres.u64[3])
          : [v10] "m" (vval_1.u64[0]), [v20] "m" (vval_2.u64[0]),
            [v11] "m" (vval_1.u64[1]), [v21] "m" (vval_2.u64[1]));
    return vres;
}

inline void dec_bzero(decimal* ptr)
{
    ptr->bits[0] = 0;
    ptr->bits[1] = 0;
    ptr->bits[2] = 0;
    ptr->bits[3] = 0;
}

inline int is_one(decimal val)
{
    int ret = FALSE;
    v256 vval = (v256)val;
    v256 one = {0};
    one.u64[0] = 1;
    norm(&vval, &one);
    if (equal(&vval, &one)) ret = TRUE;
    return ret;
}

inline int is_zero(decimal val)
{
    int res = FALSE;
    if (!val.bits[0] && !val.bits[1] && !val.bits[2]) {
        res = TRUE;
    }
    return res;
}

inline void vmult_10(v256* val)
{
    v256 tmp = *val;
    vshl(&tmp, 1);
    vshl(val, 3);
    *val = add(*val, tmp);
}

inline int vget_pow(v256 val)
{
    return (val.u64[1] >> 48) & 0x7F;
}

inline int testz(v256* vval)
{
    int ret = FALSE;
    if (!vval->u64[0] && !vval->u64[1]
        && !vval->u64[2] && !vval->u64[3])
        ret = TRUE;
    return ret;
}

inline v256 vdiv(v256* vval_1, v256 vval_2, int* ret)
{
    v256 tmp = vval_2;
    v256 vres = {0};
    if (equal(vval_1, &vval_2)) {
        *ret = TRUE;
        vres.u64[0] = 1;
        return vres;
    }
    int pos_1 = vget_pos(*vval_1, VMAX_SIZE);
    int pos_2 = vget_pos(vval_2, VMAX_SIZE);
    int pos = pos_1 - pos_2;
    if (pos > 0) vshl(&tmp, pos);
    while (pos > -1) {
        vshl(&vres, 1);
        if (vgeq(*vval_1, tmp)) {
            *vval_1 = sub(*vval_1, tmp);
            vres.u64[0] |= 1LL;
        }
        vshr(&tmp, 1);
        pos--;
    }
    if (equal(vval_1, &vval_2)) {
        vres.u64[0]++;
        vval_1->u64[0] = 0;
    }
    if (!vval_1->u64[0]) *ret = TRUE;
    return vres;
}

inline int vget_ten(v256 val)
{
    v256 tmp = val;
    int res = 0;
    uint64_t a = 0;
    while(!testz(&tmp)) {
        tmp = vdiv10(tmp, &a);
        res++;
    }
    return res;
}

inline int equal(v256* vval_1, v256* vval_2)
{
    int ret = FALSE;
    if (vval_1->u64[0] == vval_2->u64[0] &&
        vval_1->u64[1] == vval_2->u64[1] &&
        vval_1->u64[2] == vval_2->u64[2] &&
        vval_1->u64[3] == vval_2->u64[3])
        ret = TRUE;
    return ret;
}

inline int vget_pos(v256 val, uint32_t size)
{
    int pos = -1;
    v256 tval = val;
    for (size_t i = 0; i < size; i++) {
        if (tval.u64[0] & 0x1) pos = i;
        vshr(&tval, 1);
    }
    return pos;
}

inline v256 vdiv10(v256 val, uint64_t* a)
{
    v256 res = {0};
    v256 tmp = val;
    v256 val_10 = {0};
    val_10.u64[0] = 10;
    int pos = vget_pos(tmp, VMAX_SIZE);
    pos -= 3;
    if (pos > 0) vshl(&val_10, pos);
    while (pos > -1) {
        vshl(&res, 1);
        if (vgeq(tmp, val_10)) {
            tmp = sub(tmp, val_10);
            res.u64[0] |= 1LL;
        }
        vshr(&val_10, 1);
        pos--;
    }
    *a = tmp.u64[0];
    return res;
}

#endif  // DECIMAL_H
