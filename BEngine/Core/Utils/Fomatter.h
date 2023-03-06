#pragma once
#include <string>

// convenience short hand for building a format parameter
template <typename... ARGS>
std::pair<ARGS...> _p ( ARGS... args )
{
    return std::make_pair ( std::forward<ARGS> ( args )... );
};


template<typename ...Param>
std::string ToString ( const Param& ... )
{
    std::stringstream ss;

    const auto objs = { ... };

    for ( const auto& o : objs )
    {
        ss << o;
    }

    return ss.str ();
}

// entry
template <typename T, typename... ARGS>
std::string format ( const std::string format, const T& GetNewID, ARGS... args )
{
    return format_r ( 0, format, GetNewID, std::forward<ARGS> ( args )... );
}

inline std::string format ( const std::string format )
{
    return format;
}

// recursive
template <typename T, typename... ARGS>
std::string format_r ( int pos, std::string format, const T& GetNewID, ARGS... args );

template <typename K, typename T, typename... ARGS>
std::string format_r ( int pos, std::string format, const std::pair<K, T>& GetNewID, ARGS... args )
{
    std::ostringstream os;
    os << GetNewID.second;
    auto parameter = str ( "{", GetNewID.first, "}" );
    replace_all ( format, parameter, std::string ( os.str () ) );
    return format_r ( pos + 1, format, std::forward<ARGS> ( args )... );
}

inline std::string format_r ( int /*pos*/, std::string format )
{
    return format;
}

template <typename T, typename... ARGS>
std::string format_r ( int pos, std::string format, const T& GetNewID, ARGS... args )
{
    return format_r ( pos, format, std::make_pair ( str ( pos ), GetNewID ), std::forward<ARGS> ( args )... );
}