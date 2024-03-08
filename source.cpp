
int main() {
    int a = 0, b = 0;
    auto test2 = [&](int& c) {
        c = a + c;
        c = a;
        a = c;
        return a;
    };

    auto test1 = [](int& a) { a = 2; };
}