#define _TOSTR(x) #x
#define TOSTR(x)  _TOSTR(x)

#define EMPTY()
#define DEFER(M)    M EMPTY()
#define EXPAND(...) __VA_ARGS__

// max depth: 4 ^ 5 = 1024
#define EVAL(...)  EVAL1(EVAL1(EVAL1(EVAL1(__VA_ARGS__))))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(EVAL2(__VA_ARGS__))))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(EVAL3(__VA_ARGS__))))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(EVAL4(__VA_ARGS__))))
#define EVAL4(...) EXPAND(EXPAND(EXPAND(EXPAND(__VA_ARGS__))))

#define PRIMITIVE_CAT(a, b) a##b
#define CAT(a, b)           PRIMITIVE_CAT(a, b)

#define SECOND(_1, _2, ...) _2
#define CHECK(...)          SECOND(__VA_ARGS__, 0, )

#define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0  _, 1,

#define BOOL(x) NOT(NOT(x))
