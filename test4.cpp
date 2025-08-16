#include "eventuals/compose.h"    // 提供 compose 操作符，用于组合 Eventuals
#include "eventuals/generator.h"  // 提供 Generator 类型，用于生成序列
#include "eventuals/iterate.h"    // 提供 Iterate，用于遍历容器
#include "eventuals/map.h"        // 提供 Map 操作符，用于对每个元素做转换
#include "eventuals/promisify.h"  // 可将普通函数包装为 Eventual（示例未用）
#include "eventuals/reduce.h"     // 提供 Reduce 操作符，用于累积结果
#include "eventuals/then.h"       // 提供 Then，用于链式处理值
using namespace eventuals;
// 定义一个生成器函数，返回 Generator 对象，类型为 std::string
Generator::Of<std::string> SomeFunction() {
    return []() {
        // 使用 Iterate 遍历字符串列表
        return Iterate({"hello", " ", "world", "!"}) >>
               Map([](std::string&& s) {       // 对每个字符串应用 Map
                   s[0] = std::toupper(s[0]);  // 将首字母大写
                   return std::move(s);        // 返回转换后的字符串
               });
    };
}
int main(int argc, char** argv) {
    auto e = []() {
        // 使用 Generator 生成的序列，并通过 Reduce 累积成一个完整的字符串
        return SomeFunction() >> Reduce(
                                     /* 初始结果 = */ std::string(),  // Reduce 的初始值为空字符串
                                     [](auto& result) {
                                         // 对生成器每个元素执行 Then
                                         return Then([&](auto&& value) {
                                             result += value;  // 将元素拼接到结果中
                                             return true;      // 返回 true 表示继续累积
                                         });
                                     });
    };
    // 执行 Eventual 并获取结果
    CHECK_EQ("Hello World!", *e());
    return 0;
}