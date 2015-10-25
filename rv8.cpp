/**
 * C++ Italian MeetUp, 24 Ottobre Rome
 * written by: Gian Lorenzo Meocci <glmeocci@gmail.com>
 */


#include <iostream>
#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include <include/v8-extended-util.h>
#include <include/rv8.h>
#include "include/zsocket.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>

#include <chrono>
#include <algorithm>

using namespace std;
using namespace v8;
using namespace std::chrono;


int main(int argc, char* argv[])
{
    RV8 rv8_engine;

    string script_fname{"test.js"};
    if (argc == 2)
    {
        script_fname = argv[1];
    }

    //add println function
    rv8_engine.add_global_function("println", [](js_callinfo_t args){
        HandleScope handle_scope(args.GetIsolate());
        bool first = true;
        for (int i = 0; i < args.Length(); i++)
        {
            if (first)
                first = false;
            else
                cout << " ";

            pretty_print(*args[i]);
        }

        cout << endl;
    });

    //add input function
    rv8_engine.add_global_function("input", [](js_callinfo_t args){
        HandleScope handle_scope(args.GetIsolate());
        if (args.Length() > 0)
            pretty_print(*args[0]);

        string istr;
        std::getline(cin, istr);
        rv::set_return(args, istr);
    });


    // add zmq socket
    ZSocketTempl zs(rv8_engine);


    rv8_engine.add_global_function("exit", [&rv8_engine](const v8::FunctionCallbackInfo<v8::Value>& args){
        rv8_engine.invoke_jsfun("exit_callback", "bye bye");
        exit(0);
    });


    /* timing function */
    time_point<std::chrono::system_clock> tstart, tend;
    rv8_engine.add_global_function("start_timer", [&tstart](const v8::FunctionCallbackInfo<v8::Value>& args){
        tstart = std::chrono::system_clock::now();
    });

    rv8_engine.add_global_function("end_timer", [&tstart, &tend](const v8::FunctionCallbackInfo<v8::Value>& args){
        tend = std::chrono::system_clock::now();
        duration<double> elapsed_seconds = tend - tstart;
        cerr << "[DEBUG] - elapsed time: " << elapsed_seconds.count() << "s" << endl;
    });


    // execute the script
    rv8_engine.execute(script_fname);

    return 0;
}


/*

    //add sum
    rv8_engine.add_global_function("sum", [](js_callinfo_t args){
        HandleScope handle_scope(args.GetIsolate());
        int r{};
        const int args_len = args.Length();
        for (int i = 0; i < args_len; i++)
        {
            r += args[i]->ToInt32()->Value();
        }

        rv::set_return(args, r);
    });

*/
