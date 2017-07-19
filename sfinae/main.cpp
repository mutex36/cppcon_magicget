//
//  main.cpp
//  sfinae
//
//  Created by Luis von der Eltz on 13.05.17.
//  Copyright © 2017 Luis von der Eltz. All rights reserved.
//

#include <iostream>

#include <string>
#include <type_traits>
#include <array>

namespace Serialization
{
    namespace Detail
    {
        
#define CR_SERIALIZATION_MAX_FIELDS 10
#define CR_SERIALIZATION_TYPE(TYPE, ID)          \
template<>                                       \
constexpr size_t type_id<TYPE>() noexcept        \
{                                                \
    return ID;                                   \
}                                                \
    
        template<typename T>
        constexpr size_t type_id() noexcept
        {
            return 0;
        }
        CR_SERIALIZATION_TYPE(char, 1);
        CR_SERIALIZATION_TYPE(unsigned char, 2);
        CR_SERIALIZATION_TYPE(int, 3);
        CR_SERIALIZATION_TYPE(unsigned int, 4);
        CR_SERIALIZATION_TYPE(long long, 5);
        CR_SERIALIZATION_TYPE(unsigned long long, 6);
        CR_SERIALIZATION_TYPE(float, 7);
        CR_SERIALIZATION_TYPE(double, 8);
        CR_SERIALIZATION_TYPE(size_t, 9);
        
        struct type
        {
            size_t hash;
            size_t index;
            size_t size;
        };
        
        template<size_t I>
        struct ubiq
        {
            template<typename T>
            constexpr operator T() const noexcept
            {
                return T{};
            }
        };
        
        template<size_t I>
        struct ubiq_val
        {
            type* type_list;
            
            template<typename T, typename D = typename std::decay<T>::type>
            constexpr operator T() const noexcept
            {
                auto& tp = type_list[I];
                tp.hash = type_id<D>();
                tp.index = I;
                tp.size = sizeof(T);
                
                return T{};
            }
        };
        
        template<typename T, size_t I0, size_t...I>
        constexpr auto count_fields(size_t& out, std::index_sequence<I0, I...>)
            -> decltype(T{ ubiq<I0>{}, ubiq<I>{}... })
        {
            out = sizeof...(I) + 1;
            return {};
        }
        
        template<typename T, size_t...I>
        constexpr void count_fields(size_t& out, std::index_sequence<I...>)
        {
            count_fields<T>(out, std::make_index_sequence<sizeof...(I) - 1>{});
        }
        
        template<typename T, size_t...N>
        constexpr auto get_types(std::array<type, sizeof...(N)>& out, std::index_sequence<N...>)
            -> decltype(T{ ubiq<N>{}... })
        {
            T temp{ubiq_val<N>{out.data()}...};
            return temp;
        }
    }
 
    template<typename T>
    constexpr size_t GetFieldCount()
    {
        size_t r{0};
        Detail::count_fields<T>(r, std::make_index_sequence<CR_SERIALIZATION_MAX_FIELDS>{});
        return r;
    }
    
    template<typename T>
    constexpr auto GetTypes()
    {
        std::array<Detail::type, GetFieldCount<T>()> types;
        Detail::get_types<T>(types, std::make_index_sequence<types.size()>{});
        return types;
    }
}

struct test
{
    size_t a;
    float b;
    unsigned int ff;
    double g;
    unsigned int* dddf;
};

int main(int argc, const char * argv[])
{

    std::cout << "Struct has " << Serialization::GetFieldCount<test>() << " fields." << std::endl;

    for (auto& type : Serialization::GetTypes<test>())
    {
        std::cout << "Field {";
        std::cout << "Type: " << type.name;
        std::cout << ", Size: " << type.size;
        std::cout << ", Index: " << type.index;
        std::cout << ", Hash: " << type.hash;
        std::cout << "}" << std::endl;
    }
    
    return 0;
}
