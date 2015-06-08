//
// Created by jim on 6/7/15.
//

#ifndef ordinal_h__
#define ordinal_h__
namespace bd {

template<typename T>
constexpr
std::size_t ordinal(T t)
{
    return static_cast<unsigned int>(t);
}

}

#endif // ordinal_h__
