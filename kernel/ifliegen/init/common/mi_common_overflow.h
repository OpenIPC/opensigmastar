/* Sgs trade secret */
/* Copyright (c) [2019~2025] Sgs Technology Ltd.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sgs and be kept in strict confidence
(Sgs Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sgs Confidential
Information is unlawful and strictly prohibited. Sgs hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _MI_COMMON_OVERFLOW_H_
#define _MI_COMMON_OVERFLOW_H_
#include "mi_common_datatype.h"
#include "cam_os_wrapper.h"
#include "mi_common_internal.h"

#if defined(CONFIG_ENABLE_SAFE_ARITHMETIC)

#define WARN_OVERFLOW(condition, ...)                    \
    (                                                    \
        {                                                \
            MI_S32 __ret_warn_on = !!(condition);        \
            if (CAM_OS_UNLIKELY(__ret_warn_on))          \
            {                                            \
                MI_COMMON_WarnOn(__LINE__, __VA_ARGS__); \
            }                                            \
            CAM_OS_UNLIKELY(__ret_warn_on);              \
        })

#define WARN_ON_OVERFLOW(condition) WARN_OVERFLOW(condition, NULL)

#define __MUST_CHECK __attribute__((__warn_unused_result__))
static inline MI_BOOL __MUST_CHECK _MI_COMMON_OVERFLOW_MustCheck(MI_BOOL bOverFlow)
{
    return CAM_OS_UNLIKELY(bOverFlow);
}

/**
 * MI_COMMON_ADD_OVERFLOW() - Calculate addition with overflow checking
 * @a: first addend
 * @b: second addend
 * @d: pointer to store sum
 *
 * Returns true on wrap-around, false otherwise.
 *
 * @d holds the results of the attempted addition, regardless of whether
 * wrap-around occurred.
 */
#define MI_COMMON_ADD_OVERFLOW(a, b, d) _MI_COMMON_OVERFLOW_MustCheck(__builtin_add_overflow(a, b, d))

/**
 * MI_COMMON_SUB_OVERFLOW() - Calculate subtraction with overflow checking
 * @a: minuend; value to subtract from
 * @b: subtrahend; value to subtract from @a
 * @d: pointer to store difference
 *
 * Returns true on wrap-around, false otherwise.
 *
 * @d holds the results of the attempted subtraction, regardless of whether
 * wrap-around occurred.
 */
#define MI_COMMON_SUB_OVERFLOW(a, b, d) _MI_COMMON_OVERFLOW_MustCheck(__builtin_sub_overflow(a, b, d))

/**
 * MI_COMMON_MUL_OVERFLOW() - Calculate multiplication with overflow checking
 * @a: first factor
 * @b: second factor
 * @d: pointer to store product
 *
 * Returns true on wrap-around, false otherwise.
 *
 * @d holds the results of the attempted multiplication, regardless of whether
 * wrap-around occurred.
 */
#define MI_COMMON_MUL_OVERFLOW(a, b, d) _MI_COMMON_OVERFLOW_MustCheck(__builtin_mul_overflow(a, b, d))

/**
 * MI_COMMON_WRAPPING_ASSIGN_ADD() - Intentionally perform a wrapping increment assignment
 * @var: variable to be incremented
 * @offset: amount to add
 *
 * Increments @var by @offset with wrap-around. Returns the resulting
 * value of @var. Will not trip any wrap-around sanitizers.
 *
 * Returns the new value of @var.
 */
#define MI_COMMON_WRAPPING_ASSIGN_ADD(var, offset)          \
    (                                                       \
        {                                                   \
            typeof(var) *__ptr = &(var);                    \
            typeof(var)  __val;                             \
            __builtin_add_overflow(*__ptr, offset, &__val); \
            *__ptr = __val;                                 \
        })

/**
 * MI_COMMON_WRAPPING_ASSIGN_SUB() - Intentionally perform a wrapping decrement assign
 * @var: variable to be decremented
 * @offset: amount to subtract
 *
 * Decrements @var by @offset with wrap-around. Returns the resulting
 * value of @var. Will not trip any wrap-around sanitizers.
 *
 * Returns the new value of @var.
 */
#define MI_COMMON_WRAPPING_ASSIGN_SUB(var, offset)          \
    (                                                       \
        {                                                   \
            typeof(var) *__ptr = &(var);                    \
            typeof(var)  __val;                             \
            __builtin_sub_overflow(*__ptr, offset, &__val); \
            *__ptr = __val;                                 \
        })

/**
 * MI_COMMON_CHECK_ASSIGN - perform an assigning source value into destination ptr along
 *                with an overflow check.
 *
 * @value: Source value
 * @ptr: Destination pointer address
 *
 * Returns:
 * If the value would overflow the destination, it returns true. If not return
 * false. When overflow does not occur, the assigning into ptr from value
 * succeeds.
 */
#define MI_COMMON_CHECK_ASSIGN(value, ptr) _MI_COMMON_OVERFLOW_MustCheck(__builtin_add_overflow(0, value, ptr))

/**
 * MI_COMMON_CHECK_ASSIGN_PTR - perform an assigning source value into destination
 *                         pointer type variable along with an overflow check
 *
 * @value: source value; a source value is expected to have a value of a size
 *         that can be stored in a pointer-type variable.
 * @ptr: destination pointer type variable
 *
 * Returns:
 * If the value would overflow the destination, it returns true. If not return
 * false. When overflow does not occur, the assigning into ptr from value
 * succeeds.
 */
#define MI_COMMON_CHECK_ASSIGN_PTR(value, ptr)                                             \
    _MI_COMMON_OVERFLOW_MustCheck((                                                        \
        {                                                                                  \
            MI_VIRT __vPtr;                                                                \
            MI_COMMON_CHECK_ASSIGN(value, &__vPtr) ? 1 : (({ ptr = (void *)__vPtr; }), 0); \
        }))

/**
 * MI_COMMON_WRAPPING_ASSIGN - Intentionally perform a wrapping assigning source value into destination ptr
 * @value: Source value
 * @ptr: Destination pointer address
 *
 * The assigning into ptr from value will not trip any wrap-around sanitizers.
 *
 * Returns the result of wrapping assignment.
 */
#define MI_COMMON_WRAPPING_ASSIGN(value, ptr)         \
    (                                                 \
        {                                             \
            typeof(*ptr) __val;                       \
            __builtin_add_overflow(0, value, &__val); \
            *ptr = __val;                             \
        })

#else
#define MI_COMMON_ADD_OVERFLOW(a, b, d)            (*(d) = (a) + (b), 0)
#define MI_COMMON_SUB_OVERFLOW(a, b, d)            (*(d) = (a) - (b), 0)
#define MI_COMMON_MUL_OVERFLOW(a, b, d)            (*(d) = (a) * (b), 0)
#define MI_COMMON_CHECK_ASSIGN(value, ptr)         (*(ptr) = (value), 0)
#define MI_COMMON_CHECK_ASSIGN_PTR(value, ptr)     ((ptr) = (void *)(MI_VIRT)(value), 0)
#define MI_COMMON_WRAPPING_ASSIGN(value, ptr)      (*(ptr) = (typeof(*ptr))(value))
#define MI_COMMON_WRAPPING_ASSIGN_ADD(var, offset) ((var) += (offset))
#define MI_COMMON_WRAPPING_ASSIGN_SUB(var, offset) ((var) -= (offset))
#define WARN_OVERFLOW(condition, ...)              ({ CAM_OS_UNLIKELY(!!(condition)); })
#define WARN_ON_OVERFLOW(condition)                ({ CAM_OS_UNLIKELY(!!(condition)); })
#endif

#endif ///_MI_COMMON_MODPARAM_H_
