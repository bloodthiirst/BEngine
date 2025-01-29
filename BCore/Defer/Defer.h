template<typename T>
struct Defer
{
    T fnc_ptr;
    Defer(T fnc_ptr) : fnc_ptr(fnc_ptr) {}

    ~Defer()
    {
        fnc_ptr();
    }
};

#define DEFER(x) \
auto def = Defer(x)\
