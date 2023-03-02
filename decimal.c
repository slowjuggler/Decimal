#include "decimal.h"

int transformers(struct data* dptr, struct methods* mptr)
{
    int ret = SUCCESS;
    if (dptr->opcode == DEC_INT) {
        ret = from_decimal_to_int(dptr->val_1, &dptr->ival);
        sprintf(dptr->string, "%d", dptr->ival);
    } else if (dptr->opcode == DEC_FLT) {
        ret = from_decimal_to_float(dptr->val_1, &dptr->fval);
        sprintf(dptr->string, "%.6e", dptr->fval);
    } else if (dptr->opcode == INT_DEC) {
        dptr->ival = atoi(dptr->string);
        memset(&dptr->string, 0, MAX_POW + 2);
        ret = from_int_to_decimal(dptr->ival, &dptr->res);
        mptr->transform_to_str = dtoa;
        mptr->transform_to_str(dptr->string, dptr->res);
    } else if (dptr->opcode == FLT_DEC) {
        dptr->fval = atof(dptr->string);
        memset(&dptr->string, 0, MAX_POW + 2);
        ret = from_float_to_decimal(dptr->fval, &dptr->res);
        mptr->transform_to_str = dtoa;
        mptr->transform_to_str(dptr->string, dptr->res);
    }
    return ret;
}

void get_res(struct data* this, struct methods* ptr)
{
    switch (this->opcode) {
        case ADD : ptr->arithmetic = dec_add; break;
        case SUB : ptr->arithmetic = dec_sub; break;
        case MUL : ptr->arithmetic = dec_mul; break;
        case DIV : ptr->arithmetic = dec_div; break;
        case MOD : ptr->arithmetic = dec_mod; break;
        case EQL : ptr->compare = is_equal; break;
        case NEQ : ptr->compare = is_not_equal; break;
        case LES : ptr->compare = is_less; break;
        case LEQ : ptr->compare = is_less_or_equal; break;
        case GRT : ptr->compare = is_greater; break;
        case GEQ : ptr->compare = is_greater_or_equal; break;
        case FLR : ptr->flt = dec_floor; break;
        case RND : ptr->flt = dec_round; break;
        case TRN : ptr->flt = dec_truncate; break;
        case NGT : ptr->flt = dec_negate; break;
        case DEC_INT : case DEC_FLT : case INT_DEC :
        case FLT_DEC : ptr->transformers = transformers; break;
    }
    if (ptr->arithmetic) {
        this->ret = ptr->arithmetic(this->val_1, this->val_2, &this->res);
        ptr->transform_to_str = dtoa;
        ptr->transform_to_str(this->string, this->res);
    } else if (ptr->compare) {
        this->ret = ptr->compare(this->val_1, this->val_2);
        *this->string = this->ret | 0x30;
    } else if (ptr->flt) {
        this->ret = ptr->flt(this->val_1, &this->res);
        ptr->transform_to_str = dtoa;
        ptr->transform_to_str(this->string, this->res);
    } else if (ptr->transformers) {
        this->ret = ptr->transformers(this, ptr);
    }
}

void opcode_parser(char* str, struct data* this)
{
    char* ptr = str;
    for (;(*ptr < 0x30 || *ptr > 0x39 ) && *ptr != '\n'; ptr++) {
        if (*ptr == '+') {this->opcode = ADD; break;}
        else if (*ptr == '-') {this->opcode = SUB; break;}
        else if (*ptr == '*') {this->opcode = MUL; break;}
        else if (*ptr == '/') {this->opcode = DIV; break;}
        else if (*ptr == '%') {this->opcode = MOD; break;}
        else if (*ptr == 'R') {this->opcode = RND; break;}
        else if (*ptr == 'T') {this->opcode = TRN; break;}
        else if (*ptr == 'N') {this->opcode = NGT; break;}
        else if (*ptr == 'F' && *(ptr + 1) != 'D') {this->opcode = FLR; break;}
        else if (*ptr == '<' && *(ptr + 1) != '=') {this->opcode = LES; break;}
        else if (*ptr == '<' && *(ptr + 1) == '=') {this->opcode = LEQ; break;}
        else if (*ptr == '>' && *(ptr + 1) != '=') {this->opcode = GRT; break;}
        else if (*ptr == '>' && *(ptr + 1) == '=') {this->opcode = GEQ; break;}
        else if (*ptr == '=' && *(ptr + 1) == '=') {this->opcode = EQL; break;}
        else if (*ptr == '!' && *(ptr + 1) == '=') {this->opcode = NEQ; break;}
        else if (*ptr == 'D' && *(ptr + 1) == 'I') {this->opcode = DEC_INT; break;}
        else if (*ptr == 'D' && *(ptr + 1) == 'F') {this->opcode = DEC_FLT; break;}
        else if (*ptr == 'I' && *(ptr + 1) == 'D') {this->opcode = INT_DEC; break;}
        else if (*ptr == 'F' && *(ptr + 1) == 'D') {this->opcode = FLT_DEC; break;}
    }
}

void decimal_fill(decimal* ptr, char* str)
{
    v256 vval = {0};
    uint32_t pos = 0;
    uint32_t val_len = 0;
    int sign = (*str == '-') ? 1 : 0;
    uint32_t count = (sign) ? 1 : 0;
    while (*(str + count) >= 0x30 && *(str + count) <= 0x39 && *(str + count) != '\n') {
        vmult_10(&vval);
        v256 tmp = {0};
        tmp.u64[0] = *(str + count++) - 0x30;
        if (*(str + count) == '.') {
            pos = count;
            count++;
        }
        vval = add(vval, tmp);
        val_len++;
    }
    if (val_len > MAX_POW + 2) {
        puts("Too large val for decimal!");
        exit(1);
    }
    *ptr = vval.dec;
    if (sign) ptr->bits[3] = 0x80000000;
    if (pos) insert_pow(ptr, val_len - pos + sign);
}

void proc_fill(struct data* this)
{
    char s1[MAX_POW + 2] = {0};
    char s3[MAX_POW + 2] = {0};
    char string[MAX_POW * 3] = {0};
    char* str = string;
    printf("input equation: ");
    fgets(str, MAX_POW * 3, stdin);
    if (*str == 'q') exit (1);
    if (*str == '\n') return;
    for (size_t count = 0; *str != ' ' && *str != '\n';) {
        *(s1 + count++) = *str++;
    }
    opcode_parser(str, this);
    while ((*str < 0x30 || *str > 0x39) && *str != '\n') str++;
    if (*(str - 1) == '-') str--;
    for (size_t count = 0; *str != '\n';) {
        *(s3 + count++) = *str++;
    }
    if (this->opcode && this->opcode < DEC_INT) {
        decimal_fill(&this->val_1, s1);
        decimal_fill(&this->val_2, s3);
    } else if (this->opcode && this->opcode < INT_DEC) {
        decimal_fill(&this->val_1, s1);
    } else if (this->opcode && this->opcode < FLT_DEC + 1) {
        strcpy(this->string, s1);
    }
}

int dtoa(char* s, decimal inum)
{
    int ret = SUCCESS;
    int sign = (bit_read(inum.bits[3], 31)) ? 1 : 0;
    char ss1[MAX_POW + 2] = {0};
    char ss2[MAX_POW + 2] = {0};
    char* s1 = ss1;
    char* s2 = ss2;
    int count = 0;
    v256 vref = (v256)inum;
    int pow = vget_pow(vref);
    vref.u64[1] &= 0xFFFFFFFF;
    uint64_t a = 0;
    while (!testz(&vref)) {
        vref = vdiv10(vref, &a);
        if (a == 10) {
            vref.u64[0]++;
            a = 0;
        }
        *s1++ = a | 0x30;
        count++;
    }
    *s1 = 0;
    int len = count;
    while (count > -1) {
        *s2++ = *s1--;
        count--;
    }
    *s2 = 0;
    s2 = s2 - len;
    int j = 0;
    if (sign) *(s + j++) = '-';
    if (pow >= len) {
        *(s + j++) = '0';
        *(s + j++) = '.';
        while (pow > len) {
            *(s + j++) = '0';
            len++;
        }
        while(*s2) *(s + j++) = *s2++;
    } else if (pow) {
        int count = 1;
        while(*s2) {
            *(s + j++) = *s2++;
            if (count == len - pow) {
                *(s + j++) = '.';
            }
            count++;
        }
    } else {
        while(*s2) *(s + j++) = *s2++;
    }
    *(s + j) = 0;
    return ret;
}

int get_pow(decimal val)
{
    return (val.bits[3] >> 16) & 0x7F;
}

int vgeq(v256 val_1, v256 val_2)
{
    int pos_1 = vget_pos(val_1, VMAX_SIZE);
    int pos_2 = vget_pos(val_2, VMAX_SIZE);
    while (pos_1 == pos_2 && pos_1 && pos_2) {
        pos_1 = vget_pos(val_1, pos_1);
        pos_2 = vget_pos(val_2, pos_2);
        if (pos_1 == -1) return 0;
        if (pos_2 == -1 && pos_1 >= 0) return 1;
    }
    int res = ((pos_1 >= pos_2) || (!pos_1 && !pos_2)) ? 1 : 0;
    return res;
}

void vnorm96(v256* val, int* corr)
{
    v256 tmp = *val;
    int count = vget_pos(tmp, VMAX_SIZE);
    uint64_t a = 0;
    int c = 0;
    while (count > MAX_SIZE - 1) {
        tmp = vdiv10(tmp, &a);
        count = vget_pos(tmp, VMAX_SIZE);
        c++;
    }
    *corr = c;
    if (a > 4) tmp.u64[0]++;
    *val = tmp;
}

int dec_mod(decimal val_1, decimal val_2, decimal* res)
{
    int sign_1 = get_sign(val_1.bits[3]);
    int ret = SUCCESS;
    if (!is_zero(val_2) && !is_zero(val_2)) {
        v256 vval_1 = (v256)val_1;
        v256 vval_2 = (v256)val_2;
        int pow = norm(&vval_1, &vval_2);
        int div = 0;
        int a = 0;
        vdiv(&vval_1, vval_2, &div);
        vnorm96(&vval_1, &a);
        int curr_pow = pow - a;
        if (!testz(&vval_1)) {
            *res = vval_1.dec;
            if (sign_1) bit_set(res->bits[3], 31);
            insert_pow(res, curr_pow);
        }
    } else if (is_zero(val_2)) {
        ret = DIVNUL;
    } else if (is_zero(val_1)) {
        dec_bzero(res);
        ret = SUCCESS;
    }
    return ret;
}

void insert_pow(decimal* ptr, uint32_t pow)
{
    ptr->bits[3] |= pow << 16;
}

void vbzero(v256* val)
{
    val->u64[0] = 0;
    val->u64[1] = 0;
    val->u64[2] = 0;
    val->u64[3] = 0;
}

int norm(v256* vval_1, v256* vval_2)
{
    int pow_1 = vget_pow(*vval_1);
    int pow_2 = vget_pow(*vval_2);
    int d_pow = pow_1 - pow_2;
    vval_1->u64[1] &= 0xFFFFFFFF;
    vval_2->u64[1] &= 0xFFFFFFFF;
    while (d_pow > 0) {
        vmult_10(vval_2);
        d_pow--;
    }
    while (d_pow < 0) {
        vmult_10(vval_1);
        d_pow++;
        pow_1++;
    }
    return pow_1;
}

int dec_add(decimal val_1, decimal val_2, decimal* res)
{
    int ret = SUCCESS;
    int sign_1 = get_sign(val_1.bits[3]) ? 1 : 0;
    int sign_2 = get_sign(val_2.bits[3]) ? 1 : 0;
    if (sign_1 && !sign_2) {
        bit_zero(val_1.bits[3], 31);
        ret = dec_sub(val_2, val_1, res);
    } else if (!sign_1 && sign_2) {
        bit_zero(val_2.bits[3], 31);
        ret = dec_sub(val_1, val_2, res);
    } else {
        v256 vval_1 = (v256)val_1;
        v256 vval_2 = (v256)val_2;
        v256 vres = {0};
        int pow = norm(&vval_1, &vval_2);
        int corr = 0;
        vres = add(vval_1, vval_2);
        int pos = vget_pos(vres, VMAX_SIZE);
        if (pos > MAX_SIZE - 1) {
            uint64_t mag = 0;
            while (pow && pos > MAX_SIZE - 1) {
                vres = vdiv10(vres, &mag);
                pos = vget_pos(vres, VMAX_SIZE);
                pow--;
            }
            if (mag > 4) {
                v256 tmp = {0};
                tmp.u64[0] = 1;
                vres = add(vres, tmp);
                pos = vget_pos(vres, VMAX_SIZE);
            }
            if (!pow && pos > MAX_SIZE - 1) {
                vbzero(&vres);
                if (!sign_1) ret = BIGVAL;
                else ret = SMALVAL;
            }
        }
        *res = vres.dec;
        if (sign_1 && sign_2 && !ret)
            bit_set(res->bits[3], 31);
        if (!ret) insert_pow(res, pow - corr);
    }
    return ret;
}

int vl(v256 val_1, v256 val_2)
{
    int pos_1 = vget_pos(val_1, VMAX_SIZE);
    int pos_2 = vget_pos(val_2, VMAX_SIZE);
    while (pos_1 == pos_2 && pos_1 && pos_2) {
        pos_1 = vget_pos(val_1, pos_1);
        pos_2 = vget_pos(val_2, pos_2);
        if (pos_2 == -1) return 0;
        if (pos_1 == -1 && pos_2 >= 0) return 1;
    }
    return (pos_1 < pos_2) ? 1 : 0;
}

int swap(v256* vval_1, v256* vval_2) {
    int ret = FALSE;
    if (vl(*vval_1, *vval_2)) {
        v256 tmp = *vval_1;
        *vval_1 = *vval_2;
        *vval_2 = tmp;
        ret = TRUE;
    }
    return ret;
}

int dec_sub(decimal val_1, decimal val_2, decimal* res)
{
    int ret = SUCCESS;
    uint8_t sign_1 = get_sign(val_1.bits[3]) ? 1 : 0;
    uint8_t sign_2 = get_sign(val_2.bits[3]) ? 1 : 0;
    v256 vval_1 = (v256)val_1;
    v256 vval_2 = (v256)val_2;
    v256 vres = {0};
    int s = 0;
    int corr = 0;
    int pow = norm(&vval_1, &vval_2);
    if (is_zero (val_2)){
        *res = val_1;
        return ret;
    }
    if (!sign_1 && !sign_2) {
        s = swap(&vval_1, &vval_2);
        vres = sub(vval_1, vval_2);
        vnorm96(&vres, &corr);
        *res = vres.dec;
    } else if (sign_1 && sign_2) {
        s = swap(&vval_2, &vval_1);
        vres = sub(vval_2, vval_1);
        vnorm96(&vres, &corr);
        *res = vres.dec;
    } else {
        vres = add(vval_1, vval_2);
        int pos = vget_pos(vres, VMAX_SIZE);
        if (pos > MAX_SIZE - 1) {
            uint64_t mag = 0;
            while (pow && pos > MAX_SIZE - 1) {
                vres = vdiv10(vres, &mag);
                pos = vget_pos(vres, VMAX_SIZE);
                pow--;
            }
            if (mag > 4) {
                v256 tmp = {0};
                tmp.u64[0] = 1;
                vres = add(vres, tmp);
                pos = vget_pos(vres, VMAX_SIZE);
            }
            if (!pow && pos > MAX_SIZE - 1) {
                vbzero(&vres);
                if (!sign_1) ret = BIGVAL;
                else ret = SMALVAL;
            }
        }
        *res = vres.dec;
        if (sign_1 && !ret)
            bit_set(res->bits[3], 31);
    }
    if (s) bit_set(res->bits[3], 31);
    if (!ret) insert_pow(res, pow - corr);
    return ret;
}

int dec_mul(decimal val_1, decimal val_2, decimal* res)
{
    int sign_1 = get_sign(val_1.bits[3]);
    int sign_2 = get_sign(val_2.bits[3]);
    int res_sign = ((sign_1 || sign_2) && !(sign_1 && sign_2)) ? 1 : 0;
    int ret = SUCCESS;
    v256 vval_1 = (v256)val_1;
    v256 vval_2 = (v256)val_2;
    int pow_1 = vget_pow(vval_1);
    int pow_2 = vget_pow(vval_2);
    if (!is_zero(val_1) && !is_zero(val_2) && !is_one(val_1) && !is_one(val_2)) {
        vval_1.u64[1] &= 0xFFFFFFFF;
        vval_2.u64[1] &= 0xFFFFFFFF;
        v256 vres = {0};
        int ten_pow_1 = vget_ten(vval_1);
        int ten_pow_2 = vget_ten(vval_2);
        int d_pow = ten_pow_1 + ten_pow_2 - MAX_POW - 1;
        if (d_pow > pow_1 + pow_2 + 1){
            dec_bzero (res);
            if (res_sign) ret = SMALVAL;
            else ret = BIGVAL;
            return ret;
        }
        if ((pow_1 + pow_2 > MAX_POW) && (d_pow < 0)) {
            dec_bzero (res);
            insert_pow(res, MAX_POW);
            if (res_sign) bit_set(res->bits[3], 31);
            return ret;
        }
        int corr = 0;
        vres = lmul(vval_1, vval_2);
        vnorm96(&vres, &corr);
        int pow = pow_1 + pow_2 - corr;
        uint64_t a = 0;
        while (pow > MAX_POW + 1) {
            vres = vdiv10(vres, &a);
            pow--;
        }
        if (a > 4) vres.u64[0]++;
        *res = vres.dec;
        if (pow > 0) insert_pow(res, pow);
        else if (pow < 0) {
            dec_bzero(res);
            ret = BIGVAL;
        }
    } else if (is_zero(val_1)) {
        dec_bzero(res);
        if (pow_1) insert_pow(res, pow_1);
    } else if (is_zero(val_2)) {
        dec_bzero(res);
        if (pow_2) insert_pow(res, pow_1);
    } else if (is_one(val_1)) {
        *res = val_2;
        bit_zero(res->bits[3], 31);
    } else if (is_one(val_2)) {
        *res = val_1;
        bit_zero(res->bits[3], 31);
    }
    if (res_sign) bit_set (res->bits[3], 31);
    return ret;
}

int dec_div(decimal val_1, decimal val_2, decimal* res)
{
    int sign_1 = get_sign(val_1.bits[3]);
    int sign_2 = get_sign(val_2.bits[3]);
    int res_sign = ((sign_1 || sign_2) && !(sign_1 && sign_2)) ? 1 : 0;
    int ret = SUCCESS;
    if (!is_zero(val_2) && !is_zero(val_2) && !is_one(val_2)) {
        v256 vval_1 = (v256)val_1;
        v256 vval_2 = (v256)val_2;
        v256 vres = {0};
        int pow_1 = vget_pow(vval_1);
        int pow_2 = vget_pow(vval_2);
        vval_1.u64[1] &= 0xFFFFFFFF;
        vval_2.u64[1] &= 0xFFFFFFFF;
        int ten_pow_1 = vget_ten(vval_1);
        int ten_pow_2 = vget_ten(vval_2);
        int dten = ten_pow_1 - ten_pow_2;
        int curr_pow = 0;
        int div = 0;
        vres = vdiv(&vval_1, vval_2, &div);
        while (!div && curr_pow <= MAX_POW + 1 - dten) {
            vmult_10(&vres);
            vmult_10(&vval_1);
            curr_pow++;
            if (vgeq(vval_1, vval_2)) {
                v256 tmp = {0};
                tmp = vdiv(&vval_1, vval_2, &div);
                vres = add(vres, tmp);
            }
        }
        int pos = vget_pos(vres, VMAX_SIZE);
        uint64_t mag = 0;
        while (pos > MAX_SIZE - 1) {
            vres = vdiv10(vres, &mag);
            pos = vget_pos(vres, VMAX_SIZE);
            curr_pow--;
        }
        if (mag > 4) vres.u64[0]++;
        if (!div) {
            mag = 0;
            while (pow_1 > pow_2) {
                curr_pow++;
                pow_1--;
            }
            while (curr_pow > MAX_POW + 1) {
                vres = vdiv10(vres, &mag);
                curr_pow--;
            }
            if (mag > 4) vres.u64[0]++;
            while (pow_2 > pow_1) {
                curr_pow--;
                pow_2--;
            }
            if (curr_pow < 0) {
                vbzero(&vres);
                if (res_sign) ret = SMALVAL;
                else ret = BIGVAL;
            }
        } else {
            while (pow_1 > pow_2) {
                curr_pow++;
                pow_1--;
            }
            while (pow_2 > pow_1) {
                vmult_10(&vres);
                pow_2--;
            }
            int pos = vget_pos(vres, VMAX_SIZE);
            if (pos > MAX_SIZE - 1) {
                vbzero(&vres);
                if (res_sign) ret = SMALVAL;
                else ret = BIGVAL;
            }
        }
        *res = vres.dec;
        if (res_sign) bit_set(res->bits[3], 31);
        if (!ret) insert_pow(res, curr_pow);
    } else if (is_zero(val_2)) {
        ret = DIVNUL;
    } else if (is_zero(val_1)) {
        dec_bzero(res);
        ret = SUCCESS;
    } else if (is_one(val_2)) {
        *res = val_1;
        bit_zero(res->bits[3], 31);
        if (res_sign) bit_set(res->bits[3], 31);
    }
    return ret;
}

int is_less(decimal val_1, decimal val_2)
{
    int ret = FALSE;
    uint8_t sign_1 = get_sign(val_1.bits[3]) ? 1 : 0;
    uint8_t sign_2 = get_sign(val_2.bits[3]) ? 1 : 0;
    v256 vval_1 = (v256)val_1;
    v256 vval_2 = (v256)val_2;
    norm(&vval_1, &vval_2);
    if (testz(&vval_1) && testz(&vval_2))
        ret = FALSE;
    else if (sign_1 && !sign_2)
        ret = TRUE;
    else if ((sign_1 && sign_2) && vl(vval_2, vval_1))
        ret = TRUE;
    else if ((!sign_1 && !sign_2) && vl(vval_1, vval_2))
        ret = TRUE;
    return ret;
}

int is_equal(decimal val_1, decimal val_2)
{
    int ret = FALSE;
    uint8_t sign_1 = get_sign(val_1.bits[3]) ? 1 : 0;
    uint8_t sign_2 = get_sign(val_2.bits[3]) ? 1 : 0;
    if ((sign_1 != sign_2) && !is_zero(val_1) && !is_zero(val_2)) return ret;
    v256 vval_1 = (v256)val_1;
    v256 vval_2 = (v256)val_2;
    norm(&vval_1, &vval_2);
    if ((vval_1.u64[0] == vval_2.u64[0]) &&
        (vval_1.u64[1] == vval_2.u64[1]) &&
        (vval_1.u64[2] == vval_2.u64[2]) &&
        (vval_1.u64[3] == vval_2.u64[3]))
        ret = TRUE;
    if (testz(&vval_1) && testz(&vval_2)) ret = TRUE;
    return ret;
}

int is_less_or_equal(decimal val_1, decimal val_2)
{
    int ret = FALSE;
    if (is_less(val_1, val_2) || is_equal(val_1, val_2))
        ret = TRUE;
    return ret;
}

int is_not_equal(decimal val_1, decimal val_2)
{
    int ret = FALSE;
    if (!is_equal(val_1, val_2))
        ret = TRUE;
    return ret;
}

int is_greater(decimal val_1, decimal val_2)
{
    int ret = FALSE;
    uint8_t sign_1 = get_sign(val_1.bits[3]) ? 1 : 0;
    uint8_t sign_2 = get_sign(val_2.bits[3]) ? 1 : 0;
    bit_zero(val_1.bits[3], 31);
    bit_zero(val_2.bits[3], 31);
    v256 vval_1 = (v256)val_1;
    v256 vval_2 = (v256)val_2;
    norm(&vval_1, &vval_2);
    if (testz(&vval_1) && testz(&vval_2))
        ret = FALSE;
    else if (!sign_1 && sign_2)
        ret = TRUE;
    else if ((!sign_1 && !sign_2) && vl(vval_2, vval_1))
        ret = TRUE;
    else if ((sign_1 && sign_2) && vl(vval_1, vval_2))
        ret = TRUE;
    return ret;
}

int is_greater_or_equal(decimal val_1, decimal val_2)
{
    int ret = FALSE;
    if (is_greater(val_1, val_2) || is_equal(val_1, val_2))
        ret = TRUE;
    return ret;
}

int dec_truncate(decimal val, decimal* res)
{
    int ret = SUCCESS;
    uint8_t sign = get_sign(val.bits[3]) ? 1 : 0;
    bit_zero(val.bits[3], 31);
    v256 vval = (v256)val;
    uint32_t pow = vget_pow(vval);
    vval.u64[1] &= 0xFFFFFFFF;
    uint64_t a = 0;
    while (pow > 0) {
        vval = vdiv10(vval, &a);
        if (a == 10) vval.u64[0]++;
        pow--;
    }
    *res = vval.dec;
    if (sign) bit_set(res->bits[3], 31);
    return ret;
}

int dec_round(decimal val, decimal* res)
{
    int ret = SUCCESS;
    uint8_t sign = get_sign(val.bits[3]) ? 1 : 0;
    bit_zero(val.bits[3], 31);
    v256 vval = (v256)val;
    uint32_t pow = vget_pow(vval);
    vval.u64[1] &= 0xFFFFFFFF;
    uint64_t a = 0;
    while (pow > 0) {
        vval = vdiv10(vval, &a);
        if (a == 10) vval.u64[0]++;
        pow--;
    }
    if (a > 4 && a != 10) vval.u64[0]++;
    *res = vval.dec;
    if (sign) bit_set(res->bits[3], 31);
    return ret;
}

int dec_floor(decimal val, decimal* res)
{
    int ret = SUCCESS;
    uint8_t sign = get_sign(val.bits[3]) ? 1 : 0;
    bit_zero(val.bits[3], 31);
    v256 vval = (v256)val;
    uint32_t pow = vget_pow(vval);
    vval.u64[1] &= 0xFFFFFFFF;
    uint64_t a = 0;
    uint8_t f = 0;
    while (pow > 0) {
        vval = vdiv10(vval, &a);
        if (a) f = 1;
        if (a == 10) vval.u64[0]++;
        pow--;
    }
    if (sign && f) vval.u64[0]++;
    *res = vval.dec;
    if (sign) bit_set(res->bits[3], 31);
    return ret;
}

int dec_negate(decimal val, decimal* res)
{
    int ret = SUCCESS;
    if (bit_read(val.bits[3], 31))
        bit_zero(val.bits[3], 31);
    else bit_set(val.bits[3], 31);
    *res = val;
    return ret;
}

int from_int_to_decimal(int src, decimal* dst)
{
    int ret = SUCCESS;
    dec_bzero(dst);
    int sign = (bit_read(src, 31)) ? 1 : 0;
    if (sign) src *= -1;
    dst->bits[0] = src;
    if (sign)
         bit_set(dst->bits[3], 31);
    return ret;
}

int from_decimal_to_int(decimal src, int* dst)
{
    int ret = SUCCESS;
    int sign = (bit_read(src.bits[3], 31)) ? 1 : 0;
    bit_zero(src.bits[3], 31);
    v256 vval = (v256)src;
    uint32_t pow = vget_pow(vval);
    if (pow > MAX_POW + 1) {
        *dst = 0;
        return ERROR;
    }
    vval.u64[1] &= 0xFFFFFFFF;
    uint64_t a = 0;
    while (pow > 0) {
        vval = vdiv10(vval, &a);
        pow--;
    }
    if (a == 10) vval.u64[0]++;
    int pos = vget_pos(vval, VMAX_SIZE);
    if (pos > 31) {
        vbzero(&vval);
        ret = ERROR;
    }
    *dst = (int)vval.u64[0];
    if (sign) *dst *= -1;
    return ret;
}

int from_float_to_decimal(float src, decimal* dst)
{
    int ret = SUCCESS;
    dec_bzero(dst);
    int sign = (src < 0) ? 1 : 0;
    int count = (sign) ? 1 : 0;
    if (src == 0.0) return ret;
    char s[MAX_POW] = {0};
    sprintf(s, "%.6e", src);
    if (!strcmp(s, "inf") || !strcmp(s, "-inf") || !strcmp(s, "nan")) {
        return ERROR;
    }
    int tmp = 0;
    int q = 0;
    for (; *(s + count) != 'e'; count++) {
        if (*(s + count) == '.') {
            continue;
        }
        tmp *= 10;
        q = *(s + count) - 0x30;
        tmp = tmp + q;
    }
    int cor = 2;
    if (sign) cor++;
    int len = count - cor;
    char sign_exp = *(s + ++count);
    char c1 = *(s + ++count);
    char c2 = *(s + ++count);
    int exp = (c1 - 0x30) * 10 + (c2 - 0x30);
    if (sign_exp == '-') exp *= -1;
    if (tmp % 10 == 9) tmp += 1;
    while (tmp % 10 == 0) {
        tmp /= 10;
        len--;
    }
    int pow = len - exp;
    if (pow < 0) {
        v256 v = {0};
        v.u64[0] = tmp;
        while (pow < 0) {
            vmult_10(&v);
            pow++;
        }
        *dst = v.dec;
    } else {
        dst->bits[0] = tmp;
    }
    if (sign) bit_set(dst->bits[3], 31);
    insert_pow(dst, pow);
    return ret;
}

int from_decimal_to_float(decimal src, float* dst)
{
    *dst = 0.0;
    int ret = SUCCESS;
    int sign = (bit_read(src.bits[3], 31)) ? 1 : 0;
    char str1[MAX_POW + 2] = {0};
    char str2[MAX_POW + 2] = {0};
    char* s1 = str1;
    char* s2 = str2;
    int count = 0;
    v256 vref = (v256)src;
    int pow = vget_pow(vref);
    vref.u64[1] &= 0xFFFFFFFF;
    uint64_t a = 0;
    while (!testz(&vref)) {
        vref = vdiv10(vref, &a);
        if (a == 10) {
            vref.u64[0]++;
            a = 0;
        }
        *s1++ = a | 0x30;
        count++;
    }
    *s1 = 0;
    int len = count;
    while (count > -1) {
        *s2++ = *s1--;
        count--;
    }
    *s2 = 0;
    s2 = s2 - len;
    char s[MAX_POW + 2] = {0};
    int j = 0;
    if (sign) *(s + j++) = '-';
    if (pow >= len) {
        *(s + j++) = '0';
        *(s + j++) = '.';
        while (pow > len) {
            *(s + j++) = '0';
            len++;
        }
        while(*s2)
            *(s + j++) = *s2++;
    } else if (pow) {
        int count = 1;
        while(*s2) {
            *(s + j++) = *s2++;
            if (count == len - pow)
                *(s + j++) = '.';
            count++;
        }
    } else {
        while(*s2)
            *(s + j++) = *s2++;
    }
    double q = atof(s);
    *dst = (float)q;
    return ret;
}

int main () {
    while (1) {
        struct methods methods = {0};
        struct data data = {0};
        proc_fill(&data);
        get_res(&data, &methods);
        printf("%u %s\n", data.ret, data.string);
    }
    return 0;
}
