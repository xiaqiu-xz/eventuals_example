#include "eventuals/expected.h"   // 引入 expected 类型支持，用于表示可能失败的值
#include <string>                 // 使用 std::string
#include "eventuals/promisify.h"  // 将 Eventual 转换为 std::future
#include "eventuals/then.h"       // Then 组合器，用于处理异步结果
using namespace eventuals;        // 使用 Eventuals 命名空间
int main() {
    // 定义一个简单的函数 f，返回 expected<int>，值为 40
    auto f = []() { return expected<int>(40); };
    // 定义一个 Eventual pipeline 的生成函数 e
    auto e = [&]() {
        return f() >>
               // 第一个 Then：将 expected<int> 中的值加 1，并返回 tl::expected<int, std::string>
               Then([](int i) -> expected<int> { return tl::expected<int, std::string>(i + 1); }) >>
               // 第二个 Then：将整数再次封装为 expected<int>
               Then([](int i) { return Just(expected<int>(i)); }) >>
               // 第三个 Then：检查 expected 是否有值，并再次加 1
               Then([](expected<int> e) {
                   CHECK(e.has_value());                               // 确认 e 中有值
                   e = tl::expected<int, std::string>(e.value() + 1);  // 值加 1
                   return e;                                           // 返回新的 expected
               });
    };
    // 使用 * 阻塞式获取 Eventual 的最终结果，并打印
    std::cout << *e() << std::endl;
}
