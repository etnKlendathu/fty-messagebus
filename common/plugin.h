/*  =========================================================================
    fty_common_messagebus_interface - class description

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

#include "fty/messagebus/message.h"
#include <fty/expected.h>
#include <functional>
#include <string>

namespace fty::messagebus::plugin {

class IMessageBus
{
public:
    using MessageListener = std::function<void(const Message&)>;

    virtual ~IMessageBus() = default;

    /// Try a connection with message bus
    virtual Expected<void> connect(const std::string& connectionString) noexcept = 0;

    /// Send request to a queue and wait to receive response
    /// @param requestQueue    The queue to use
    /// @param message         The message to send
    /// @param receiveTimeOut  Wait for response until timeout is reach
    /// @return message as response
    virtual Expected<Message> request(const std::string& queue, const Message& message, int receiveTimeOut) noexcept = 0;

    /// Subscribe to a topic
    /// @param topic             The topic to subscribe
    /// @param messageListener   The message listener to call on message
    virtual Expected<void> subscribe(const std::string& topic, MessageListener listener) noexcept = 0;

    /// Unsubscribe to a topic
    /// @param topic             The topic to unsubscribe
    virtual Expected<void> unsubscribe(const std::string& topic) noexcept = 0;

    /// Publish message to a topic
    /// @param topic     The topic to use
    /// @param message   The message object to send
    virtual Expected<void> publish(const std::string& topic, const Message& message) noexcept = 0;

    /// Receive message from queue
    /// @param queue             The queue where receive message
    /// @param messageListener   The message listener to use for this queue
    virtual Expected<void> receive(const std::string& queue, MessageListener listener) noexcept = 0;

    /// Send a reply to a queue
    /// @param replyQueue      The queue to use
    /// @param message         The message to send
    virtual Expected<void> sendReply(const std::string& queue, const Message& message) noexcept = 0;

    /// Send request to a queue
    /// @param requestQueue    The queue to use
    /// @param message         The message to send
    virtual Expected<void> sendRequest(const std::string& queue, const Message& message) noexcept = 0;

    /// Send request to a queue and receive response to a specific listener
    /// @param requestQueue    The queue to use
    /// @param message         The message to send
    /// @param messageListener The listener where to receive response (on queue set to reply to field)
    virtual Expected<void> sendRequest(const std::string& queue, const Message& message, MessageListener listener) noexcept = 0;

public:
    template <typename FuncT, typename ClsT>
    Expected<void> subscribe(const std::string& topic, FuncT&& func, ClsT* cls)
    {
        return subscribe(topic, [f = std::move(func), c = cls](const Message& msg) -> void {
            std::invoke(f, *c, Message(msg));
        });
    }

protected:
    IMessageBus() = default;
};

} // namespace fty::messagebus::plugin
