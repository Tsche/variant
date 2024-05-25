# `slo::variant` - a fast variant

This C++23 library implements various tricks to speed up compile time of a variant type. While there are some slight differences, this library aims to fulfill most of the requirements specified in [[variant]](https://standards.pydong.org/c++23/variant). This was initially written as reference implementation for the techniques discussed in [a deep-dive blog post about unions](https://pydong.org/posts/implementing-variant/).

## Differences
### Storage
A variant is essentially just a record of a sum type and a discriminator. The most natural way to implement this in C++ is by defining variant as a non-union class-type that has two members: the actual union type and some small integer as discriminator. Unfortunately this can introduce additional, undesired padding.

However, if all alternative types are standard-layout types and the compiler has C++26 `std::is_within_lifetime` (or an intrinsic that can be used to implement it) we can potentially do a little better. If those conditions are met `slo::variant` will automatically prefer the inverted storage strategy. Essentially it works by wrapping every alternative in a struct type that has the tag as first data member - outside of constant evaluated context the tag can be inspected through any alternative (even inactive ones!) since they all share the same common initial sequence. Since we can't just access the tag through an inactive member In constant evaluated context, we need to do a bit more mental gymnastics. To retrieve the tag we must find the currently active alternative - `std::is_within_lifetime` can help with that. Once we have found the active member, we can access the tag through it.

### Underlying union layout
To generate the underlying union from a list of types, most implementations opt to define it as a recursive union type. Since this recursive type must be walked for a lot of operations (notably `std::get` and the in-place constructors), its recursion depth is a major contributor to bad compile times. Some libraries such as boost:variant2 alleviate that issue by partially specializing the recursive union for a bunch of alternatives at once if possible. This does flatten the recursion depth quite a bit, but it is still linear.

To get rid of crazy recursion depth with very large variants, `slo::Variant` switches strategy if instantiated with a lot of alternatives. The alternative strategy uses a complete binary tree layout for the underlying union instead. The trade-off is essentially a little bit of time to generate the tree once template gets instantiated for much quicker and also more consistently quick to compile `get`, in-place constructors and so on.

### Wrapping existing unions
Sometimes for some reason you already have a bare union. To allow you to use it safely, like you could've if it was a variant, the `slo::Union` alias template can be used to generate a variant from it. The `slo::Union` template takes pointers to data members instead of types - so instead of `slo::variant<int, char>` you would write `slo::Union<&Foo::member_1, &Foo::member_2>`.

The interesting thing about wrapping a bare union like this is that we now have a flat union type. None of the recursion worries matter anymore and as an added bonus we can (optionally) attempt to hide the discriminator in tail padding of the union member.

## Visitation
`slo::visit` can use one of 3 visitation strategies
- Generate a sufficiently large switch with macros, discard all unneeded cases. The Microsoft STL way.
- Trigger an optimization pass in GCC/Clang to turn a fold expression into a switch.
- Generate an array of pointers to generated dispatcher functions

The latter is the fall-back strategy. This is the only strategy that requires an indirect call. Interestingly at the time of writing libstdc++ and libc++ both _always_ generate an array of function pointers when visiting multiple variants - even if the total amount of possible states is very low. `slo::visit` does not - it will also generate jump tables when visiting multiple variants.


## Usage
TODO


## FAQ
### Why `slo`? Isn't this supposed to be fast?
**S**tandard **L**ibrary **O**ptimizations. Also turns out not many people use the `slo` namespace in their (public) projects.

## License

`slo::variant` is provided under the [MIT License](LICENSE). Feel free to use and modify it in your projects. The techniques used in this library are explained in the aforementioned blog post, which is published under the [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) license.


## Contributing

If you'd like to contribute to `slo::variant`, please fork the repository, make your changes, and submit a pull request. Contributions are highly welcomed - including but not limited to bug reports, bug fixes, new features, and improvements.


## Contact

For questions or issues related to this library, please [open an issue](https://github.com/tsche/variant/issues).
