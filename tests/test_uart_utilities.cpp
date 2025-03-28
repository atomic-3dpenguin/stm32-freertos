// test_uart_utilities.cpp
#include <gtest/gtest.h>
#include "line_parser.hpp"
#include "tx_queue.hpp"

TEST(LineParserTest, ParsesCompleteLine) {
    LineParser parser;
    const char* input = "hello world\r";

    bool lineReady = false;
    for (std::size_t i = 0; input[i] != '\0'; ++i) {
        lineReady = parser.push(input[i]);
    }

    EXPECT_TRUE(lineReady);
    EXPECT_TRUE(parser.hasLine());
    EXPECT_STREQ(parser.getLine(), "hello world");
    EXPECT_FALSE(parser.hasLine());
}

TEST(LineParserTest, HandlesMultipleLines) {
    LineParser parser;
    const char* input = "cmd1\ncmd2\n";

    std::vector<std::string> lines;
    for (std::size_t i = 0; input[i] != '\0'; ++i) {
        if (parser.push(input[i]) && parser.hasLine()) {
            lines.emplace_back(parser.getLine());
        }
    }

    ASSERT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[0], "cmd1");
    EXPECT_EQ(lines[1], "cmd2");
}

TEST(TxQueueManagerTest, EnqueueDequeueSuccess) {
    TxQueueManager queue;
    const char* msg = "test message";

    ASSERT_TRUE(queue.enqueue(msg));

    char out[TxQueueManager::MaxMsgSize] = {};
    ASSERT_TRUE(queue.dequeue(out));
    EXPECT_STREQ(out, msg);
}

TEST(TxQueueManagerTest, EmptyQueueReturnsFalse) {
    TxQueueManager queue;
    char out[TxQueueManager::MaxMsgSize] = {};
    EXPECT_FALSE(queue.dequeue(out));
}
