//==============================================================================
// Set /////////////////////////////////////////////////////////////////////////
//==============================================================================
// Austin Quick, 2016 - 2018
//------------------------------------------------------------------------------
// https://github.com/Daskie/QHash
//------------------------------------------------------------------------------



#pragma once



#include <memory>

#include "Hash.hpp"



namespace qc {



namespace config {

namespace set {

constexpr size_t defNBuckets(16); // number of buckets when unspecified

}

}



//======================================================================================================================
// Set /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//======================================================================================================================
// Setup as a array of buckets, each having a linked list (not std::list) of
// nodes, each containing a hash, a pointer to the next node, and a value.
// Will always have a minimum of 1 bucket, but may have 0 size.
// Memory for the number of bucket's worth of nodes is pre-allocated. This is a
// huge performance boost with the cost of extra memory usage for non-full sets.
//------------------------------------------------------------------------------

template <typename V, typename H = Hash<V>>
class Set {

    static_assert(std::is_copy_constructible_v<V>, "value type must be copy constructable");

    //--------------------------------------------------------------------------
    // Types

    public:

    using key_type = V;
    using value_type = V;
    using hasher = H;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = size_t;
    using difference_type = ptrdiff_t;



    //==========================================================================
    // Node
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    private:

    struct Node {

        size_t hash;
        Node * next;
        V value;

        template <typename V_> Node(size_t hash, Node * next, V_ && value);

    };



    //==========================================================================
    // Iterator
    //--------------------------------------------------------------------------
    // Used to iterate through the set. Comes in mutable and const varieties.
    //--------------------------------------------------------------------------

    private:

    template <bool t_const> class Iterator;

    public:

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;



    //--------------------------------------------------------------------------
    // Instance Variables

    private:

    size_t m_size;				         // total number of elements
    size_t m_nBuckets;			         // number of buckets
    std::unique_ptr<Node *[]> m_buckets; // the buckets
    Node * m_nodeStore;                  // a supply of preallocated nodes (an optimization)
    bool m_rehashing;					 // the set is currently rehashing



    //==========================================================================
    // Set
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    explicit Set(size_t minNBuckets = config::set::defNBuckets);
    Set(const Set<V, H> & other);
    Set(Set<V, H> && other);
    template <typename InputIt> Set(InputIt first, InputIt last);
    explicit Set(std::initializer_list<V> values);



    //==========================================================================
    // ~Set
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    ~Set();



    //==========================================================================
    // operator=
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    Set & operator=(const Set<V, H> & other);
    Set & operator=(Set<V, H> && other);
    Set & operator=(std::initializer_list<V> values);



    //==========================================================================
    // swap
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    void swap(Set<V, H> & set);



    //==========================================================================
    // insert
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    std::pair<iterator, bool> insert(const V & value);
    std::pair<iterator, bool> insert(V && value);
    template <typename InputIt> void insert(InputIt first, InputIt last);
    void insert(std::initializer_list<V> values);

    std::pair<iterator, bool> insert_h(size_t hash, const V & value);
    std::pair<iterator, bool> insert_h(size_t hash, V && value);



    //==========================================================================
    // emplace
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:
    
    template <typename V_> std::pair<iterator, bool> emplace(V_ && value);

    template <typename V_> std::pair<iterator, bool> emplace_h(size_t hash, V_ && value);



    //==========================================================================
    // begin
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    iterator begin();
    const_iterator cbegin() const;



    //==========================================================================
    // end
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    iterator end();
    const_iterator cend() const;



    //==========================================================================
    // find
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    iterator find(const V & value);
    const_iterator find(const V & value) const;
    const_iterator cfind(const V & value) const;

    iterator find_h(size_t hash);
    const_iterator find_h(size_t hash) const;
    const_iterator cfind_h(size_t hash) const;



    //==========================================================================
    // erase
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    bool erase(const V & value);
    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);

    bool erase_h(size_t hash);



    //==========================================================================
    // count
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    size_t count(const V & value) const;

    size_t count_h(size_t hash) const;



    //==========================================================================
    // rehash
    //--------------------------------------------------------------------------
    // Resizes the set so that there are at lease minNBuckets buckets.
    // All elements are re-organized.
    // Relatively expensive method.
    //--------------------------------------------------------------------------

    public:

    void rehash(size_t minNBuckets);



    //==========================================================================
    // reserve
    //--------------------------------------------------------------------------
    // Ensures at least nBuckets are already allocated.
    //--------------------------------------------------------------------------

    public:

    void reserve(size_t nBuckets);



    //==========================================================================
    // clear
    //--------------------------------------------------------------------------
    // clears the set. all buckets are cleared. when finished, size = 0
    //--------------------------------------------------------------------------

    public:

    void clear();



    //==========================================================================
    // operator==
    //--------------------------------------------------------------------------
    // Returns whether the elements of the two sets are the same
    //--------------------------------------------------------------------------

    public:

    bool operator==(const Set<V, H> & m) const;



    //==========================================================================
    // operator!=
    //--------------------------------------------------------------------------
    // Returns whether the elements of the two sets are different
    //--------------------------------------------------------------------------

    public:

    bool operator!=(const Set<V, H> & m) const;



    //--------------------------------------------------------------------------
    // Accessors

    public:

    size_t size() const;

    bool empty() const;

    size_t bucket_count() const;

    size_t bucket_size(size_t bucketI) const;

    size_t bucket(const V & value) const;



    //--------------------------------------------------------------------------
    // Private Methods

    private:

    size_t detBucketI(size_t hash) const;

};



//======================================================================================================================
// Iterator ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//======================================================================================================================
// Basic iterator used to iterate forwards over the set.
// iterates forward over the bucket, then moves to the next bucket.
//------------------------------------------------------------------------------

template <typename V, typename H>
template <bool t_const> // may be E or const E
class Set<V, H>::Iterator {

    friend Set<V, H>;

    //--------------------------------------------------------------------------
    // Types

    using IV = std::conditional_t<t_const, const V, V>;

    using iterator_category = std::forward_iterator_tag;
    using value_type = IV;
    using difference_type = ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    //--------------------------------------------------------------------------
    // Instance Variables

    private:

    const Set<V, H> * m_set;
    size_t m_bucket;
    typename Set<V, H>::Node * m_node;



    //==========================================================================
    // Iterator
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    Iterator(const Set<V, H> & set);
    
    private:

    Iterator(const Set<V, H> & set, size_t bucket, typename Set<V, H>::Node * node);

    public:

    template <bool t_const_> Iterator(const Iterator<t_const_> & iterator);



    //==========================================================================
    // ~Iterator
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    ~Iterator() = default;



    //==========================================================================
    // operator=
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    template <bool t_const_> Iterator<t_const> & operator=(const Iterator<t_const_> & iterator);



    //==========================================================================
    // operator++
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    Iterator<t_const> & operator++();



    //==========================================================================
    // operator++ int
    //--------------------------------------------------------------------------
    // 

    //--------------------------------------------------------------------------

    Iterator<t_const> operator++(int);



    //==========================================================================
    // operator==
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    template <bool t_const_> bool operator==(const Iterator<t_const_> & it) const;



    //==========================================================================
    // operator!=
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    template <bool t_const_> bool operator!=(const Iterator<t_const_> & it) const;



    //==========================================================================
    // operator*
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    IV & operator*() const;



    //==========================================================================
    // operator->
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    IV * operator->() const;



    //==========================================================================
    // hash
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    size_t hash() const;

};



//======================================================================================================================
// Functions ///////////////////////////////////////////////////////////////////////////////////////////////////////////
//======================================================================================================================



//==============================================================================
// swap
//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------

template <typename V, typename H> void swap(Set<V, H> & m1, Set<V, H> & m2);



}



#include "Set.tpp"