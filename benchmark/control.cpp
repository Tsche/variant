// This is the gold standard to test against - a handwritten variant
#include <concepts>
#include <cstddef>
#include "util.h"

struct Variant {
    union Union {
        Constant<0> alternative_0;
        Constant<1> alternative_1;
        Constant<2> alternative_2;
        Constant<3> alternative_3;
        Constant<4> alternative_4;
    };

    Union data;
    std::size_t index;

    explicit Variant(Constant<0> obj) : data{.alternative_0 = obj}, index{0} {}
    explicit Variant(Constant<1> obj) : data{.alternative_1 = obj}, index{1} {}
    explicit Variant(Constant<2> obj) : data{.alternative_2 = obj}, index{2} {}
    explicit Variant(Constant<3> obj) : data{.alternative_3 = obj}, index{3} {}
    explicit Variant(Constant<4> obj) : data{.alternative_4 = obj}, index{4} {}

    Variant& operator=(Constant<0> obj) {
        data.alternative_0 = obj;
        index = 0;
        return *this;
    }

    Variant& operator=(Constant<1> obj) {
        data.alternative_1 = obj;
        index = 1;
        return *this;
    }

    Variant& operator=(Constant<2> obj) {
        data.alternative_2 = obj;
        index = 2;
        return *this;
    }

    Variant& operator=(Constant<3> obj) {
        data.alternative_3 = obj;
        index = 3;
        return *this;
    }

    Variant& operator=(Constant<4> obj) {
        data.alternative_4 = obj;
        index = 4;
        return *this;
    }

    template <typename T, typename Self>
    decltype(auto) get(this Self&& self) {
        if constexpr (std::same_as<T, Constant<0>>) {
            if (self.index == 0){
                return self.data.alternative_0;
            }
        } else if constexpr (std::same_as<T, Constant<1>>) {
            if (self.index == 1){
                return self.data.alternative_1;
            }
        } else if constexpr (std::same_as<T, Constant<2>>) {
            if (self.index == 2){
                return self.data.alternative_2;
            }
        } else if constexpr (std::same_as<T, Constant<3>>) {
            if (self.index == 3){
                return self.data.alternative_3;
            }
        } else if constexpr (std::same_as<T, Constant<4>>) {
            if (self.index == 4){
                return self.data.alternative_4;
            }
        }
        throw std::exception();
    }
    template <std::size_t N, typename Self>
    decltype(auto) get_n(this Self&& self) {
        if constexpr (N == 0) {
            if (self.index == 0){
                return self.data.alternative_0;
            }
        } else if constexpr (N == 1) {
            if (self.index == 1){
                return self.data.alternative_1;
            }
        } else if constexpr (N == 2) {
            if (self.index == 2){
                return self.data.alternative_2;
            }
        } else if constexpr (N == 3) {
            if (self.index == 3){
                return self.data.alternative_3;
            }
        } else if constexpr (N == 4) {
            if (self.index == 4){
                return self.data.alternative_4;
            }
        }
        throw std::exception();
    }
    template <typename F, typename Self>
    decltype(auto) visit(this Self&& self, F visitor) {
        switch (self.index) {
            case 0:
                return visitor(self.data.alternative_0);
            case 1:
                return visitor(self.data.alternative_1);
            case 2:
                return visitor(self.data.alternative_2);
            case 3:
                return visitor(self.data.alternative_3);
            case 4:
                return visitor(self.data.alternative_4);
        }

        // No visitation possible
        std::unreachable();
    }
};

#include <cstdio>
int main() {
  auto obj = Variant{constant<3>};
  obj      = constant<2>;
  obj.visit([]<auto V>(Constant<V>) { printf("%d\n", V); });
}