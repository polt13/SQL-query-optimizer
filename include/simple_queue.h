#ifndef SQUEUE
#define SQUEUE

template <typename U>
struct queue_entry {
  U obj;
  queue_entry* next;
  queue_entry(const U& u) : obj(u), next{nullptr} {}
};

template <typename T>
class simple_queue {
  size_t len;
  queue_entry<T>* head;
  queue_entry<T>* tail;

 public:
  void enqueue(const T& obj) {
    if (len == 0)
      head = tail = new queue_entry<T>(obj);
    else {
      tail->next = new queue_entry<T>(obj);
      tail = tail->next;
    }

    len++;
  }

  T pop() {
    T obj = head->obj;

    queue_entry<T>* temp = head;

    head = head->next;

    delete temp;

    --len;

    return obj;
  }

  T peek() { return head->obj; }

  simple_queue() : len{}, head{nullptr}, tail{nullptr} {}

  size_t getLen() const { return len; }

  ~simple_queue() {
    for (size_t i = 0; i < len; i++) {
      queue_entry<T>* t = head;
      head = head->next;
      delete t;
    }
  }
};

#endif