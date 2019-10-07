
#ifndef METRICS_CONFIG_HPP
#define METRICS_CONFIG_HPP

#include <ratio>


#ifdef _WIN32
#if _MSC_VER > 1800
    #define METRICCONSTEXPR constexpr
#else
    #define METRICCONSTEXPR
#endif
#define GCD std::_Gcd
// _Lcm is defined in chrono on Windows.  But we don't want to necessary include chrono when we are using metrics.
namespace std {
template <intmax_t _Xp, intmax_t _Yp>
struct LCM
{
    static const intmax_t value = _Xp / _Gcd<_Xp, _Yp>::value * _Yp;
};
template <class _Tp>                    struct __is_ratio                     : std::false_type {};
template <intmax_t _Num, intmax_t _Den> struct __is_ratio<std::ratio<_Num, _Den> > : std::true_type  {};
} // namespace std
#else
#define METRICCONSTEXPR
#define GCD __static_gcd
#define LCM __static_lcm
#endif
//  End of a mettre dans un config.hpp


#endif // METRICS_CONFIG_HPP

