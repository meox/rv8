#include <iostream>
#include <sstream>
#include <include/zsocket.h>
#include <include/v8-extended-util.h>


using namespace std;
using namespace v8;

zmq::context_t zmq_context{1};

ZSocketTempl::ZSocketTempl(RV8& rv8_engine) : RVObject<ZSocket>{rv8_engine}, engine{rv8_engine}
{
    engine.add_global_function("zsocket", [this](js_callinfo_t args){
        Isolate *isolate = args.GetIsolate();
        HandleScope handle_scope(isolate);
        const size_t args_len = args.Length();

        ZSocket* c_obj = new ZSocket{};
        v_zsocket.push_back(c_obj);

        if (args_len > 0)
        {
            String::Utf8Value s(args[0]);
            c_obj->ip = *s;
        }

        if (args_len > 1)
            c_obj->port = args[1]->ToUint32()->Value();


        // add object function here
        add_function("bind", [](js_callinfo_t args, ZSocket* obj){
            stringstream ip_bind;
            ip_bind << "tcp://" << obj->ip << ":" << obj->port;
            cerr << "bind: " <<  ip_bind.str() << endl;
            obj->socket = new zmq::socket_t{zmq_context, ZMQ_REP};
            obj->socket->bind(ip_bind.str());
        }, c_obj);

        add_function("connect", [](js_callinfo_t args, ZSocket* obj){
            stringstream ip_connect;
            ip_connect << "tcp://" << obj->ip << ":" << obj->port;
            cerr << "connect: " <<  ip_connect.str() << endl;
            obj->socket = new zmq::socket_t{zmq_context, ZMQ_REQ};
            obj->socket->connect(ip_connect.str());
        }, c_obj);

        add_function("recv", [](js_callinfo_t args, ZSocket* obj){
            zmq::message_t request;
            obj->socket->recv(&request);

            rv::set_return(args, zeromq::to_string(request));
        }, c_obj);

        add_function("send", [](js_callinfo_t args, ZSocket* obj){
            stringstream st;

            for (int i = 0; i < args.Length(); i++)
            {
                HandleScope handle_scope(args.GetIsolate());
                pretty_print(st, *args[i]);
            }
            
            auto msg_str = st.str();
            if (msg_str.size() > 0)
            {
                auto msg = zeromq::create_message(st.str());
                obj->socket->send(msg);
            }
        }, c_obj);

        args.GetReturnValue().Set(get_object());
    });
}
