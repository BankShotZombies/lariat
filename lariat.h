/**
 * @file lariat.h
 * @author Adam Lonstein (adam.lonstein@digipen.com)
 * @brief Lariat is a "linked list of arrays". This data structure has the option
 *        to insert and erase values just as you would with a vector, but instead
 *        of a dynamically resizing array, it adds nodes to the list with fixed size 
 *        arrays of data, and splits/deletes those nodes as values are added/removed. 
 *        it can insert, erase, index search, find, and compact.
 *        
 * @date 02-13-2024
 */

////////////////////////////////////////////////////////////////////////////////
#ifndef LARIAT_H
#define LARIAT_H
////////////////////////////////////////////////////////////////////////////////

#include <string>     // error strings
#include <utility>    // error strings
#include <cstring>     // memcpy

class LariatException : public std::exception {
  private:  
    int m_ErrCode;
    std::string m_Description;

  public:
    LariatException(int ErrCode, const std::string& Description) :
        m_ErrCode(ErrCode), m_Description(Description) {}

    virtual int code(void) const { 
      return m_ErrCode; 
    }

    virtual const char *what(void) const throw() {
      return m_Description.c_str();
    }

    virtual ~LariatException() throw() {
    }

    enum LARIAT_EXCEPTION {E_NO_MEMORY, E_BAD_INDEX, E_DATA_ERROR};
};

// forward declaration for 1-1 operator<< 
template<typename T, int Size> 
class Lariat;

template<typename T, int Size> 
std::ostream& operator<< (std::ostream& os, Lariat<T, Size> const & rhs);

template <typename T, int Size>
class Lariat 
{
    // allow lariat to have access to other lariat of different parameters
    template<typename U, int USize>
    friend class Lariat;

    public:

        Lariat();                  // default constructor                        
        Lariat( Lariat const& rhs); // copy constructor
        ~Lariat(); // destructor
        
        // copy constructor of different parameters
        template <typename U, int USize>
        Lariat(const Lariat<U, USize>& rhs);

        // assignment operator
        Lariat<T, Size>& operator=(const Lariat<T, Size>& other);

        // inserts
        void insert(int index, const T& value);
        void push_back(const T& value);
        void push_front(const T& value);

        // deletes
        void erase(int index);
        void pop_back();
        void pop_front();

        //access
        T&       operator[](int index);       // for l-values
        const T& operator[](int index) const; // for r-values
        T&       first();
        T const& first() const;
        T&       last();
        T const& last() const;

        unsigned find(const T& value) const;       // returns index, size (one past last) if not found

        friend std::ostream& operator<< <T,Size>( std::ostream &os, Lariat<T, Size> const & list );

        size_t size(void) const;   // total number of items (not nodes)
        void clear(void);          // make it empty

        void compact();             // push data in front reusing empty positions and delete remaining nodes
    private:
        struct LNode { // DO NOT modify provided code
            LNode *next  = nullptr;
            LNode *prev  = nullptr;
            int    count = 0;         // number of items currently in the node
            T values[ Size ];
        };

        struct ElementInfo
        {
            LNode* node;
            int localIndex;
        };

        // DO NOT modify provided code
        LNode *head_;           // points to the first node
        LNode *tail_;           // points to the last node
        int size_;              // the number of items (not nodes) in the list
        mutable int nodecount_; // the number of nodes in the list
        int asize_;             // the size of the array within the nodes

    private:

        // Pushes a value when the list is empty
        void push_first_value(const T& value);

        // Pushes back a value in a specific node.
        void push_back_in_node(LNode* node, const T& value);

        // This insert a value in a full node.
        void insert_in_full_node(LNode* node, int localIndex, const T& value);

        // takes a full node and splits into two nodes of an aproximately equivalent number of elements.
        LNode* split(LNode* node);

        // takes a global index to find in the list and returns both a pointer to the node in the list and the local index of the element in the returned node.
        ElementInfo find_element(int index);

        // swaps every element between the index and the last element in the node's array in place.
        void shiftUp(LNode* node, int localIndex);
        // swaps each element from local index onward with the element immediately before it.
        void shiftDown(LNode* node, int localIndex);

        // deletes a node
        void deleteNode(LNode* node);

};

#include "lariat.cpp"

#endif // LARIAT_H
