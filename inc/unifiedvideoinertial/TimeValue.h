/** @file
    @brief Header providing a C++ wrapper around TimeValueC.h

    @date 2014

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2014 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

// Internal Includes
#include "TimeValueC.h"

// Library/third-party includes
// - none

// Standard includes
#include <chrono>
#include <iomanip>
#include <sstream>

struct UVBI_TimeValue;

namespace videotracker {
namespace util {

    /// @brief C++-friendly typedef for the UVBI_TimeValue structure.
    typedef ::UVBI_TimeValue TimeValue;
    /// @brief Functionality related to time and the UVBI_TimeValue abstraction.
    /// @ingroup UtilTime
    ///
    /// Note that this is for C API-bordering areas. For purely C++ code, please
    /// use std::chrono for your time needs.
    namespace time {

        /// @brief Set the given TimeValue to the current time.
        inline void getNow(TimeValue &tv) { uvbiTimeValueGetNow(&tv); }

        /// @brief Get a TimeValue for the current time.
        inline TimeValue getNow() {
            TimeValue tv;
            getNow(tv);
            return tv;
        }

        /// @brief Get a double containing seconds between the time points.
        /// @sa uvbiTimeValueDurationSeconds()
        inline double duration(TimeValue const &a, TimeValue const &b) {
            return uvbiTimeValueDurationSeconds(&a, &b);
        }

        /// @brief Converts to a precise decimal string.
        inline std::string toDecimalString(TimeValue tv) {
            std::ostringstream os;
            uvbiTimeValueNormalize(&tv);
            // Ensure we have only non-negative components, pulling out the
            // negatives. (Normalize ensures we have 0 or the same sign between
            // components)
            bool negative = false;
            if (tv.seconds < 0) {
                negative = true;
                tv.seconds *= -1;
            }
            if (tv.microseconds < 0) {
                negative = true;
                tv.microseconds *= -1;
            }
            // Output sign
            if (negative) {
                os << "-";
            }
            // Output seconds
            os << tv.seconds << ".";
            // set up decimal padding
            os << std::setw(6) << std::setfill('0');
            // Output microseconds
            os << tv.microseconds;

            return os.str();
        }

        /// @brief Convert a TimeValue to a struct timeval
        inline void toStructTimeval(struct timeval &dest,
                                    TimeValue const &src) {
            uvbiTimeValueToStructTimeval(&dest, &src);
        }

        /// @brief Convert a struct timeval to a TimeValue
        inline void fromStructTimeval(TimeValue &dest,
                                      struct timeval const &src) {
            osvrStructTimevalToTimeValue(&dest, &src);
        }

        /// @brief Convert a struct timeval to a TimeValue
        inline TimeValue toTimeValue(struct timeval const &src) {
            TimeValue dest;
            osvrStructTimevalToTimeValue(&dest, &src);
            return dest;
        }
        /// @brief Convert a struct timeval to a TimeValue
        inline TimeValue fromStructTimeval(struct timeval const &src) {
            return toTimeValue(src);
        }

#ifdef KALMANFRAMEWORK_STRUCT_TIMEVAL_INCLUDED
        /// @brief Convert a TimeValue to a struct timeval
        inline struct timeval toStructTimeval(TimeValue const &src) {
            struct timeval dest;
            uvbiTimeValueToStructTimeval(&dest, &src);
            return dest;
        }
#endif // KALMANFRAMEWORK_STRUCT_TIMEVAL_INCLUDED

    } // namespace time
} // namespace util
} // namespace videotracker

/// Add a util::TimeValue and a std::chrono::duration
///
/// NB: Can't have this in the usual namespaces because TimeValue is actually a
/// typedef for the (un-namespaced) C struct, so ADL doesn't go looking in those
/// namespaces, just in std:: and std::chrono:: (because of the second
/// argument).
template <typename Rep, typename Period>
inline UVBI_TimeValue
operator+(UVBI_TimeValue const &tv,
          std::chrono::duration<Rep, Period> const &additionalTime) {
    using namespace std::chrono;
    using SecondsDuration = duration<UVBI_TimeValue_Seconds>;
    using USecondsDuration = duration<UVBI_TimeValue_Microseconds, std::micro>;
    auto ret = tv;
    auto seconds = duration_cast<SecondsDuration>(additionalTime);
    ret.seconds += seconds.count();
    ret.microseconds +=
        duration_cast<USecondsDuration>(additionalTime - seconds).count();
    uvbiTimeValueNormalize(&ret);
    return ret;
}
