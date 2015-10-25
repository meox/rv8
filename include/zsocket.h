#pragma once


#include <iostream>
#include <include/rv8.h>
#include <include/zmq.hpp>
#include "util_zero.hpp"


using namespace v8;

extern zmq::context_t zmq_context;

struct ZSocket
{
    std::string ip{"127.0.0.1"};
    uint32_t port{9999};
    zmq::socket_t *socket{nullptr};

    ~ZSocket() { delete socket; }
};


class ZSocketTempl final : public RVObject<ZSocket>
{
public:
    ZSocketTempl(RV8& rv8_engine);

    ~ZSocketTempl()
    {
        for (auto e : v_zsocket)
            delete e;
    }

private:
    RV8& engine;
    std::vector<ZSocket*> v_zsocket;
};


