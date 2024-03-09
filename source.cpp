#ifdef __clang__
#define KERNEL __attribute__((annotate("kernel")))
#elif defined(_MSC_VER)
#define KERNEL __declspec(kernel)
#else
#define KERNEL
#endif
class tmp {
   public:
    int a;
    int b;
    int c;
    int d;
    int& e = c;
};

int a;
int main() {
    int a = 0, b = 0;
    auto test2 = [&](int& c) {
        c = a + c;
        c = a;
        a = c;
        return a;
    };
    tmp t;
    t.d = 100;
    auto test1 = [&](int& a, tmp& t) KERNEL {
        a = 2 + t.d;
        t.c = a;
        test2(a);
    };
}