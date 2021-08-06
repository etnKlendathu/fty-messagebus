#include "fty/messagebus/message-bus.h"
#include "libloader.h"
#include "common/plugin.h"

namespace fty {

// =========================================================================================================================================

class PluginManager
{
public:
    static PluginManager& instance()
    {
        static PluginManager manager;
        return manager;
    }

public:
    template<typename PluginT>
    Expected<PluginT*> plugin(const std::string& plugName)
    {
        if (!m_plugins.count(plugName)) {
            m_plugins.emplace(plugName, std::make_shared<LibLoader>(plugName));
        }

        if (auto ret = m_plugins.at(plugName)->load<PluginT>()) {
            return *ret;
        } else {
            return unexpected(ret.error());
        }
    }

private:
    std::map<std::string, std::shared_ptr<LibLoader>> m_plugins;
};


// =========================================================================================================================================

MessageBus::MessageBus(std::unique_ptr<messagebus::plugin::IMessageBus>&& plug):
    m_impl(std::move(plug))
{
}

MessageBus::~MessageBus()
{
}

Expected<MessageBus> MessageBus::create(Provider provider, const std::string& connection) noexcept
{
    std::unique_ptr<messagebus::plugin::IMessageBus> plug;
    if (provider == Provider::Mlm) {
        auto mlm = PluginManager::instance().plugin<messagebus::plugin::IMessageBus>("libplugin-mlm.so");
        if (!mlm) {
            return unexpected(mlm.error());
        }
        plug.reset(*mlm);
        auto res = plug->connect(connection);
        if (!res) {
            return unexpected(res.error());
        }
        return MessageBus(std::move(plug));
    }
    return unexpected("wrong");
}

Expected<Message> MessageBus::request(const std::string& queue, const Message& msg) noexcept
{
    return m_impl->request(queue, msg, 1000);
}

Expected<void> MessageBus::send(const std::string& queue, const Message& msg) noexcept
{
    return m_impl->publish(queue, msg);
}

Expected<void> MessageBus::reply(const std::string& queue, const Message& req, const Message& answ) noexcept
{
    answ.meta.correlationId = req.meta.correlationId;
    answ.meta.to            = req.meta.replyTo;
    answ.meta.from          = req.meta.to;

    return m_impl->sendReply(queue, answ);
}

Expected<void> MessageBus::subscribe(const std::string& queue, std::function<void(const Message&)>&& func) noexcept
{
    return m_impl->subscribe(queue, func);
}

/// Unsubscribes from a queue
/// @param queue the queue to unsubscribe
/// @return Success or error
Expected<void> MessageBus::unsubscribe(const std::string& queue) noexcept
{
    return m_impl->unsubscribe(queue);
}

}
