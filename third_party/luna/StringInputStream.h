//
// Created by xi on 18-11-15.
//

#ifndef LUNA_STRINGREADSTREAM_H
#define LUNA_STRINGREADSTREAM_H

#include <assert.h>
#include <string_view>

namespace luna
{

class StringInputStream
{
public:
    using Iterator = std::string_view::const_iterator;

    explicit StringInputStream(std::string_view json) :
        json_(json),
        iter_(json_.begin())
    {}

    bool HasNext() const
    {
        return iter_ != json_.end();
    }

    char Peek() const
    {
        return HasNext() ? *iter_ : '\0';
    }

    Iterator GetIter() const
    {
        return iter_;
    }

    char Next()
    {
        char ch = Peek();
        if (HasNext())
        {
            ++iter_;
            return ch;
        }
        else
        {
            return '\0';
        }

    }

    void Expect(char ch)
    {
        assert(ch == Peek());
        Next();
    }

    //noncopyable
    StringInputStream(const StringInputStream&) = delete;
    StringInputStream& operator=(const StringInputStream&) = delete;

private:
    std::string_view json_;
    Iterator iter_;
};

} //namespace luna

#endif //LUNA_STRINGREADSTREAM_H
