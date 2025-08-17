#include "eventuals/just.h"       // 引入 Just 组合器，用于将值注入异步管道
#include "eventuals/lock.h"       // 引入 Lock 和 Synchronizable，用于同步操作
#include "eventuals/promisify.h"  // 引入 Promisify，将 Eventual 转换为 std::future
#include <iostream>               // 标准输入输出库，用于 std::cout
using namespace eventuals;  // 使用 Eventuals 命名空间，简化代码
// 定义 Foo 类，继承自 Synchronizable 以支持同步操作
struct Foo : public Synchronizable {
    // 默认构造函数
    Foo() {}
    // 移动构造函数，确保 Synchronizable 的正确初始化
    Foo(Foo&& that) : Synchronizable() {}
    // 定义异步操作 Operation，返回 Eventual<std::string>
    auto Operation() {
        // Synchronized 确保操作在锁保护下执行，防止并发访问
        // Just("operation") 注入字符串 "operation" 到异步管道
        // Wait 等待某个条件，接受一个通知回调
        return Synchronized(Just("operation") >> Wait([](auto notify) {
                                // Wait 的回调返回一个条件函数，决定是否继续
                                // 这里始终返回 false，表示不继续等待，直接返回 Just 的值
                                return [](auto&&...) { return false; };
                            }));
    }
};
int main() {
    // 创建 Foo 对象
    Foo foo;
    // 使用移动构造函数将 foo 移动到 foo2
    Foo foo2 = std::move(foo);
    // 调用 foo2 的 Operation 方法，获取异步操作的 Eventual
    auto value = foo2.Operation();
    // 解引用 Eventual，阻塞直到获取结果，并打印
    std::cout << *value << '\n';
    return 0;  // 程序正常退出
}