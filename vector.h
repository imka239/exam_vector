//
// Created by ASUS on 18.06.2019.
//

#ifndef BIGINT_VARIANT_H
#define BIGINT_VARIANT_H

#include <memory>
#include <variant>

template <class T>
struct iterator {
    typedef T value_type;
    typedef T& reference;
    typedef T* pointer;
    typedef std::random_access_iterator_tag iterator_category;
    typedef size_t difference_type;


    template<typename> friend struct const_iterator;
    template<typename> friend class vector;

    iterator() = default;
    iterator(iterator const &) = default;
    ~iterator() = default;

    iterator& operator++() {
        ++ptr_;
        return *this;
    }

    const iterator operator++(int) {
        iterator x(ptr_);
        ++(*this);
        return x;
    }

    iterator& operator--() {
        --ptr_;
        return *this;
    }

    const iterator operator--(int) {
        iterator x(ptr_);
        --(*this);
        return x;
    }

    friend bool operator==(const iterator& first, const iterator& sec) {
        return first.ptr_ == sec.ptr_;
    }

    friend bool operator!=(const iterator& first, const iterator& sec) {
        return first.ptr_ != sec.ptr_;
    }

    reference operator*() {
        return *ptr_;
    }

    pointer operator->() {
        return ptr_;
    }

    iterator& operator+=(const size_t& i) {
        ptr_ += i;
        return *this;
    }

    iterator& operator-=(const size_t& i) {
        ptr_ -= i;
        return *this;
    }

    reference operator[](const size_t& i) {
        return ptr_[i];
    }

    friend difference_type operator-(const iterator& first, const iterator & sec) {
        return first.ptr_ - sec.ptr_;
    }

    friend iterator operator+(iterator that, size_t n) {
        that += n;
        return that;
    }

    friend iterator operator+(size_t n, iterator that) {
        that += n;
        return that;
    }

    friend iterator operator-(iterator that, size_t n) {
        that -= n;
        return that;
    }

protected:

    explicit iterator(pointer ptr) : ptr_(ptr) {};

    pointer ptr_ = nullptr;

};

template <class T>
struct const_iterator {
    typedef T value_type;
    typedef T const & reference;
    typedef const T* pointer;
    typedef std::random_access_iterator_tag iterator_category;
    typedef size_t difference_type;

    template<typename> friend class vector;

    const_iterator() = default;
    const_iterator(iterator<T> const& p) : ptr_(p.ptr_) {}
    const_iterator(const_iterator const &) = default;
    ~const_iterator() = default;

    const_iterator& operator++() {
        ++ptr_;
        return *this;
    }

    const const_iterator operator++(int) {
        const_iterator x(ptr_);
        ++(*this);
        return x;
    }

    const_iterator& operator--() {
        --ptr_;
        return *this;
    }

    const const_iterator operator--(int) {
        const_iterator x(ptr_);
        --(*this);
        return x;
    }

    friend bool operator==(const const_iterator& first, const const_iterator& sec) {
        return first.ptr_ == sec.ptr_;
    }

    friend bool operator!=(const const_iterator& first, const const_iterator& sec) {
        return first.ptr_ != sec.ptr_;
    }

    reference operator*() {
        return *ptr_;
    }

    pointer operator->() {
        return ptr_;
    }

    const_iterator& operator+=(const size_t& i) {
        ptr_ += i;
        return *this;
    }

    const_iterator& operator-=(const size_t& i) {
        ptr_ -= i;
        return *this;
    }

    reference operator[](const size_t& i) {
        return ptr_[i];
    }

    friend difference_type operator-(const const_iterator& first, const const_iterator & sec) {
        return first.ptr_ - sec.ptr_;
    }

    friend const_iterator operator+(const_iterator that, size_t n) {
        that += n;
        return that;
    }

    friend const_iterator operator+(size_t n, const_iterator that) {
        that += n;
        return that;
    }

    friend const_iterator operator-(const_iterator that, size_t n) {
        that -= n;
        return that;
    }

protected:

    explicit const_iterator(const T* ptr) : ptr_(ptr) {};

    pointer ptr_ = nullptr;

};


template <class T>
class vector {
    struct private_work {
        size_t size_ = 0;
        size_t capacity_ = 0;
        size_t amount_ = 0;

        T* get_next() {
            return reinterpret_cast<T*>(&amount_ + 1);
        }
    };

    void* alloc_one_element(size_t n) {
        return operator new(n);
    }

    private_work* alloc_many_elements(size_t n) {
        auto new_work = static_cast<private_work*> (alloc_one_element(sizeof(T) * n + sizeof(private_work)));
        new_work->size_ = 0;
        new_work->amount_ = 0;
        new_work->capacity_ = n;
        return new_work;
    }

    template<class ... Args>
    void construct(T* p, Args const& ...args) {
        assert(p != nullptr);
        new (p) T (args...);
    }

    void construct(T* p, T *copy) {
        assert(copy != nullptr && p != nullptr);
        new (p) T (* copy);
    }

    void destruct(T* p) noexcept {
        p->~T();
    }

    void deallocate(private_work* destroy) {
        operator delete(static_cast<void * >(destroy));
    }

    void fail_destruct(size_t n, T * p) noexcept {
        for (size_t i = 0; i < n; ++i) {
            destruct(p + i);
        }
    }

    private_work* copy(size_t n) {
        private_work * ans = alloc_many_elements(n);
        size_t i = 0;
        try {
            for (; i < std::min(get_size(), n); ++i) {
                construct(ans->get_next() + i, get_ptr() + i);
            }
        } catch (...) {
            fail_destruct(i, ans->get_next());
            deallocate(ans);
            throw;
        }
        ans->capacity_ = n;
        ans->size_ = get_size();
        ans->amount_ = 0;
        return ans;
    }

    typedef std::variant<private_work*, T> var;

    var data_;

    private_work* get_variant() const noexcept {
        assert(data_.index() == 0);
        return std::get<0>(data_);
    }

    size_t get_capacity() const noexcept {
        assert(get_variant() != nullptr);
        return get_variant()->capacity_;
    }

    size_t get_size() const noexcept {
        assert(get_variant() != nullptr);
        return get_variant()->size_;
    }

    size_t get_amount() const noexcept {
        assert(get_variant() != nullptr);
        return get_variant()->amount_;
    }

    T* get_ptr() const noexcept {
        assert(get_variant() != nullptr);
        return get_variant()->get_next();
    }

    bool small() const noexcept {
        return (data_.index() == 1 || get_variant() == nullptr);
    }

    void detach() {
        if (data_.index() == 0 && get_variant() != nullptr && get_variant()->amount_ > 0) {
            private_work * new_work = copy(get_capacity());
            --get_variant()->amount_;
            data_ = new_work;
        }
    }

public:
    static_assert((sizeof(data_)) <= (sizeof(void *) + std::max(sizeof(T), sizeof(void *))));

    vector() noexcept = default;
    ~vector() noexcept {
        clear();
    }

    size_t size() const noexcept {
        if (data_.index() == 0) {
            return get_variant() == nullptr ? 0 : get_size();
        }
        return 1;
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    void clear() {
        if (!small()) {
            if (get_amount() == 0) {
                assert(data_.index() == 0);
                fail_destruct(get_size(), get_ptr());
                deallocate(std::get<0>(data_));
                std::get<0>(data_) = nullptr;
            } else {
                --get_variant()->amount_;
                data_ = nullptr;
            }
        } else {
            data_ = nullptr;
        }
    }

    void push_back(T const & value) {
        detach();
        if (empty() && get_variant() == nullptr) {
            try {
                data_ = value;
            } catch (...) {
                data_ = nullptr;
                throw;
            }
        } else {
            if (small()) {
                private_work * new_work = alloc_many_elements(4);
                new_work->size_ = 2;
                try {
                    construct(new_work->get_next(), std::get<1>(data_));
                } catch (...) {
                    deallocate(new_work);
                    throw;
                }
                try {
                    construct(new_work->get_next() + 1, value);
                } catch (...) {
                    destruct(new_work->get_next());
                    deallocate(new_work);
                    throw;
                }
                data_ = new_work;
            } else {
                if (get_size() >= get_capacity()) {
                    private_work* new_work = copy(get_capacity() * 2);
                    try {
                        construct(new_work->get_next() + get_size(), value);
                    } catch (...) {
                        fail_destruct(get_size(), new_work->get_next());
                        deallocate(new_work);
                        throw;
                    }
                    assert(data_.index() == 0);
                    fail_destruct(get_size(), get_ptr());
                    deallocate(std::get<0>(data_));
                    std::get<0>(data_) = nullptr;
                    data_ = new_work;
                } else {
                    construct(get_ptr() + get_size(), value);
                }
                ++get_variant()->size_;
            }
        }
    }

    void pop_back() {
        assert(size() > 0);
        if (small()) {
            data_ = nullptr;
        } else {
            destruct(get_ptr() + get_size() - 1);
            --get_variant()->size_;
        }
    }

    T& operator[](size_t i) {
        detach();
        if (small()) {
            assert(i == 0);
            return std::get<1>(data_);
        } else {
            assert(data_.index() == 0);
            assert(get_variant() != nullptr);
            assert(i < get_size());
            return *(get_ptr() + i);
        }
    }

    T const & operator[](size_t i) const {
        if (small()) {
            assert(i == 0);
            return std::get<1>(data_);
        } else {
            assert(data_.index() == 0);
            assert(get_variant() != nullptr);
            assert(i < get_size());
            return *(get_ptr() + i);
        }
    }



    vector(const vector& a) : vector() {
        if (&a != this) {
            if (a.small()) {
                if (!a.empty()) {
                    push_back(a[0]);
                }
            } else {
                data_ = a.get_variant();
                ++get_variant()->amount_;
            }
        }
    }

    vector& operator= (const vector& a) {
        if (a != *this) {
            clear();
            data_ = a.data_;
            if (!a.small()) {
                ++get_variant()->amount_;
            }
        }
        return *this;
    }

    T& front() {
        assert(size() > 0);
        return (*this)[0];
    }

    const T& front() const {
        assert(size() > 0);
        return (*this)[0];
    }

    T& back() {
        assert(size() > 0);
        return (*this)[get_variant()->size_ - 1];
    }

    const T& back() const {
        assert(size() > 0);
        return (*this)[get_variant()->size_ - 1];
    }


    void reserve(size_t n) {
        if (empty()) {
            private_work *new_work = alloc_many_elements(n);
            data_ = new_work;
        } else if (small()) {
            if (n > 0) {
                private_work * new_work = alloc_many_elements(n + 4);
                new_work->size_ = 1;
                try {
                    construct(new_work->get_next(), std::get<1>(data_));
                } catch (...) {
                    deallocate(new_work);
                    throw;
                }
                data_ = new_work;
            }
        } else {
            private_work* new_work = copy(get_capacity() + n);
            fail_destruct(get_size(), get_ptr());
            deallocate(std::get<0>(data_));
            std::get<0>(data_) = nullptr;
            data_ = new_work;
        }
    }

    typedef iterator<T> iterator;
    typedef const_iterator<T> const_iterator;

    typedef std::reverse_iterator <iterator> reverse_iterator;
    typedef std::reverse_iterator <const_iterator> const_reverse_iterator;

    iterator end() {
        if (empty()) {
            return iterator(nullptr);
        }
        detach();
        return small() ? iterator(&(std::get<1>(data_))) : iterator(get_ptr() + size());
    }

    iterator begin() {
        if (empty()) {
            return iterator(nullptr);
        }
        detach();
        return small() ? iterator(&(std::get<1>(data_))) : iterator(get_ptr());
    }

    const_iterator end() const noexcept {
        return small() ? const_iterator(&(std::get<1>(data_))) : const_iterator(get_ptr() + size());
    }

    const_iterator begin() const noexcept {
        return small() ? const_iterator(&(std::get<1>(data_))) : const_iterator(get_ptr());
    }

    const_iterator cend() const noexcept{
        return end();
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    reverse_iterator rbegin() {
        detach();
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        detach();
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    void swap(vector& that) {
        if (this == &that) {
            return;
        }
        bool a1 = (data_.index() == 0);
        bool a2 = (that.data_.index() == 0);
        if (a1 && a2) {
            std::swap(data_, that.data_);
        } else if (!a1 && a2) {
            that.swap(*this);
        } else if (a1 && !a2) {
            private_work* new_work = get_variant();
            try {
                data_ = that.data_;
            } catch (...) {
                that.data_ = nullptr;
                data_ = new_work;
                throw;
            }
            that.data_ = new_work;
        } else {
            try {
                std::swap(data_, that.data_);
            } catch (...) {
                throw;
            }
        }
    }

    bool operator<(const vector& that) const noexcept {
        for (size_t i = 0; i < std::min(size(), that.size()); i++) {
            if (that[i] < (*this)[i]) {
                return false;
            }
            if (that[i] > (*this)[i]) {
                return true;
            }
        }
        return (this->size() < that.size());
    }

    bool operator>(const vector & that) const noexcept {
        return that < (*this);
    }

    bool operator==(const vector & that) const noexcept {
        return (((*this) <= that) && ((*this) >= that));
    }

    bool operator!=(const vector & that) const noexcept {
        return !((*this) == that);
    }

    bool operator<=(const vector & that) const noexcept {
        return !((*this) > that);
    }

    bool operator>=(const vector & that) const noexcept {
        return !((*this) < that);
    }

    T* data() noexcept {
        if (empty()) {
            return nullptr;
        }
        return small() ? &std::get<1>(data_) : get_ptr();
    }

    const T* data() const noexcept {
        if (empty()) {
            return nullptr;
        }
        return small() ? &std::get<1>(data_) : get_ptr();
    }

    void erase(const_iterator pos) {
        erase(pos, pos + 1);
    }

    void erase(const_iterator L, const_iterator R) {
        detach();
        if (R == end()) {
            for (; L != R; --R) {
                pop_back();
            }
        } else {
            size_t fir = L - cbegin();
            size_t sec = R - cbegin();
            size_t sz = size();
            vector new_v;
            new_v.reserve(sz - (R - L) + 4);
            size_t i = 0;
            try {
                for (; i < fir; ++i) {
                    new_v.push_back((*this)[i]);
                }
            } catch(...) {
                new_v.clear();
                throw;
            }
            i = sec;
            try {
                for (; i < sz; i++) {
                    new_v.push_back((*this)[i]);
                }
            } catch(...) {
                new_v.clear();
                throw;
            }
            *this = new_v;
        }
    }

    void insert(const_iterator pos, T const val) {
        detach();
        if (empty() || pos == end()) {
            push_back(val);
        } else {
            size_t sz = size();
            vector new_v;
            new_v.reserve(sz + 4);
            size_t i = 0;
            try {
                for (; i < pos - cbegin(); ++i) {
                    new_v.push_back((*this)[i]);
                }
                new_v.push_back(val);
            } catch(...) {
                new_v.clear();
                throw;
            }
            try {
                for (; i < sz; ++i) {
                    new_v.push_back((*this)[i]);
                }
            } catch(...) {
                new_v.clear();
                throw;
            }
            *this = new_v;
        }
    }
};

template<class T>
void swap(vector<T>& first, vector<T> & that) {
    first.swap(that);
}

#endif //TEST_VARIANT_H
