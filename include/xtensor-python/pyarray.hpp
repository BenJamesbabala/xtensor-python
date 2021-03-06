/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef PY_ARRAY_HPP
#define PY_ARRAY_HPP

#include <cstddef>
#include <vector>
#include <algorithm>
#include "xtensor/xsemantic.hpp"
#include "xtensor/xiterator.hpp"

#include "pycontainer.hpp"
#include "pybuffer_adaptor.hpp"

namespace xt
{
    template <class T>
    class pyarray;
}

namespace pybind11
{
    namespace detail
    {
        template <class T>
        struct handle_type_name<xt::pyarray<T>>
        {
            static PYBIND11_DESCR name()
            {
                return _("numpy.ndarray[") + make_caster<T>::name() + _("]");
            }
        };

        template <typename T>
        struct pyobject_caster<xt::pyarray<T>>
        {
            using type = xt::pyarray<T>;

            bool load(handle src, bool)
            {
                value = type::ensure(src);
                return static_cast<bool>(value);
            }

            static handle cast(const handle &src, return_value_policy, handle)
            {
                return src.inc_ref();
            }

            PYBIND11_TYPE_CASTER(type, handle_type_name<type>::name());
        };
    }
}

namespace xt
{

    template <class A>
    class pyarray_backstrides
    {

    public:

        using array_type = A;
        using value_type = typename array_type::size_type;
        using size_type = typename array_type::size_type;

        pyarray_backstrides() = default;
        pyarray_backstrides(const array_type& a);

        value_type operator[](size_type i) const;

    private:

        const array_type* p_a;
    };

    template <class T>
    struct xiterable_inner_types<pyarray<T>>
        : xcontainer_iterable_types<pyarray<T>>
    {
    };

    template <class T>
    struct xcontainer_inner_types<pyarray<T>>
    {
        using container_type = pybuffer_adaptor<T>;
        using shape_type = std::vector<typename container_type::size_type>;
        using strides_type = shape_type;
        using backstrides_type = pyarray_backstrides<pyarray<T>>;
        using inner_shape_type = pybuffer_adaptor<std::size_t>;
        using inner_strides_type = pystrides_adaptor<sizeof(T)>;
        using inner_backstrides_type = backstrides_type;
        using temporary_type = pyarray<T>;
    };

    /**
     * @class pyarray
     * @brief Wrapper on the Python buffer protocol.
     */
    template <class T>
    class pyarray : public pycontainer<pyarray<T>>,
                    public xcontainer_semantic<pyarray<T>>
    {

    public:

        using self_type = pyarray<T>;
        using semantic_base = xcontainer_semantic<self_type>;
        using base_type = pycontainer<self_type>;
        using container_type = typename base_type::container_type;
        using value_type = typename base_type::value_type; 
        using pointer = typename base_type::pointer;
        using size_type = typename base_type::size_type;
        using shape_type = typename base_type::shape_type;
        using strides_type = typename base_type::strides_type;
        using backstrides_type = typename base_type::backstrides_type;
        using inner_shape_type = typename base_type::inner_shape_type;
        using inner_strides_type = typename base_type::inner_strides_type;
        using inner_backstrides_type = typename base_type::inner_backstrides_type;

        pyarray() = default;
        pyarray(const value_type& t);
        pyarray(nested_initializer_list_t<T, 1> t);
        pyarray(nested_initializer_list_t<T, 2> t);
        pyarray(nested_initializer_list_t<T, 3> t);
        pyarray(nested_initializer_list_t<T, 4> t);
        pyarray(nested_initializer_list_t<T, 5> t);

        pyarray(pybind11::handle h, pybind11::object::borrowed_t);
        pyarray(pybind11::handle h, pybind11::object::stolen_t);
        pyarray(const pybind11::object &o);
        
        explicit pyarray(const shape_type& shape, layout l = layout::row_major);
        pyarray(const shape_type& shape, const strides_type& strides);

        template <class E>
        pyarray(const xexpression<E>& e);

        template <class E>
        self_type& operator=(const xexpression<E>& e);

        using base_type::begin;
        using base_type::end;

        static self_type ensure(pybind11::handle h);
        static bool check_(pybind11::handle h);

    private:

        inner_shape_type m_shape;
        inner_strides_type m_strides;
        mutable inner_backstrides_type m_backstrides;
        container_type m_data;

        void init_array(const shape_type& shape, const strides_type& strides);
        void init_from_python();

        const inner_shape_type& shape_impl() const;
        const inner_strides_type& strides_impl() const;
        const inner_backstrides_type& backstrides_impl() const;

        container_type& data_impl();
        const container_type& data_impl() const;

        friend class xcontainer<pyarray<T>>;
    };
    
    /**************************************
     * pyarray_backstrides implementation *
     **************************************/

    template <class A>
    inline pyarray_backstrides<A>::pyarray_backstrides(const array_type& a)
        : p_a(&a)
    {
    }

    template <class A>
    inline auto pyarray_backstrides<A>::operator[](size_type i) const -> value_type
    {
        value_type sh = p_a->shape()[i];
        value_type res = sh == 1 ? 0 : (sh - 1) * p_a->strides()[i];
        return  res;
    }

    /**************************
     * pyarray implementation *
     **************************/

    template <class T>
    inline pyarray<T>::pyarray(const value_type& t)
    {
        base_type::reshape(xt::shape<shape_type>(t), layout::row_major);
        nested_copy(m_data.begin(), t);
    }

    template <class T>
    inline pyarray<T>::pyarray(nested_initializer_list_t<T, 1> t)
    {
        base_type::reshape(xt::shape<shape_type>(t), layout::row_major);
        nested_copy(m_data.begin(), t);
    }

    template <class T>
    inline pyarray<T>::pyarray(nested_initializer_list_t<T, 2> t)
    {
        base_type::reshape(xt::shape<shape_type>(t), layout::row_major);
        nested_copy(m_data.begin(), t);
    }

    template <class T>
    inline pyarray<T>::pyarray(nested_initializer_list_t<T, 3> t)
    {
        base_type::reshape(xt::shape<shape_type>(t), layout::row_major);
        nested_copy(m_data.begin(), t);
    }

    template <class T>
    inline pyarray<T>::pyarray(nested_initializer_list_t<T, 4> t)
    {
        base_type::reshape(xt::shape<shape_type>(t), layout::row_major);
        nested_copy(m_data.begin(), t);
    }

    template <class T>
    inline pyarray<T>::pyarray(nested_initializer_list_t<T, 5> t)
    {
        base_type::reshape(xt::shape<shape_type>(t), layout::row_major);
        nested_copy(m_data.begin(), t);
    }

    template <class T>
    inline pyarray<T>::pyarray(pybind11::handle h, pybind11::object::borrowed_t)
        : base_type(h, pybind11::object::borrowed)
    {
        init_from_python();
    }

    template <class T>
    inline pyarray<T>::pyarray(pybind11::handle h, pybind11::object::stolen_t)
        : base_type(h, pybind11::object::stolen)
    {
        init_from_python();
    }

    template <class T>
    inline pyarray<T>::pyarray(const pybind11::object &o)
        : base_type(o)
    {
        init_from_python();
    }

    template <class T>
    inline pyarray<T>::pyarray(const shape_type& shape, layout l)
    {
        strides_type strides;
        base_type::fill_default_strides(shape, l, strides);
        init_array(shape, strides);
    }

    template <class T>
    inline pyarray<T>::pyarray(const shape_type& shape, const strides_type& strides)
    {
        init_array(shape, strides);
    }

    template <class T>
    template <class E>
    inline pyarray<T>::pyarray(const xexpression<E>& e)
    {
        semantic_base::assign(e);
    }

    template <class T>
    template <class E>
    inline auto pyarray<T>::operator=(const xexpression<E>& e) -> self_type&
    {
        return semantic_base::operator=(e);
    }

    template <class T>
    inline auto pyarray<T>::ensure(pybind11::handle h) -> self_type
    {
        return base_type::ensure(h);
    }

    template <class T>
    inline bool pyarray<T>::check_(pybind11::handle h)
    {
        return base_type::check_(h);
    }

    template <class T>
    inline void pyarray<T>::init_array(const shape_type& shape, const strides_type& strides)
    {
        strides_type adapted_strides(strides);

        std::transform(strides.begin(), strides.end(), adapted_strides.begin(),
                [](auto v) { return sizeof(T) * v; });

        int flags = NPY_ARRAY_ALIGNED;
        if(!std::is_const<T>::value)
        {
            flags |= NPY_ARRAY_WRITEABLE;
        }
        int type_num = detail::numpy_traits<T>::type_num;
        
        npy_intp* shape_data = reinterpret_cast<npy_intp*>(const_cast<size_type*>(shape.data()));
        npy_intp* strides_data = reinterpret_cast<npy_intp*>(adapted_strides.data());
        auto tmp = pybind11::reinterpret_steal<pybind11::object>(
                PyArray_New(&PyArray_Type, static_cast<int>(shape.size()), shape_data, type_num, strides_data,
                            nullptr, static_cast<int>(sizeof(T)), flags, nullptr)
                );
        
        if(!tmp)
            throw std::runtime_error("NumPy: unable to create ndarray");

        this->m_ptr = tmp.release().ptr();
        init_from_python();
    }

    template <class T>
    inline void pyarray<T>::init_from_python()
    {
        m_shape = inner_shape_type(reinterpret_cast<size_type*>(PyArray_SHAPE(this->python_array())),
                                   static_cast<size_type>(PyArray_NDIM(this->python_array())));
        m_strides = inner_strides_type(reinterpret_cast<size_type*>(PyArray_STRIDES(this->python_array())),
                                       static_cast<size_type>(PyArray_NDIM(this->python_array())));
        m_backstrides = backstrides_type(*this);
        m_data = container_type(reinterpret_cast<pointer>(PyArray_DATA(this->python_array())),
                                static_cast<size_type>(PyArray_SIZE(this->python_array())));
    }

    template <class T>
    inline auto pyarray<T>::shape_impl() const -> const inner_shape_type&
    {
        return m_shape;
    }

    template <class T>
    inline auto pyarray<T>::strides_impl() const -> const inner_strides_type&
    {
        return m_strides;
    }

    template <class T>
    inline auto pyarray<T>::backstrides_impl() const -> const inner_backstrides_type&
    {
        // The pyarray object may be copied, invalidating m_backstrides. Building
        // it each time it is needed avoids tricky bugs.
        m_backstrides = backstrides_type(*this);
        return m_backstrides;
    }

    template <class T>
    inline auto pyarray<T>::data_impl() -> container_type&
    {
        return m_data;
    }

    template <class T>
    inline auto pyarray<T>::data_impl() const -> const container_type&
    {
        return m_data;
    }

}

#endif


