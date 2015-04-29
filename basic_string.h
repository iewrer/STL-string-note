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
  
  struct _Rep_Base {
    size_type    _M_length;
    size_type    _M_capacity;
    _Atomic_word _M_refcount;    // 用于COW的原子引用计数
  }
};

}
