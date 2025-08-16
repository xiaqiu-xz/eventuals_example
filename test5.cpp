#include "eventuals/just.h"       // 引入 Just，用于创建立即返回固定值的 Eventual
#include "eventuals/promisify.h"  // 可将普通函数包装为 Eventual（示例未用）
// 使用命名空间简化写法，实际项目中也可只引入所需符号
using namespace eventuals;
int main(int argc, char** argv) {
    // 定义一个 Eventual 对象 e，返回一个整数 42
    auto e = []() {
        return Just(42);  // Just 创建一个立即返回值的 Eventual
    };
    // 执行 Eventual 并获取结果，阻塞直到结果准备好
    int i = *e();  // BLOCKING! 阻塞等待 Eventual 计算完成
    // 检查结果是否符合预期
    CHECK_EQ(42, i);
    return 0;
}