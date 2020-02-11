#ifndef LIBIM_UTILS_H
#define LIBIM_UTILS_H
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <string>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>


#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)

// Annonymous variable
#ifdef _MSC_VER
# define ANOVAR(str) CONCATENATE(str, __COUNTER__)
#else
# define ANOVAR(str) CONCATENATE(str, __LINE__)
#endif

namespace libim::utils {
    namespace detail {
        inline bool compare_char(char c1, char c2)
        {
            if(c1 == c2) {
                return true;
            }
            else if (std::toupper(c1) == std::toupper(c2)) {
                return true;
            }
            return false;
        }

        [[nodiscard]] inline std::string trim(const char* str, std::size_t len)
        {
            std::size_t end = 0;
            while(end++ < len) {
                if(str[end] == '\0') {
                    break;
                }
            }

            return std::string(str, end);
        }

        inline bool strcpy(char* dest, std::size_t d_size, const std::string_view src)
        {
            if(d_size < src.size()) {
                return false;
            }

        #if defined(__STDC_LIB_EXT1__) || defined(_MSC_VER)
            return strncpy_s(dest, d_size, src.data(), src.size()) == 0;
        #else
            return std::strncpy(dest, src.data(), d_size) != nullptr;
        #endif
        }

        template <typename T>
        auto promote_to_printable_integer_type(T i) -> decltype(+i)
        {
            // Make char type printable as number and not as character.
            static_assert (std::is_arithmetic_v<T>);
            return +i;
        }

    } // detail


    // Helper type for the visitor
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

    template <typename Lambda>
    [[nodiscard]] auto at_scope_exit(Lambda&& callback)
    {
        struct at_exit_scope_t final
        {
            at_exit_scope_t(Lambda&& cb) :
                cb_(std::forward<Lambda>(cb)) {}
            ~at_exit_scope_t() {
               cb_();
            }
        private:
            Lambda cb_;
        };

        return at_exit_scope_t { std::forward<Lambda>(callback) };
    }

    #ifdef AT_SCOPE_EXIT
    #  error macro AT_SCOPE_EXIT already defined
    #endif

    #define AT_SCOPE_EXIT(...) \
        const auto ANOVAR(sexit_) = libim::utils::at_scope_exit(__VA_ARGS__)



    template <typename T,
              typename TIter = decltype(std::begin(std::declval<T>())),
              typename = decltype(std::end(std::declval<T>()))>
    constexpr auto enumerate(T&& iterable)
    {
        struct iterator
        {
            size_t i;
            TIter iter;
            bool operator != (const iterator & other) const { return iter != other.iter; }
            void operator ++ () { ++i; ++iter; }
            auto operator * () const { return std::tie(std::as_const(i), std::as_const(*iter)); }
            auto operator * () { return std::tie(std::as_const(i), *iter); }
        };

        struct iterable_wrapper
        {
            T iterable;
            auto begin() { return iterator{ 0, std::begin(iterable) }; }
            auto end() { return iterator{ 0, std::end(iterable) }; }
            auto begin() const { return iterator{ 0, std::begin(iterable) }; }
            auto end() const { return iterator{ 0, std::end(iterable) }; }
        };

        return iterable_wrapper{ std::forward<T>(iterable) };
    }

    template<typename T>
    constexpr auto cenumerate(T&& iterable)
    {
        return enumerate<std::add_const_t<T>,
                decltype(std::cbegin(std::declval<T>()))>(
                    std::forward<std::add_const_t<T>>(iterable)
        );
    }


    template<typename T>
    typename T::iterator copy(typename T::iterator s_begin, std::size_t count, T& dest)
    {
        dest.reserve(count);
        auto s_end = std::next(s_begin, count);
        std::copy(s_begin, s_end, std::back_inserter(dest));
        return s_end;
    }


    /* Case insensitive comparison of two strings */
    [[nodiscard]] inline bool iequal(const std::string& s1, const std::string& s2)
    {
        return ((s1.size() == s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), &detail::compare_char)
        );
    }

    [[nodiscard]] inline bool iequal(std::string_view s1, std::string_view s2)
    {
        return ((s1.size() == s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), &detail::compare_char)
        );
    }


    template<typename T>
    [[nodiscard]] inline constexpr auto to_underlying(T t)
    {
        static_assert(std::is_enum_v<T>, "T must be enum type");
        using U = std::underlying_type_t<T>;
        return static_cast<U>(t);
    }

    template<typename T, typename = void>
    struct underlying_type { using type = T; };

    template<typename T>
    struct underlying_type<T, std::enable_if_t<std::is_enum_v<T>>> {
        using type = std::underlying_type_t<T>;
    };

    template<typename T>
    using underlying_type_t = typename underlying_type<T>::type;


    // Returns number of digits in the number.
    template<typename T>
    [[nodiscard]] inline std::size_t numdigits(T i)
    {
        static_assert (std::is_integral_v<T> && std::is_unsigned_v<T>, "T must be unsigned integral type");
        return i > 0 ? static_cast<std::size_t>(std::log10(i)) + 1 : 1;
    }


    template<std::size_t base = 10, std::size_t precision = 0, typename T>
    [[nodiscard]] static std::string to_string(T n)
    {
        static_assert(base == 8 || base == 10 || base == 16, "invalid encoding base");
        static_assert(std::is_arithmetic_v<T>, "T is not a arithmetic type");
        static_assert(!std::is_floating_point_v<T> || base == 10,
            "floating point can only be represented in base 10"
        );

        std::stringstream ss;
        ss.exceptions(std::ios::failbit);

        if constexpr(base == 8) {
            ss << std::oct << std::showbase;
        }
        else if constexpr (base == 10) {
            ss << std::dec;
        }
        else
        {
            ss << "0x"
               << std::uppercase
               << std::hex;
        }

        if constexpr(precision != 0)
        {
            ss << std::setw(precision)
               << std::setfill('0')
               << std::fixed
               << std::setprecision(precision);
        }

        ss << detail::promote_to_printable_integer_type(n);
        return ss.str();
    }


    template<std::size_t N>
    inline bool strcpy(char (&dest)[N], const std::string_view src)
    {
        return detail::strcpy(dest, N, src);
    }

    template<std::size_t N>
    inline bool strcpy(std::array<char, N>& dest, const std::string_view src)
    {
        return detail::strcpy(dest.data(), N, src);
    }


    template<std::size_t N>
    [[nodiscard]] inline std::string trim(const char (&str)[N])
    {
        return detail::trim(str, N);
    }

    template<std::size_t N>
    [[nodiscard]] inline std::string trim(const std::array<char, N>& str)
    {
        return detail::trim(str.data(), N);
    }


    // Transforms inputed string to all lower case charactes.
    inline void to_lower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c){ return std::tolower(c); }
        );
    }

    // Checks if string s ends with x
    [[nodiscard]] inline bool ends_with(const std::string& s, std::string_view x)
    {
        return s.size() >= x.size() && s.compare(s.size() - x.size(), s.npos, x) == 0;
    }

    // Case Insensitive check if string s ends with x
    [[nodiscard]] inline bool iends_with(const std::string& s, std::string_view x)
    {
        auto it = x.begin();
        return s.size() >= x.size() &&
            std::all_of(std::next(s.begin(), s.size() - x.size()), s.end(),
                [&it](const char & c){
                    return ::tolower(c) == ::tolower(*(it++));
            });
    }
}

#endif // LIBIM_UTILS_H
