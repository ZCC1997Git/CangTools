#include <DDG.hpp>
#include <LoopComputing.hpp>
#include <array>
#include <iostream>
#define MakeNode(...) MakeNode(#__VA_ARGS__, __VA_ARGS__)
using subArray = std::array<int, 512>;
using Array = std::array<subArray, 512>;

int main() {
    KFTool::DDG ddg;
    Array arr1 = {0};
    Array arr2 = {0};

    std::array<size_t, 2> start_l = {0, 0};
    std::array<size_t, 2> end_l = {512, 512};
    int k = 1;
    KFTool::KernelNode<2> kn0(3, 2);
    kn0.MakeNode(
        [](auto i, auto j, int k, Array& arr1, Array& arr2) {
            arr1[i][j] = k;
            arr2[i][j] = k;
        },
        start_l, end_l, k, arr1, arr2);
    ddg.AddKernel(kn0);

    KFTool::KernelNode<2> kn1(1, 1);
    kn1.MakeNode([](auto i, auto j, Array& arr) { arr[i][j]++; }, start_l,
                 end_l, arr1);
    ddg.AddKernel(kn1);

    int sum = 0;
    KFTool::KernelNode<2> kn2(3, 1);
    kn2.MakeNode([](auto i, auto j, Array& arr1, Array& arr2,
                    int& sum) { sum += (arr1[i][j] + arr2[i][j]); },
                 start_l, end_l, arr1, arr2, sum);
    ddg.AddKernel(kn2);

    int sum1 = 0;
    KFTool::KernelNode<2> kn3(2, 1);
    kn3.MakeNode(
        [](auto i, auto j, Array& arr, int& sum) { sum += (arr[i][j]); },
        start_l, end_l, arr1, sum1);
    ddg.AddKernel(kn3);

    int sum2 = 0;
    KFTool::KernelNode<2> kn4(2, 1);
    kn4.MakeNode(
        [](auto i, auto j, Array& arr, int& sum) { sum += (arr[i][j]); },
        start_l, end_l, arr2, sum2);
    ddg.AddKernel(kn4);

    ddg.OutputGraph("pre_fustion");
    ddg.FusionKernel(&kn0, &kn1);
    ddg.OutputGraph("aft_fustion");
    ddg.Execution();
    // ddg.OutputGraph();
    std::cout << sum << '\t' << sum1 << '\t' << sum2 << std::endl;
    std::cout << (sum == sum1 + sum2) << std::endl;
    std::cout << (sum == 512 * 512 * 3) << std::endl;
    return 0;
}