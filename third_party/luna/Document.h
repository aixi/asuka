//
// Created by xi on 18-11-16.
//

#ifndef LUNA_DOCUMENT_H
#define LUNA_DOCUMENT_H

#include <luna/Value.h>
#include <luna/Parser.h>
#include <luna/StringInputStream.h>

namespace luna
{

class Document : public Value
{
public:

    Document();

    Parser::Status Parse(std::string_view json)
    {
        StringInputStream is(json);
        return ParseStream(is);
    }

public: //Handler

    void Null()
    {
        AddValue(Value(ValueType::kNull));
    }

    void Bool(bool b)
    {
        AddValue(Value(b));
    }

    void Double(double d)
    {
        AddValue(Value(d));
    }

    void String(std::string_view sv)
    {
        AddValue(Value(sv));
    }

    void Key(std::string_view k)
    {
        AddValue(Value(k));
    }

    void StartArray()
    {
        stack_.emplace_back(AddValue(Value(ValueType::kArray)));
    }

    void EndArray()
    {
        assert(!stack_.empty() && stack_.back().GetType() == ValueType::kArray);
        stack_.pop_back();
    }

    void StartObject()
    {
        stack_.emplace_back(AddValue(Value(ValueType::kObject)));
    }

    void EndObject()
    {
        assert(!stack_.empty() && stack_.back().GetType() == ValueType::kObject);
        stack_.pop_back();
    }



private:

    template <typename ReadStream>
    Parser::Status ParseStream(ReadStream& is)
    {
        return Parser::Parse(is, *this);
    }

    Value* AddValue(Value&& value);

    struct Level
    {
        explicit Level(Value* value_) :
            value(value_),
            value_count(0)
        {}

        ValueType GetType() const
        {
            return value->GetType();
        }

        Value* LastValue()
        {
            if (GetType() == ValueType::kArray)
            {
                return &value->a_->data.back();
            }
            else
            {
                return &value->o_->data.back().value;
            }
        }

        Value* value;
        int value_count;
    };

    std::vector<Level> stack_;
    Value key_;

}; //class Document

} //namespace luna

#endif //LUNA_DOCUMENT_H