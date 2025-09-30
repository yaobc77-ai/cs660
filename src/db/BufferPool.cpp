#include <db/BufferPool.hpp>
#include <db/Database.hpp>
#include <numeric>

using namespace db;

BufferPool::BufferPool() {
    // TODO pa0
    free_.reserve(DEFAULT_NUM_PAGES);
    for (size_t i = 0; i < DEFAULT_NUM_PAGES; ++i) free_.push_back(i);
}

BufferPool::~BufferPool() {
    // TODO pa0
    std::vector<PageId> to_flush;
    to_flush.reserve(dirty_.size());
    for (const auto &pid : dirty_) to_flush.push_back(pid);
    for (const auto &pid : to_flush) flushPage(pid);
}


Page &BufferPool::getPage(const PageId &pid) {
    // TODO pa0
    auto it = index_.find(pid);
    if (it != index_.end()) {

        auto pit = pos_.find(pid);
        if (pit != pos_.end()) {
            lru_.erase(pit->second);
        }
        lru_.push_front(pid);
        pos_[pid] = lru_.begin();
        return pages_[it->second];
    }

    size_t slot;
    if (!free_.empty()) {
        slot = free_.back();
        free_.pop_back();
    } else {

        if (lru_.empty()) {

            throw std::logic_error("BufferPool: LRU empty but no free slots");
        }
        PageId victim = lru_.back();
        lru_.pop_back();

        auto vit = index_.find(victim);
        slot = vit->second;

        if (dirty_.count(victim)) flushPage(victim);

        index_.erase(vit);
        pos_.erase(victim);
        dirty_.erase(victim);
    }


    DbFile &f = getDatabase().get(pid.file);
    f.readPage(pages_[slot], pid.page);

    index_[pid] = slot;
    lru_.push_front(pid);
    pos_[pid] = lru_.begin();

    return pages_[slot];
}
void BufferPool::markDirty(const PageId &pid) {
    // TODO pa0
    if (index_.count(pid)) dirty_.insert(pid);
}

bool BufferPool::isDirty(const PageId &pid) const {
    // TODO pa0
    return dirty_.count(pid) != 0;
}

bool BufferPool::contains(const PageId &pid) const {
    // TODO pa0
    return index_.count(pid) != 0;
}

void BufferPool::discardPage(const PageId &pid) {
    // TODO pa0
    auto it = index_.find(pid);
    if (it == index_.end()) return;

    const size_t slot = it->second;
    free_.push_back(slot);

    auto pit = pos_.find(pid);
    if (pit != pos_.end()) {
        lru_.erase(pit->second);
        pos_.erase(pit);
    }
    dirty_.erase(pid);
    index_.erase(it);
}

void BufferPool::flushPage(const PageId &pid) {
    // TODO pa0
    auto it = index_.find(pid);
    if (it == index_.end()) return;
    if (!dirty_.count(pid)) return;

    DbFile &f = getDatabase().get(pid.file);
    f.writePage(pages_[it->second], pid.page);
    dirty_.erase(pid);
}

void BufferPool::flushFile(const std::string &file) {
    // TODO pa0
    std::vector<PageId> to_flush;
    to_flush.reserve(dirty_.size());
    for (const auto &pid : dirty_) {
        if (pid.file == file) to_flush.push_back(pid);
    }
    for (const auto &pid : to_flush) flushPage(pid);
}
