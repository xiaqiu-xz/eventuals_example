#include "eventuals/expected.h"   // 提供 expected<T> 类型，用于包装返回值或错误
#include "eventuals/promisify.h"  // 将普通函数封装为 Eventual（可选，示例未用）
#include "eventuals/then.h"       // 提供 Then 操作符，用于链式处理结果
using namespace eventuals;
// 定义一个返回 expected<int> 的函数 SomeFunction
expected<int> SomeFunction(int i) {
    if (i > 100) {
        // 如果 i 大于 100，返回一个错误（unexpected）
        return make_unexpected("> 100");
    } else {
        // 否则返回正常值 i（expected）
        return i;  // 也可以写作 return expected(i);
    }
}
int main(int argc, char** argv) {
    // 使用链式调用将 SomeFunction(42) 的结果转换为字符串
    // Then 会接收 expected 的值，如果是正常值就调用 lambda
    // 最终返回 "42"
    CHECK_EQ("42", *(SomeFunction(42) >> Then([](int i) {
                         return std::to_string(i);  // 将整数转换为字符串
                     })));
    return 0;
}