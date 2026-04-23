
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

template<typename T>
class Deque {
private:
    std::vector<T> data_;
    size_t front_idx_;
    size_t back_idx_;
    size_t capacity_;
    
    void resize() {
        size_t new_capacity = capacity_ * 2;
        std::vector<T> new_data(new_capacity);
        
        size_t j = 0;
        if (!empty()) {
            size_t i = front_idx_;
            do {
                new_data[j++] = data_[i];
                i = (i + 1) % capacity_;
            } while (i != back_idx_);
        }
        
        data_ = std::move(new_data);
        front_idx_ = 0;
        back_idx_ = j;
        capacity_ = new_capacity;
    }
    
public:
    class iterator {
    private:
        Deque* deque_;
        size_t idx_;
        
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        
        iterator() : deque_(nullptr), idx_(0) {}
        iterator(Deque* deque, size_t idx) : deque_(deque), idx_(idx) {}
        
        reference operator*() const {
            size_t actual_idx = (deque_->front_idx_ + idx_) % deque_->capacity_;
            return deque_->data_[actual_idx];
        }
        
        pointer operator->() const {
            return &(**this);
        }
        
        iterator& operator++() {
            ++idx_;
            return *this;
        }
        
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        iterator& operator--() {
            --idx_;
            return *this;
        }
        
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        
        iterator& operator+=(difference_type n) {
            idx_ += n;
            return *this;
        }
        
        iterator& operator-=(difference_type n) {
            idx_ -= n;
            return *this;
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
            return idx_ - other.idx_;
        }
        
        reference operator[](difference_type n) const {
            return *(*this + n);
        }
        
        bool operator==(const iterator& other) const {
            return deque_ == other.deque_ && idx_ == other.idx_;
        }
        
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
        
        bool operator<(const iterator& other) const {
            return idx_ < other.idx_;
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
    
    Deque() : front_idx_(0), back_idx_(0), capacity_(8) {
        data_.resize(capacity_);
    }
    
    ~Deque() = default;
    
    Deque(const Deque& other) : data_(other.data_), front_idx_(other.front_idx_), 
                               back_idx_(other.back_idx_), capacity_(other.capacity_) {}
    
    Deque& operator=(const Deque& other) {
        if (this != &other) {
            data_ = other.data_;
            front_idx_ = other.front_idx_;
            back_idx_ = other.back_idx_;
            capacity_ = other.capacity_;
        }
        return *this;
    }
    
    void push_front(const T& value) {
        if ((back_idx_ + 1) % capacity_ == front_idx_) {
            resize();
        }
        
        front_idx_ = (front_idx_ - 1 + capacity_) % capacity_;
        data_[front_idx_] = value;
    }
    
    void push_back(const T& value) {
        if ((back_idx_ + 1) % capacity_ == front_idx_) {
            resize();
        }
        
        data_[back_idx_] = value;
        back_idx_ = (back_idx_ + 1) % capacity_;
    }
    
    void pop_front() {
        if (empty()) return;
        front_idx_ = (front_idx_ + 1) % capacity_;
    }
    
    void pop_back() {
        if (empty()) return;
        back_idx_ = (back_idx_ - 1 + capacity_) % capacity_;
    }
    
    T& front() {
        return data_[front_idx_];
    }
    
    const T& front() const {
        return data_[front_idx_];
    }
    
    T& back() {
        return data_[(back_idx_ - 1 + capacity_) % capacity_];
    }
    
    const T& back() const {
        return data_[(back_idx_ - 1 + capacity_) % capacity_];
    }
    
    T& operator[](size_t index) {
        return data_[(front_idx_ + index) % capacity_];
    }
    
    const T& operator[](size_t index) const {
        return data_[(front_idx_ + index) % capacity_];
    }
    
    T& at(size_t index) {
        if (index >= size()) {
            throw std::out_of_range("Deque index out of range");
        }
        return operator[](index);
    }
    
    const T& at(size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("Deque index out of range");
        }
        return operator[](index);
    }
    
    bool empty() const {
        return front_idx_ == back_idx_;
    }
    
    size_t size() const {
        if (back_idx_ >= front_idx_) {
            return back_idx_ - front_idx_;
        } else {
            return capacity_ - front_idx_ + back_idx_;
        }
    }
    
    void clear() {
        front_idx_ = 0;
        back_idx_ = 0;
    }
    
    iterator begin() {
        return iterator(this, 0);
    }
    
    iterator end() {
        return iterator(this, size());
    }
    
    iterator insert(iterator pos, const T& value) {
        if (pos == begin()) {
            push_front(value);
            return begin();
        } else if (pos == end()) {
            push_back(value);
            return iterator(this, size() - 1);
        } else {
            size_t insert_pos = pos.idx_;
            push_back(back());
            
            for (size_t i = size() - 1; i > insert_pos; --i) {
                (*this)[i] = (*this)[i - 1];
            }
            
            (*this)[insert_pos] = value;
            return iterator(this, insert_pos);
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
            size_t erase_pos = pos.idx_;
            for (size_t i = erase_pos; i < size() - 1; ++i) {
                (*this)[i] = (*this)[i + 1];
            }
            pop_back();
            return iterator(this, erase_pos);
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
