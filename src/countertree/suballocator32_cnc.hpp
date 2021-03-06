//----------------------------------------------------------------------------
/// @file suballocator32.hpp
/// @brief
///
/// @author Copyright (c) 2010 2013 Francisco José Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanyingfile LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#ifndef __CNTREE_SUBALLOCATOR32_CNC_HPP
#define __CNTREE_SUBALLOCATOR32_CNC_HPP

#include <limits>
#include <type_traits>
#include <boost/countertree/alloc/pool32.hpp>
#include <boost/countertree/filter_suballoc.hpp>
#include <boost/countertree/common/singleton.hpp>
#include <boost/countertree/common/spinlock.hpp>
#include <boost/countertree/filter_suballoc.hpp>


namespace countertree
{
namespace c_common = countertree::common ;
namespace c_alloc = countertree::alloc ;

//#############################################################################
//                                                                           ##
//     ################################################################      ##
//     #                                                              #      ##
//     #                       C L A S S                              #      ##
//     #                                                              #      ##
//     #    B A S I C _ S U B A L L O C 3 2_ C N C < ALLOC_T ,T1 >    #      ##
//     #                                                              #      ##
//     ################################################################      ##
//                                                                           ##
//#############################################################################
//
//-------------------------------------------------------------
/// @class basic_suballoc32_cnc
///
/// @remarks This class is an allocator with a incremental pool
///          of logaritmic number of elements.
///          The goal of this class is to permit the existence of
///           suballocator <void>
//----------------------------------------------------------------
template<typename Alloc_t, typename T1 >
class basic_suballoc32_cnc
{
static_assert(std::is_same<typename Alloc_t::value_type,T1>::value, "Error incoherent types") ;
public :
//***************************************************************************
//                 P U B L I C    D E F I N I T I O N S
//***************************************************************************
typedef Alloc_t                                 Allocator ;

typedef typename c_common::lock_data_cnc     lock_data ;
typedef typename c_common::lock_cnc          mylock ;

typedef uint32_t    size_type ;//Quantities of elements
typedef int32_t     difference_type ;//Difference between two pointers

typedef  T1     	                value_type; //Element type
typedef  T1 *	                    pointer; //Pointer to element
typedef  T1 &	                    reference; //Reference to element
typedef  const T1 * 	            const_pointer; //Constant pointer to element
typedef  const T1 &                 const_reference; //Constant reference to element
/*
typedef typename Allocator::value_type	    value_type; //Element type
typedef typename Allocator::pointer	        pointer; //Pointer to element
typedef typename Allocator::reference	    reference; //Reference to element
typedef typename Allocator::const_pointer	const_pointer; //Constant pointer to element
typedef typename Allocator::const_reference const_reference; //Constant reference to element
*/
static uint32_t const SizeElem = sizeof (value_type );
typedef typename Allocator::template rebind <uint8_t>::other AllocByte;
typedef  c_alloc::pool32 <SizeElem, true,AllocByte >  MyPool ;
typedef c_common::singleton <MyPool>   singleton ;

protected:
//**************************************************************************
//             P R O T E C T E D    V A R I A B L E S
//**************************************************************************
MyPool & Pila;
Allocator A ;
mutable lock_data LD;

public:
//***************************************************************************
//   C O N S T R U C T O R ,  D E S T R U C T O R
//
//  explicit basic_suballoc32 ( void)
//  virtual ~basic_suballoc32 ( void)
//
//***************************************************************************
explicit basic_suballoc32_cnc ( void ):Pila (singleton::instance()) { };
basic_suballoc32_cnc ( const Allocator &Alfa ):Pila (singleton::instance() ),A(Alfa) { };
basic_suballoc32_cnc ( const basic_suballoc32_cnc  &Alfa ):Pila (singleton::instance()),A(Alfa.A) { };


virtual ~basic_suballoc32_cnc ( void) {};

//***************************************************************************
//     A D D R E S S     A N D    M A X _ S I Z E
//
//  pointer       address ( reference r);
//  const_pointer address ( const_reference r);
//
//  size_type max_size() const;
//
//***************************************************************************

//------------------------------------------------------------------------
//  function : address
/// @brief provide the address of a reference to an object
/// @param [in] r : reference
/// @return Address of the reference
//------------------------------------------------------------------------
pointer address(reference r) { return &r; };

//------------------------------------------------------------------------
//  function : address
/// @brief provide the address of a const_reference to an object
/// @param [in] r : reference
/// @return Address of the const_reference
//------------------------------------------------------------------------
const_pointer address(const_reference r) { return &r; };

//------------------------------------------------------------------------
//  function : max_size
/// @brief Maximun size of memory for to allocate
/// @return maximun size of memory
/// @remarks
//------------------------------------------------------------------------
size_type max_size() const
{   //--------------------------- begin ------------------------
    return (std::numeric_limits<size_type>::max)() / SizeElem;
};

//**************************************************************************
//    A L L O C A T E , D E A L L O C A T E
//
//  pointer allocate  ( size_type cnt, const_pointer Alfa = NULL);
//  void    deallocate( pointer p, size_type n= 1 );
//
//***************************************************************************

//------------------------------------------------------------------------
//  function : allocate
/// @brief Allocate a block of memory
/// @param [in] cnt : number of objects to allocate
/// @param [in] pointer unused
/// @return pointer to the object allocated
/// @remarks
//------------------------------------------------------------------------
pointer allocate(size_type cnt, const_pointer Alfa = NULL)
{   mylock ML (LD);
    return static_cast<pointer> ( (cnt == 1)?(Pila.allocate()):(A.allocate (cnt)) );
};

//------------------------------------------------------------------------
//  function : deallocate
/// @brief deallocate a block of memory
/// @param [in] p : pointer to deallocate
/// @param [in] n : number of objects to deallocate
/// @return
/// @remarks
//------------------------------------------------------------------------
void deallocate(pointer p, size_type n=1 )
{   //----------------------- begin ---------------------------
    assert ( n >= 1);
    mylock ML (LD);
    if ( n == 1 ) Pila.deallocate ( static_cast< void*> (p) );
    else  A.deallocate ( p , n );
};

//***************************************************************************
//    C O N S T R U C T ,  D E S T R O Y
//
//   void construct(pointer p, const value_type & t);
//   void destroy(pointer p);
//
//***************************************************************************

//------------------------------------------------------------------------
//  function : construct
/// @brief Construct a new object in the memory pointed by p
/// @param [in] p : pointer to the memory for to construct the object
/// @param [in] t : object used as parameter as copy constructor
/// @return
/// @remarks
//------------------------------------------------------------------------
void construct(pointer p, const value_type & t)
{   //A.construct (p,t);
    ::new ((void*)p) value_type  (t);
};
//------------------------------------------------------------------------
//  function : construct
/// @brief Construct a new object in the memory pointed by p
/// @param [in] p : pointer to the memory for to construct the object
/// @param [in] t : rvalue used as parameter as copy constructor
/// @return
/// @remarks
//------------------------------------------------------------------------
void construct(pointer p, value_type && t)
{   //A.construct (p,std::move (t) );
    ::new ((void*)p) value_type (std::move (t) );
};
//------------------------------------------------------------------------
//  function : construct
/// @brief Construct a new object in the memory pointed by p
/// @param [in] p : pointer to the memory for to construct the object
/// @param [in] __args : arguments used for to build the node object
/// @return
/// @remarks
//------------------------------------------------------------------------
template<typename _Up, typename... _Args>
void  construct(_Up* __p, _Args&&... __args)
{   //A.construct(__p,std::forward<_Args>(__args)...);
    ::new ((void*)__p) _Up (std::forward<_Args>(__args)...);
};
//------------------------------------------------------------------------
//  function : destroy
/// @brief destroy the object without freeing the memory
/// @param [in] p : pointer p to the object to destroy
/// @return
/// @remarks
//------------------------------------------------------------------------
void destroy(pointer p)
{   //A.destroy (p);
    ((value_type*)p)->~value_type();
};
//
//***************************************************************************
};//        E N D     B A S I C _ S U B A L L O C 3 2     C L A S S
//***************************************************************************
//#############################################################################
//                                                                           ##
//     ################################################################      ##
//     #                                                              #      ##
//     #                       C L A S S                              #      ##
//     #                                                              #      ##
//     #   B A S I C _ S U B A L L O C 3 2_ C N C < ALLOC_T ,VOID >   #      ##
//     #             ( Specialization for type void )                 #      ##
//     ################################################################      ##
//                                                                           ##
//#############################################################################

//
//-------------------------------------------------------------
/// @class basic_suballoc32_cnc
///
/// @remarks This class is an allocator with a incremental pool
///          of alogaritmic number of elements
//----------------------------------------------------------------
template<typename Allocator  >
class basic_suballoc32_cnc<Allocator, void >
{
public :
//***************************************************************************
//                 P U B L I C    D E F I N I T I O N S
//***************************************************************************
typedef uint32_t    size_type ;//Quantities of elements
typedef int32_t     difference_type ;//Difference between two pointers
typedef void     	                value_type; //Element type
explicit basic_suballoc32_cnc ( void ) { };
basic_suballoc32_cnc ( const Allocator & ) { };
basic_suballoc32_cnc ( const basic_suballoc32_cnc  & ) { };
//***************************************************************************
};//        E N D     B A S I C _ S U B A L L O C 3 2      C L A S S
//***************************************************************************

//#############################################################################
//                                                                           ##
//     ################################################################      ##
//     #                                                              #      ##
//     #                       C L A S S                              #      ##
//     #                                                              #      ##
//     #      S U B A L L O C A T O R 3 2_ C N C < ALLOCATOR >        #      ##
//     #                                                              #      ##
//     ################################################################      ##
//                                                                           ##
//#############################################################################


//-------------------------------------------------------------
/// @class suballocator32_cnc
///
/// @remarks This class is an allocator with a incremental pool
///          of alogaritmic number of elements
//----------------------------------------------------------------
template    <  typename Allocator = std::allocator<void>  >
class suballocator32_cnc :
public basic_suballoc32_cnc < typename filter_suballoc<Allocator>::name,
                              typename filter_suballoc<Allocator>::name::value_type  >
{
public :
//***************************************************************************
//                  D E F I N I T I O N S
//***************************************************************************
typedef typename filter_suballoc<Allocator>::name   FAllocator ;
typedef typename FAllocator::value_type            value_type ;

typedef basic_suballoc32_cnc< FAllocator ,value_type > mybasic_suballoc32_cnc ;

//***************************************************************************
//  C O N S T R U C T O R S , D E S T R U C T O R S , R E B I N D
//
//  explicit suballocator32() ;
//  suballocator32( suballocator32<Allocator> const&) ;
//
//  template<typename U>
//  suballocator32(suballocator32<U> const&) ;
//
//  virtual ~suballocator32() ;
//
//  template<typename U>
//  struct rebind
//
//***************************************************************************
//------------------------------------------------------------------------
//  function : suballocator32
/// @brief constructor of the class
//------------------------------------------------------------------------
explicit suballocator32_cnc ( void ) { };
//------------------------------------------------------------------------
//  function : suballocator32
/// @brief constructor of the class
//------------------------------------------------------------------------
suballocator32_cnc ( const Allocator &Alfa ):mybasic_suballoc32_cnc(Alfa) { };
//------------------------------------------------------------------------
//  function : suballocator32
/// @brief Copy constructor
/// @param [in] Alfa : this parameter is passed to the basic_suballocator
//------------------------------------------------------------------------
suballocator32_cnc ( const suballocator32_cnc &Alfa ):mybasic_suballoc32_cnc (Alfa) { };
//------------------------------------------------------------------------
//  function : suballocator32_cnc
/// @brief Copy constructor fron an allocator of other type
/// @param [in] The parameter is not used
//------------------------------------------------------------------------
template<typename U>
suballocator32_cnc(suballocator32_cnc<U> const&) {};
//------------------------------------------------------------------------
//  function : ~basic_suballoc32
/// @brief Destructor of the class
//------------------------------------------------------------------------
virtual ~suballocator32_cnc() { };
//--------------------------------------------------------------------------
//    convert an allocator<T> to allocator<U>
//--------------------------------------------------------------------------
template<typename U>
struct rebind
{   typedef  typename FAllocator::template rebind<U>::other base_other ;
    typedef suballocator32_cnc <base_other> other;
};
//***************************************************************************
//           B O O L E A N    O P E R A T O R S
//
//  bool operator==(suballocator32 const&) ;
//  bool operator!=(suballocator32 const& ) ;
//
//  template <typename Allocator2>
//  bool operator==(suballocator32<Allocator2> const&)
//
//  template <typename Allocator2>
//  bool operator!=(suballocator32<Allocator2> const& )
//
//***************************************************************************
//------------------------------------------------------------------------
//  function : operator==
/// @brief equality operator
/// @param [in] reference to the suballocator to compare
/// @return always return true
/// @remarks
//------------------------------------------------------------------------
bool operator==(suballocator32_cnc const&) {  return true; };
//------------------------------------------------------------------------
//  function : operator !=
/// @brief inequality operator
/// @param [in]reference to the suballocator to compare
/// @return always return false
/// @remarks
//------------------------------------------------------------------------
bool operator!=(suballocator32_cnc const& )  {   return false;  };
//------------------------------------------------------------------------
//  function : operator==
/// @brief equality operator
/// @param [in] reference to the suballocator to compare
/// @return always return true
/// @remarks
//------------------------------------------------------------------------
template <typename Allocator2>
bool operator==(suballocator32_cnc<Allocator2> const&)  { return true; };
//------------------------------------------------------------------------
//  function : operator !=
/// @brief inequality operator
/// @param [in]reference to the suballocator to compare
/// @return always return false
/// @remarks
//------------------------------------------------------------------------
template <typename Allocator2>
bool operator!=(suballocator32_cnc<Allocator2> const& ) { return false; };
//***************************************************************************
};//        E N D     S U B A L L O C A T O R 3 2      C L A S S
//***************************************************************************

//***************************************************************************
};//     E N D    A L L O C   &   C N T R E E    N A M E S P A C E
//***************************************************************************
namespace std
{
template < typename T>
void swap ( countertree::suballocator32_cnc < T > & A ,
            countertree::suballocator32_cnc < T > & B   ){ };
};
#endif
