#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include "eventuals/eventual.h"
#include "eventuals/event-loop.h"
#include "eventuals/then.h"
#include "eventuals/promisify.h"
using namespace eventuals;
auto AsynchronousFunction() {
    return Just(0) >> Then([](int) {
               std::cout << "异步任务执行..." << std::endl;
               std::this_thread::sleep_for(std::chrono::seconds(1));
               return std::string("Hello from async!");
           });
}
int main() {
    // 必须先构造默认的事件循环，以便 Eventuals 库可以管理异步任务。
    EventLoop::ConstructDefault();
    // --- 示例1: 使用 Terminate() 与 std::future 集成 ---
    std::cout << "--- 示例1: 使用 std::future 等待异步操作 ---" << std::endl;
    // 1. 获取一个 Eventual 对象
    auto e = AsynchronousFunction();
    // 2. 使用 Terminate() 将 Eventual 转换为 std::promise + std::future
    //    'future' 用于获取结果，'k' 是一个继续器（contuniation），用于启动任务
    auto [future, k] = Promisify("chained-tasks", std::move(e));
    // 3. 启动异步任务
    k.Start();
    // 4. 在主线程中阻塞，等待 future 结果可用
    //    future.get() 会阻塞当前线程直到异步任务完成并返回值
    auto result1 = future.get();
    std::cout << "使用 std::future 获取结果: " << result1 << std::endl;
    std::cout << std::endl;
    // --- 示例2: 在测试中使用阻塞方式 '*' 获取结果 ---
    // 注意：这种方式仅推荐用于测试和简单场景，因为它会阻塞主线程。
    std::cout << "--- 示例2: 使用 '*' 阻塞式获取结果 ---" << std::endl;
    // 使用解引用操作符 '*' 直接获取异步操作的结果
    // 内部实现上，这会阻塞当前线程直到异步操作完成
    auto result2 = *AsynchronousFunction();
    std::cout << "使用 '*' 阻塞式获取结果: " << result2 << std::endl;
    // 在程序结束前，必须清理事件循环
    EventLoop::DestructDefault();
    return 0;
}