#ifndef __UTILZERO__HPP__
#define __UTILZERO__HPP__

#include "zmq.hpp"
#include <iostream>
#include <sstream>

namespace zeromq
{
    template <typename T>
    zmq::message_t create_message(std::stringstream& st, T&& s)
    {
        st << s;
        const std::string str = st.str();
        const auto l = str.size()+1;
        zmq::message_t msg (l);

        auto buff = reinterpret_cast<char*>(msg.data());
        memcpy ((void *) buff, str.c_str(), l);
        buff[l-1] = '\0';

        return msg;
    }


    template <typename T, typename ...Args>
    zmq::message_t create_message(std::stringstream& st, T&& s, Args&& ... args)
    {
        st << s;
        return create_message(st, std::forward<Args>(args)...);
    }


    template <typename ...Args>
    zmq::message_t create_message(Args&& ... args)
    {
        std::stringstream st;
        return create_message(st, std::forward<Args>(args)...);
    }


    template <typename T>
    zmq::message_t create_message_raw(const T& raw_msg, const size_t plen)
    {
        zmq::message_t msg (plen);
        memcpy (reinterpret_cast<void*>(msg.data()), raw_msg, plen);
        return msg;
    }

    inline std::string to_string(const zmq::message_t& msg)
    {
        return std::string{reinterpret_cast<const char*>(msg.data())};
    }


    inline int get_socket_type(const std::string& s_type_str) //todo: use switch + hash + constexpr
    {
        if (s_type_str == "ZMQ_REP")
            return ZMQ_REP;
        else if (s_type_str == "ZMQ_REQ")
            return ZMQ_REQ;
        else if (s_type_str == "ZMQ_PUB")
            return ZMQ_PUB;

        return -1;
    }
}

#endif
