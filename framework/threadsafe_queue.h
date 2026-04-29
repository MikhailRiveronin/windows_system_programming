#pragma once

#include <memory>
#include <mutex>

template <typename T>
class Threadsafe_Queue
{
public:
    Threadsafe_Queue() : head(std::make_unique<Node>()), tail(head.get())
    {
    }

    Threadsafe_Queue(Threadsafe_Queue const&) = delete;
    Threadsafe_Queue& operator=(Threadsafe_Queue const&) = delete;

    T try_pop()
    {
        auto old_head = try_pop_head();
        return old_head ? old_head->data : 0;
    }

    T wait_and_pop()
    {
        auto old_head = wait_pop_head();
        return old_head->data;
    }

    void push(T value)
    {
        auto new_node = std::make_unique<Node>(std::move(value));
        Node* new_tail = new_node.get();
        {
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            tail->data = std::move(value);
            tail->next = std::move(new_node);
            tail = new_tail;
        }

        cond_var.notify_one();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        return head.get() == get_tail();
    }

private:
    class Node
    {
    public:
        T data;
        std::unique_ptr<Node> next;

        Node() : data(T())
        {
        }

        Node(T data) : data(std::move(data))
        {
        }
    };

    std::unique_ptr<Node> head;
    Node* tail;

    mutable std::mutex head_mutex;
    std::mutex tail_mutex;
    std::condition_variable cond_var;

    Node* get_tail()
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<Node> pop_head()
    {
        std::unique_ptr<Node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

    std::unique_ptr<Node> try_pop_head()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == get_tail())
        {
            return nullptr;
        }

        return pop_head();
    }

    std::unique_ptr<Node> wait_pop_head()
    {
        std::unique_lock<std::mutex> head_lock(head_mutex);
        cond_var.wait(head_lock, [this]()
        {
            head.get() != get_tail();
        });

        return pop_head();
    }
};
