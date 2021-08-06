#pragma once
#include <fty/event.h>
#include <fty/messagebus/message.h>
#include <malamute.h>
#include <memory>

namespace fty::messagebus::plugin {

class Mlm;

class MlmListener
{
public:
    Event<const std::string&, const Message&> messageEvent;
    Event<>               syncMessageEvent;

private:
    MlmListener(Mlm* mlm);
    void start();

    static void destroyActor(zactor_t* actor);
    static void listener(zsock_t* pipe, void* args);

    void listenerMainloop(zsock_t* pipe);
    void listenerHandleMailbox(const char* subject, const char* from, zmsg_t* message);
    void listenerHandleStream(const char* subject, const char* from, zmsg_t* message);

private:
    friend class Mlm;
    std::unique_ptr<zactor_t, decltype(&MlmListener::destroyActor)> m_listener;
    Mlm*                                                            m_mlm;
    std::string                                                     m_syncUuid;
    Message                                                         m_syncResponse;
};

} // namespace fty::messagebus::plugin
