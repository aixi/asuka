//
// Created by xi on 18-11-22.
//

#ifndef LUNA_STRINGWRITESTREAM_H
#define LUNA_STRINGWRITESTREAM_H

#include <vector>
#include <string_view>

namespace luna
{

class StringOutputStream
{
public:

    StringOutputStream() = default;

    void Put(char ch)
    {
        buffer_.push_back(ch);
    }

    void Put(std::string_view str)
    {
        buffer_.insert(buffer_.end(), str.begin(), str.end());
    }

    void Put(std::string_view str, int repeat)
    {
        for (int i = 0; i < repeat; ++i)
        {
            Put(str);
        }
    }

    std::string_view GetStringView() const
    {
        return std::string_view(buffer_.data(), buffer_.size());
    }

    StringOutputStream(const StringOutputStream&) = delete;
    StringOutputStream& operator=(const StringOutputStream&) = delete;

private:
    std::vector<char> buffer_;
};

}

#endif //LUNA_STRINGWRITESTREAM_H
