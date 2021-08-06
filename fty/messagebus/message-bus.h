/*  =========================================================================
    message-bus.h - Common message bus wrapper

    Copyright (C) 2014 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
 */

#pragma once
#include <fty/expected.h>
#include "fty/messagebus/message.h"
#include <functional>
#include <memory>

namespace fty {

// =========================================================================================================================================

namespace messagebus::plugin {
class IMessageBus;
}

// =========================================================================================================================================

/// Common message bus temporary wrapper
class MessageBus
{
public:
    enum class Provider
    {
        Mlm,
        Mqtt,
        Amqp
    };

    /// Creates message bus
    [[nodiscard]] static Expected<MessageBus> create(Provider provider, const std::string& connection) noexcept;

public:
    ~MessageBus();
    MessageBus(const MessageBus&) = delete;
    MessageBus(MessageBus&&) noexcept = default;

    /// Sends message to the queue and wait to receive response
    /// @param queue the queue to use
    /// @param msg the message to send
    /// @return Response message or error
    [[nodiscard]] Expected<Message> request(const std::string& queue, const Message& msg) noexcept;

    /// Publishes message to a topic
    /// @param queue the queue to use
    /// @param msg the message object to send
    /// @return Success or error
    [[nodiscard]] Expected<void> send(const std::string& queue, const Message& msg) noexcept;

    /// Sends a reply to a queue
    /// @param queue the queue to use
    /// @param req request message on which you send response
    /// @param answ response message
    /// @return Success or error
    [[nodiscard]] Expected<void> reply(const std::string& queue, const Message& req, const Message& answ) noexcept;

    /// Subscribes to a queue
    /// @example
    ///     bus.subsribe("queue", &MyCls::listener, this);
    /// @param queue the queue to subscribe
    /// @param fnc the member function to subscribe
    /// @param cls class instance
    /// @return Success or error
    template <typename Func, typename Cls>
    [[nodiscard]] Expected<void> subscribe(const std::string& queue, Func&& fnc, Cls* cls) noexcept
    {
        return subscribe(queue, [f = std::move(fnc), c = cls](const Message& msg) -> void {
            std::invoke(f, *c, Message(msg));
        });
    }

    /// Subscribes to a queue
    /// @param queue the queue to subscribe
    /// @param func the function to subscribe
    /// @return Success or error
    [[nodiscard]] Expected<void> subscribe(const std::string& queue, std::function<void(const Message&)>&& func) noexcept;

    /// Unsubscribes from a queue
    /// @param queue the queue to unsubscribe
    /// @return Success or error
    [[nodiscard]] Expected<void> unsubscribe(const std::string& queue) noexcept;

private:
    MessageBus(std::unique_ptr<messagebus::plugin::IMessageBus>&& plug);

private:
    std::unique_ptr<messagebus::plugin::IMessageBus> m_impl;
};

// =========================================================================================================================================

} // namespace fty
