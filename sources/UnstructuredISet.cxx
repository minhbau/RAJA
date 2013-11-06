/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Implementation file for unstructured index set classes
 *
 * \author  Rich Hornung, Center for Applied Scientific Computing, LLNL
 * \author  Jeff Keasler, Applications, Simulations And Quality, LLNL
 *
 ******************************************************************************
 */

#include "RAJA/UnstructuredISet.hxx"

#include <cstdlib>
#include <iostream>
#include <algorithm>


namespace RAJA {


/*
*************************************************************************
*
* Public UnstructuredISet class methods.
*
*************************************************************************
*/

#if defined(RAJA_USE_CTOR_DELEGATION)
UnstructuredISet::UnstructuredISet(const Index_type* indx, Index_type len)
: UnstructuredISet(len)
{
   std::copy(indx, indx + len, m_indx); 
}

UnstructuredISet::UnstructuredISet(const std::vector<Index_type>& indx)
: UnstructuredISet(static_cast<Index_type>(indx.size()))
{
   std::copy(&indx[0], &indx[0] + m_len, m_indx); 
}

UnstructuredISet::UnstructuredISet(const UnstructuredISet& other)
: UnstructuredISet(other.m_len)
{
   std::copy(other.m_indx, other.m_indx + other.m_len, m_indx);
}
#else
UnstructuredISet::UnstructuredISet(const Index_type* indx, Index_type len)
: m_indx(0), m_len(len)
{
   allocateIndexData(m_len);
   std::copy(indx, indx + len, m_indx);
}

UnstructuredISet::UnstructuredISet(const std::vector<Index_type>& indx)
: m_indx(0), m_len(indx.size())
{
   allocateIndexData(m_len);
   std::copy(indx.begin(), indx.end(), m_indx);    
}

UnstructuredISet::UnstructuredISet(const UnstructuredISet& other)
: m_indx(0), m_len(other.m_len)
{
   allocateIndexData(m_len);
   std::copy(other.m_indx, other.m_indx + other.m_len, m_indx);
}
#endif

UnstructuredISet& UnstructuredISet::operator=(const UnstructuredISet& rhs)
{
   if ( &rhs != this ) {
      UnstructuredISet copy(rhs);
      this->swap(copy);
   }
   return *this;
}

UnstructuredISet::~UnstructuredISet()
{
   if (m_indx) free( m_indx );
}

void UnstructuredISet::swap(UnstructuredISet& other)
{
   using std::swap;
   swap(m_len, other.m_len);
   swap(m_indx, other.m_indx);
}

void UnstructuredISet::print(std::ostream& os) const
{
   os << "\nUnstructuredISet : length = "
      << m_len << std::endl;
   for (Index_type i = 0; i < m_len; ++i) {
      os << "\t" << m_indx[i] << std::endl;
   }
}

/*
*************************************************************************
*
* Private UnstructuredISet class methods.
*
*************************************************************************
*/
UnstructuredISet::UnstructuredISet(Index_type len) 
: m_len(len), m_indx(0)
{
   if ( len > 0 ) {
      posix_memalign((void **)&m_indx, DATA_ALIGN, m_len*sizeof(Index_type)) ;
   } 
}

void UnstructuredISet::allocateIndexData(Index_type len)
{
   if ( len > 0 ) {
      posix_memalign((void **)&m_indx, DATA_ALIGN, len*sizeof(Index_type)) ;
   } 
}


}  // closing brace for namespace statement