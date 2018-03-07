// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Secret_inline.h
 *
 */

#ifndef GC_SECRET_INLINE_H_
#define GC_SECRET_INLINE_H_

#ifdef MAX_INLINE
#define MAYBE_INLINE inline
#else
#define MAYBE_INLINE
#endif

#include "BMR/Register_inline.h"

namespace GC {

template <class T>
inline void XOR(Register& res, const Register& left, const Register& right)
{
#ifdef FREE_XOR
    Secret<T>::cast(res).XOR(Secret<T>::cast(left), Secret<T>::cast(right));
#else
    Secret<T>::cast(res).op(Secret<T>::cast(left), Secret<T>::cast(right), 0x0110);
#endif
}

template <class T>
inline Register XOR(const Register& left, const Register& right)
{
	Register res(T::new_reg());
	XOR<T>(res, left, right);
	return res;
}

template <class T>
inline void AND(Register& res, const Register& left, const Register& right)
{
#ifdef KEY_SIGNAL
#ifdef DEBUG_REGS
    cout << "*" << res.get_id() << " = AND(*" << left.get_id() << ",*" << right.get_id() << ")" << endl;
#endif
#else
#endif
    Secret<T>::cast(res).op(Secret<T>::cast(left), Secret<T>::cast(right), 0x0001);
}

template <class T>
inline Register AND(const Register& left, const Register& right)
{
    Register res = T::new_reg();
	AND<T>(res, left, right);
	return res;
}

template<class T>
inline Secret<T> GC::Secret<T>::operator+(const Secret<T> x) const
{
    Secret<T> res;
    res.xor_(max(registers.size(), x.registers.size()), *this, x);
    return res;
}

template <class T>
MAYBE_INLINE void Secret<T>::xor_(int n, const Secret<T>& x, const Secret<T>& y)
{
    int min_n = min((size_t)n, min(x.registers.size(), y.registers.size()));
    resize_regs(min_n);
    for (int i = 0; i < min_n; i++)
    {
        XOR<T>(registers[i], x.get_reg(i), y.get_reg(i));
    }

    if (min_n < n)
    {
        const vector<Register>* more_regs;
        if (y.registers.size() < x.registers.size())
            more_regs = &x.registers;
        else
            more_regs = &y.registers;
        registers.insert(registers.end(), more_regs->begin() + min_n,
                more_regs->begin() + min((size_t)n, more_regs->size()));
    }
}

template <class T>
MAYBE_INLINE void Secret<T>::andrs(int n, const Secret<T>& x, const Secret<T>& y)
{
    resize_regs(n);
    for (int i = 0; i < n; i++)
        AND<T>(registers[i], x.get_reg(i), y.get_reg(0));
}

} /* namespace GC */

#endif /* GC_SECRET_INLINE_H_ */
