#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <stdexcept>

enum class Error
{
    NoError,
    CorruptedArchive
};

static constexpr char Separator = ' ';
static constexpr const char *True = "true";
static constexpr const char *False = "false";

class Serializer
{
    std::ostream &out_;
public:
    explicit Serializer(std::ostream& out)
        : out_(out)
    {
    }

    template <class T>
    Error save(const T& object)
    {
        return object.serialize(*this);
    }

    template <class... ArgsT>
    Error operator()(ArgsT... args)
    {
        return process(args...);
    }

private:
    Error process(bool arg)
    {
        if (arg)
            out_ << True;
        else
            out_ << False;
        return Error::NoError;
    }

    Error process(uint64_t arg)
    {
        out_ << arg;
        return Error::NoError;
    }
    
    template <class T, class... ArgsT>
    Error process(T arg, ArgsT... args)
    {
        Error e = process(arg);
        if (e == Error::NoError) {
            out_ << Separator;
            return process(std::forward<ArgsT>(args)...);
        } else {
            return e;
        }
    }               
};


class Deserializer
{
    std::istream &in_;
public:
    explicit Deserializer(std::istream &in)
        : in_(in)
    {
    }

    template <class T>
    Error load(T& object)
    {
        return object.deserialize(*this);
    }

    template <class... ArgsT>
    Error operator()(ArgsT&... args)
    {
        return process(args...);
    }

private:
    Error process(bool &arg)
    {
        std::string s;
        in_ >> s;
        if (s == True)
            arg = true;
        else if (s == False)
            arg = false;
        else
            return Error::CorruptedArchive;
        return Error::NoError;
    }
    Error process(uint64_t &arg)
    {
        std::string s;
        in_ >> s;
        try {
            arg = std::stoull(s);
            return Error::NoError;
        } catch (std::invalid_argument) {
            return Error::CorruptedArchive;
        } catch (std::out_of_range) {
            return Error::CorruptedArchive;
        }
    }
    
    template <class T, class... ArgsT>
    Error process(T &arg, ArgsT&... args)
    {
        Error e = process(arg);
        if (e == Error::NoError) {
            int c = in_.get();
            if (c != Separator)
                return Error::CorruptedArchive;
            return process(args...);
        } else {
            return e;
        }
    }
};

