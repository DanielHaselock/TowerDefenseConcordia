#include "Observer.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

namespace
{
    class DummySubject : public Game::Observable<DummySubject>
    {
    public:
        int value = 0;

        void setValue(int v)
        {
            value = v;
            notify();
        }
    };

    class RecordingObserver : public Game::Observer<DummySubject>
    {
    public:
        explicit RecordingObserver(DummySubject& subject) : Game::Observer<DummySubject>(subject) {}

        int callCount = 0;
        int lastSeenValue = 0;

        void update(const DummySubject& subject) override
        {
            ++callCount;
            lastSeenValue = subject.value;
        }
    };
}

TEST_CASE("Observable notifies every attached observer with the current subject state", "[Observer]")
{
    DummySubject subject;
    RecordingObserver observerA(subject);
    RecordingObserver observerB(subject);

    subject.setValue(42);

    CHECK(observerA.callCount == 1);
    CHECK(observerA.lastSeenValue == 42);
    CHECK(observerB.callCount == 1);
    CHECK(observerB.lastSeenValue == 42);
}

TEST_CASE("Observable rejects a null observer", "[Observer]")
{
    DummySubject subject;

    CHECK_THROWS_AS(subject.attach(nullptr), std::invalid_argument);
}

TEST_CASE("Observable rejects detaching a null observer", "[Observer]")
{
    DummySubject subject;

    CHECK_THROWS_AS(subject.detach(nullptr), std::invalid_argument);
}

TEST_CASE("Observable stops notifying a detached observer", "[Observer]")
{
    DummySubject subject;
    RecordingObserver observer(subject);

    subject.setValue(1);
    REQUIRE(observer.callCount == 1);

    subject.detach(&observer);
    subject.setValue(2);

    CHECK(observer.callCount == 1); // unchanged after detach
}

TEST_CASE("Observable notifies on every state change, not just the first", "[Observer]")
{
    DummySubject subject;
    RecordingObserver observer(subject);

    subject.setValue(1);
    subject.setValue(2);
    subject.setValue(3);

    CHECK(observer.callCount == 3);
    CHECK(observer.lastSeenValue == 3);
}
