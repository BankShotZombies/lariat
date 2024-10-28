/**
 * @file lariat.cpp
 * @author Adam Lonstein (adam.lonstein@digipen.com)
 * @brief Lariat is a "linked list of arrays". This data structure has the option
 *        to insert and erase values just as you would with a vector, but instead
 *        of a dynamically resizing array, it adds nodes to the list, and
 *        splits/deletes those nodes as values are added/removed. it can insert, erase,
 *        index search, find, and compact.
 *        
 * @date 02-13-2024
 */

#include <iostream>
#include <iomanip>

#if 1

/**
 * @brief Outputs lariat to stream
 * 
 * @param os - outstream to output to
 * @param list - lariat to print
 */
template <typename T, int Size>
std::ostream& operator<<( std::ostream &os, Lariat<T, Size> const & list )
{
    typename Lariat<T, Size>::LNode * current  = list.head_;
    int index = 0;
    while ( current ) {
        os << "Node starting (count " << current->count << ")\n";
        for ( int local_index = 0; local_index < current->count; ++local_index ) {
            os << index << " -> " << current->values[local_index] << std::endl;
            ++index;
        }
        os << "-----------\n";
        current = current->next;
    }
    return os;
}

/**
 * @brief Construct a new Lariat< T,  Size>:: Lariat object
 * 
 * @tparam T - the type the container will be
 * @tparam Size - size of each node in container
 */
template<typename T, int Size>
Lariat<T, Size>::Lariat() : head_(nullptr), tail_(nullptr), size_(0), nodecount_(0), asize_(Size)
{
    
}

/**
 * @brief Copy constructor
 * 
 * @tparam T - the type the container will be
 * @tparam Size - size of each node in container
 * @param rhs - lariat to copy
 */
template<typename T, int Size>
Lariat<T, Size>::Lariat(const Lariat& rhs) : head_(nullptr), tail_(nullptr), size_(0), nodecount_(0), asize_(Size)
{
    LNode* walker = rhs.head_;

    while(walker != nullptr)
    {
        // add all values to copied lariat
        for(int i = 0; i < walker->count; ++i)
        {
            push_back(walker->values[i]);
        }

        walker = walker->next;
    }
}

/**
 * @brief Copy constructor for lariat of different template specifications
 * 
 * @tparam T - the type the container will be
 * @tparam Size - size of each node in container
 * @tparam U - the type the lariat being copied will be
 * @tparam USize - the size the lariat being copied will be
 * @param rhs 
 */
template<typename T, int Size>
template<typename U, int USize>
Lariat<T, Size>::Lariat(const Lariat<U, USize>& rhs) : head_(nullptr), tail_(nullptr), size_(0), nodecount_(0), asize_(Size)
{
    typename Lariat<U, USize>::LNode* walker = rhs.head_;

    while(walker != nullptr)
    {
        // add all values to copied lariat
        for(int i = 0; i < walker->count; ++i)
        {
            push_back(static_cast<T>(walker->values[i]));
        }

        walker = walker->next;
    }
}

/**
 * @brief Assignment operator for lariat
 * 
 * @param other - lariat that will be assigned
 */
template<typename T, int Size>
Lariat<T, Size>& Lariat<T, Size>::operator=(const Lariat<T, Size>& other)
{
    clear(); // clear all current values

    LNode* walker = other.head_;

    while(walker != nullptr)
    {
        // add all values to new assigned lariat
        for(int i = 0; i < walker->count; ++i)
        {
            push_back(walker->values[i]);
        }

        walker = walker->next;
    }

    // return the copied lariat
    return *this;
}

/**
 * @brief Destroy the Lariat< T,  Size>:: Lariat object
 */
template<typename T, int Size>
Lariat<T, Size>::~Lariat()
{
    // clear and delete all nodes and values
    clear();
}

/**
 * @brief Insert an element into the data structure at the index, between the 
 *        element at [index - 1] and the element at [index]
 * 
 * @param index - index to insert
 * @param value - value to insert
 */
template<typename T, int Size>
void Lariat<T, Size>::insert(int index, const T& value)
{

    // if the index is invalid, throw exception
    if(index < 0 || index > size_)
    {
        throw LariatException(LariatException::E_BAD_INDEX, "Subscript is out of range");
    }

    // if the index is last, simply push back
    if(index == size_)
    {
        push_back(value);

        return;
    }
    else if(index == 0) // if the index is first, simply push front
    {
        push_front(value);

        return;
    }

    ElementInfo insertInfo = find_element(index);

    // If insert node has room
    if(insertInfo.node->count < asize_)
    {
        // Put value at the end of the node
        insertInfo.node->values[insertInfo.node->count] = value;
        insertInfo.node->count++;
        size_++;

        // Put the added value at the correct index and shift all subsequent values up
        shiftUp(insertInfo.node, insertInfo.localIndex);
    }
    else
    {
        // if the node doesn't have room, split it and insert
        insert_in_full_node(insertInfo.node, insertInfo.localIndex, value);
    }
}

/**
 * @brief push a value to the back of the container
 * 
 * @param value - the value to push
 */
template<typename T, int Size>
void Lariat<T, Size>::push_back(const T& value)
{
    // If it's the first value being inserted
    if(head_ == nullptr && tail_ == nullptr)
    {
        push_first_value(value);

        return;
    }

    // If the tail is full
    if(tail_->count == asize_)
    {
        split(tail_);
    }

    // Put the value in the available node
    push_back_in_node(tail_, value);
}

/**
 * @brief push a value to the front of the container
 * 
 * @param value - value to push
 */
template<typename T, int Size>
void Lariat<T, Size>::push_front(const T& value)
{
    if(head_ == nullptr)
    {
        // If head is empty, just push value
        push_first_value(value);

        return;
    }
    else if(head_->count == asize_)
    {
        // Put the last value in list at the front and shift all subsequent values up
        shiftUp(head_, 0);

        // Get the overflow value
        T overflow = head_->values[0];
        
        // Set the value at front to correct value
        head_->values[0] = value;

        // Perform the split
        LNode* splitNode = split(head_);

        // Push the overflow value back into the split node
        push_back_in_node(splitNode, overflow);
    }
    else
    {
        // If the head has room, put the value at the end
        head_->values[head_->count] = value;
        head_->count++;
        size_++;

        // Put the added value at the front and shift all subsequent values up
        shiftUp(head_, 0);
    }
}

/**
 * @brief Erase a value at an index
 * 
 * @param index - index to erase
 */
template<typename T, int Size>
void Lariat<T, Size>::erase(int index)
{
    if(index == 0)
    {
        pop_front();

        return;
    }
    else if(index == size_)
    {
        pop_back();

        return;
    }

    Lariat<T, Size>::ElementInfo elementInfo = find_element(index);

    // Shift all the elements in the node beyond the local index left one element, covering the element being erased.
    shiftDown(elementInfo.node, elementInfo.localIndex);

    // update count and size values
    elementInfo.node->count--;
    size_--;

    // delete the node if it's empty
    if(elementInfo.node->count == 0)
    {
        deleteNode(elementInfo.node);
    }
}

/**
 * @brief delete the last value in the container
 */
template<typename T, int Size>
void Lariat<T, Size>::pop_back()
{
    // update count and size values
    tail_->count--;
    size_--;

    // delete the tail if it's empty
    if(tail_->count == 0)
    {
        deleteNode(tail_);
    }
}

/**
 * @brief delete the first value in the container
 */
template<typename T, int Size>
void Lariat<T, Size>::pop_front()
{
    if(head_ == nullptr)
        return;

    // Shift all elements from the first value down
    shiftDown(head_, 0);

    // update count and size values
    head_->count--;
    size_--;

    // delete the head if it's empty
    if(head_->count == 0)
    {
        deleteNode(head_);
    }
}

/**
 * @brief Index operator
 * @param index - index to get
 */
template<typename T, int Size>
T& Lariat<T, Size>::operator[](int index)
{
    Lariat<T, Size>::ElementInfo elementInfo = find_element(index);

    return elementInfo.node->values[elementInfo.localIndex];
}

/**
 * @brief Index operator
 * @param index - index to get
 */
template<typename T, int Size>
const T& Lariat<T, Size>::operator[](int index) const
{
    Lariat<T, Size>::ElementInfo elementInfo = find_element(index);

    return elementInfo.node->values[elementInfo.localIndex];
}

/**
 * @brief returns the first value of the container
 */
template<typename T, int Size>
T& Lariat<T, Size>::first()
{
    return head_->values[0];
}

/**
 * @brief returns the first value of the container
 */
template<typename T, int Size>
const T& Lariat<T, Size>::first() const
{
    return head_->values[0];
}

/**
 * @brief returns the last value of the container
 */
template<typename T, int Size>
T& Lariat<T, Size>::last()
{
    return tail_->values[tail_->count - 1];
}

/**
 * @brief returns the last value of the container
 */
template<typename T, int Size>
const T& Lariat<T, Size>::last() const
{
    return tail_->values[tail_->count - 1];
}

/**
 * @brief Walks through the list and finds the index of matching value
 * 
 * @param value - value to find
 * @return index of the value. if it can't find found, returns the size of the list.
 */
template<typename T, int Size>
unsigned Lariat<T, Size>::find(const T& value) const
{
    LNode* walker = head_;

    int index = 0;

    while(walker != nullptr)
    {
        // go through every value in the node
        for(int i = 0; i < walker->count; ++i, ++index)
        {
            // if the value matches, return
            if(walker->values[i] == value)
            {
                return index;
            }
        }

        walker = walker->next;
    }

    // otherwise return the size of the list
    return size_;
}

/**
 * @brief returns the size of the list
 */
template<typename T, int Size>
size_t Lariat<T, Size>::size(void) const
{
    return size_;
}

/**
 * @brief deletes and clears every node in the list
 */
template<typename T, int Size>
void Lariat<T, Size>::clear(void)
{
    LNode* walker = head_;

    // walk through the list
    while(walker != nullptr)
    {
        // delete all nodes
        LNode* next = walker->next;

        delete walker;

        walker = next;
    }

    // reset values
    size_ = 0;
    nodecount_ = 0;

    head_ = nullptr;
    tail_ = nullptr;
}

/**
 * @brief Compact takes all the data stored in the linked list and moves it into the
          smallest number of nodes possible. Then it frees all empty nodes at the
          end of the list.
 * 
 */
template<typename T, int Size>
void Lariat<T, Size>::compact()
{
    if(head_ == nullptr)
    {
        return;
    }

    LNode* leftFoot = head_;
    LNode* rightFoot = head_->next;

    // loop through the list with both feet until the left node is not already full and the right foot hasn't walked off the list.
    while(leftFoot->count == asize_ && rightFoot != nullptr)
    {
        leftFoot = leftFoot->next;
        rightFoot = rightFoot->next;
    }

    // walk through the list while the right foot hasn't lost the list.
    while(rightFoot != nullptr)
    {
        int rightFootCount = rightFoot->count;

        rightFoot->count = 0;

        // step the left foot to the next node if it is full
        if(leftFoot->count == asize_)
        {
            leftFoot = leftFoot->next;
        }

        for(int i = 0; i < rightFootCount; ++i)
        {
            // move the values from the right foot to the left foot
            leftFoot->values[leftFoot->count] = rightFoot->values[i];
            leftFoot->count++;

            // step the left foot to the next node if it is full
            if(leftFoot->count == asize_)
            {
                leftFoot = leftFoot->next;
            }
        }

        rightFoot = rightFoot->next;
    }

    // remove all the extra nodes from the end of the list
    while(tail_->count == 0)
    {
        deleteNode(tail_);
    }
}

/**
 * @brief Pushes a value when the list is empty
 * 
 * @param value - value to push
 */
template<typename T, int Size>
void Lariat<T, Size>::push_first_value(const T& value)
{
    LNode* newNode;

    // create the first node
    try
    {
        newNode = new LNode;
    }
    catch(const std::bad_alloc& e)
    {
        throw LariatException(LariatException::E_NO_MEMORY, e.what());
    }

    // set its values
    newNode->prev = nullptr;
    newNode->next = nullptr;

    // both the head and tail will be this first node
    head_ = newNode;
    tail_ = newNode;

    // assign the first value
    newNode->values[tail_->count] = value;

    newNode->count++;

    // update size counts
    size_++;
    nodecount_++;
}

/**
 * @brief Pushes back a value in a specific node.
 * 
 * @param node - node to push value in
 * @param value - value to push
 */
template<typename T, int Size>
void Lariat<T, Size>::push_back_in_node(LNode* node, const T& value)
{
    if(node->count >= asize_)
    {
        node = split(node); // If the node doesn't have room, split it
    }

    // Push the value
    node->values[node->count] = value;
    node->count++;
    size_++;
}

/**
 * @brief This insert a value in a full node. It will have to split the node to achieve this. The function has documentation next to each step of how the
 *        node values change.
 * 
 *        Let's say the node has values
 *        1, 5, 6, 7, 8, 2, 3, 4
 *        We want to insert 9 at local index 5 so it turns to
 *        1, 5, 6, 7, 8 split 9, 2, 3, 4
 * 
 * @param node - node to insert
 * @param localIndex - local index of node to insert
 * @param value - value to insert
 */
template<typename T, int Size>
void Lariat<T, Size>::insert_in_full_node(LNode* node, int localIndex, const T& value)
{
    // Put the last value in list at the correct index and shift all subsequent values up
    shiftUp(node, localIndex); // 1, 5, 6, 7, 8, 4, 2, 3

    // Get the overflow value
    T overflow = node->values[localIndex]; // overflow = 4
    
    // Set the value at index to correct value
    node->values[localIndex] = value; // 1, 5, 6, 7, 8, 9, 2, 3

    // Perform the split
    LNode* splitNode = split(node); // 1, 5, 6, 7, 8 split 9, 2, 3

    // Push the overflow value back into the split node
    push_back_in_node(splitNode, overflow); // 1, 5, 6, 7, 8 split 9, 2, 3, 4
}

/**
 * @brief takes a full node and splits into two nodes of an aproximately 
 *        equivalent number of elements.
 * 
 * @param node - the node to split
 * 
 * @return - returns the second node created as a result of the split
 */
template<typename T, int Size>
typename Lariat<T, Size>::LNode* Lariat<T, Size>::split(LNode* node)
{
    bool isTail = node == tail_;

    int numSplit = node->count / 2; // The number of elements to move to the split node

    // if it's even, keep an extra element in the origianal node. this is so when we add a value to the split node, they will be approximately same length.
    if(node->count % 2 == 0)
    {
        numSplit -= 1;
    }

    LNode* splitNode;

    // create the split node
    try
    {
        splitNode = new LNode;
    }
    catch(const std::bad_alloc& e)
    {
        throw LariatException(LariatException::E_NO_MEMORY, e.what());
    }

    // Move the elements into the split node
    for(int i = node->count - numSplit, j = 0; i < node->count; ++i, ++j)
    {
        splitNode->values[j] = node->values[i];
    }

    // Update the node counts
    splitNode->count += numSplit;
    node->count -= numSplit;

    // Update the node values
    splitNode->next = node->next;
    node->next = splitNode;
    splitNode->prev = node;

    if(splitNode->next != nullptr)
    {
        splitNode->next->prev = splitNode;
    }

    nodecount_++;

    // If we split the tail, update the tail
    if(isTail)
    {
        tail_ = splitNode;
    }

    return splitNode;
}

/**
 * @brief takes a global index to find in the list and returns both a pointer to the 
 *        node in the list and the local index of the element in the returned node.
 * 
 * @param index - global index to find
 * @return struct with node and localindex of node
 */
template<typename T, int Size>
typename Lariat<T, Size>::ElementInfo Lariat<T, Size>::find_element(int index)
{
    ElementInfo info;

    int indexSum = 0; // Keeps track of the sum of the index size as we walk through the list

    LNode* walker = head_;

    // Walk until we get to the node
    while (indexSum + walker->count <= index)
    {
        indexSum += walker->count;

        walker = walker->next;
    }

    info.node = walker;
    info.localIndex = index - indexSum; // Calculate the local index

    return info;
}

/**
 * @brief swaps every element between the index and the last element in the node's array
          in place.
 * 
 * @param node - node to shift
 * @param localIndex - local index to start the shift at
 */
template<typename T, int Size>
void Lariat<T, Size>::shiftUp(LNode* node, int localIndex)
{
    for(int i = localIndex + 1; i < node->count; ++i)
    {
        // swap every element from the local index value onward with the local index value
        T temp = node->values[localIndex];
        node->values[localIndex] = node->values[i];
        node->values[i] = temp;
    }
}

/**
 * @brief swaps each element from local index onward with the element immediately before it.
 * 
 * @param node - node to perform shift
 * @param localIndex - local index to start shift
 */
template<typename T, int Size>
void Lariat<T, Size>::shiftDown(LNode* node, int localIndex)
{
    for(int i = localIndex; i < node->count; ++i)
    {
        // swap every node with the one before it
        node->values[i] = node->values[i + 1];
    }
}

/**
 * @brief deletes a node
 * 
 * @param node - node to delete
 */
template<typename T, int Size>
void Lariat<T, Size>::deleteNode(LNode* node)
{
    if(node == head_)
    {
        // return if there is nothing to delete
        if(head_ == nullptr)
        {
            return;
        }

        // if only the head exists
        if(head_->next == nullptr)
        {
            // delete it and reset head and tail
            delete head_;

            head_ = nullptr;
            tail_ = nullptr;
        }
        else
        {
            // otherwise, delete the head and set the head to its next
            LNode* temp = head_->next;

            delete head_;

            head_ = temp;
        }
    }
    else if(node == tail_)
    {
        LNode* temp = tail_->prev;

        // set the node before the tail to its proper value
        temp->next = nullptr;
        tail_->next = nullptr;

        delete tail_;

        // set the tail to what its previous was
        tail_ = temp;
    }
    else
    {
        // update the prev/next values of the node before and after
        node->prev->next = node->next;
        node->next->prev = node->prev;

        // delete the node
        delete node;
    }

    // a node was removed
    nodecount_--;
}

#else // fancier 
#endif
