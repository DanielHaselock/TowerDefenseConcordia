#pragma once
#include <list>
#include <functional>
/**
 * @class Event
 * @brief Generic observer-pattern event that callbacks can subscribe to and that can be dispatched with arguments.
 *
 * Args are the argument types passed to every subscribed callback when the
 * event is dispatched (e.g. Event<int, int> for an event carrying two ints).
 * Used in this module as the basis for UI-style notifications (e.g. a
 * button being clicked) decoupled from whoever handles them.
 */
template<typename ... Args> class Event
{
	public:
		std::list<std::function<void(Args&...)>> mActions; ///< All callbacks currently subscribed to this event, in subscription order.

		/**
		 * @brief Registers a callback to be invoked on dispatch().
		 * @param pAction Callback taking a reference to each of Args.
		 * @return Index of the callback within mActions at the time of insertion, usable with remove().
		 */
		size_t subscribe(std::function<void(Args&...)> pAction)
		{
			mActions.push_back(pAction);
			return mActions.size() - 1;
		}

		/**
		 * @brief Unsubscribes the callback at the given position.
		 * @param pPos Index previously returned by subscribe(). Note this is a
		 *        position within the list at call time, not a stable handle:
		 *        it becomes invalid if earlier callbacks are removed first.
		 */
		void remove(size_t pPos)
		{
			auto it = mActions.begin();
			std::advance(it, (int)pPos);

			mActions.erase(it);
		}

		/// Invokes every subscribed callback with the given arguments, in subscription order.
		void dispatch(Args&... pArgs)
		{
			if (mActions.size() == 0)
				return;

			for (std::function<void(Args&...)>& action : mActions)
				action(pArgs...);
		}
};