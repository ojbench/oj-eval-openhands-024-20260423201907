
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <algorithm>
#include <iterator>

template<typename T>
class Deque {
private:
    static const size_t CHUNK_SIZE = 8;
    static const size_t INITIAL_MAP_SIZE = 8;
    
    T** map_;
    size_t map_size_;
    size_t start_node_;
    size_t start_offset_;
    size_t end_node_;
    size_t end_offset_;
    size_t size_;
    
    void create_map(size_t map_size) {
        map_size_ = map_size;
        map_ = new T*[map_size_];
        for (size_t i = 0; i < map_size_; ++i) {
            map_[i] = nullptr;
        }
    }
    
    void create_node(size_t node_index) {
        if (map_[node_index] == nullptr) {
            map_[node_index] = new T[CHUNK_SIZE];
        }
    }
    
    void destroy_node(size_t node_index) {
        if (map_[node_index] != nullptr) {
            delete[] map_[node_index];
            map_[node_index] = nullptr;
        }
    }
    
    void expand_map() {
        size_t new_map_size = map_size_ * 2;
        T** new_map = new T*[new_map_size];
        
        size_t new_start_node = new_map_size / 4;
        
        for (size_t i = 0; i < new_map_size; ++i) {
            new_map[i] = nullptr;
        }
        
        for (size_t i = 0; i < map_size_; ++i) {
            new_map[new_start_node + i] = map_[i];
        }
        
        delete[] map_;
        map_ = new_map;
        map_size_ = new_map_size;
        start_node_ = new_start_node;
        end_node_ = new_start_node + map_size_ / 2 - 1;
    }
    
    void shrink_map() {
        if (map_size_ <= INITIAL_MAP_SIZE) return;
        
        size_t new_map_size = map_size_ / 2;
        T** new_map = new T*[new_map_size];
        
        size_t new_start_node = new_map_size / 4;
        
        for (size_t i = 0; i < new_map_size; ++i) {
            new_map[i] = nullptr;
        }
        
        for (size_t i = 0; i < map_size_; ++i) {
            if (map_[i] != nullptr) {
                new_map[new_start_node + i] = map_[i];
            }
        }
        
        delete[] map_;
        map_ = new_map;
        map_size_ = new_map_size;
        start_node_ = new_start_node;
        end_node_ = new_start_node + map_size_ / 2 - 1;
    }
    
public:
    class iterator {
    private:
        Deque* deque_;
        size_t node_;
        size_t offset_;
        
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        
        iterator() : deque_(nullptr), node_(0), offset_(0) {}
        
        iterator(Deque* deque, size_t node, size_t offset) 
            : deque_(deque), node_(node), offset_(offset) {}
        
        reference operator*() const {
            return deque_->map_[node_][offset_];
        }
        
        pointer operator->() const {
            return &deque_->map_[node_][offset_];
        }
        
        iterator& operator++() {
            if (++offset_ == CHUNK_SIZE) {
                offset_ = 0;
                ++node_;
            }
            return *this;
        }
        
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        iterator& operator--() {
            if (offset_ == 0) {
                offset_ = CHUNK_SIZE - 1;
                --node_;
            } else {
                --offset_;
            }
            return *this;
        }
        
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        
        iterator& operator+=(difference_type n) {
            difference_type total_offset = node_ * CHUNK_SIZE + offset_ + n;
            node_ = total_offset / CHUNK_SIZE;
            offset_ = total_offset % CHUNK_SIZE;
            return *this;
        }
        
        iterator& operator-=(difference_type n) {
            return *this += (-n);
        }
        
        iterator operator+(difference_type n) const {
            iterator tmp = *this;
            return tmp += n;
        }
        
        iterator operator-(difference_type n) const {
            iterator tmp = *this;
            return tmp -= n;
        }
        
        difference_type operator-(const iterator& other) const {
            return (node_ * CHUNK_SIZE + offset_) - (other.node_ * CHUNK_SIZE + other.offset_);
        }
        
        reference operator[](difference_type n) const {
            return *(*this + n);
        }
        
        bool operator==(const iterator& other) const {
            return node_ == other.node_ && offset_ == other.offset_;
        }
        
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
        
        bool operator<(const iterator& other) const {
            return node_ < other.node_ || (node_ == other.node_ && offset_ < other.offset_);
        }
        
        bool operator>(const iterator& other) const {
            return other < *this;
        }
        
        bool operator<=(const iterator& other) const {
            return !(other < *this);
        }
        
        bool operator>=(const iterator& other) const {
            return !(*this < other);
        }
        
        friend class Deque;
    };
    
    Deque() {
        create_map(INITIAL_MAP_SIZE);
        start_node_ = map_size_ / 2;
        start_offset_ = CHUNK_SIZE / 2;
        end_node_ = start_node_;
        end_offset_ = start_offset_;
        size_ = 0;
    }
    
    ~Deque() {
        clear();
        delete[] map_;
    }
    
    Deque(const Deque& other) : Deque() {
        for (const auto& item : other) {
            push_back(item);
        }
    }
    
    Deque& operator=(const Deque& other) {
        if (this != &other) {
            clear();
            for (const auto& item : other) {
                push_back(item);
            }
        }
        return *this;
    }
    
    void push_front(const T& value) {
        if (start_offset_ == 0) {
            if (start_node_ == 0) {
                expand_map();
            }
            --start_node_;
            start_offset_ = CHUNK_SIZE - 1;
        } else {
            --start_offset_;
        }
        
        create_node(start_node_);
        map_[start_node_][start_offset_] = value;
        
        if (size_ == 0) {
            end_node_ = start_node_;
            end_offset_ = start_offset_;
        }
        
        ++size_;
    }
    
    void push_back(const T& value) {
        create_node(end_node_);
        map_[end_node_][end_offset_] = value;
        
        if (size_ == 0) {
            start_node_ = end_node_;
            start_offset_ = end_offset_;
        }
        
        ++size_;
        
        if (++end_offset_ == CHUNK_SIZE) {
            end_offset_ = 0;
            if (++end_node_ == map_size_) {
                expand_map();
            }
        }
    }
    
    void pop_front() {
        if (size_ == 0) return;
        
        map_[start_node_][start_offset_].~T();
        
        if (++start_offset_ == CHUNK_SIZE) {
            start_offset_ = 0;
            ++start_node_;
        }
        
        --size_;
        
        if (size_ == 0) {
            end_node_ = start_node_;
            end_offset_ = start_offset_;
        }
    }
    
    void pop_back() {
        if (size_ == 0) return;
        
        if (end_offset_ == 0) {
            end_offset_ = CHUNK_SIZE - 1;
            --end_node_;
        } else {
            --end_offset_;
        }
        
        map_[end_node_][end_offset_].~T();
        
        --size_;
        
        if (size_ == 0) {
            start_node_ = end_node_;
            start_offset_ = end_offset_;
        }
    }
    
    T& front() {
        return map_[start_node_][start_offset_];
    }
    
    const T& front() const {
        return map_[start_node_][start_offset_];
    }
    
    T& back() {
        if (end_offset_ == 0) {
            return map_[end_node_ - 1][CHUNK_SIZE - 1];
        } else {
            return map_[end_node_][end_offset_ - 1];
        }
    }
    
    const T& back() const {
        if (end_offset_ == 0) {
            return map_[end_node_ - 1][CHUNK_SIZE - 1];
        } else {
            return map_[end_node_][end_offset_ - 1];
        }
    }
    
    T& operator[](size_t index) {
        size_t total_offset = start_node_ * CHUNK_SIZE + start_offset_ + index;
        size_t node = total_offset / CHUNK_SIZE;
        size_t offset = total_offset % CHUNK_SIZE;
        return map_[node][offset];
    }
    
    const T& operator[](size_t index) const {
        size_t total_offset = start_node_ * CHUNK_SIZE + start_offset_ + index;
        size_t node = total_offset / CHUNK_SIZE;
        size_t offset = total_offset % CHUNK_SIZE;
        return map_[node][offset];
    }
    
    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Deque index out of range");
        }
        return operator[](index);
    }
    
    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Deque index out of range");
        }
        return operator[](index);
    }
    
    bool empty() const {
        return size_ == 0;
    }
    
    size_t size() const {
        return size_;
    }
    
    void clear() {
        while (!empty()) {
            pop_front();
        }
    }
    
    iterator begin() {
        return iterator(this, start_node_, start_offset_);
    }
    
    iterator end() {
        if (size_ == 0) {
            return iterator(this, start_node_, start_offset_);
        }
        return iterator(this, end_node_, end_offset_);
    }
    
    iterator insert(iterator pos, const T& value) {
        if (pos == begin()) {
            push_front(value);
            return begin();
        } else if (pos == end()) {
            push_back(value);
            return iterator(this, end_node_, end_offset_ - 1);
        } else {
            Deque temp;
            iterator it = begin();
            while (it != pos) {
                temp.push_back(*it);
                ++it;
            }
            temp.push_back(value);
            while (it != end()) {
                temp.push_back(*it);
                ++it;
            }
            *this = temp;
            return iterator(this, start_node_, start_offset_ + (pos - begin()));
        }
    }
    
    iterator erase(iterator pos) {
        if (pos == begin()) {
            pop_front();
            return begin();
        } else if (pos == end() - 1) {
            pop_back();
            return end();
        } else {
            Deque temp;
            iterator it = begin();
            while (it != pos) {
                temp.push_back(*it);
                ++it;
            }
            ++it;
            while (it != end()) {
                temp.push_back(*it);
                ++it;
            }
            *this = temp;
            return iterator(this, start_node_, start_offset_ + (pos - begin()));
        }
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    Deque<int> dq;
    std::string line;
    
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        
        if (command == "push_front") {
            int x;
            iss >> x;
            dq.push_front(x);
        } else if (command == "push_back") {
            int x;
            iss >> x;
            dq.push_back(x);
        } else if (command == "pop_front") {
            if (!dq.empty()) {
                dq.pop_front();
            }
        } else if (command == "pop_back") {
            if (!dq.empty()) {
                dq.pop_back();
            }
        } else if (command == "front") {
            if (!dq.empty()) {
                std::cout << dq.front() << "\n";
            } else {
                std::cout << "empty\n";
            }
        } else if (command == "back") {
            if (!dq.empty()) {
                std::cout << dq.back() << "\n";
            } else {
                std::cout << "empty\n";
            }
        } else if (command == "size") {
            std::cout << dq.size() << "\n";
        } else if (command == "empty") {
            std::cout << (dq.empty() ? "true" : "false") << "\n";
        } else if (command == "print") {
            for (const auto& val : dq) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        } else if (command == "clear") {
            dq.clear();
        } else if (command == "exit") {
            break;
        }
    }
    
    return 0;
}
