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

namespace map {

constexpr size_t defNSlots(16); // number of buckets when unspecified

}

}



//======================================================================================================================
// Map /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//======================================================================================================================
// Setup as a array of buckets (buckets), each having a linked list (not
// std::list) of nodes, each containing a hash, a pointer to the next node, and
// an element value.
// Will always have a minimum of 1 bucket, but may have 0 size.
// Memory for the number of bucket's worth of nodes is pre-allocated. This is a
// huge performance boost with the cost of extra memory usage for un-full maps.
//------------------------------------------------------------------------------

template <typename K, typename E, typename H = Hash<K>>
class Map {

    static_assert(std::is_copy_constructible_v<K>, "key type must be copy constructable");
    static_assert(std::is_default_constructible_v<E>, "element type must by default constructable");
    static_assert(std::is_move_constructible_v<E>, "element type must by move constructable");

    //--------------------------------------------------------------------------
    // Types

    public:

    using V = std::pair<K, E>;

    using key_type = K;
    using mapped_type = E;
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
    // Serves as a node for Slot as a basic linked list.
    //--------------------------------------------------------------------------

    private:

    struct Node {

        size_t hash;
        Node * next;
        V value;

        template <typename K_, typename E_> Node(size_t hash, Node * next, K_ && key, E_ && element);

    };



    //==========================================================================
    // Iterator
    //--------------------------------------------------------------------------
    // Used to iterate through the map. Comes in mutable and const varieties.
    //--------------------------------------------------------------------------

    private:

    template <bool t_const> class Iterator;

    public:

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;



    //--------------------------------------------------------------------------
    // Instance Variables

    private:

    size_t m_size;				        // total number of elements
    size_t m_nSlots;			        // number of buckets
    std::unique_ptr<Node *[]> m_buckets;  // the buckets
    Node * m_nodeStore;                 // a supply of preallocated nodes (an optimization)
    bool m_rehashing;					// the map is currently rehashing



    //==========================================================================
    // Map
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    explicit Map(size_t minNSlots = config::map::defNSlots);
    Map(const Map<K, E, H> & other);
    Map(Map<K, E, H> && other);
    template <typename InputIt> Map(InputIt first, InputIt last);
    explicit Map(std::initializer_list<V> values);



    //==========================================================================
    // ~Map
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    ~Map();



    //==========================================================================
    // operator=
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    Map & operator=(const Map<K, E, H> & other);
    Map & operator=(Map<K, E, H> && other);
    Map & operator=(std::initializer_list<V> values);



    //==========================================================================
    // swap
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    void swap(Map<K, E, H> & map);



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
    
    template <typename K_, typename E_> std::pair<iterator, bool> emplace(K_ && key, E_ && element);

    template <typename K_, typename E_> std::pair<iterator, bool> emplace_h(size_t hash, K_ && key, E_ && element);



    //==========================================================================
    // at
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    E & at(const K & key) const;

    E & at_h(size_t hash) const;



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

    iterator find(const K & key);
    const_iterator find(const K & key) const;
    const_iterator cfind(const K & key) const;

    iterator find_h(size_t hash);
    const_iterator find_h(size_t hash) const;
    const_iterator cfind_h(size_t hash) const;



    //==========================================================================
    // find_e
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    iterator find_e(const E & element);
    const_iterator find_e(const E & element) const;
    const_iterator cfind_e(const E & element) const;



    //==========================================================================
    // operator[]
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    E & operator[](const K & key);
    E & operator[](K && key);

    E & access_h(size_t hash, const K & key);
    E & access_h(size_t hash, K && key);



    //==========================================================================
    // erase
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    bool erase(const K & key);
    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);

    bool erase_h(size_t hash);



    //==========================================================================
    // count
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    size_t count(const K & key) const;

    size_t count_h(size_t hash) const;



    //==========================================================================
    // rehash
    //--------------------------------------------------------------------------
    // Resizes the map so that there are at lease minNSlots buckets.
    // All elements are re-organized.
    // Relatively expensive method.
    //--------------------------------------------------------------------------

    public:

    void rehash(size_t minNSlots);



    //==========================================================================
    // reserve
    //--------------------------------------------------------------------------
    // Ensures at least nSlots are already allocated.
    //--------------------------------------------------------------------------

    public:

    void reserve(size_t nSlots);



    //==========================================================================
    // clear
    //--------------------------------------------------------------------------
    // clears the map. all buckets are cleared. when finished, size = 0
    //--------------------------------------------------------------------------

    public:

    void clear();



    //==========================================================================
    // operator==
    //--------------------------------------------------------------------------
    // Returns whether the elements of the two maps are the same
    //--------------------------------------------------------------------------

    public:

    bool operator==(const Map<K, E, H> & m) const;



    //==========================================================================
    // operator!=
    //--------------------------------------------------------------------------
    // Returns whether the elements of the two maps are different
    //--------------------------------------------------------------------------

    public:

    bool operator!=(const Map<K, E, H> & m) const;



    //--------------------------------------------------------------------------
    // Accessors

    public:

    size_t size() const;

    bool empty() const;

    size_t nSlots() const;
    size_t bucket_count() const;

    size_t bucketSize(size_t bucketI) const;
    size_t bucket_size(size_t bucketI) const;

    size_t bucket(const K & key) const;
    size_t bucket(const K & key) const;



    //--------------------------------------------------------------------------
    // Private Methods

    private:

    size_t detSlotI(size_t hash) const;

};



//======================================================================================================================
// Iterator ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//======================================================================================================================
// Basic iterator used to iterate forwards over the map.
// iterates forward over the bucket, then moves to the next bucket.
//------------------------------------------------------------------------------

template <typename K, typename E, typename H>
template <bool t_const> // may be E or const E
class Map<K, E, H>::Iterator {

    friend Map<K, E, H>;

    //--------------------------------------------------------------------------
    // Types

    using IE = std::conditional_t<t_const, const E, E>;
    using IV = std::conditional_t<t_const, const typename Map<K, E, H>::V, typename Map<K, E, H>::V>;

    using iterator_category = std::forward_iterator_tag;
    using value_type = IV;
    using difference_type = ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    //--------------------------------------------------------------------------
    // Instance Variables

    private:

    const Map<K, E, H> * m_map;
    size_t m_slot;
    typename Map<K, E, H>::Node * m_node;



    //==========================================================================
    // Iterator
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    public:

    Iterator(const Map<K, E, H> & map);
    
    private:

    Iterator(const Map<K, E, H> & map, size_t slot, typename Map<K, E, H>::Node * node);

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



    //==========================================================================
    // key
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    const K & key() const;



    //==========================================================================
    // element
    //--------------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------------

    IE & element() const;

};



//======================================================================================================================
// Functions ///////////////////////////////////////////////////////////////////////////////////////////////////////////
//======================================================================================================================



//==============================================================================
// swap
//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------

template <typename K, typename E, typename H> void swap(Map<K, E, H> & m1, Map<K, E, H> & m2);



}



#include "Map.tpp"