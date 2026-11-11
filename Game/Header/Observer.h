#pragma once

#include "Observable.h"

namespace Game {

    /**
     * @brief Observer interface for a given Subject type (classic GoF Observer).
     *
     * Implemented by view/display classes that want to be notified whenever
     * the Subject they are attached to changes state.
     */
    template <typename Subject>
    class Observer {
    public:
        /** @brief Attaches this observer to the supplied subject. */
        explicit Observer(Subject& subject) : subject(subject) {
            subject.attach(this);
        }

        /** @brief Detaches this observer when it is destroyed. */
        virtual ~Observer() {
            subject.detach(this);
        }

        /// Called by the observed Subject whenever its state changes.
        virtual void update(const Subject& subject) = 0;

    private:
        Subject& subject;
    };

} // namespace Game
