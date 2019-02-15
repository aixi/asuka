//
// Created by xi on 18-11-23.
//

#ifndef LUNA_PRETTYGENERATOR_H
#define LUNA_PRETTYGENERATOR_H

#include <luna/Generator.h>

namespace luna
{

//FIXME: this class will generate ill indent json

template <typename OutputStream>
class PrettyGenerator
{
public:
    explicit PrettyGenerator(OutputStream& os, std::string_view indent = "    ");

    void Null()
    {
        generator_.Null();
        Indent();
    }

    void Bool(bool b)
    {
        generator_.Bool(b);
        Indent();
    }

    void Double(double d)
    {
        generator_.Double(d);
        Indent();
    }

    void String(std::string_view str)
    {
        generator_.String(str);
        Indent();
    }


    void StartArray()
    {
        generator_.StartArray();
        ++indent_level_;
        Indent();
    }

    void EndArray()
    {
        --indent_level_;
        Indent();
        generator_.EndArray();
    }

    void Key(std::string_view str)
    {
        generator_.Key(str);
    }

    void StartObject()
    {
        generator_.StartObject();
        ++indent_level_;
        Indent();
    }

    void EndObject()
    {
        --indent_level_;
        Indent();
        generator_.EndObject();
    }

    PrettyGenerator(const PrettyGenerator&) = delete;
    PrettyGenerator& operator=(const PrettyGenerator) = delete;

private:
    void PutNewline()
    {
        //FIXME: windows's line break is different to linux's
        os_.Put('\n');
    }

    void PutIndent()
    {
        os_.Put(indent_, indent_level_);
    }


    void Indent()
    {
        if (indent_level_ > 0)
        {
            PutNewline();
            PutIndent();
        }
    }

private:
    OutputStream& os_;
    Generator<OutputStream> generator_;
    int indent_level_;
    std::string_view indent_;
};

template <typename OutputStream>
PrettyGenerator<OutputStream>::PrettyGenerator(OutputStream& os, std::string_view indent) :
    os_(os),
    generator_(os_),
    indent_level_(0),
    indent_(indent)
{}

}

#endif //LUNA_PRETTYGENERATOR_H
