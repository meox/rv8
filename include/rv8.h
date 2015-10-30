#pragma once

/**
 * C++ Italian MeetUp, 24 Ottobre Rome
 * written by: Gian Lorenzo Meocci <glmeocci@gmail.com>
 */


#include <vector>
#include <memory>
#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include <include/allocator.h>
#include <include/v8-extended-util.h>

using namespace v8;


using js_callinfo_t = const v8::FunctionCallbackInfo<v8::Value>&;
using js_function_t = std::function<void(js_callinfo_t args)>;

template <typename T>
using rv_function_t = std::function<void(js_callinfo_t args, T* obj)>;


namespace rv
{
    inline Local<String> string(Isolate* isolate, const std::string& str)
    {
        EscapableHandleScope handle_scope(isolate);

        Local<String> s = String::NewFromUtf8(isolate, str.c_str(), NewStringType::kNormal).ToLocalChecked();
        return handle_scope.Escape(s);
    }

    inline void set_return(js_callinfo_t args, const std::string& r)
    {
        args.GetReturnValue().Set(rv::string(args.GetIsolate(), r));
    }

    inline void set_return(js_callinfo_t args, int r)
    {
        args.GetReturnValue().Set(Integer::New(args.GetIsolate(), r));
    }
}


class RV8
{
public:

    RV8()
    {
        V8::InitializeICU();
        V8::InitializeExternalStartupData("rv8");
        platform = platform::CreateDefaultPlatform();
        V8::InitializePlatform(platform);
        V8::Initialize();

        // Create a new Isolate and make it the current one
        create_params.array_buffer_allocator = &allocator;
        engine_isolate = Isolate::New(create_params);

        Isolate::Scope isolate_scope(engine_isolate);

        // Create a stack-allocated handle scope
        HandleScope handle_scope(engine_isolate);

        // Create a template for the global object
        Local<ObjectTemplate> local_global = ObjectTemplate::New(engine_isolate);

        global.Reset(engine_isolate, local_global);
        context.Reset(engine_isolate, v8::Context::New(engine_isolate, NULL, local_global));
    }


    void add_global_function(const std::string& fun_name, js_function_t f)
    {
        js_function_t* cfun = new js_function_t(f);
        global_functions.push_back(cfun);

        HandleScope handle_scope(engine_isolate);
        Local<ObjectTemplate> local_global = Local<ObjectTemplate>::New(engine_isolate, global);

        local_global->Set(
            rv::string(engine_isolate, fun_name),
            v8::FunctionTemplate::New(engine_isolate, [](js_callinfo_t args) -> void {
                Isolate *isolate = args.GetIsolate();
                HandleScope handle_scope(isolate);

                Handle<External> data = Handle<External>::Cast(args.Data());

                js_function_t *fun = static_cast<js_function_t*>(data->Value());
                (*fun)(args);

            }, v8::External::New(engine_isolate, cfun))
        );
        context.Reset(engine_isolate, v8::Context::New(engine_isolate, NULL, local_global));
    }


    Isolate* get_isolate() { return engine_isolate; }
    Persistent<Context>& get_context() { return context; }


    void execute(const std::string& script_fname)
    {
        Isolate::Scope isolate_scope(engine_isolate);

        // Create a stack-allocated handle scope
        HandleScope handle_scope(engine_isolate);

        // Enter the context for compiling and running the hello world script
        Local<Context> local_context = Local<Context>::New(engine_isolate, context);
        Context::Scope context_scope(local_context);

        // Create a string containing the JavaScript source code
        Local<String> source = rv::string(engine_isolate, load_fromfile(script_fname));

        TryCatch try_catch(engine_isolate);
        ScriptOrigin origin(rv::string(engine_isolate, script_fname));
        // Compile the source code
        Local<Script> script;
        if (!v8::Script::Compile(local_context, source, &origin).ToLocal(&script))
        {
            ReportException(engine_isolate, &try_catch);
        }
        else
        {
            // Run the script to get the result
            auto result = script->Run(local_context);

            if (result.IsEmpty())
            {
                ReportException(engine_isolate, &try_catch);
            }
            else
            {
                // Convert the result to an UTF8 string and print it
                //String::Utf8Value utf8(result);
                //printf("%s\n", *utf8);
                std::cerr << "* END *" << std::endl;
            }
        }
    }


    template <typename ...Args>
    bool invoke_jsfun(const std::string& function_name, Args&& ...args)
    {
        HandleScope handle_scope(engine_isolate);
        Local<String> jf_name = rv::string(engine_isolate, function_name);
        Local<Value> jsfun;

        Local<Context> _context = Local<Context>::New(engine_isolate, context);
        if (
            !_context->Global()->Get(_context, jf_name).ToLocal(&jsfun)
            ||
            !jsfun->IsFunction()
        )
        {
            return false;
        }

        const size_t args_len = sizeof...(args);
        Handle<Value> arr[args_len];

        Local<Function> fun = Local<Function>::Cast(jsfun);
        fill_parameter(arr, 0, std::forward<Args>(args)...);
        fun->Call(_context, _context->Global(), args_len, arr);

        return true;
    }


    ~RV8()
    {
        for (auto f : global_functions)
            delete f;

        engine_isolate->Dispose();
        V8::Dispose();
        V8::ShutdownPlatform();
        delete platform;
    }

private:
    template <typename ...Args>
    void fill_parameter(Handle<Value>* arr, uint32_t p, int v, Args&& ...args)
    {
        Local<Integer> jv = Integer::New(engine_isolate, v);
        arr[p] = jv;
        fill_parameter(arr, p+1, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    void fill_parameter(Handle<Value>* arr, uint32_t p, std::string v, Args&& ...args)
    {
        Local<String> jv = rv::string(engine_isolate, v);
        arr[p] = jv;
        fill_parameter(arr, p+1, std::forward<Args>(args)...);
    }

    void fill_parameter(Handle<Value>* arr, uint32_t p)
    {
    }

private:
    ArrayBufferAllocator allocator;
    Isolate::CreateParams create_params;
    Isolate* engine_isolate;
    Platform* platform;
    Persistent<ObjectTemplate> global;
    Persistent<Context> context;
    std::vector<js_function_t*> global_functions;
};



template <typename T>
class RVObject
{
public:
    using holder_t = std::pair<rv_function_t<T>, T*>*;

    RVObject(RV8& rv8_engine) : engine{rv8_engine}
    {
        Isolate *isolate = engine.get_isolate();
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        // Enter the context for compiling and running the hello world script
        Local<Context> local_context = Local<Context>::New(isolate, engine.get_context());
        Context::Scope context_scope(local_context);

        v8::Local<v8::Object> obj = Object::New(isolate);
        p_obj.Reset(isolate, obj);
    }

    void add_function(const std::string& fun_name, rv_function_t<T> fun, T* c_obj)
    {
        Isolate *isolate = engine.get_isolate();
        HandleScope handle_scope(isolate);

        holder_t params = new std::pair<rv_function_t<T>, T*>(move(fun), c_obj);
        v_holder.push_back(params);

        Local<Object> local_obj = Local<Object>::New(isolate, p_obj);
        local_obj->Set(
            rv::string(isolate, fun_name),
            v8::FunctionTemplate::New(isolate, [](js_callinfo_t args) -> void {
                Isolate *isolate = args.GetIsolate();
                HandleScope handle_scope(isolate);

                Handle<External> data = Handle<External>::Cast(args.Data());

                auto params = static_cast<holder_t>(data->Value());
                rv_function_t<T> fun = std::get<0>(*params);
                T* c_obj = std::get<1>(*params);
                fun(args, c_obj);

            }, v8::External::New(isolate, params))->GetFunction()
        );
    }

    Local<Object> get_object()
    {
        Isolate *isolate = engine.get_isolate();
        EscapableHandleScope handle_scope(isolate);

        Local<Object> obj = Local<Object>::New(isolate, p_obj);
        return handle_scope.Escape(obj);
    }

    void deleter(T* obj)
    {
        p_obj.SetWeak(obj, [](Persistent<Value> object, void *parameter){
            T *c_obj = static_cast<T*>(parameter);
            std::cerr << "deleting ..." << std::endl;
        });
    }

    virtual ~RVObject()
    {
        for (auto e : v_holder)
            delete e;
    }

private:
    RV8& engine;
    Persistent<Object> p_obj;
    std::vector<holder_t> v_holder;
};

