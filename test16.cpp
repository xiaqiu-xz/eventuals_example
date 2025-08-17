#include "eventuals/lock.h"       // 引入 Lock 和 Synchronizable，用于同步操作
#include "eventuals/promisify.h"  // 引入 Promisify，将 Eventual 转换为 std::future
#include <iostream>               // 标准输入输出库，用于 std::cout
using namespace eventuals;        // 使用 Eventuals 命名空间，简化代码
// ⚠️ 这里的例子演示了 ConditionVariable 的一个潜在 bug：
// 如果 Wait 在条件已经满足的情况下依然把 eventual 入队，
// 那么后续 Notify 时可能访问到已经被释放的对象，从而导致 UAF（use-after-free）。
int main() {
    struct Foo : public Synchronizable {
        Foo() : condition_variable_(&lock()) {
            std::cout << "Foo 构造完成，ConditionVariable 绑定 Lock\n";
        }
        // NotifyAll 用于唤醒所有等待的任务
        auto NotifyAll() {
            return Synchronized(Then([this]() mutable {
                std::cout << "[NotifyAll] 执行 NotifyAll()\n";
                condition_variable_.NotifyAll();
            }));
        }
        // Wait 用于等待条件满足
        auto Wait() {
            return Synchronized(condition_variable_.Wait([]() {
                // 这里的谓词永远返回 false，表示条件已经满足，无需等待
                std::cout << "[Wait] 条件已满足，直接返回\n";
                return false;
            }));
        }
        ConditionVariable condition_variable_;  // 条件变量，用于同步
    };
    // 创建一个 Foo 实例，包含锁和条件变量
    Foo foo;
    // 调用 Wait，因谓词返回 false，所以不会真的阻塞
    std::cout << "调用 foo.Wait() ...\n";
    *foo.Wait();
    std::cout << "foo.Wait() 已完成\n";
    // 调用 NotifyAll，应该没有任何等待的任务被唤醒
    std::cout << "调用 foo.NotifyAll() ...\n";
    *foo.NotifyAll();
    std::cout << "foo.NotifyAll() 已完成\n";
    std::cout << "程序结束\n";
}
