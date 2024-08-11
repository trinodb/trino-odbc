#pragma once

#include <algorithm>
#include <functional>
#include <vector>

/* Remove a callback from a vector of callbacks */
template <typename TFunc>
void removeCallbackFromVector(std::vector<std::function<TFunc>>& callbackVector,
                              const std::function<TFunc>& removeCallback) {
  /*
  This thing deserves some explanation.

  When we unregister a callback, we need to find that callback
  in the vector of callbacks and remove just that callback.

  This needs to be done by comparing both the target type and
  target of the callback. The target type is like the member function,
  and the target is a pointer to the callable stored in the std::function.

  The latter should disambiguate between the same member function stored
  on different instances of a class, which is the behavior we want.
  */
  auto it =
      std::remove_if(callbackVector.begin(),
                     callbackVector.end(),
                     [&removeCallback](const std::function<TFunc>& callback) {
                       bool sameTargetType = callback.target_type() ==
                                             removeCallback.target_type();
                       bool sameTarget = callback.target<TFunc>() ==
                                         removeCallback.target<TFunc>();
                       return sameTargetType and sameTarget;
                     });
  callbackVector.erase(it, callbackVector.end());
}
