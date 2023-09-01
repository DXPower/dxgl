#pragma once

#include <string_view>
#include <string>
#include <iterator>
#include <stdexcept>

namespace std {
    template<class CharT, class Traits>
    class basic_ostream;
}

namespace dxtl {
    template<typename CharT, typename Traits = std::char_traits<CharT>> 
    class basic_cstring_view : private std::basic_string_view<CharT, Traits> {
        using sv_t = std::basic_string_view<CharT, Traits>;
        using str_t = std::basic_string<CharT, Traits>;

    public:
        constexpr basic_cstring_view() noexcept = default;

        template<std::size_t N>
        constexpr basic_cstring_view(const CharT (&literal)[N]) noexcept
            : sv_t(literal, N) { }

        constexpr basic_cstring_view(const CharT* str)
            : sv_t(str) { }

        constexpr basic_cstring_view(const str_t& str) : sv_t(str) { }

        constexpr basic_cstring_view(sv_t str)
            : sv_t((str.remove_suffix(1), str))
        {
            if (*(&str.back() + 1) != '\0') {
                throw std::runtime_error("basic_cstring_view string_view parameter not null terminated");
            }
        }

        template<typename It, typename EndIt>
        constexpr basic_cstring_view(It first, It last)
            : sv_t(first, (first != last) ? std::prev(last) : last) 
        {
            if (first == last || *std::prev(last) != '\0') {
                throw std::runtime_error("basic_cstring_view last iterator not null terminated");
            }
        }

        constexpr basic_cstring_view(std::nullptr_t) = delete;

        using typename sv_t::traits_type;
        using typename sv_t::value_type;
        using typename sv_t::pointer;
        using typename sv_t::const_pointer;
        using typename sv_t::reference;
        using typename sv_t::const_reference;
        using typename sv_t::const_iterator;
        using typename sv_t::iterator;
        using typename sv_t::const_reverse_iterator;
        using typename sv_t::reverse_iterator;
        using typename sv_t::size_type;
        using typename sv_t::difference_type;

        using sv_t::begin;
        using sv_t::end;
        using sv_t::cbegin;
        using sv_t::cend;
        using sv_t::rbegin;
        using sv_t::rend;
        using sv_t::crbegin;
        using sv_t::crend;

        using sv_t::operator[];
        using sv_t::at;
        using sv_t::front;
        using sv_t::back;
        using sv_t::data;

        using sv_t::size;
        using sv_t::length;
        using sv_t::max_size;
        using sv_t::empty;

        using sv_t::remove_prefix;
        using sv_t::copy;
        using sv_t::substr;
        using sv_t::compare;
        using sv_t::starts_with;
        using sv_t::ends_with;
        using sv_t::find;
        using sv_t::rfind;
        using sv_t::find_first_of;
        using sv_t::find_first_not_of;
        using sv_t::find_last_of;
        using sv_t::find_last_not_of;

        using sv_t::npos;
        
        constexpr typename sv_t::const_pointer c_str() const {
            return data();
        }

        constexpr void swap(basic_cstring_view& rhs) {
            this->sv_t::swap(rhs);
        }

        friend std::basic_ostream<CharT, Traits>& operator<<(
            std::basic_ostream<CharT, Traits>& out,
            const basic_cstring_view& rhs
        ) {
            out << rhs;
            return out;
        }

        constexpr operator str_t() const {
            return str_t(begin(), end());
        }

        constexpr operator sv_t() const {
            return sv_t(begin(), end());
        }

        friend constexpr auto operator<=>(const basic_cstring_view& lhs, const basic_cstring_view& rhs) = default;

        friend constexpr auto operator<=>(const basic_cstring_view& lhs, const sv_t& rhs) {
            return ((sv_t) lhs) <=> rhs;
        };

        friend constexpr auto operator<=>(const sv_t& lhs ,const basic_cstring_view& rhs) {
            return lhs <=> (sv_t) rhs;
        };;
    };

    template<typename CharT, typename Traits>
    constexpr void swap(basic_cstring_view<CharT, Traits>& lhs, basic_cstring_view<CharT, Traits>& rhs) {
        lhs.swap(rhs);
    }

    using cstring_view = basic_cstring_view<char>;
}
