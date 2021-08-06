#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "fty/messagebus/message-bus.h"
#include <malamute.h>

TEST_CASE("Common")
{
    static std::string endpoint = "inproc://test-agent";

    zactor_t* malamute = zactor_new(mlm_server, const_cast<char*>("Malamute"));
    REQUIRE(malamute);
    zstr_sendx(malamute, "BIND", endpoint.c_str(), NULL);

    SECTION("Connect")
    {
        auto ret = fty::MessageBus::create(fty::MessageBus::Provider::Mlm, fmt::format("agent=test-agent;endpoint={}", endpoint));
        if (!ret) {
            std::cerr << ret.error() << std::endl;
        }
        CHECK(ret);
    }

    SECTION("Wrong connect")
    {
        auto ret = fty::MessageBus::create(fty::MessageBus::Provider::Mlm, fmt::format("agent=test-agent;endpoint={}-my", endpoint));
        if (!ret) {
            std::cerr << ret.error() << std::endl;
        }
        CHECK(!ret);
    }

    SECTION("Ping pong")
    {
        auto srv = fty::MessageBus::create(fty::MessageBus::Provider::Mlm, fmt::format("agent=pong;endpoint={}", endpoint));
        auto cln = fty::MessageBus::create(fty::MessageBus::Provider::Mlm, fmt::format("agent=ping;endpoint={}", endpoint));
        CHECK(srv);
        CHECK(cln);

        auto sret = srv->subscribe("play", [&](const fty::Message& msg){
            fty::Message pong;
            pong.setData(fmt::format("Pong on ping {}", msg.userData[0]));
            CHECK(srv->reply("play", msg, pong));
        });
        CHECK(sret);

        fty::Message msg;
        msg.meta.to = "pong";
        msg.meta.from = "ping";
        msg.setData("some data");
        auto cret = cln->request("play", msg);
        REQUIRE(cret);
        CHECK(cret->userData[0] == "Pong on ping some data");
    }

    zactor_destroy(&malamute);
}
