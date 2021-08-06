#pragma once
#include "common/plugin.h"
#include <fty/event.h>
#include <fty/expected.h>
#include <malamute.h>
#include <mutex>

namespace fty::messagebus::plugin {

class MlmListener;

class Mlm : public IMessageBus
{
public:
    Mlm();
    ~Mlm();

    Expected<void> connect(const std::string& connectionString) noexcept override;

    Expected<Message> request(const std::string& queue, const Message& message, int receiveTimeOut) noexcept override;
    Expected<void>    subscribe(const std::string& topic, MessageListener listener) noexcept override;
    Expected<void>    unsubscribe(const std::string& topic) noexcept override;
    Expected<void>    publish(const std::string& topic, const Message& message) noexcept override;
    Expected<void>    receive(const std::string& queue, MessageListener messageListener) noexcept override;
    Expected<void>    sendReply(const std::string& queue, const Message& message) noexcept override;
    Expected<void>    sendRequest(const std::string& queue, const Message& message) noexcept override;
    Expected<void>    sendRequest(const std::string& queue, const Message& message, MessageListener listener) noexcept override;

private:
    static void destroyMlm(mlm_client_t*);

    Slot<const std::string&, const Message&> onMessage = {&Mlm::handleMessage, this};

    void handleMessage(const std::string& subject, const Message& msg);

private:
    using MlmClient = std::unique_ptr<mlm_client_t, decltype(&Mlm::destroyMlm)>;

    std::string                                  m_agent;
    std::string                                  m_endpoint;
    MlmClient                                    m_client;
    std::mutex                                   m_mutex;
    std::map<const std::string, MessageListener> m_subscriptions;
    std::string                                  m_publishTopic;

    friend class MlmListener;
    std::unique_ptr<MlmListener> m_listener;
};

} // namespace fty::messagebus::plugin

extern "C" {
fty::messagebus::plugin::Mlm* pluginInstance();
}
