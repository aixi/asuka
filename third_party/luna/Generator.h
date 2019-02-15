//
// Created by xi on 18-11-15.
//

#ifndef LUNA_GENERATOR_H
#define LUNA_GENERATOR_H

#include <string.h>
#include <vector>
#include <algorithm>
#include <luna/Value.h>


namespace luna
{

template <typename OutputStream>
class Generator
{
public:
    explicit Generator(OutputStream& os) :
        os_(os),
        stack_()
    {}

    void Null()
    {
        Prefix();
        os_.Put("null");
    }

    void Bool(bool b)
    {
        Prefix();
        os_.Put(b ? "true" : "false");
    }

    void Double(double d)
    {
        Prefix();
        char buf[32];
        int n = sprintf(buf, "%.17g", d);
        //add zero 0 -> 0.0
        auto iter = std::find_if_not(buf, buf + n, isdigit);
        if (iter == buf + n)
        {
            strcat(buf, ".0");
        }
        os_.Put(buf);
    }

    void String(std::string_view str)
    {
        Prefix();
        os_.Put('\"');
        for (char ch : str)
        {
            switch (ch)
            {
                case '\"':
                    os_.Put("\\\"");
                    break;
                case '\\':
                    os_.Put("\\\\");
                    break;
                case '\b':
                    os_.Put("\\b");
                    break;
                case '\f':
                    os_.Put("\\f");
                    break;
                case '\n':
                    os_.Put("\\n");
                    break;
                case '\r':
                    os_.Put("\\r");
                    break;
                case '\t':
                    os_.Put("\\t");
                    break;
                default:
                    if (ch < 0x20)
                    {
                        char buf[7] = {0};
                        sprintf(buf, "\\u%04x", ch);
                        os_.Put(buf);
                    }
                    else
                    {
                        os_.Put(ch);
                    }
            }
        }
        os_.Put('\"');
    }

    void StartArray()
    {
        Prefix();
        stack_.emplace_back(true);
        os_.Put('[');
    }

    void EndArray()
    {
        assert(!stack_.empty());
        assert(stack_.back().is_in_array);
        stack_.pop_back();
        os_.Put(']');
    }

    void Key(std::string_view key)
    {
        Prefix();
        os_.Put('\"');
        os_.Put(key);
        os_.Put('\"');
    }

    void StartObject()
    {
        Prefix();
        stack_.emplace_back(false);
        os_.Put('{');
    }

    void EndObject()
    {
        assert(!stack_.empty());
        assert(!stack_.back().is_in_array);
        stack_.pop_back();
        os_.Put('}');
    }


    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

private:
    void Prefix();

private:
    struct Level
    {
        explicit Level(bool is_in_array_) :
            is_in_array(is_in_array_),
            value_count(0)
        {}

        bool is_in_array;
        int value_count;
    };

    OutputStream& os_;

    std::vector<Level> stack_;
}; //class Generator

template <typename T>
void Generator<T>::Prefix()
{
    if (stack_.empty())
    {
        return;
    }
    auto& top = stack_.back();
    if (top.is_in_array)
    {
        if (top.value_count > 0)
        {
            os_.Put(',');
        }
    }
    else //in object
    {
        if (top.value_count % 2 == 1)
        {
            os_.Put(':');
        }
        else if (top.value_count > 0)
        {
            os_.Put(',');
        }
    }
    ++top.value_count;
}

} //namespace luna

#endif //LUNA_GENERATOR_H
