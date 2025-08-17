#include "eventuals/just.h"       // 引入 Just 组合器，用于将值注入异步管道
#include "eventuals/lock.h"       // 引入 Lock 和 Synchronizable，用于同步操作
#include "eventuals/promisify.h"  // 引入 Promisify，将 Eventual 转换为 std::future
#include <iostream>               // 标准输入输出库，用于 std::cout
using namespace eventuals;  // 使用 Eventuals 命名空间，简化代码
int main() {
    // 定义一个 Foo 类型，继承自 Synchronizable（表示这个对象支持同步原语）
    struct Foo : public Synchronizable {
        auto Operation() {
            // Synchronized() 会把内部的操作变成“同步操作”，避免多线程竞争
            return Synchronized(
                       // Then 表示在前一步完成后执行下一步逻辑
                       Then([]() {
                           // 返回 Just(42)，表示立即产生一个值 42
                           return Just(42);
                       })) >>
                   // 再加一个 Then，把上一步的值 i 原封不动返回
                   Then([](auto i) { return i; });
        }
    };
    Foo foo;
    // foo.Operation() 构建了一个管道，返回类型是一个 "Deferred pipeline"
    // 这里 `*foo.Operation()` 会触发隐式的同步执行并解包结果
    // 所以得到的就是 int 值 42
    std::cout << *foo.Operation() << '\n';
}
