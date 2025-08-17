#include "eventuals/just.h"       // 引入 Just 组合器，用于将值注入异步管道
#include "eventuals/lock.h"       // 引入 Lock 和 Synchronizable，用于同步操作
#include "eventuals/promisify.h"  // 引入 Promisify，将 Eventual 转换为 std::future
#include "eventuals/if.h"         // 引入 If 组合器，用于条件逻辑
#include <iostream>               // 标准输入输出库，用于 std::cout
using namespace eventuals;        // 使用 Eventuals 命名空间，简化代码
int main() {
    // 定义一个支持多条件变量的同步对象 Foo
    struct Foo : public Synchronizable {
        // 等待某个 id 对应的条件变量被唤醒
        auto WaitFor(int id) {
            return Synchronized(Then([this, id]() {
                // 在 map 中查找或插入对应 id 的条件变量
                auto [iterator, inserted] = condition_variables_.emplace(id, &lock());
                ConditionVariable& condition_variable = iterator->second;
                // 返回等待操作（会阻塞直到被唤醒）
                return condition_variable.Wait();
            }));
        }
        // 唤醒某个 id 对应的一个等待者
        auto NotifyFor(int id) {
            return Synchronized(Then([this, id]() {
                auto iterator = condition_variables_.find(id);
                return If(iterator == condition_variables_.end())
                    // 如果没找到对应条件变量，返回 false
                    .yes([]() { return false; })
                    // 找到了就唤醒一个等待的协程/线程
                    .no([iterator]() {
                        ConditionVariable& condition_variable = iterator->second;
                        condition_variable.Notify();
                        return true;
                    });
            }));
        }
        // 唤醒某个 id 对应的所有等待者
        auto NotifyAllFor(int id) {
            return Synchronized(Then([this, id]() {
                auto iterator = condition_variables_.find(id);
                return If(iterator == condition_variables_.end())
                    // 没找到条件变量返回 false
                    .yes([]() { return false; })
                    // 找到了就唤醒所有等待者
                    .no([iterator]() {
                        ConditionVariable& condition_variable = iterator->second;
                        condition_variable.NotifyAll();
                        return true;
                    });
            }));
        }
        // 保存不同 id 对应的条件变量
        std::map<int, ConditionVariable> condition_variables_;
    };
    Foo foo;
    // 创建 3 个等待 id=42 的 future
    auto [future1, k1] = Promisify("k1", foo.WaitFor(42));
    auto [future2, k2] = Promisify("k2", foo.WaitFor(42));
    auto [future3, k3] = Promisify("k3", foo.WaitFor(42));
    // 启动这三个等待操作
    k1.Start();
    k2.Start();
    k3.Start();
    // 确认三个 future 都还没完成（立即等待会超时）
    assert(std::future_status::timeout == future1.wait_for(std::chrono::seconds(0)));
    assert(std::future_status::timeout == future2.wait_for(std::chrono::seconds(0)));
    assert(std::future_status::timeout == future3.wait_for(std::chrono::seconds(0)));
    // 尝试唤醒 id=41（不存在），返回 false
    *foo.NotifyFor(41);
    // 唤醒 id=42 的一个等待者（成功，返回 true）
    *foo.NotifyFor(42);
    // future1 会被唤醒并完成
    future1.get();
    // 另外两个 future 依旧处于等待状态
    assert(std::future_status::timeout == future2.wait_for(std::chrono::seconds(0)));
    assert(std::future_status::timeout == future3.wait_for(std::chrono::seconds(0)));
    // 唤醒 id=42 的所有等待者（成功，返回 true）
    std::cout << *foo.NotifyAllFor(42);
    // future2、future3 被唤醒并完成
    future2.get();
    future3.get();
}
