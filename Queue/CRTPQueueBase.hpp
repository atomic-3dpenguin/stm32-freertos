#pragma once

#include "FreeRTOSQueue.hpp"  // must already contain your aligned, safe queue implementation

namespace OS::FreeRTOS {

template <typename Derived, typename MessageType, size_t QueueSize>
class CRTPQueueBase {
  protected:
    static_assert(sizeof(MessageType) > 0, "CRTPQueueBase requires MessageType to be complete before use.");

    using QueueT = FreeRTOSQueue<MessageType, QueueSize>;

    CRTPQueueBase() = default;

    [[nodiscard]] QueueT& getQueue() { return messageQueue; }

    [[nodiscard]] const QueueT& getQueue() const { return messageQueue; }

    bool enqueueMessage(const MessageType& msg) {
        return messageQueue.send(msg) == QUEUE_STATUS::QUEUE_SUCCESS;
    }

    std::optional<MessageType> dequeueMessage(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        return messageQueue.receive(timeout);
    }

    bool hasPendingMessages() const {
        return messageQueue.hasMessage();
    }

  private:
    alignas(alignof(MessageType)) QueueT messageQueue;
};

}  // namespace OS::FreeRTOS
