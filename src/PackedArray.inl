#pragma once
#include <cstdint>
#include <vector>
namespace vkr {
template <typename T>
class PackedArray {
   public:
    using Handle = int64_t;

   private:
    std::vector<int32_t> sparse;
    std::vector<int32_t> free;
    std::vector<int64_t> sparseIndex;
    std::vector<T> data;
    int32_t firstFree{-1};

   public:
    void clear() {
        sparse.clear();
        free.clear();
        sparseIndex.clear();
        data.clear();
        firstFree = -1;
    }

    Handle add(const T& elem) {
        data.push_back(elem);
        if (firstFree < 0) {
            auto index = sparse.size();
            sparse.push_back(data.size() - 1);
            free.push_back(-1);
            sparseIndex.push_back(index);

            return index;
        } else {
            auto index = firstFree;
            sparse[firstFree] = data.size() - 1;
            firstFree = free[firstFree];
            free[index] = -1;
            sparseIndex.push_back(index);

            return index;
        }
    }

    void remove(Handle handle) {
        if ((size_t)handle > sparse.size()) {
            return;
        } else {
            if (sparse[handle] >= 0) {
                auto compactIndex = (size_t)sparse[handle];
                // Update sparse part
                sparse[handle] = -1;
                free[handle] = firstFree;
                firstFree = handle;

                if (compactIndex < data.size() - 1) {
                    // Move last element to the deleted one if not last element
                    data[compactIndex] = data[data.size() - 1];
                    sparseIndex[compactIndex] = sparseIndex[data.size() - 1];
                    sparse[sparseIndex[compactIndex]] = compactIndex;
                }

                data.pop_back();
                sparseIndex.pop_back();
            }
        }
    }

    T* get(Handle handle) {
        if ((size_t)handle > sparse.size() || sparse[handle] < 0) return 0;
        return &data[sparse[handle]];
    }

    T* getData() { return data.data(); }

    std::size_t getSize() { return data.size(); }
};
}  // namespace vkr