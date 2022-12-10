#include <cmath>
#include <cstdint>

namespace clanguml {
namespace t20020 {
struct A {
    int a1() { return 0; }
    int a2() { return 1; }
    int a3() { return 2; }
};

struct B {
    void log() { }

    int b1() { return 3; }
    int b2() { return 4; }
};

int tmain()
{
    A a;
    B b;

    int result{0};

    if (reinterpret_cast<uint64_t>(&a) % 100 == 0ULL) {
        result = a.a1();
    }
    else if (reinterpret_cast<uint64_t>(&a) % 64 == 0ULL) {
        if (a.a2() > 2)
            result = b.b1();
        else
            result = b.b2();
    }
    else {
        result = a.a3();
    }

    b.log();

    // This if/else should not be included in the diagram at all
    // as the calls to std will be excluded by the diagram filters
    if (result != 2) {
        result = std::exp(result);
    }
    else if(result == 3) {
        result = 4;
    }
    else {
        result = std::exp(result + 1);
    }

    return result;
}
}
}