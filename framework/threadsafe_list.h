#pragma once

#include <memory>
#include <mutex>

template <typename T>
class Threadsafe_List
{
public:
    Threadsafe_List()
    {
    }

    Threadsafe_List(Threadsafe_List const&) = delete;

    ~Threadsafe_List()
    {
    }

    Threadsafe_List& operator=(Threadsafe_List const&) = delete;

    void push_front(T const& data)
    {
        auto new_node = std::make_unique<Node>(data);
        std::lock_guard<std::mutex> head_lock(head.mutex);
        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }

    template <typename Function>
    void for_each(Function f)
    {
        Node* current = &head;
        std::unique_lock<std::mutex> current_lock(head.mutex);
        while (Node* next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lock(next->mutex);
            current_lock.unlock();
            f(next->data);
            current = next;
            current_lock = std::move(next_lock);
        }
    }

    template <typename Predicate>
    T find_first_if(Predicate p)
    {
        Node* current = &head;
        std::unique_lock<std::mutex> current_lock(head.mutex);
        while (Node* next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lock(next->mutex);
            current_lock.unlock();
            if (p(next->data))
            {
                return next->data;
            }

            current = next;
            current_lock = std::move(next_lock);
        }

        return T();
    }

    template <typename Predicate>
    void remove_if(Predicate p)
    {
        Node* current = &head;
        std::unique_lock<std::mutex> current_lock(head.mutex);
        while (Node* next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lock(next->mutex);
            if (p(next->data))
            {
                std::unique_ptr<Node> old_next = std::move(current->next);
                current->next = std::move(next->next);
                next_lock.unlock();
            }
            else
            {
                current_lock.unlock();
                current = next;
                current_lock = std::move(next_lock);
            }
        }
    }

private:
    struct Node
    {
        std::mutex mutex;
        T data;
        std::unique_ptr<Node> next;

        Node() : next(std::make_unique<Node>())
        {
        }

        Node(T const& data) : data(data)
        {
        }
    };

    Node head;

};
