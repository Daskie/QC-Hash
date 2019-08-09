namespace qc {

// Map
//==============================================================================

//=== Public Functions =========================================================

// Map::operator==
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
inline bool operator==(const QC_MAP & s1, const QC_MAP & s2) {
    if (s1.m_size != s2.m_size) {
        return false;
    }

    if (&s1 == &s2) {
        return true;
    }

    for (const auto & v : s1) {
        if (!s2.contains(v)) {
            return false;
        }
    }

    return true;
}

// Map::operator!=
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
inline bool operator!=(const QC_MAP & s1, const QC_MAP & s2) {
    return !(s1 == s2);
}

// Map::swap
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
inline void swap(QC_MAP & s1, QC_MAP & s2) noexcept {
    s1.swap(s2);
}

//=== Public Methods ===========================================================

// Map::Map
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
QC_MAP::Map(size_t minCapacity, const H & hash, const E & equal, const A & alloc) :
    m_size(),
    m_bucketCount(minCapacity <= config::map::minCapacity ? config::map::minBucketCount : detail::hash::ceil2(minCapacity << 1)),
    m_buckets(nullptr),
    m_hash(hash),
    m_equal(equal),
    m_alloc(alloc)
{}

QC_MAP_TEMPLATE
QC_MAP::Map(size_t minCapacity, const A & alloc) :
    Map(minCapacity, H(), E(), alloc)
{}

QC_MAP_TEMPLATE
QC_MAP::Map(size_t minCapacity, const H & hash, const A & alloc) :
    Map(minCapacity, hash, E(), alloc)
{}

QC_MAP_TEMPLATE
QC_MAP::Map(const A & alloc) :
    Map(config::map::minCapacity, H(), E(), alloc)
{}

QC_MAP_TEMPLATE
template <typename It>
QC_MAP::Map(It first, It last, size_t minCapacity, const H & hash, const E & equal, const A & alloc) :
    Map(minCapacity ? minCapacity : std::distance(first, last), hash, equal, alloc)
{
    insert(first, last);
}

QC_MAP_TEMPLATE
template <typename It>
QC_MAP::Map(It first, It last, size_t minCapacity, const A & alloc) :
    Map(first, last, minCapacity, H(), E(), alloc)
{}

QC_MAP_TEMPLATE
template <typename It>
QC_MAP::Map(It first, It last, size_t minCapacity, const H & hash, const A & alloc) :
    Map(first, last, minCapacity, hash, E(), alloc)
{}

QC_MAP_TEMPLATE
QC_MAP::Map(std::initializer_list<T> entries, size_t minCapacity, const H & hash, const E & equal, const A & alloc) :
    Map(minCapacity ? minCapacity : entries.size(), hash, equal, alloc)
{
    insert(entries);
}

QC_MAP_TEMPLATE
QC_MAP::Map(std::initializer_list<T> entries, size_t minCapacity, const A & alloc) :
    Map(entries, minCapacity, H(), E(), alloc)
{}

QC_MAP_TEMPLATE
QC_MAP::Map(std::initializer_list<T> entries, size_t minCapacity, const H & hash, const A & alloc) :
    Map(entries, minCapacity, hash, E(), alloc)
{}

QC_MAP_TEMPLATE
QC_MAP::Map(const Map & other) :
    Map(other, std::allocator_traits<A>::select_on_container_copy_construction(other.m_alloc))
{}

QC_MAP_TEMPLATE
QC_MAP::Map(const Map & other, const A & alloc) :
    m_size(other.m_size),
    m_bucketCount(other.m_bucketCount),
    m_buckets(nullptr),
    m_hash(other.m_hash),
    m_equal(other.m_equal),
    m_alloc(alloc)
{
    m_allocate();
    m_copyBuckets(other.m_buckets);
}

QC_MAP_TEMPLATE
QC_MAP::Map(Map && other) :
    Map(std::move(other), std::move(other.m_alloc))
{}

QC_MAP_TEMPLATE
QC_MAP::Map(Map && other, const A & alloc) :
    Map(std::move(other), A(alloc))
{}

QC_MAP_TEMPLATE
QC_MAP::Map(Map && other, A && alloc) :
    m_size(other.m_size),
    m_bucketCount(other.m_bucketCount),
    m_buckets(other.m_buckets),
    m_hash(std::move(other.m_hash)),
    m_equal(std::move(other.m_equal)),
    m_alloc(std::move(alloc))
{
    other.m_size = 0u;
    other.m_bucketCount = 0u;
    other.m_buckets = nullptr;
}

// Map::~Map
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
QC_MAP::~Map() {
    if (m_buckets) {
        m_clear<false>();
        m_deallocate();
    }
}

// Map::operatator=
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
QC_MAP & QC_MAP::operator=(std::initializer_list<T> entries) {
    clear();
    insert(entries);

    return *this;
}

QC_MAP_TEMPLATE
QC_MAP & QC_MAP::operator=(const Map & other) {
    if (&other == this) {
        return *this;
    }

    if (m_buckets) {
        m_clear<false>();
        if (m_bucketCount != other.m_bucketCount || m_alloc != other.m_alloc) {
            m_deallocate();
        }
    }

    m_size = other.m_size;
    m_bucketCount = other.m_bucketCount;
    m_hash = other.m_hash;
    m_equal = other.m_equal;
    if constexpr (AllocatorTraits::propagate_on_container_copy_assignment::value) {
        m_alloc = std::allocator_traits<A>::select_on_container_copy_construction(other.m_alloc);
    }

    if (other.m_buckets) {
        if (!m_buckets) {
            m_allocate();
        }
        m_copyBuckets(other.m_buckets);
    }

    return *this;
}

QC_MAP_TEMPLATE
QC_MAP & QC_MAP::operator=(Map && other) noexcept {
    if (&other == this) {
        return *this;
    }

    if (m_buckets) {
        m_clear<false>();
        m_deallocate();
    }

    m_size = other.m_size;
    m_bucketCount = other.m_bucketCount;
    m_hash = std::move(other.m_hash);
    m_equal = std::move(other.m_equal);
    if constexpr (AllocatorTraits::propagate_on_container_move_assignment::value) {
        m_alloc = std::move(other.m_alloc);
    }

    if (AllocatorTraits::propagate_on_container_move_assignment::value || m_alloc == other.m_alloc) {
        m_buckets = other.m_buckets;
        other.m_buckets = nullptr;
    }
    else {
        m_allocate();
        m_moveBuckets(other.m_buckets);
        other.m_clear<false>();
        other.m_deallocate();
    }

    other.m_size = 0u;
    other.m_bucketCount = 0u;

    return *this;
}

// Map::insert
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::insert(const T & entry) -> std::pair<iterator, bool> {
    return emplace(entry);
}

QC_MAP_TEMPLATE
auto QC_MAP::insert(T && entry) -> std::pair<iterator, bool> {
    return emplace(std::move(entry));
}

QC_MAP_TEMPLATE
template <typename It>
void QC_MAP::insert(It first, It last) {
    while (first != last) {
        emplace(*first);
        ++first;
    }
}

QC_MAP_TEMPLATE
void QC_MAP::insert(std::initializer_list<T> entries) {
    for (const T & entry : entries) {
        emplace(entry);
    }
}

// Map::emplace
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::emplace(const T & entry) -> std::pair<iterator, bool> {
    if constexpr (k_isSet) {
        return try_emplace(entry);
    }
    else {
        return try_emplace(entry.first, entry.second);
    }
}

QC_MAP_TEMPLATE
auto QC_MAP::emplace(T && entry) -> std::pair<iterator, bool> {
    if constexpr (k_isSet) {
        return try_emplace(std::move(entry));
    }
    else {
        return try_emplace(std::move(entry.first), std::move(entry.second));
    }
}

QC_MAP_TEMPLATE
template <typename K_, typename V_>
auto QC_MAP::emplace(K_ && key, V_ && val) -> std::pair<iterator, bool> {
    static_assert(!k_isSet, "This is not a set operation");
    return try_emplace(std::forward<K_>(key), std::forward<V_>(val));
}

QC_MAP_TEMPLATE
template <typename... KArgs, typename... VArgs>
auto QC_MAP::emplace(std::piecewise_construct_t, std::tuple<KArgs...> && kArgs, std::tuple<VArgs...> && vArgs) -> std::pair<iterator, bool> {
    static_assert(!k_isSet, "This is not a set operation");
    return m_emplace(std::move(kArgs), std::move(vArgs), std::index_sequence_for<KArgs...>(), std::index_sequence_for<VArgs...>());
}

QC_MAP_TEMPLATE
template <typename KTuple, typename VTuple, size_t... t_kIndices, size_t... t_vIndices>
auto QC_MAP::m_emplace(KTuple && kTuple, VTuple && vTuple, std::index_sequence<t_kIndices...>, std::index_sequence<t_vIndices...>) -> std::pair<iterator, bool> {
    return try_emplace(K(std::get<t_kIndices>(std::move(kTuple))...), std::get<t_vIndices>(std::move(vTuple))...);
}

// Map::try_emplace
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
template <typename... VArgs>
auto QC_MAP::try_emplace(const K & key, VArgs &&... vargs) -> std::pair<iterator, bool> {
    return m_try_emplace(m_hash(key), key, std::forward<VArgs>(vargs)...);
}

QC_MAP_TEMPLATE
template <typename... VArgs>
auto QC_MAP::try_emplace(K && key, VArgs &&... vargs) -> std::pair<iterator, bool> {
    return m_try_emplace(m_hash(key), std::move(key), std::forward<VArgs>(vargs)...);
}

QC_MAP_TEMPLATE
template <typename K_, typename... VArgs>
auto QC_MAP::m_try_emplace(size_t hash, K_ && key, VArgs &&... vargs) -> std::pair<iterator, bool> {
    static_assert(sizeof...(VArgs) == 0u || std::is_default_constructible_v<V>, "Value type must be default constructible");

    if (!m_buckets) m_allocate();
    size_t i(m_indexOf(hash));
    Dist dist(1u);

    while (true) {
        Bucket & bucket(m_buckets[i]);

        // Can be inserted
        if (bucket.dist < dist) {
            if (m_size >= (m_bucketCount >> 1)) {
                m_rehash(m_bucketCount << 1);
                return m_try_emplace(hash, std::forward<K_>(key), std::forward<VArgs>(vargs)...);
            }

            // Talue here has smaller dist, robin hood
            if (bucket.dist) {
                m_propagate(bucket.entry(), i + 1u, bucket.dist + 1u);
                bucket.entry().~T();
            }

            // Open slot
            AllocatorTraits::construct(m_alloc, &bucket.key, std::forward<K_>(key));
            if constexpr (!k_isSet) {
                AllocatorTraits::construct(m_alloc, &bucket.val, std::forward<VArgs>(vargs)...);
            }

            bucket.dist = dist;
            ++m_size;
            return { iterator(&bucket), true };
        }

        // Talue already exists
        if (m_equal(bucket.key, key)) {
            return { iterator(&bucket), false };
        }

        ++i;
        ++dist;

        if (i >= m_bucketCount) i = 0u;
    }

    // Will never reach reach this return
    return { end(), false };
}

QC_MAP_TEMPLATE
void QC_MAP::m_propagate(T & entry, size_t i, Dist dist) {
    while (true) {
        if (i >= m_bucketCount) i = 0u;
        Bucket & bucket(m_buckets[i]);

        if (!bucket.dist) {
            AllocatorTraits::construct(m_alloc, &bucket.entry(), std::move(entry));
            bucket.dist = dist;
            return;
        }

        if (bucket.dist < dist) {
            std::swap(entry, bucket.entry());
            std::swap(dist, bucket.dist);
        }

        ++i;
        ++dist;
    }
}

// Map::erase
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::erase(const K & key) {
    iterator it(find(key));
    if (it == end()) {
        return 0u;
    }
    m_erase(it);
    if (m_size <= (m_bucketCount >> 3) && m_bucketCount > config::set::minBucketCount) {
        m_rehash(m_bucketCount >> 1);
    }
    return 1u;
}

QC_MAP_TEMPLATE
auto QC_MAP::erase(const_iterator position) -> iterator {
    iterator endIt(end());
    if (position != endIt) {
        m_erase(position);
        if (m_size <= (m_bucketCount >> 3) && m_bucketCount > config::map::minBucketCount) {
            m_rehash(m_bucketCount >> 1);
            endIt = end();
        }
    }
    return endIt;
}

QC_MAP_TEMPLATE
auto QC_MAP::erase(const_iterator first, const_iterator last) -> iterator {
    if (first != last) {
        do {
            m_erase(first);
            ++first;
        } while (first != last);
        reserve(m_size);
    }
    return end();
}

QC_MAP_TEMPLATE
void QC_MAP::m_erase(const_iterator position) {
    size_t i(position.m_bucket - m_buckets), j(i + 1u);

    while (true) {
        if (j >= m_bucketCount) j = 0u;

        if (m_buckets[j].dist <= 1u) {
            break;
        }

        m_buckets[i].entry() = std::move(m_buckets[j].entry());
        m_buckets[i].dist = m_buckets[j].dist - 1u;

        ++i; ++j;
        if (i >= m_bucketCount) i = 0u;
    }

    m_buckets[i].entry().~T();
    m_buckets[i].dist = 0u;
    --m_size;
}

// Map::clear
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
void QC_MAP::clear() {
    m_clear<true>();
}

QC_MAP_TEMPLATE
template <bool t_zeroDists>
void QC_MAP::m_clear() {
    if constexpr (std::is_trivially_destructible_v<T>) {
        if constexpr (t_zeroDists) {
            if (m_size) m_zeroDists();
        }
    }
    else {
        for (size_t i(0u), n(0u); n < m_size; ++i) {
            if (m_buckets[i].dist) {
                m_buckets[i].entry().~T();
                if constexpr (t_zeroDists) {
                    m_buckets[i].dist = 0u;
                }
                ++n;
            }
        }
    }

    m_size = 0u;
}

// Map::contains
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
bool QC_MAP::contains(const K & key) const {
    return contains(key, m_hash(key));
}

QC_MAP_TEMPLATE
bool QC_MAP::contains(const K & key, size_t hash) const {
    return find(key, hash) != cend();
}

// Map::count
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::count(const K & key) const {
    return contains(key);
}

QC_MAP_TEMPLATE
size_t QC_MAP::count(const K & key, size_t hash) const {
    return contains(key, hash);
}

// Map::at
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
std::add_lvalue_reference_t<V> QC_MAP::at(const K & key) {
    static_assert(!k_isSet, "This is not a set operation");
    return find(key)->second;
}

QC_MAP_TEMPLATE
std::add_lvalue_reference_t<const V> QC_MAP::at(const K & key) const {
    static_assert(!k_isSet, "This is not a set operation");
    return find(key)->second;
}

// Map::operator[]
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
std::add_lvalue_reference_t<V> QC_MAP::operator[](const K & key) {
    static_assert(!k_isSet, "This is not a set operation");
    return try_emplace(key).first->second;
}

QC_MAP_TEMPLATE
std::add_lvalue_reference_t<V> QC_MAP::operator[](K && key) {
    static_assert(!k_isSet, "This is not a set operation");
    return try_emplace(std::move(key)).first->second;
}

// Map::begin
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::begin() noexcept -> iterator {
    return m_begin<false>();
}

QC_MAP_TEMPLATE
auto QC_MAP::begin() const noexcept -> const_iterator {
    return m_begin<true>();
}

QC_MAP_TEMPLATE
auto QC_MAP::cbegin() const noexcept -> const_iterator {
    return m_begin<true>();
}

QC_MAP_TEMPLATE
template <bool t_const>
auto QC_MAP::m_begin() const noexcept -> Iterator<t_const> {
    if (!m_size) {
        return m_end<t_const>();
    }

    for (size_t i(0u); ; ++i) {
        if (m_buckets[i].dist) {
            return Iterator<t_const>(m_buckets + i);
        }
    }
}

// Map::end
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::end() noexcept -> iterator {
    return m_end<false>();
}

QC_MAP_TEMPLATE
auto QC_MAP::end() const noexcept -> const_iterator {
    return m_end<true>();
}

QC_MAP_TEMPLATE
auto QC_MAP::cend() const noexcept -> const_iterator {
    return m_end<true>();
}

QC_MAP_TEMPLATE
template <bool t_const>
auto QC_MAP::m_end() const noexcept -> Iterator<t_const> {
    return Iterator<t_const>(m_buckets + m_bucketCount);
}

// Map::find
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::find(const K & key) -> iterator {
    return find(key, m_hash(key));
}

QC_MAP_TEMPLATE
auto QC_MAP::find(const K & key) const -> const_iterator {
    return find(key, m_hash(key));
}

QC_MAP_TEMPLATE
auto QC_MAP::find(const K & key, size_t hash) -> iterator {
    return m_find<false>(key, hash);
}

QC_MAP_TEMPLATE
auto QC_MAP::find(const K & key, size_t hash) const -> const_iterator {
    return m_find<true>(key, hash);
}

QC_MAP_TEMPLATE
template <bool t_const>
auto QC_MAP::m_find(const K & key, size_t hash) const -> Iterator<t_const> {
    if (!m_buckets) {
        return m_end<t_const>();
    }

    size_t i(m_indexOf(hash));
    Dist dist(1u);

    while (true) {
        const Bucket & bucket(m_buckets[i]);

        if (bucket.dist < dist) {
            return m_end<t_const>();
        }

        if (m_equal(bucket.key, key)) {
            return Iterator<t_const>(&bucket);
        }

        ++i;
        ++dist;

        if (i >= m_bucketCount) i = 0u;
    };

    // Will never reach reach this return
    return m_end<t_const>();
}

// Map::equal_range
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::equal_range(const K & key) -> std::pair<iterator, iterator> {
    return equal_range(key, m_hash(key));
}

QC_MAP_TEMPLATE
auto QC_MAP::equal_range(const K & key) const -> std::pair<const_iterator, const_iterator> {
    return equal_range(key, m_hash(key));
}

QC_MAP_TEMPLATE
auto QC_MAP::equal_range(const K & key, size_t hash) -> std::pair<iterator, iterator> {
    return m_equal_range<false>(key, hash);
}

QC_MAP_TEMPLATE
auto QC_MAP::equal_range(const K & key, size_t hash) const -> std::pair<const_iterator, const_iterator> {
    return m_equal_range<true>(key, hash);
}

QC_MAP_TEMPLATE
template <bool t_const>
auto QC_MAP::m_equal_range(const K & key, size_t hash) const -> std::pair<Iterator<t_const>, Iterator<t_const>> {
    Iterator<t_const> it(m_find<t_const>(key, hash));
    return { it, it };
}

// Map::reserve
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
void QC_MAP::reserve(size_t capacity) {
    rehash(capacity << 1);
}

// Map::rehash
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
void QC_MAP::rehash(size_t bucketCount) {
    bucketCount = detail::hash::ceil2(bucketCount);
    if (bucketCount < config::map::minBucketCount) bucketCount = config::map::minBucketCount;
    else if (bucketCount < (m_size << 1)) bucketCount = m_size << 1u;

    if (bucketCount != m_bucketCount) {
        if (m_buckets) {
            m_rehash(bucketCount);
        }
        else {
            m_bucketCount = bucketCount;
        }
    }
}

QC_MAP_TEMPLATE
void QC_MAP::m_rehash(size_t bucketCount) {
    size_t oldSize(m_size);
    size_t oldBucketCount(m_bucketCount);
    Bucket * oldBuckets(m_buckets);

    m_size = 0u;
    m_bucketCount = bucketCount;
    m_allocate();

    for (size_t i(0u), n(0u); n < oldSize; ++i) {
        Bucket & bucket(oldBuckets[i]);
        if (bucket.dist) {
            emplace(std::move(bucket.entry()));
            bucket.entry().~T();
            ++n;
        }
    }

    AllocatorTraits::deallocate(m_alloc, oldBuckets, oldBucketCount + 1u);
}

// Map::swap
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
void QC_MAP::swap(Map & other) noexcept {
    std::swap(m_size, other.m_size);
    std::swap(m_bucketCount, other.m_bucketCount);
    std::swap(m_buckets, other.m_buckets);
    std::swap(m_hash, other.m_hash);
    std::swap(m_equal, other.m_equal);
    if constexpr (AllocatorTraits::propagate_on_container_swap::value) {
        std::swap(m_alloc, other.m_alloc);
    }
}

// Map::empty
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
bool QC_MAP::empty() const noexcept {
    return m_size == 0u;
}

// Map::size
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::size() const noexcept {
    return m_size;
}

// Map::max_size
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::max_size() const {
    return max_bucket_count() >> 1u;
}

// Map::capacity
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::capacity() const {
    return m_bucketCount >> 1u;
}

// Map::bucket_count
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::bucket_count() const {
    return m_bucketCount;
}

// Map::max_bucket_count
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::max_bucket_count() const {
    return std::numeric_limits<size_t>::max() - 1u;
}

// Map::bucket
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::bucket(const K & key) const {
    return m_indexOf(m_hash(key));
}

// Map::bucket_size
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
size_t QC_MAP::bucket_size(size_t i) const {
    if (i >= m_bucketCount || !m_buckets) {
        return 0u;
    }

    Dist dist(1u);
    while (m_buckets[i].dist > dist) {
        ++i;
        ++dist;

        if (i >= m_bucketCount) i = 0u;
    }
    
    size_t n(0u);
    while (m_buckets[i].dist == dist) {
        ++i;
        ++dist;
        ++n;

        if (i >= m_bucketCount) i = 0u;
    }

    return n;
}

// Map::load_factor
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
float QC_MAP::load_factor() const {
    return float(m_size) / float(m_bucketCount);
}

// Map::max_load_factor
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
float QC_MAP::max_load_factor() const {
    return 0.5f;
}

// Map::hash_function
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::hash_function() const -> hasher {
    return m_hash;
}

// Map::key_eq
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::key_eq() const -> key_equal {
    return m_equal;
}

// Map::get_allocator
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
auto QC_MAP::get_allocator() const -> allocator_type {
    return m_alloc;
}

//=== Private Methods ==========================================================

QC_MAP_TEMPLATE
size_t QC_MAP::m_indexOf(size_t hash) const {
    return hash & (m_bucketCount - 1u);
}

QC_MAP_TEMPLATE
void QC_MAP::m_allocate() {
    m_buckets = AllocatorTraits::allocate(m_alloc, m_bucketCount + 1u);
    m_zeroDists();
    m_buckets[m_bucketCount].dist = std::numeric_limits<Dist>::max();
}

QC_MAP_TEMPLATE
void QC_MAP::m_deallocate() {
    AllocatorTraits::deallocate(m_alloc, m_buckets, m_bucketCount + 1u);
    m_buckets = nullptr;
}

QC_MAP_TEMPLATE
void QC_MAP::m_zeroDists() {
    if constexpr (sizeof(Bucket) <= sizeof(size_t) || sizeof(Dist) < 4u && (sizeof(Bucket) / sizeof(Dist) <= 2u)) {
        std::memset(m_buckets, 0, m_bucketCount * sizeof(Bucket));
    }
    else {
        for (size_t i(0u); i < m_bucketCount; ++i) m_buckets[i].dist = 0u;
    }
}

QC_MAP_TEMPLATE
void QC_MAP::m_copyBuckets(const Bucket * buckets) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        if (m_size) {
            std::memcpy(m_buckets, buckets, m_bucketCount * sizeof(Bucket));
        }
    }
    else {
        for (size_t i(0u), n(0u); n < m_size; ++i) {
            if (m_buckets[i].dist = buckets[i].dist) {
                AllocatorTraits::construct(m_alloc, &m_buckets[i].entry(), buckets[i].entry());
                ++n;
            }
        }
    }
}

QC_MAP_TEMPLATE
void QC_MAP::m_moveBuckets(Bucket * buckets) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        if (m_size) {
            std::memcpy(m_buckets, buckets, m_bucketCount * sizeof(Bucket));
        }
    }
    else {
        for (size_t i(0u), n(0u); n < m_size; ++i) {
            if (m_buckets[i].dist = buckets[i].dist) {
                AllocatorTraits::construct(m_alloc, &m_buckets[i].entry(), std::move(buckets[i].entry()));
                ++n;
            }
        }
    }
}

// Iterator
//==============================================================================

//=== Public Methods ===========================================================

// Map::Iterator::Iterator
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
template <bool t_const>
template <bool t_const_, typename>
constexpr QC_MAP::Iterator<t_const>::Iterator(const Iterator<!t_const> & other) noexcept :
    m_bucket(other.m_bucket)
{}

QC_MAP_TEMPLATE
template <bool t_const>
template <typename Bucket_>
constexpr QC_MAP::Iterator<t_const>::Iterator(Bucket_ * bucket) noexcept :
    m_bucket(const_cast<Bucket *>(bucket))
{}

// Map::Iterator::operator*
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
template <bool t_const>
auto QC_MAP::Iterator<t_const>::operator*() const -> value_type & {
    return m_bucket->entry();
}

// Map::Iterator::operator->
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
template <bool t_const>
auto QC_MAP::Iterator<t_const>::operator->() const -> value_type * {
    return &m_bucket->entry();
}

// Map::Iterator::operator++
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
template <bool t_const>
auto QC_MAP::Iterator<t_const>::operator++() -> Iterator & {
    do {
        ++m_bucket;
    } while (!m_bucket->dist);

    return *this;
}

// Map::Iterator::operator++ int
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
template <bool t_const>
auto QC_MAP::Iterator<t_const>::operator++(int) -> Iterator {
    Iterator temp(*this);
    operator++();
    return temp;
}

// Map::Iterator::operator==
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
template <bool t_const>
template <bool t_const_>
bool QC_MAP::Iterator<t_const>::operator==(const Iterator<t_const_> & o) const {
    return m_bucket == o.m_bucket;
}

// Map::Iterator::operator!=
//------------------------------------------------------------------------------

QC_MAP_TEMPLATE
template <bool t_const>
template <bool t_const_>
bool QC_MAP::Iterator<t_const>::operator!=(const Iterator<t_const_> & o) const {
    return m_bucket != o.m_bucket;
}

}
