/** @file
    @brief Header

    Must be c-safe!

    @date 2014

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

/*
// Copyright 2014 Sensics, Inc.
// Copyright 2019 Collabora, Ltd.
//
// SPDX-License-Identifier: Apache-2.0
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
*/

#pragma once

/* Internal Includes */
#include "APIBaseC.h"

/* Library/third-party includes */
/* none */

/* Standard includes */
/* none */

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup UtilMath
    @{
*/
/** @brief A structure defining a 2D vector, which represents position
 */
typedef struct OSVR_Vec2 {
    /** @brief Internal array data. */
    double data[2];
} OSVR_Vec2;

/** @brief A structure defining a 3D vector, often a position/translation.
 */
typedef struct UVBI_Vec3 {
    /** @brief Internal array data. */
    double data[3];
} UVBI_Vec3;

#define OSVR_VEC_MEMBER(COMPONENT, INDEX)                                      \
    /** @brief Accessor for Vec3 component COMPONENT */                        \
    UVBI_INLINE double osvrVec3Get##COMPONENT(UVBI_Vec3 const *v) {            \
        return v->data[INDEX];                                                 \
    }                                                                          \
    /** @brief Setter for Vec3 component COMPONENT */                          \
    UVBI_INLINE void osvrVec3Set##COMPONENT(UVBI_Vec3 *v, double val) {        \
        v->data[INDEX] = val;                                                  \
    }

OSVR_VEC_MEMBER(X, 0)
OSVR_VEC_MEMBER(Y, 1)
OSVR_VEC_MEMBER(Z, 2)

#undef OSVR_VEC_MEMBER

/** @brief Set a Vec3 to the zero vector */
UVBI_INLINE void osvrVec3Zero(UVBI_Vec3 *v) {
    osvrVec3SetX(v, 0);
    osvrVec3SetY(v, 0);
    osvrVec3SetZ(v, 0);
}
/** @brief A structure defining a quaternion, often a unit quaternion
 * representing 3D rotation.
 */
typedef struct UVBI_Quaternion {
    /** @brief Internal data - direct access not recommended */
    double data[4];
} UVBI_Quaternion;

#define OSVR_QUAT_MEMBER(COMPONENT, INDEX)                                     \
    /** @brief Accessor for quaternion component COMPONENT */                  \
    UVBI_INLINE double uvbiQuatGet##COMPONENT(UVBI_Quaternion const *q) {      \
        return q->data[INDEX];                                                 \
    }                                                                          \
    /** @brief Setter for quaternion component COMPONENT */                    \
    UVBI_INLINE void uvbiQuatSet##COMPONENT(UVBI_Quaternion *q, double val) {  \
        q->data[INDEX] = val;                                                  \
    }

OSVR_QUAT_MEMBER(W, 0)
OSVR_QUAT_MEMBER(X, 1)
OSVR_QUAT_MEMBER(Y, 2)
OSVR_QUAT_MEMBER(Z, 3)

#undef OSVR_QUAT_MEMBER

/** @brief Set a quaternion to the identity rotation */
UVBI_INLINE void uvbiQuatSetIdentity(UVBI_Quaternion *q) {
    uvbiQuatSetW(q, 1);
    uvbiQuatSetX(q, 0);
    uvbiQuatSetY(q, 0);
    uvbiQuatSetZ(q, 0);
}

/** @brief A structure defining a 3D (6DOF) rigid body pose: translation and
    rotation.
*/
typedef struct OSVR_Pose3 {
    /** @brief Position vector */
    UVBI_Vec3 translation;
    /** @brief Orientation as a unit quaternion */
    UVBI_Quaternion rotation;
} OSVR_Pose3;

/** @brief Set a pose to identity */
UVBI_INLINE void osvrPose3SetIdentity(OSVR_Pose3 *pose) {
    uvbiQuatSetIdentity(&(pose->rotation));
    osvrVec3Zero(&(pose->translation));
}

/** @} */

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
template <typename StreamType>
inline StreamType &operator<<(StreamType &os, UVBI_Vec3 const &vec) {
    os << "(" << vec.data[0] << ", " << vec.data[1] << ", " << vec.data[2]
       << ")";
    return os;
}
template <typename StreamType>
inline StreamType &operator<<(StreamType &os, UVBI_Quaternion const &quat) {
    os << "(" << uvbiQuatGetW(&quat) << ", (" << uvbiQuatGetX(&quat) << ", "
       << uvbiQuatGetY(&quat) << ", " << uvbiQuatGetZ(&quat) << "))";
    return os;
}
#endif
