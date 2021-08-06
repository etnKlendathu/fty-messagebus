#include "mlm-listener.h"
#include "mlm.h"
#include <fty_log.h>
#include "mlm-message.h"

namespace fty::messagebus::plugin {

MlmListener::MlmListener(Mlm* mlm)
    : m_listener(nullptr, &MlmListener::destroyActor)
    , m_mlm(mlm)
{
    zsys_handler_set(nullptr);
}

void MlmListener::start()
{
    m_listener.reset(zactor_new(&MlmListener::listener, reinterpret_cast<void*>(this)));
}

void MlmListener::destroyActor(zactor_t* actor)
{
    if (actor) {
        zactor_destroy(&actor);
    }
}

void MlmListener::listener(zsock_t* pipe, void* args)
{
    MlmListener* mbm = reinterpret_cast<MlmListener*>(args);
    mbm->listenerMainloop(pipe);
}

void MlmListener::listenerMainloop(zsock_t* pipe)
{
    zpoller_t* poller = zpoller_new(pipe, mlm_client_msgpipe(m_mlm->m_client.get()), nullptr);
    zsock_signal(pipe, 0);
    logTrace("{} - listener mainloop ready", m_mlm->m_agent);

    bool stopping = false;
    while (!stopping) {
        void* which = zpoller_wait(poller, -1);

        if (which == pipe) {
            zmsg_t* message       = zmsg_recv(pipe);
            char*   actor_command = zmsg_popstr(message);
            zmsg_destroy(&message);

            //  $TERM actor command implementation is required by zactor_t interface
            if (streq(actor_command, "$TERM")) {
                stopping = true;
                zstr_free(&actor_command);
            } else {
                logWarn("{} - received '{}' on pipe, ignored", actor_command ? actor_command : "(null)");
                zstr_free(&actor_command);
            }
        } else if (which == mlm_client_msgpipe(m_mlm->m_client.get())) {
            zmsg_t* message = mlm_client_recv(m_mlm->m_client.get());
            if (message == nullptr) {
                stopping = true;
            } else {
                const char* subject = mlm_client_subject(m_mlm->m_client.get());
                const char* from    = mlm_client_sender(m_mlm->m_client.get());
                const char* command = mlm_client_command(m_mlm->m_client.get());

                if (streq(command, "MAILBOX DELIVER")) {
                    listenerHandleMailbox(subject, from, message);
                } else if (streq(command, "STREAM DELIVER")) {
                    listenerHandleStream(subject, from, message);
                } else {
                    logError("{} - unknown malamute pattern '{}' from '{}' subject '{}'", m_mlm->m_agent, command, from, subject);
                }
                zmsg_destroy(&message);
            }
        }
    }

    zpoller_destroy(&poller);

    logDebug("{} - listener mainloop terminated", m_mlm->m_agent);
}

void MlmListener::listenerHandleMailbox(const char* subject, const char* from, zmsg_t* message)
{
    logDebug("{} - received mailbox message from '{}' subject '{}'", m_mlm->m_agent, from, subject);

    auto msg = fromMalamuteMsg(message);

    bool syncResponse = false;
    if (!m_syncUuid.empty()) {
        if (msg.meta.correlationId == m_syncUuid) {
            m_syncResponse = msg;
            m_syncUuid     = "";
            syncResponse   = true;
            syncMessageEvent();
        }
    }
    if (syncResponse == false) {
        messageEvent(subject, msg);
    }
}

void MlmListener::listenerHandleStream(const char* subject, const char* from, zmsg_t* message)
{
    logTrace("{} - received stream message from '{}' subject '{}'", m_mlm->m_agent, from, subject);
    auto msg = fromMalamuteMsg(message);
    messageEvent(subject, msg);
}


}
