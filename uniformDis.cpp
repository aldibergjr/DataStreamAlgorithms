#include <iostream>
#include <random>

int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999);

    for (int n=0; n<1000; ++n)
        std::cout << dis(gen) << ' ';
    std::cout << '\n';
}