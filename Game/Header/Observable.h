#pragma once

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace Game {

    template <typename Subject>
    class Observer;

    /**
     * @brief Observable (Subject) mix-in for the Observer pattern.
     *
     * A class Foo becomes observable by publicly inheriting from
     * Observable<Foo> and calling notify() whenever its state changes.
     * Every attached Observer<Foo> is then called back with a const
     * reference to Foo.
     */
    template <typename Subject>
    class Observable {
    public:
        /** @brief Allows observable subjects to be destroyed through a base pointer. */
        virtual ~Observable() = default;

        /// Registers an observer to be notified of future state changes.
        void attach(Observer<Subject>* observer) {
            if (observer == nullptr) {
                throw std::invalid_argument("Cannot attach a null observer!");
            }
            observers.push_back(observer);
        }

        /// Unregisters a previously attached observer.
        void detach(Observer<Subject>* observer) {
            if (observer == nullptr) {
                throw std::invalid_argument("Cannot detach a null observer!");
            }
            observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
        }

    protected:
        /// Notifies every attached observer that this Subject's state changed.
        void notify() {
            for (Observer<Subject>* observer : observers) {
                observer->update(static_cast<const Subject&>(*this));
            }
        }

    private:
        std::vector<Observer<Subject>*> observers;
    };

} // namespace Game
