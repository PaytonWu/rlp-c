// Copyright(c) 2022 - present, Payton Wu (payton.wu@outlook.com) & abc contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined(RLP_V1_OBJECT_FWD_H)
#define RLP_V1_OBJECT_FWD_H

#pragma once

#include "rlp/object_fwd_decl.h"

#include <cstdint>

namespace rlp {

RLP_API_VERSION_NAMESPACE(v1) {

    struct object_array {
        uint32_t size;
        object* ptr;
    };

    struct object_map {
        uint32_t size;
        object_kv* ptr;
    };

    struct object_str {
        uint32_t size;
        const char* ptr;
    };

    struct object_bin {
        uint32_t size;
        const char* ptr;
    };

    struct object_ext {
        int8_t type() const { return static_cast<int8_t>(ptr[0]); }
        const char* data() const { return &ptr[1]; }
        uint32_t size;
        const char* ptr;
    };

    template <typename T>
    struct has_as {
    private:
        template <typename U>
        static auto check(U*) ->
            // Check v1 specialization
            typename std::is_same<
            decltype(adaptor::as<U>()(std::declval<msgpack::object>())),
            T
            >::type;
        template <typename...>
        static std::false_type check(...);
    public:
        using type = decltype(check<T>(RLP_NULLPTR));
        static constexpr bool value = type::value;
    };

    /// Object class that corresponding to MessagePack format object
    /**
     * See https://github.com/msgpack/msgpack-c/wiki/v1_1_cpp_object
     */
    struct object {
        union union_type {
            bool boolean;
            uint64_t u64;
            int64_t  i64;
#if defined(RLP_USE_LEGACY_NAME_AS_FLOAT)
            RLP_DEPRECATED("please use f64 instead")
                double   dec; // obsolete
#endif // RLP_USE_LEGACY_NAME_AS_FLOAT
            double   f64;
            msgpack::object_array array;
            msgpack::object_map map;
            msgpack::object_str str;
            msgpack::object_bin bin;
            msgpack::object_ext ext;
        };

        msgpack::type::object_type type;
        union_type via;

        /// Cheking nil
        /**
         * @return If the object is nil, then return true, else return false.
         */
        bool is_nil() const { return type == msgpack::type::NIL; }

#if defined(RLP_USE_CPP03)

        /// Get value as T
        /**
         * If the object can't be converted to T, msgpack::type_error would be thrown.
         * @tparam T The type you want to get.
         * @return The converted object.
         */
        template <typename T>
        T as() const;

#else  // defined(RLP_USE_CPP03)

        /// Get value as T
        /**
         * If the object can't be converted to T, msgpack::type_error would be thrown.
         * @tparam T The type you want to get.
         * @return The converted object.
         */
        template <typename T>
        typename std::enable_if<msgpack::has_as<T>::value, T>::type as() const;

        /// Get value as T
        /**
         * If the object can't be converted to T, msgpack::type_error would be thrown.
         * @tparam T The type you want to get.
         * @return The converted object.
         */
        template <typename T>
        typename std::enable_if<!msgpack::has_as<T>::value, T>::type as() const;

#endif // defined(RLP_USE_CPP03)

        /// Convert the object
        /**
         * If the object can't be converted to T, msgpack::type_error would be thrown.
         * @tparam T The type of v.
         * @param v The value you want to get. `v` is output parameter. `v` is overwritten by converted value from the object.
         * @return The reference of `v`.
         */
        template <typename T>
        typename msgpack::enable_if<
            !msgpack::is_array<T>::value && !msgpack::is_pointer<T>::value,
            T&
        >::type
            convert(T& v) const;

        template <typename T, std::size_t N>
        T(&convert(T(&v)[N]) const)[N];


#if !defined(RLP_DISABLE_LEGACY_CONVERT)
        /// Convert the object (obsolete)
        /**
         * If the object can't be converted to T, msgpack::type_error would be thrown.
         * @tparam T The type of v.
         * @param v The pointer of the value you want to get. `v` is output parameter. `*v` is overwritten by converted value from the object.
         * @return The pointer of `v`.
         */
        template <typename T>
        RLP_DEPRECATED("please use reference version instead")
            typename msgpack::enable_if<
            msgpack::is_pointer<T>::value,
            T
            >::type
            convert(T v) const;
#endif // !defined(RLP_DISABLE_LEGACY_CONVERT)

        /// Convert the object if not nil
        /**
         * If the object is not nil and can't be converted to T, msgpack::type_error would be thrown.
         * @tparam T The type of v.
         * @param v The value you want to get. `v` is output parameter. `v` is overwritten by converted value from the object if the object is not nil.
         * @return If the object is nil, then return false, else return true.
         */
        template <typename T>
        bool convert_if_not_nil(T& v) const;

        /// Default constructor. The object is set to nil.
        object();

        /// Copy constructor. Object is shallow copied.
        object(const msgpack_object& o);

        /// Construct object from T
        /**
         * If `v` is the type that is corresponding to MessegePack format str, bin, ext, array, or map,
         * you need to call `object(const T& v, msgpack::zone& z)` instead of this constructor.
         *
         * @tparam T The type of `v`.
         * @param v The value you want to convert.
         */
        template <typename T>
        explicit object(const T& v);

        /// Construct object from T
        /**
         * The object is constructed on the zone `z`.
         * See https://github.com/msgpack/msgpack-c/wiki/v1_1_cpp_object
         *
         * @tparam T The type of `v`.
         * @param v The value you want to convert.
         * @param z The zone that is used by the object.
         */
        template <typename T>
        object(const T& v, msgpack::zone& z);

        /// Construct object from T (obsolete)
        /**
         * The object is constructed on the zone `z`.
         * Use `object(const T& v, msgpack::zone& z)` instead of this constructor.
         * See https://github.com/msgpack/msgpack-c/wiki/v1_1_cpp_object
         *
         * @tparam T The type of `v`.
         * @param v The value you want to convert.
         * @param z The pointer to the zone that is used by the object.
         */
        template <typename T>
        RLP_DEPRECATED("please use zone reference version instead of the pointer version")
            object(const T& v, msgpack::zone* z);

        template <typename T>
        object& operator=(const T& v);

        operator msgpack_object() const;

        struct with_zone;

    protected:
        struct implicit_type;

    public:
        implicit_type convert() const;
    };

    class type_error : public std::bad_cast { };

    struct object::implicit_type {
        implicit_type(object const& o) : obj(o) { }
        ~implicit_type() { }

        template <typename T>
        operator T();

    private:
        object const& obj;
    };

    /// @cond
} // RLP_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace msgpack

#endif // RLP_V1_OBJECT_FWD_HPP
