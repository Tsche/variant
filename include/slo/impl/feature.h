#pragma once

#define USING($operation) ((1 $operation 1) != 0)

#define ON  +
#define OFF -

/**
 * @def SLO_HIDE_IN_PADDING
 * @brief SLO_HIDE_IN_PADDING only affects variants created from an existing union.
 *        If this is enabled the union and the tag may overlap, or in other words
 *        the tag may be hidden in padding. This might reduce overall padding and
 *        afffect size of the variant type.
 *
 * @attention You might need to force your union to not be a standard-layout type. To do this
 *            simply define a private non-static data member of any type (ie `char`).
 *
 * @warning Certain operations (such as move construction) _might_ clobber the tail padding
 *          and overwrite the tag. This only matters if an alternative constructor threw,
 *          as this might put the variant in a seemingly valid state while it's invalid.
 *
 */
#ifndef SLO_HIDE_IN_PADDING
#  define SLO_HIDE_IN_PADDING OFF
#endif

/**
 * @def SLO_TREE_THRESHOLD
 * @brief SLO_TREE_THRESHOLD controls at which amount of alternative types the strategy
 *        for the generated underlying union is being switched to a tree-like structure.
 *
 */
#ifndef SLO_TREE_THRESHOLD
#  define SLO_TREE_THRESHOLD 42
#endif

/**
 * @def SLO_MACRO_VISIT
 * @brief SLO_MACRO_VISIT controls whether macro-generated switches or fold expressions
 *        are being used to facilitate visitation. Turn this `ON` if the gcc/clang fold
 *        expression does not work with your target compiler.
 *
 * @attention For MSVC the default behavior is to use macro visitation, since MSVC does not
 *            collapse repeated branches into jump tables unless being spoonfed an actual
 *            switch.
 * 
 */
#ifndef SLO_MACRO_VISIT
#  if defined(_MSC_VER)
#    define SLO_MACRO_VISIT ON
#  else
#    define SLO_MACRO_VISIT OFF
#  endif
#endif