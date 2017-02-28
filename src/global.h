#ifndef GLOBAL_H
#define GLOBAL_H

template<typename... Args> struct QtOverload { 
    template<typename C, typename R> 
    static constexpr auto of( R (C::*pmf)(Args...) ) -> decltype(pmf) { 
        return pmf;
    } 
};

#endif // GLOBAL_H
