/** @file
    @brief Header for interoperation between the Eigen math library, the
   internal mini math library, and VRPN's quatlib

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
#include "unifiedvideoinertial/MathTypesC.h"

// Library/third-party includes
#include "unifiedvideoinertial/EigenExtras.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

// Standard includes
// - none

namespace videotracker {
namespace util {

    /** @addtogroup UtilMath
    @{
    */

    /// @brief Wrap an UVBI_Vec3 in an Eigen object that allows it to
    /// interoperate with Eigen as though it were an Eigen::Vector3d
    ///
    /// @param vec A vector to wrap
    /// @returns an Eigen::Map allowing use of the UVBI_Vec3 as an
    /// Eigen::Vector3d.
    inline Eigen::Map<Eigen::Vector3d> vecMap(UVBI_Vec3 &vec) {
        return Eigen::Map<Eigen::Vector3d>(&(vec.data[0]));
    }

    /// @overload
    /// For constant vectors.
    inline Eigen::Map<const Eigen::Vector3d> vecMap(UVBI_Vec3 const &vec) {
        return Eigen::Map<const Eigen::Vector3d>(&(vec.data[0]));
    }

    /// @brief Convert an UVBI_Quaternion to an Eigen::Quaterniond
    inline Eigen::Quaterniond fromQuat(UVBI_Quaternion const &q) {
        return Eigen::Quaterniond(uvbiQuatGetW(&q), uvbiQuatGetX(&q),
                                  uvbiQuatGetY(&q), uvbiQuatGetZ(&q));
    }

    /// @brief Convert an Eigen::Quaterniond to a UVBI_Quaternion
    inline void toQuat(Eigen::Quaterniond const &src, UVBI_Quaternion &q) {
        uvbiQuatSetW(&q, src.w());
        uvbiQuatSetX(&q, src.x());
        uvbiQuatSetY(&q, src.y());
        uvbiQuatSetZ(&q, src.z());
    }

    /// @brief Turn an OSVR_Pose3 into an Eigen::Transform
    ///
    /// @param pose Input pose
    /// @returns an Eigen::Isometry3d (convertible to an Eigen::Affine3d)
    /// 3-dimensional transform object equivalent to pose.
    inline Eigen::Isometry3d fromPose(OSVR_Pose3 const &pose) {
        Eigen::Isometry3d newPose;
        newPose.fromPositionOrientationScale(vecMap(pose.translation),
                                             fromQuat(pose.rotation),
                                             Eigen::Vector3d::Constant(1));
        return newPose;
    }

    /// @brief Turn an Eigen::Isometry3d (transform) into an OSVR_Pose3
    ///
    /// @param[in] xform Input transform.
    /// @param[out] pose Destination to set based on xform.
    inline void toPose(Eigen::Isometry3d const &xform, OSVR_Pose3 &pose) {
        vecMap(pose.translation) = xform.translation();
        Eigen::Quaterniond quat(xform.rotation());
        toQuat(quat, pose.rotation);
    }

    /// @brief Turn an Eigen::Matrix4d (transform) into an OSVR_Pose3
    ///
    /// @param[in] mat Input transform. Assumed to contain only position and
    /// orientation.
    /// @param[out] pose Destination to set based on xform.
    inline void toPose(Eigen::Matrix4d const &mat, OSVR_Pose3 &pose) {
        Eigen::Affine3d xform(mat);
        vecMap(pose.translation) = xform.translation();
        Eigen::Quaterniond quat(xform.rotation());
        toQuat(quat, pose.rotation);
    }

    /// Namespace containing const_map() and map() function
    /// implementations/overloads to allow easy interoperation of OSVR types and
    /// Eigen types.
    namespace eigen_interop {
        inline Eigen::Map<const Eigen::Vector3d>
        const_map(UVBI_Vec3 const &vec) {
            return vecMap(vec);
        }

        inline Eigen::Map<Eigen::Vector3d> map(UVBI_Vec3 &vec) {
            return vecMap(vec);
        }

        inline Eigen::Map<const Eigen::Vector3d> map(UVBI_Vec3 const &vec) {
            return const_map(vec);
        }
        namespace detail {
            template <char Coefficient> struct QuatAccessor;
            template <> struct QuatAccessor<'W'> {
                static double get(UVBI_Quaternion const &q) {
                    return uvbiQuatGetW(&q);
                }

                static void set(UVBI_Quaternion &q, double v) {
                    return uvbiQuatSetW(&q, v);
                }
            };
            template <> struct QuatAccessor<'X'> {
                static double get(UVBI_Quaternion const &q) {
                    return uvbiQuatGetX(&q);
                }

                static void set(UVBI_Quaternion &q, double v) {
                    return uvbiQuatSetX(&q, v);
                }
            };

            template <> struct QuatAccessor<'Y'> {
                static double get(UVBI_Quaternion const &q) {
                    return uvbiQuatGetY(&q);
                }

                static void set(UVBI_Quaternion &q, double v) {
                    return uvbiQuatSetY(&q, v);
                }
            };

            template <> struct QuatAccessor<'Z'> {
                static double get(UVBI_Quaternion const &q) {
                    return uvbiQuatGetZ(&q);
                }

                static void set(UVBI_Quaternion &q, double v) {
                    return uvbiQuatSetZ(&q, v);
                }
            };

            template <char Coefficient, typename T = UVBI_Quaternion>
            class QuatCoefficientMap {
              public:
                QuatCoefficientMap(T &quat) : m_quat(&quat) {}
                operator double() const {
                    return QuatAccessor<Coefficient>::get(*m_quat);
                }
                QuatCoefficientMap const &operator=(double v) const {
                    QuatAccessor<Coefficient>::set(*m_quat, v);
                    return *this;
                }

              private:
                T *m_quat;
            };
            template <typename T = UVBI_Quaternion const> class BaseQuatMap {
              public:
                BaseQuatMap(T &quat) : m_quat(&quat) {}
#if 0
				BaseQuatMap(BaseQuatMap const&) = delete;
				BaseQuatMap(BaseQuatMap && other) : m_quat(other.m_quat) {}
#endif

                /// Read-only accessor for the quaternion as an Eigen
                /// quaternion.
                Eigen::Quaterniond quat() const { return fromQuat(*m_quat); }

                /// Read-only conversion operator for the quaternion as an Eigen
                /// quaternion.
                operator Eigen::Quaterniond() const { return quat(); }

                /// Read-only accessor for the w component of the quaternion.
                double w() const { return uvbiQuatGetW(m_quat); }
                /// Read-only accessor for the x component of the quaternion.
                double x() const { return uvbiQuatGetX(m_quat); }
                /// Read-only accessor for the y component of the quaternion.
                double y() const { return uvbiQuatGetY(m_quat); }
                /// Read-only accessor for the z component of the quaternion.
                double z() const { return uvbiQuatGetZ(m_quat); }

              protected:
                T *m_quat;
            };

            typedef BaseQuatMap<> ConstQuatMap;
            class QuatMap : public BaseQuatMap<UVBI_Quaternion> {
              public:
                QuatMap(UVBI_Quaternion &quat) : BaseQuatMap(quat) {}
#if 0
				QuatMap(QuatMap const&) = delete;
				QuatMap(QuatMap && other) : BaseQuatMap(std::move(other)) {}
#endif
                /// Assignment from another QuatMap (whether const or not)
                template <typename T>
                QuatMap const &operator=(BaseQuatMap<T> const &other) const {
                    *m_quat = *other.m_quat;
                    return *this;
                }
                /// Assignment operator from an Eigen quaternion.
                template <typename Derived>
                QuatMap const &
                operator=(Eigen::QuaternionBase<Derived> const &other) const {
                    uvbiQuatSetW(m_quat, other.w());
                    uvbiQuatSetX(m_quat, other.x());
                    uvbiQuatSetY(m_quat, other.y());
                    uvbiQuatSetZ(m_quat, other.z());
                    return *this;
                }

                /// Read-write accessor for the w component of the quaternion.
                QuatCoefficientMap<'W'> w() const {
                    return QuatCoefficientMap<'W'>(*m_quat);
                }
                /// Read-write accessor for the x component of the quaternion.
                QuatCoefficientMap<'X'> x() const {
                    return QuatCoefficientMap<'X'>(*m_quat);
                }
                /// Read-write accessor for the y component of the quaternion.
                QuatCoefficientMap<'Y'> y() const {
                    return QuatCoefficientMap<'Y'>(*m_quat);
                }
                /// Read-write accessor for the z component of the quaternion.
                QuatCoefficientMap<'Z'> z() const {
                    return QuatCoefficientMap<'Z'>(*m_quat);
                }
            };
        } // namespace detail

        inline detail::ConstQuatMap const_map(UVBI_Quaternion const &quat) {
            return detail::ConstQuatMap(quat);
        }

        inline detail::ConstQuatMap map(UVBI_Quaternion const &quat) {
            return const_map(quat);
        }

        inline detail::QuatMap map(UVBI_Quaternion &quat) {
            return detail::QuatMap(quat);
        }

        namespace detail {
            template <typename T = OSVR_Pose3 const> class BasePoseMap {
              public:
                BasePoseMap(T &pose) : m_pose(&pose) {}
#if 0
				BasePoseMap(BasePoseMap const&) = delete;
				BasePoseMap(BasePoseMap && other) : m_pose(other.m_pose) {}
#endif
                typedef Eigen::Isometry3d TransformType;
                typedef typename TransformType::MatrixType MatrixType;

                /// Read-only accessor for the pose as an Eigen transform.
                TransformType transform() const { return fromPose(*m_pose); }
                /// Read-only conversion operator for the pose as an Eigen
                /// transform.
                operator TransformType() const { return transform(); }

                /// Read-only accessor for the pose as an Eigen Matrix
                MatrixType matrix() const { return fromPose(*m_pose).matrix(); }

                /// Read-only accessor for the translation as an Eigen vector
                /// map.
                Eigen::Map<const Eigen::Vector3d> translation() const {
                    return const_map(m_pose->translation);
                }
                /// Read-only accessor for the translation as an Eigen
                /// quaternion.
                ConstQuatMap rotation() const {
                    return const_map(m_pose->rotation);
                }

              protected:
                T *m_pose;
            };

            typedef BasePoseMap<> ConstPoseMap;

            class PoseMap : public BasePoseMap<OSVR_Pose3> {
              public:
                PoseMap(OSVR_Pose3 &pose) : BasePoseMap(pose) {}
#if 0
				PoseMap(PoseMap const&) = delete;
				PoseMap(PoseMap && other) : BasePoseMap(std::move(other)) {}
#endif
                /// Assignment operator from another PoseMap (const or not)
                template <typename T>
                PoseMap const &operator=(BasePoseMap<T> const &other) const {
                    *m_pose = *other.m_pose;
                    return *this;
                }

                /// Assignment operator from an Eigen isometry
                PoseMap const &operator=(Eigen::Isometry3d const &other) const {
                    toPose(other, *m_pose);
                    return *this;
                }

                /// Assignment operator from an Eigen 4x4 matrix - note that it
                /// is assumed to be an isometry!
                PoseMap const &operator=(Eigen::Matrix4d const &other) const {
                    toPose(other, *m_pose);
                    return *this;
                }

                /// Read-write accessor for the translation portion of the pose
                /// as an Eigen vector (map)
                Eigen::Map<Eigen::Vector3d> translation() const {
                    return map(m_pose->translation);
                }

                /// Read-write accessor for the rotation portion of the pose as
                /// an Eigen quaternion (map)
                QuatMap rotation() const { return map(m_pose->rotation); }
            };
        } // namespace detail

        inline detail::ConstPoseMap const_map(OSVR_Pose3 const &pose) {
            return detail::ConstPoseMap(pose);
        }

        inline detail::ConstPoseMap map(OSVR_Pose3 const &pose) {
            return const_map(pose);
        }

        inline detail::PoseMap map(OSVR_Pose3 &pose) {
            return detail::PoseMap(pose);
        }
    } // namespace eigen_interop
    /** @} */

} // namespace util
} // namespace videotracker
