#pragma once

#include <db/types.hpp>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <array>
#include <string>
#include <algorithm> // for std::find in tests

namespace db {
    constexpr size_t DEFAULT_NUM_PAGES = 50;

/**
 * @brief Represents a buffer pool for database pages.
 * @details The BufferPool caches pages read from disk. It supports fetching a page,
 * marking a page dirty, checking presence/dirty state, flushing to disk, and discarding pages.
 * @note A BufferPool owns the Page objects stored inside it.
 */
    class BufferPool {
        // TODO pa0: add private members

        // LRU list: front = Most Recently Used (MRU), back = Least Recently Used (LRU)
        std::list<PageId> lru_;

        // Fixed-capacity page storage
        std::array<Page, DEFAULT_NUM_PAGES> pages_{};

        // Free slot indices into `pages_`
        std::vector<size_t> free_;

        // Hash for PageId (avoid relying on global specializations)
        struct PageIdHash {
            size_t operator()(const PageId &p) const {
                return std::hash<std::string>()(p.file) ^ (std::hash<size_t>()(p.page) << 1);
            }
        };

        // PageId -> slot index
        std::unordered_map<PageId, size_t, PageIdHash> index_;

        // PageId -> iterator in LRU list (for O(1) updates)
        std::unordered_map<PageId, std::list<PageId>::iterator, PageIdHash> pos_;

        // Dirty page set
        std::unordered_set<PageId, PageIdHash> dirty_;

    public:
        /**
         * @brief Construct a BufferPool with DEFAULT_NUM_PAGES capacity.
         */
        explicit BufferPool();

        /**
         * @brief Destructor: flush all remaining dirty pages to disk.
         */
        ~BufferPool();

        BufferPool(const BufferPool &) = delete;
        BufferPool(BufferPool &&) = delete;
        BufferPool &operator=(const BufferPool &) = delete;
        BufferPool &operator=(BufferPool &&) = delete;

        /**
         * @brief Get (and pin as MRU) the page with the given id.
         * @details If the page is not cached, read it from disk. If the pool is full,
         * evict the LRU page (flush first if dirty).
         */
        Page &getPage(const PageId &pid);

        /**
         * @brief Mark a cached page as dirty.
         */
        void markDirty(const PageId &pid);

        /**
         * @brief Check if a page is marked dirty.
         */
        [[nodiscard]] bool isDirty(const PageId &pid) const;

        /**
         * @brief Check if a page is present in the buffer pool.
         */
        [[nodiscard]] bool contains(const PageId &pid) const;

        /**
         * @brief Discard a page from the buffer pool without writing it back.
         * @details Removes all metadata (LRU, dirty, indexes) and frees its slot.
         */
        void discardPage(const PageId &pid);

        /**
         * @brief Flush a single page to disk (no-op if not dirty or not cached).
         * @details Clears the dirty mark on success.
         */
        void flushPage(const PageId &pid);

        /**
         * @brief Flush all dirty pages belonging to a given file.
         */
        void flushFile(const std::string &file);
    };
} // namespace db
