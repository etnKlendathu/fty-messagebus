#include "mlm.h"
#include "common/helper.h"
#include "mlm-listener.h"
#include "mlm-message.h"
#include <fty/event.h>
#include <fty/string-utils.h>
#include <fty_log.h>

namespace fty::messagebus::plugin {


// =========================================================================================================================================

Mlm::Mlm()
    : m_client(mlm_client_new(), &Mlm::destroyMlm)
    , m_listener(new MlmListener(this))
{
}

Mlm::~Mlm()
{
}

Expected<void> Mlm::connect(const std::string& connectionString) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);

    static std::regex re("([a-zA-Z0-9]+)\\s*=\\s*(.+)");
    for (const auto& opt : fty::split(connectionString, ";")) {
        auto [key, value] = fty::split<std::string, std::string>(opt, re);
        if (key == "agent") {
            m_agent = value;
        } else if (key == "endpoint") {
            m_endpoint = value;
        }
    }

    if (m_agent.empty() || m_endpoint.empty()) {
        return unexpected("Wrong parameters");
    }

    if (mlm_client_connect(m_client.get(), m_endpoint.c_str(), 1000, m_agent.c_str()) < 0) {
        return unexpected("Mlm error: Error connecting to endpoint '{}'", m_endpoint);
    }

    onMessage.connect(m_listener->messageEvent);
    m_listener->start();

    return {};
}

void Mlm::destroyMlm(mlm_client_t* p)
{
    if (p) {
        logDebug("destroy mlm");
        mlm_client_destroy(&p);
    }
}

Expected<Message> Mlm::request(const std::string& queue, const Message& message, int receiveTimeOut) noexcept
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (message.meta.to.empty()) {
            return unexpected("Request message must have a 'to' field.");
        }

        if (message.meta.correlationId.empty()) {
            message.meta.correlationId = utils::generateUuid();
        }

        message.meta.from    = m_agent;
        message.meta.timeout = receiveTimeOut;
        message.meta.replyTo = m_agent;

        zmsg_t* msgMlm = toMalamuteMsg(message);

        m_listener->m_syncUuid = message.meta.correlationId;
        if (mlm_client_sendto(m_client.get(), message.meta.to.value().c_str(), queue.c_str(), nullptr, 200, &msgMlm) < 0) {
            return unexpected("Cannot send message");
        }

        if (auto ret = m_listener->syncMessageEvent.wait(receiveTimeOut); !ret) {
            return unexpected(ret.error());
        }

        return m_listener->m_syncResponse;
    } catch (const std::exception& ex) {
        return unexpected(ex.what());
    } catch (...) {
        return unexpected("Unspecified error");
    }
}

Expected<void> Mlm::subscribe(const std::string& topic, MessageListener messageListener) noexcept
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (mlm_client_set_consumer(m_client.get(), topic.c_str(), "") == -1) {
            return unexpected("Failed to set consumer on Malamute connection.");
        }

        m_subscriptions.emplace(topic, messageListener);
        logTrace("{} - subscribed to topic '{}'", m_agent, topic);
        return {};
    } catch (const std::exception& ex) {
        return unexpected(ex.what());
    } catch (...) {
        return unexpected("Unspecified error");
    }
}

Expected<void> Mlm::unsubscribe(const std::string& topic) noexcept
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto iterator = m_subscriptions.find(topic);

        if (iterator == m_subscriptions.end()) {
            return unexpected("Trying to unsubscribe on non-subscribed topic.");
        }

        // Our current Malamute version is too old...
        logWarn("{} - mlm_client_remove_consumer() not implemented", m_agent);

        m_subscriptions.erase(iterator);
        logTrace("{} - unsubscribed to topic '{}'", m_agent, topic);
        return {};
    } catch (const std::exception& ex) {
        return unexpected(ex.what());
    } catch (...) {
        return unexpected("Unspecified error");
    }
}

void Mlm::handleMessage(const std::string& subject, const Message& msg)
{
    auto iterator = m_subscriptions.find(subject);
    if (iterator != m_subscriptions.end()) {
        try {
            (iterator->second)(msg);
        } catch (const std::exception& e) {
            logError("Error in listener of queue '{}': '{}'", iterator->first, e.what());
        } catch (...) {
            logError("Error in listener of queue '{}': 'unknown error'", iterator->first);
        }
    } else {
        logWarn("Message skipped");
    }
}

Expected<void> Mlm::publish(const std::string& topic, const Message& message) noexcept
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_publishTopic.empty()) {
            m_publishTopic = topic;
            if (mlm_client_set_producer(m_client.get(), m_publishTopic.c_str()) == -1) {
                return unexpected("Failed to set producer on Malamute connection.");
            }
            logTrace("{} - registered as stream producter on '{}'", m_agent, m_publishTopic);
        }

        if (topic != m_publishTopic) {
            return unexpected("MessageBusMalamute requires publishing to declared topic.");
        }

        logTrace("{} - publishing on topic '{}'", m_agent, m_publishTopic);
        zmsg_t* msg = toMalamuteMsg(message);
        if (mlm_client_send(m_client.get(), topic.c_str(), &msg) < 0) {
            return unexpected("Cannot publish message to {} for {}", topic, m_agent);
        }
        return {};
    } catch (const std::exception& ex) {
        return unexpected(ex.what());
    } catch (...) {
        return unexpected("Unspecified error");
    }
}

Expected<void> Mlm::receive(const std::string& queue, MessageListener messageListener) noexcept
{
    auto iterator = m_subscriptions.find(queue);
    if (iterator != m_subscriptions.end()) {
        return unexpected("Already have queue map to listener");
    }

    m_subscriptions.emplace(queue, messageListener);
    logTrace("{} - receive from queue '{}'", m_agent, queue);
    return {};
}

Expected<void> Mlm::sendReply(const std::string& replyQueue, const Message& message) noexcept
{
    if (message.meta.correlationId.empty()) {
        return unexpected("Reply must have a correlation id.");
    }

    if (message.meta.to.empty()) {
        logWarn("{} - request should have a to field", m_agent);
    }

    zmsg_t* msg = toMalamuteMsg(message);

    if (mlm_client_sendto(m_client.get(), message.meta.to.value().c_str(), replyQueue.c_str(), nullptr, 200, &msg) < 0) {
        return unexpected("Cannot reply to {} for {}", message.meta.to.value(), m_agent);
    }

    return {};
}

Expected<void> Mlm::sendRequest(const std::string& requestQueue, const Message& message) noexcept
{
    std::string to      = requestQueue;
    std::string subject = requestQueue;

    if (message.meta.correlationId.empty()) {
        logWarn("{} - request should have a correlation id", m_agent);
    }

    if (message.meta.replyTo.empty()) {
        logWarn("{} - request should have a reply to field", m_agent);
    }

    if (message.meta.to.empty()) {
        logWarn("{} - request should have a to field", m_agent);
    } else {
        to      = message.meta.to;
        subject = requestQueue;
    }
    zmsg_t* msg = toMalamuteMsg(message);

    if (mlm_client_sendto(m_client.get(), to.c_str(), subject.c_str(), nullptr, 200, &msg) < 0) {
        return unexpected("{} - cannot send request to {}", m_agent, message.meta.to.value());
    }

    return {};
}

Expected<void> Mlm::sendRequest(const std::string& queue, const Message& message, MessageListener listener) noexcept
{
    if (message.meta.replyTo.empty()) {
        return unexpected("Request must have a reply to queue.");
    }

    receive(message.meta.replyTo, listener);
    return sendRequest(queue, message);
}

// =========================================================================================================================================

} // namespace fty::messagebus::plugin

extern "C" {
fty::messagebus::plugin::Mlm* pluginInstance()
{
    return new fty::messagebus::plugin::Mlm();
}
}
