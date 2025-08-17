#include "eventuals/eventual.h"
#include "eventuals/terminal.h"
#include "eventuals/just.h"
#include <iostream>
// 使用 eventuals 命名空间，方便直接写 Just / Terminal 等
using namespace eventuals;
int main() {
    // 构建一个最简单的 Eventual pipeline。
    // Just("hello world") 表示将字符串常量 "hello world"
    // 注入到 pipeline 中，作为后续处理的输入。
    auto e = Just("hello world")
             // 使用 Terminal() 终结 pipeline。
             // Terminal() 必须指定至少一个回调，用来处理执行结果。
             >> Terminal()
                    // start()：当 pipeline 成功执行时调用。
                    // 这里 result 就是 Just 注入的字符串。
                    .start([](auto&& result) { std::cout << "Success: " << result << std::endl; })
                    // fail()：当 pipeline 执行失败时调用。
                    // 这里 error 一般是异常或错误信息。
                    .fail([](auto&& error) { std::cerr << "Error: " << error << std::endl; })
                    // stop()：当 pipeline 被主动停止时调用。
                    .stop([]() { std::cerr << "Stopped!" << std::endl; });
    // Build() 将 pipeline 构建为一个 Continuation（延续对象）。
    // 注意：只有 Build() 之后的 Continuation 才能调用 Start() 启动。
    auto k = Build(std::move(e));
    // 启动 pipeline。
    // Start() 会触发执行流程，从 Just 注入值，
    // 再传递到 Terminal，最终调用对应的回调。
    k.Start();
    return 0;
}
