#include "eventuals/lock.h"       // 引入锁机制，用于异步临界区管理
#include "eventuals/then.h"       // Then 组合器，用于异步链式处理
#include "eventuals/promisify.h"  // 将 Eventual 转换为 std::future，方便阻塞等待结果
using namespace eventuals;
int main() {
    // 定义一个 Lock（相当于互斥锁），用于控制 e1/e2 的执行顺序
    Lock lock;
    // 定义第一个异步事件 e1：
    auto e1 = [&]() {
        return Eventual<std::string>()   // 声明产生 std::string 的异步事件
                   .start([](auto& k) {  // start() 定义如何启动事件
                       // 在独立线程中调用 k.Start("t1")，异步产生值 "t1"
                       std::thread thread([&k]() mutable { k.Start("t1"); });
                       thread.detach();  // 分离线程，避免阻塞主线程
                   }) >>
               Acquire(&lock)                     // 获取锁，保证临界区互斥
               >> Then([](std::string&& value) {  // Then 用于处理获取到的值
                     return std::move(value);     // 直接返回 "t1"
                 });
    };
    // 定义第二个异步事件 e2（与 e1 类似，但产生 "t2"）：
    auto e2 = [&]() {
        return Eventual<std::string>().start([](auto& k) {
            std::thread thread([&k]() mutable { k.Start("t2"); });
            thread.detach();
        }) >> Acquire(&lock) >>
               Then([](std::string&& value) { return std::move(value); });
    };
    // 定义第三个事件 e3：用于释放锁
    auto e3 = [&]() {
        return Release(&lock)  // 释放锁
               >> Then([]() {  // 然后返回 "t3"
                     return "t3";
                 });
    };
    // 将 e1/e2/e3 转换为 (future, continuation) 对，便于阻塞等待结果
    auto [future1, t1] = Promisify("e1", e1());
    auto [future2, t2] = Promisify("e2", e2());
    auto [future3, t3] = Promisify("e3", e3());
    // 启动 e1
    t1.Start();
    std::cout << future1.get() << '\n';  // 阻塞等待 e1 的结果，输出 "t1"
    // 启动 e2 和 e3（e2 需要锁，e3 会释放锁）
    t2.Start();
    t3.Start();
    // future3.get() 会输出 "t3"
    std::cout << future3.get() << '\n';
    // future2.get() 等待 e2 拿到锁后执行，输出 "t2"
    std::cout << future2.get() << '\n';
}
