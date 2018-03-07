// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * EvalSecret.h
 *
 */

#ifndef GC_SECRET_H_
#define GC_SECRET_H_

#include "BMR/Register.h"
#include "BMR/CommonParty.h"
#include "BMR/AndJob.h"

#include "GC/Clear.h"
#include "GC/Memory.h"
#include "GC/Access.h"

#include "Math/Share.h"

namespace GC
{

class AuthValue
{
public:
    static string type_string() { return "authenticated value"; }
    word share;
    int128 mac;
    AuthValue() : share(0), mac(0) {}
    void assign(const word& value, const int128& mac_key, bool first_player);
    void check(const word& mac_key) const;
    friend ostream& operator<<(ostream& o, const AuthValue& auth_value);
};

class Mask
{
public:
    word share;
    int128 mac;
    Mask() : share(0) {}
};

class SpdzShare : public Share<gf2n>
{
public:
    void assign(const gf2n& value, const gf2n& mac_key, bool first_player)
    { Share<gf2n>::assign(value, first_player ? 0 : 1, mac_key); }
};

template <class T>
class Secret
{
    CheckVector<Register> registers;

    T& get_new_reg();

public:
#ifdef SPDZ_AUTH
    typedef SpdzShare DynamicType;
#else
    typedef AuthValue DynamicType;
#endif

    static string type_string() { return "evaluation secret"; }
    static string phase_name() { return T::name(); }

    static int default_length;

    static typename T::out_type out;

    static T& cast(Register& reg) { return *reinterpret_cast<T*>(&reg); }
    static const T& cast(const Register& reg) { return *reinterpret_cast<const T*>(&reg); }

    static Secret<T> input(party_id_t from, const int128& input, int n_bits = -1);
    void random(int n_bits, int128 share);
    void random_bit();
    static Secret<T> reconstruct(const int128& x, int length);
    template <class U>
    static void store_clear_in_dynamic(U& mem, const vector<ClearWriteAccess>& accesses)
    { T::store_clear_in_dynamic(mem, accesses); }
    void store(Memory<AuthValue>& mem, size_t address);
    static Secret<T> carryless_mult(const Secret<T>& x, const Secret<T>& y);
    static void output(Register& reg);

    static void load(vector< ReadAccess< Secret<T> > >& accesses, const Memory<SpdzShare>& mem);
    static void store(Memory<SpdzShare>& mem, vector< WriteAccess< Secret<T> > >& accesses);

    static void andrs(Processor< Secret<T> >& processor, const vector<int>& args)
    { T::andrs(processor, args); }

    Secret();
    Secret(const Integer& x) { *this = x; }

    void load(int n, const Integer& x);
    void operator=(const Integer& x) { load(default_length, x); }
    void load(int n, const Memory<AuthValue>& mem, size_t address);

    Secret<T> operator<<(int i);
    Secret<T> operator>>(int i);

    void bitcom(Memory< Secret<T> >& S, const vector<int>& regs);
    void bitdec(Memory< Secret<T> >& S, const vector<int>& regs) const;

    Secret<T> operator+(const Secret<T> x) const;
    Secret<T>& operator+=(const Secret<T> x) { *this = *this + x; return *this; }

    void xor_(int n, const Secret<T>& x, const Secret<T>& y);
    void andrs(int n, const Secret<T>& x, const Secret<T>& y);

    template <class U>
    void reveal(U& x);

    int size() const { return registers.size(); }
    CheckVector<Register>& get_regs() { return registers; }
    const CheckVector<Register>& get_regs() const { return registers; }

    const T& get_reg(int i) const;
    T& get_reg(int i);
    void resize_regs(int n) { registers.resize(n, T::new_reg()); }
};

template <class T>
inline ostream& operator<<(ostream& o, Secret<T>& secret)
{
	o << "(" << secret.size() << " secret bits)";
	return o;
}

} /* namespace GC */

#endif /* GC_SECRET_H_ */
