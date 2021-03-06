/** @file
    @brief Implementation

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include "TrackingSystem_Impl.h"
#include "unifiedvideoinertial/TrackingDebugDisplay.h"
#include "videotrackershared/EdgeHoleBlobExtractor.h"

// Library/third-party includes
// - none

// Standard includes
// - none

namespace videotracker {
namespace uvbi {

    TrackingSystem_Impl::TrackingSystem_Impl(ConfigParams const &params)
        : blobExtractor(
              makeBlobExtractor(params.blobParams, params.extractParams)),
          debugDisplay(new TrackingDebugDisplay(params)),
          calib(Eigen::Vector3d(params.cameraPosition), params.cameraIsForward),
          cameraPose(Eigen::Isometry3d::Identity()),
          cameraPoseInv(Eigen::Isometry3d::Identity()) {}

    TrackingSystem_Impl::~TrackingSystem_Impl() {
        // out line to break circular dep with this and the debug display.
    }

    void TrackingSystem_Impl::triggerDebugDisplay(TrackingSystem &tracking) {
        debugDisplay->triggerDisplay(tracking, *this);
    }
} // namespace uvbi
} // namespace videotracker
