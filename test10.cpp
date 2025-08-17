#include "eventuals/expected.h"  // 引入 expected 类型支持，用于表示可能成功或失败的值
#include <string>                // 使用 std::string
#include "eventuals/promisify.h"  // 用于将 Eventual 转换为 std::future（本例未使用 future）
#include "eventuals/then.h"  // 引入 Then 组合器，用于在 Eventual pipeline 中处理结果
using namespace eventuals;  // 使用 Eventuals 命名空间
int main() {
    // 定义一个简单函数 f，返回 expected<int>，初始值为 40
    auto f = []() { return expected<int>(40); };
    // 定义一个 Eventual pipeline 的生成函数 e
    auto e = [&]() {
        return f() >>
               // 第一步 Then：将 f() 返回的整数加 1，并用 tl::expected<int, std::string> 封装
               Then([](int i) -> expected<int> { return tl::expected<int, std::string>(i + 1); }) >>
               // 第二步 Then：将整数再次封装为 expected<int>，并注入 Eventual 管道
               Then([](int i) { return Just(expected<int>(i)); }) >>
               // 第三步 Then：对 expected<int> 做检查和处理
               Then([](expected<int> e) {
                   CHECK(e.has_value());  // 确保 expected 中有值
                   // 对值进行加 1 操作，并生成新的 expected
                   e = tl::expected<int, std::string>(e.value() + 1);
                   return e;  // 返回新的 expected
               });
    };
    // 使用 '*' 阻塞式获取 Eventual pipeline 的最终结果，并打印
    // 注意：这种写法仅适用于测试或简单场景
    std::cout << *e() << std::endl;
}
