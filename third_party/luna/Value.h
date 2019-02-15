//
// Created by xi on 18-11-15.
//

#ifndef LUNA_VALUE_H
#define LUNA_VALUE_H

#include <stddef.h>
#include <assert.h>
#include <vector>
#include <atomic>
#include <memory>
#include <string>
#include <string_view>

namespace luna
{

class Document;
struct Member;

enum class ValueType
{
    kNull,
    kBool,
    kNumber,
    kString,
    kArray,
    kObject
};


class Value
{
    friend class Document;

public:
    using ObjectIterator = std::vector<Member>::iterator;
    using ConstObjectIterator = std::vector<Member>::const_iterator;

public:

    Value(const Value& rhs);

    Value& operator=(const Value& rhs);

    //FIXME: move semantic pass by value ?
    //NOTE: Effective Modern C++ item 42
    Value(Value&& rhs) noexcept;

    Value& operator=(Value&& rhs) noexcept;

    ~Value();

    explicit Value(ValueType type = ValueType::kNull);

    explicit Value(bool b);

    explicit Value(double d);

    explicit Value(std::string_view sv);

    bool IsNull() const
    {
        return type_ == ValueType::kNull;
    }

    bool IsBool() const
    {
        return type_ == ValueType::kBool;
    }

    bool IsNumber() const
    {
        return type_ == ValueType::kNumber;
    }

    bool IsString() const
    {
        return type_ == ValueType::kString;
    }

    bool IsArray() const
    {
        return type_ == ValueType::kArray;
    }

    bool IsObject() const
    {
        return type_ == ValueType::kObject;
    }


    ValueType GetType() const
    {
        return type_;
    }

    bool GetBool() const
    {
        assert(type_ == ValueType::kBool);
        return b_;
    }

    Value& SetBool(bool b)
    {
        //NOTE: effective C++ 3e Chinese version p.256
        //NOTE: We do not need assert, because the value's ValueType could change
        this->~Value();
        return *new (this) Value(b);
    }

    double GetDouble() const
    {
        assert(type_ == ValueType::kNumber);
        return d_;
    }

    Value& SetDouble(double d)
    {
        this->~Value();
        return *new (this) Value(d);
    }

    std::string_view GetStringView() const
    {
        assert(type_ == ValueType::kString);
        return std::string_view(s_->data.data(), s_->data.size());
    }

    std::string GetString() const
    {
        assert(type_ == ValueType::kString);
        return std::string(s_->data.data(), s_->data.size());
    }

    Value& SetString(std::string_view sv)
    {
        this->~Value();
        return *new (this) Value(sv);
    }

    Value& SetArray()
    {
        this->~Value();
        return *new (this) Value(ValueType::kArray);
    }


    const std::vector<Value>& GetArray() const
    {
        assert(type_ == ValueType::kArray);
        return a_->data;
    }

    const std::vector<Member>& GetObject() const
    {
        assert(type_ == ValueType::kObject);
        return o_->data;
    }

    const Value& operator[](size_t i) const
    {
        assert(type_ == ValueType::kArray);
        return a_->data[i];
    }

    Value& operator[](size_t i)
    {
        assert(type_ == ValueType::kArray);
        return a_->data[i];
    }

    template <typename T>
    Value& AddArrayElement(T&& value)
    {
        assert(type_ == ValueType::kArray);
        a_->data.emplace_back(std::forward<T>(value));
        return a_->data.back();
    }

    template <typename T>
    Value& AddObjectElement(const std::string_view& key, T&& value)
    {
        return AddObjectElement(Value(key), Value(std::forward<T>(value)));
    }

    Value& AddObjectElement(Value&& key, Value&& value);

    ObjectIterator FindObjectElement(std::string_view key);

    ConstObjectIterator FindObjectElement(std::string_view key) const;

    Value& operator[](std::string_view key);

    const Value& operator[](std::string_view key) const;

    template <typename Generator>
    void Generate(Generator& generator) const;

private:
    ValueType type_;

    template <typename T>
    struct SimpleRefCount
    {
        template <typename... Args>
        explicit SimpleRefCount(Args&&... args) :
            refs(1),
            data(std::forward<Args>(args)...)
        {}

        //NOTE: https://stackoverflow.com/questions/10693913/c11-anonymous-union-with-non-trivial-members
        ~SimpleRefCount()
        {
            assert(refs == 0);
        }

        int IncrAndGet()
        {
            //NOTE: The combination of atomic operations is not atomic
            //e.g. refs.store(refs.load(std::memory_order_relaxed) + 1);
            //FIXME: memory_order_relaxed ?
            return ++refs;
        }

        int DecrAndGet()
        {
            assert(refs > 0);
            //FIXME: memory_order_relaxed ?
            return --refs;
        }

        std::atomic<int> refs;
        T data;
    };

    using StringWithRefCount = SimpleRefCount<std::vector<char>>;
    using ArrayWithRefCount = SimpleRefCount<std::vector<Value>>;
    using ObjectWithRefCount = SimpleRefCount<std::vector<Member>>;

    //FIXME: std::variant ?

    union
    {
        bool b_;
        double d_;
        //NOTE: put the object of class SimpleRefCount as a data member of union will lead compile error
        //NOTE: because this class has non-trivial ctor, so we use it's pointer as union member
        StringWithRefCount* s_;
        ArrayWithRefCount* a_;
        ObjectWithRefCount* o_;
    };
};

struct Member
{
    Member(Value&& key_, Value&& value_) :
        key(std::move(key_)),
        value(std::move(value_))
    {}

    Member(std::string_view key_, Value&& value_) :
        key(key_),
        value(std::move(value_))
    {}

    Value key;
    Value value;
};

template <typename Generator>
void Value::Generate(Generator& generator) const
{
    switch (type_)
    {
        case ValueType::kNull:
            generator.Null();
            break;
        case ValueType::kBool:
            generator.Bool(b_);
            break;
        case ValueType::kNumber:
            generator.Double(d_);
            break;
        case ValueType::kString:
            generator.String(GetStringView());
            break;
        case ValueType::kArray:
        {
            generator.StartArray();
            const std::vector<Value>& array = GetArray();
            for (const Value& value : array)
            {
                value.Generate(generator);
            }
            generator.EndArray();
        }
            break;
        case ValueType::kObject:
        {
            generator.StartObject();
            const std::vector<Member>& object = GetObject();
            for (const Member& m : object)
            {
                generator.Key(m.key.GetStringView());
                m.value.Generate(generator);
            }
            generator.EndObject();
        }
            break;
    }
}

} //namespace luna

#endif //LUNA_VALUE_H
