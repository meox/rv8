#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <include/v8.h>
#include <include/v8-extended-util.h>

using namespace v8;
using namespace std;



std::string load_fromfile(const string& fname)
{
    ifstream f(fname);
    if (!f)
    {
        throw std::runtime_error(string{"file "} + fname + " doesn't exists");
    }

    stringstream file_content;
    char s[1025];
    while (!f.eof())
    {
        auto n = f.readsome(s, 1024);
        s[n] = 0;
        file_content << s;
        if (n < 1024)
            break;
    }

    return file_content.str();
}


void pretty_print(std::stringstream& stream, Value* v)
{
    if (v->IsArray())
    {
        auto arr = Array::Cast(v);
        bool first{true};
        stream << "[";
        for (uint32_t i = 0; i < arr->Length(); i++)
        {
            if (first)
                first = false;
            else
                stream << ",";

            auto element = arr->Get(i);
            pretty_print(stream, *element);
        }
        stream << "]";
    }
    else
    {
        String::Utf8Value str(v->ToString());
        stream << *str;
    }
}


void pretty_print(Value* v)
{
    stringstream stream;
    pretty_print(stream, v);

    cout << stream.str();
}


void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch)
{
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value exception(try_catch->Exception());
    string exception_string{*exception};

    v8::Local<v8::Message> message = try_catch->Message();
    if (message.IsEmpty())
    {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        cerr << exception_string << endl;
    }
    else
    {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
        v8::Local<v8::Context> context(isolate->GetCurrentContext());
        string filename_string{*filename};
        int linenum = message->GetLineNumber(context).FromJust();
        cerr << filename_string << ":" << linenum << ":" << exception_string << endl;

        // Print line of source code.
        v8::String::Utf8Value sourceline(message->GetSourceLine(context).ToLocalChecked());
        cerr << *sourceline << endl;
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++)
        {
            cerr << " " << endl;
        }

        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++)
        {
            cerr << "^" << endl;
        }
        cerr << endl;
        
        v8::String::Utf8Value stack_trace(try_catch->StackTrace(context).ToLocalChecked());
        if (stack_trace.length() > 0)
        {
            string stack_trace_string{*stack_trace};
            cerr << stack_trace_string << endl;
        }
    }
}

