/* Low-level utility routines.
 *
 * The routines are:
 *   mpz_set_PyInt(mpz_t, PyObject)         Python 2.X only.
 *   mpz_set_PyLong(mpz_t, PyObject)
 *   mpf_normalize(mpf_t)
 *   Pympz_FROM_MPZ(mpz_t)
 *   Pympq_FROM_MPQ(mpq_t)
 *   Pympf_FROM_MPF(mpf_t)
 */

#ifdef FALSE
#if PY_MAJOR_VERSION == 2
static void
mpz_set_PyInt(mpz_t rop, PyObject *obj)
{
    assert(PyInt_CheckExact(obj));
    mpz_set_si(rop, PyInt_AsLong(obj));
    return;
}
#endif
#endif

/*
 * to convert-from-long, we have a dependence on longs' internals:
 * we concentrate this dependence _right here_.
 */

static void
mpz_set_PyLong(mpz_t rop, PyObject * obj)
{
    int negative;
    long len;
    PyLongObject *lptr = (PyLongObject *) obj;

    assert(PyLong_CheckExact(obj));

    mpz_set_si(rop, 0);
#if PY_MAJOR_VERSION >= 3
    if(lptr->ob_base.ob_size < 0) {
        len = - lptr->ob_base.ob_size;
        negative = 1;
    } else {
        len = lptr->ob_base.ob_size;
        negative = 0;
    }
#else
    if(lptr->ob_size < 0) {
        len = - lptr->ob_size;
        negative = 1;
    } else {
        len = lptr->ob_size;
        negative = 0;
    }
#endif
    mpz_import(rop, len, -1, sizeof(lptr->ob_digit[0]), 0,
                sizeof(lptr->ob_digit[0])*8 - Py2or3Long_SHIFT, lptr->ob_digit);
    if(negative)
        mpz_neg(rop, rop);
}
/*
 * Normalize the internal representation of an mpf. GMP allocates 1
 * or more additional limbs to store the mantissa of an mpf. The
 * additional limbs may or may not be used but when used, they can
 * confuse comparisions. We will normalize all mpf such that the additional
 * limbs, if used, are set to 0.
 */

static void
mpf_normalize(mpf_t op)
{
    long size, prec, toclear, temp;
    int i;
    mp_limb_t bit1, rem, carry;

    prec = mpf_get_prec(op);
    size = mpf_size(op);
    toclear = size - ((prec / GMP_NUMB_BITS) + 1);
    if(toclear>0) {
        bit1 = (op->_mp_d[toclear-1] & ((mp_limb_t)1 << (GMP_NUMB_BITS - 1))) ? 1 : 0;
        rem = (op->_mp_d[toclear-1] & (((mp_limb_t)1 << (GMP_NUMB_BITS - 1)) - 1)) ? 1 : 0;
        carry = bit1 && ((op->_mp_d[toclear] & 1) || rem);
    } else {
        carry = 0;
    }
    if(options.debug) {
        fprintf(stderr, "prec %ld size %ld toclear %ld carry %ld\n",
               prec, size, toclear, carry);
        for(i=0; i<size; i++)
            fprintf(stderr,"[%d]=%lx\n", i, op->_mp_d[i]);
    }
    temp = toclear;
    if(temp>0) {
        op->_mp_d[--temp] = 0;
    }
    if(carry) {
        if(options.debug) {
            fprintf(stderr, "adding carry bit\n");
        }
        carry = mpn_add_1(op->_mp_d + toclear, op->_mp_d + toclear, size-toclear, carry);
        if(carry) {
            if(options.debug) {
                fprintf(stderr, "carry bit extended\n");
            }
            op->_mp_d[size-1] = 1;
            op->_mp_exp++;
        }
    }
    if(options.debug) {
        for(i=0; i<size; i++)
            fprintf(stderr,"[%d]=%lx\n", i, op->_mp_d[i]);
    }
}

static PympzObject *
Pympz_FROM_MPZ(mpz_t z)
{
    PympzObject *self;

    if(!(self = PyObject_New(PympzObject, &Pympz_Type)))
        return NULL;
    self->z[0] = z[0];
    return self;
}

#ifdef FALSE
static PympqObject *
Pympq_FROM_MPQ(mpq_t q)
{
    PympqObject *self;

    if(!(self = PyObject_New(PympqObject, &Pympq_Type)))
        return NULL;
    self->q[0] = q[0];
    return self;
}

static PympfObject *
Pympf_FROM_MPF(mpf_t f, unsigned int bits)
{
    PympfObject *self;

    if(!(self = PyObject_New(PympfObject, &Pympf_Type)))
        return NULL;
    if(bits < options.minprec)
        bits = options.minprec;
    self->f[0] = f[0];
    self->rebits = bits;
    return self;
}
#endif
/* End of low-level utility routines. */
