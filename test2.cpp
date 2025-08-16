#include "eventuals/compose.h"    // 提供 >> 操作符，用于组合 Eventuals
#include "eventuals/just.h"       // 提供 Just<T>，直接返回一个值的 Eventual
#include "eventuals/promisify.h"  // 将普通函数转为 Eventual（此示例未用）
#include "eventuals/raise.h"      // 提供 Raise<T>，触发异常的 Eventual
using namespace eventuals;
int main(int argc, char** argv) {
    // 定义一个 Eventual 链 e
    auto e = []() {
        return Just("hello")                     // 生成一个 Eventual，直接返回 "hello"
               >> Raise(RuntimeError("Oh no!"))  // 抛出异常 RuntimeError，Eventual 链中断
               >> Just("world");  // 如果没有异常，这里会返回 "world"，但本例不会执行
    };
    try {
        *e();          // 执行 Eventual 链，阻塞直到完成或者抛异常
        std::abort();  // 如果没有抛异常，程序终止（理论上不会执行到这里）
    } catch (const TypeErasedError& e) {  // 捕获 Eventuals 中抛出的异常
        CHECK_STREQ("Oh no!", e.what());  // 检查异常信息是否符合预期
    }
    return 0;
}
