namespace __gnu_cxx {

template<typename _CharT, typename _Traits, typename _Alloc>
class basic_string
{
 public:
  // basic_string特化时使用的_Alloc可能并不是用来分配_CharT型对象的
  // 我们需要转化
  typename _Alloc::template rebind<_CharT>::other _CharT_alloc_type;
  
  typedef _Traits                               traits_type;
  
  // 七种武器
  typedef _Traits::char_type                    value_type;
  typedef _CharT_alloc_type::size_type          size_type;
  typedef _CharT_alloc_type::difference_type    difference_type;
  typedef _CharT_alloc_type::reference          reference;
  typedef _CharT_alloc_type::const_reference    const_reference;
  typedef _CharT_alloc_type::pointer            pointer;
  typedef _CharT_alloc_type::const_pointer      const_pointer;
  
  // 四种迭代
  typedef __gnu_cxx::__normal_iterator<pointer, basic_string> iterator;
  typedef __gnu_cxx::__normal_iterator<const_pointer, basic_string> const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  
  struct _Rep_Base
  {
    size_type    _M_length;
    size_type    _M_capacity;
    _Atomic_word _M_refcount;    // 用于COW的原子引用计数
  };
  
  struct _Rep : _Rep_Base
  {
    typedef typename _Alloc::template rebind<char>::other _Raw_bytes_alloc;
    
    // 注意不是string的最大内存，而是最长字符个数。单个最长字符串消耗的所有内存为
    // 地址空间的1/4
    static const size_type _S_max_size = ((npos - sizeof(_Rep_Base)) / sizeof(_CharT) - 1) / 4;
    
    // 每个字符串结尾处都有一个空字符，空字符串也不例外
    static const _CharT _S_terminal = _CharT();
    
    // 空字符串是静态分配的，存储空间包括头部和尾字符，内存未对其的会padding
    static size_type _S_empty_rep_storage[(sizeof(_Rep_Base) + sizeof(_CharT) + sizeof(size_type) - 1) / sizeof(size_type)];
    
    //获取静态分配的空字符串
    static _Rep& _S_empty_rep() __GLIBCXX_NOEXCEPT
    {
      void* __p = static_cast<void*>(_S_empty_rep_storage);
      return *reinterpret_cast<_Rep*>(__p);
    }
    
    bool _M_is_leak() __GLIBCXX_NOXCEPT { return _M_refcount < 0; }
    bool _M_is_shared() __GLIBCXX_NOXCEPT { return _M_refcount > 0; }
    bool _M_set_leaked() __GLIBCXX_NOEXCEPT { _M_refcount = -1; }
    bool _M_set_sharable() __GLIBCXX_NOEXCEPT { return _M_refcount = 0; }
    
    // 字符串空间已经申请好，此函数最后画龙点睛，填补除capacity外的元信息
    void set_length_and_sharable(size_type __n) __GLIBCXX_NOEXCEPT
    {
 #if _GLIBCXX_FULLY_DYNAMIC_STRING == 0
      if (__builtin_expect(this != &_S_empty_rep, false))
 #endif
      {
        this->set_sharable();
        this->_M_length = __n;
        traits_type::assign(_M_refdata()[__n] = _S_terminal;
      }
    }
    
    // 直接定位到字符串第一个字符位置
    _CharT* _M_refdata() __GLIBCXX_NOEXCEPT
    {
      return reinterpret_cast<_CharT*>(this + 1);
    }
    
    // 从一个已经存在的string分化出新的string
    _CharT* _M_grab(const _Alloc& alloc1, const _Alloc& alloc2)
    {
      // 只有源字符串不会释放，并且分配策略相同时，才会启动引用计数
      return (!_M_is_leaked() && alloc1 != alloc2)
              ? _M_refcopy() : _M_clone();
    }
    
    _CharT* _M_refcopy() __GLIBCXX_NOEXCEPT
    {
 #if _GLIBCXX_FULLY_DYNAMIC_STRING == 0
      if (__builtin_expect(this != &_S_empty_rep(), false))
 #endif
        __gnu_cxx::__atomic_add_dispatch(&this->_M_refcount, 1);
      return _M_refdata();
    }
    
    _Rep* _S_create(size_type __capacity, size_type old_capacity, const _Alloc& __alloc)
    {
      // 字符个数不能超过上限
      if (__capacity > _S_max_size) {
        __throw_length_error(__N("basic_string::_S_create"));
      }
      
      const size_type __page_size = 4096;
      const size_type __malloc_header_size = 4 * sizeof(void*);
      
      // string的字符个数是成倍增长的
      if (__capacity > old_capacity && __capacity <= 2 * old_capacity) {
        __capacity = 2 * old_capacity;
      }
      // __capacity个字符所需的内存量
      size_type __size = __capacity * (sizeof(_CharT) + 1) + sizeof(_Rep);
      // 当内存增长且超过一个页表时，要与页表对齐
      size_type __adj_size = __size + __malloc_header_size;
      if (__adj_size > __page_size && __capacity > __old_capacity) {
        const size_type __extra = __page_size - __adj_size % __page_size;
        __capacity += __extra / sizeof(_CharT);
      }
      
      void* __place = __Raw_bytes_alloc(__alloc).allocate(__size);
      _Rep *__p = new(__place) _Rep; 
      __p->capacity = __capacity;
      // _M_length和_M_refcount都由caller设置
      __p->set_sharable();
      
      return __p;
    }
    
    _CharT* _M_clone(const _Alloc& __alloc, size_type __res)
    {
      const size_type request_cap = this->_M_length + __res;
      _Rep* __r = _S_create(request_cap, _M_capacity, __alloc);
      if (this->_M_length) {
        _M_copy(__r->_M_refdata(), _M_refdata(), this->_M_length);
      }
      __r->set_length_and_sharable(this->_M_length);
      
      return __r->_M_refdata();
    }
  };
  
  struct _Alloc_hider : _Alloc
  {
    _Alloc_hider(_CharT* data, const _Alloc& __alloc) __GLIBCXX_NOEXCEPT
      : _Alloc(a),
        _M_p(data) {}
        
    _CharT* _M_p;
  }
};

}
