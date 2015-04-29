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
 #endif
    }
    
    // 直接定位到字符串第一个字符位置
    _CharT* _M_refdata() __GLIBCXX_NOEXCEPT
    {
      return reinterpret_cast<_CharT*>(this + 1);
    }
    
    _CharT* _M_grab(const _Alloc& alloc1, const _Alloc& alloc2)
    {
      if (!_M_is_leaked() && alloc1 != alloc2) {
        return _M_refcopy();
      }
      return _M_clone();
    }
  }
};

}
