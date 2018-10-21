#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

class Marked {
    std::string id_;
public:
    Marked(std::string id): id_(id) {}

    std::string id() const {
        return id_;
    }
};

class ClassA;
class ClassB;

enum class Type {
    PROXY = 0, MEDIATOR = 1, OBSERBER = 2
};

template<Type class_type>
class ClassC {
public:
    Type type() const {
        return class_type;
    }
};

class ClassB : public Marked {
    int value_;

    friend class ClassC<Type::OBSERBER>;
    std::unordered_set<const ClassC<Type::OBSERBER> *> subscribers_;

    void notifyAll() const;
public:
    ClassB(std::string id) : Marked(id) {}

    void foo() const {
        printf("Calling `foo` in B(%s)\n", this->id().c_str());
        notifyAll();
    }

    void setValue(int new_value) {
        printf("Settind `value` to %d in B(%s)\n", new_value, this->id().c_str());
        value_ = new_value;
        notifyAll();
    }

    int getValue() const {
        return value_;
    }
};

template<>
class ClassC<Type::PROXY> {
public:
    ClassC(ClassB & server): server_(server) { }

    void foo() const {
        printf("Calling `foo` from proxy of B(%s)\n", server_.id().c_str());
        server_.foo();
    }
private:
    const ClassB & server_;
};

class ClassA : public Marked {
    friend class ClassC<Type::OBSERBER>;
    void notify(const ClassB & from) const;

    const ClassC<Type::PROXY> * proxy_;
public:
    ClassA(std::string id) : Marked(id), proxy_(nullptr) {}
    ClassA(std::string id, const ClassC<Type::PROXY> & proxy) : Marked(id), proxy_(&proxy) { }

    void proxyFoo() const;
};

template<>
class ClassC<Type::MEDIATOR> {
    std::unordered_map<std::string, const ClassB *> objects_;

public:
    ClassC() { }

    bool addObject(const ClassB & object, const std::string && id) {
        auto insert_info = objects_.insert(std::make_pair(id, &object));
        return insert_info.second;
    }

    void callFoo(const std::string && id, const ClassA & from) const {
        auto element_iterator = objects_.find(id);
        if (element_iterator == objects_.end()) {
            throw;
        }

        printf("Calling `foo` of B(%s) in mediator from A(%s)\n", element_iterator->second->id().c_str(), from.id().c_str());
        element_iterator->second->foo();
    }
};

template<>
class ClassC<Type::OBSERBER> {
    friend class ClassB;
    void notify(const ClassB & from) const {
        auto it = subscribers_.find(&from);

        if (it != subscribers_.end()) {
            for (auto subscriber : it->second) {
                subscriber->notify(from);
            }
        }
    }

    std::unordered_map<const ClassB *, std::unordered_set<const ClassA *>> subscribers_;
public:
    void addSubscriber(const ClassA & subscriber, ClassB & publisher) {
        publisher.subscribers_.insert(this);
        subscribers_[&publisher].insert(&subscriber);
    }
};

void ClassA::notify(const ClassB & from) const {
    printf("Notify in A(%s) from B(%s)\n", this->id().c_str(), from.id().c_str());
}

void ClassA::proxyFoo() const {
    printf("Calling `foo` from A(%s)\n", this->id().c_str());
    proxy_->foo();
}

void ClassB::notifyAll() const {
    for (auto subscriber : subscribers_) {
        subscriber->notify(*this);
    }
}

int main(int argc, char * argv[]) {
    std::cout << "-----\n";
    std::cout << "PROXY testing\n";
    std::cout << "-----\n";

    ClassB b_proxy_0("B server");
    ClassC<Type::PROXY> proxy(b_proxy_0);
    ClassA a_proxy_0("Client", proxy);
    a_proxy_0.proxyFoo();

    std::cout << "-----\n";
    std::cout << "MEDIATOR testing\n";
    std::cout << "-----\n";

    ClassB b_mediator_1("Object 1"), b_mediator_2("Object 2"), b_mediator_3("Object 3");
    ClassC<Type::MEDIATOR> mediator;
    mediator.addObject(b_mediator_1, "1");
    mediator.addObject(b_mediator_2, "2");
    mediator.addObject(b_mediator_3, "3");

    ClassA caller("Caller");
    mediator.callFoo("1", caller);

    std::cout << "-----\n";
    std::cout << "OBSERVER testing\n";
    std::cout << "-----\n";

    ClassA a_observer_1("Observer 1"), a_observer_2("Observer 2"), a_observer_3("Observer 3");
    ClassB b_observer_1("Publisher for 1"), b_observer_2_3("Publisher for 2, 3");
    ClassC<Type::OBSERBER> observer;

    observer.addSubscriber(a_observer_1, b_observer_1);
    observer.addSubscriber(a_observer_2, b_observer_2_3);
    observer.addSubscriber(a_observer_3, b_observer_2_3);

    b_observer_1.foo();
    b_observer_2_3.setValue(10);

    std::cout << "-----\n";
    
    return 0;
}