#include "Player.h"
#include "Observer.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

namespace
{
    // Minimal Observer<Player> that records every notification it receives.
    struct CoinRecorder : public Game::Observer<App::Player>
    {
        int calls = 0;
        float lastSeen = -1.f;

        CoinRecorder(App::Player& player) : Game::Observer<App::Player>(player) {}

        void update(const App::Player& player) override
        {
            ++calls;
            lastSeen = player.GetCoins();
        }
    };
}

TEST_CASE("Player notifies attached observers on every coin change", "[Player]")
{
    App::Player player;
    CoinRecorder recorder(player);

    player.ChangeCoins(-10);
    CHECK(recorder.calls == 1);
    CHECK(recorder.lastSeen == 20.f);

    player.ChangeCoins(5);
    CHECK(recorder.calls == 2);
    CHECK(recorder.lastSeen == 25.f);
}

TEST_CASE("Player floors coins at zero but still notifies", "[Player]")
{
    App::Player player;
    CoinRecorder recorder(player);

    player.ChangeCoins(-100);
    CHECK(player.GetCoins() == 0.f);
    CHECK(recorder.calls == 1);
    CHECK(recorder.lastSeen == 0.f);
}

TEST_CASE("Player notifies attached observers on every health change", "[Player]")
{
    App::Player player;
    CoinRecorder recorder(player);

    player.ChangeHealth(-1);
    CHECK(recorder.calls == 1);
    CHECK(player.GetHealth() == STARTING_HEALTH - 1);
    CHECK_FALSE(player.IsDefeated());
}

TEST_CASE("Player floors health at zero and reports defeat", "[Player]")
{
    App::Player player;

    player.ChangeHealth(-(STARTING_HEALTH + 5));
    CHECK(player.GetHealth() == 0);
    CHECK(player.IsDefeated());
}

TEST_CASE("Observer detaches automatically when destroyed", "[Player]")
{
    App::Player player;
    int calls = 0;
    {
        // Recorder whose counter outlives it, to prove no notification
        // happens after destruction.
        struct CountedRecorder : public Game::Observer<App::Player>
        {
            int& calls;
            CountedRecorder(App::Player& player, int& calls)
                : Game::Observer<App::Player>(player), calls(calls) {}
            void update(const App::Player& player) override { ++calls; }
        } recorder(player, calls);

        player.ChangeCoins(1);
        CHECK(calls == 1);
    }

    // The recorder's destructor detached it; further changes still apply to
    // the balance but notify nobody.
    player.ChangeCoins(1);
    CHECK(calls == 1);
    CHECK(player.GetCoins() == STARTING_COINS + 2);
}
