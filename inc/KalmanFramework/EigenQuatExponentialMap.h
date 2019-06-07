/** @file
    @brief Header

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

#ifndef INCLUDED_EigenQuatExponentialMap_h_GUID_7E15BC44_BCFB_438B_902B_BA0787BEE405
#define INCLUDED_EigenQuatExponentialMap_h_GUID_7E15BC44_BCFB_438B_902B_BA0787BEE405

// Internal Includes
// - none

// Library/third-party includes
#include <Eigen/Core>
#include <Eigen/Geometry>

// Standard includes
// - none

namespace osvr {
namespace util {
    namespace ei_quat_exp_map {

        template <typename Scalar> struct FourthRootMachineEps;
        template <> struct FourthRootMachineEps<double> {
            /// machine epsilon is 1e-53, so fourth root is roughly 1e-13
            static double get() { return 1.e-13; }
        };
        template <> struct FourthRootMachineEps<float> {
            /// machine epsilon is 1e-24, so fourth root is 1e-6
            static float get() { return 1.e-6f; }
        };
        /// Computes the "historical" (un-normalized) sinc(Theta)
        /// (sine(theta)/theta for theta != 0, defined as the limit value of 0
        /// at theta = 0)
        template <typename Scalar> inline Scalar sinc(Scalar theta) {
            /// fourth root of machine epsilon is recommended cutoff for taylor
            /// series expansion vs. direct computation per
            /// Grassia, F. S. (1998). Practical Parameterization of Rotations
            /// Using the Exponential Map. Journal of Graphics Tools, 3(3),
            /// 29-48. http://doi.org/10.1080/10867651.1998.10487493
            Scalar ret;
            if (theta < FourthRootMachineEps<Scalar>::get()) {
                // taylor series expansion.
                ret = Scalar(1.f) - theta * theta / Scalar(6.f);
                return ret;
            }
            // direct computation.
            ret = std::sin(theta) / theta;
            return ret;
        }

        /// fully-templated free function for quaternion expontiation
        template <typename Derived>
        inline Eigen::Quaternion<typename Derived::Scalar>
        quat_exp(Eigen::MatrixBase<Derived> const &vec) {
            EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived, 3);
            using Scalar = typename Derived::Scalar;
            /// Implementation inspired by
            /// Grassia, F. S. (1998). Practical Parameterization of Rotations
            /// Using the Exponential Map. Journal of Graphics Tools, 3(3),
            /// 29–48. http://doi.org/10.1080/10867651.1998.10487493
            ///
            /// However, that work introduced a factor of 1/2 which I could not
            /// derive from the definition of quaternion exponentiation and
            /// whose absence thus distinguishes this implementation. Without
            /// that factor of 1/2, the exp and ln functions successfully
            /// round-trip and match other implementations.
            Scalar theta = vec.norm();
            Scalar vecscale = sinc(theta);
            Eigen::Quaternion<Scalar> ret;
            ret.vec() = vecscale * vec;
            ret.w() = std::cos(theta);
            return ret.normalized();
        }

        /// Taylor series expansion of theta over sin(theta), aka cosecant, for
        /// use near 0 when you want continuity and validity at 0.
        template <typename Scalar>
        inline Scalar cscTaylorExpansion(Scalar theta) {
            return Scalar(1) +
                   // theta ^ 2 / 6
                   (theta * theta) / Scalar(6) +
                   // 7 theta^4 / 360
                   (Scalar(7) * theta * theta * theta * theta) / Scalar(360) +
                   // 31 theta^6/15120
                   (Scalar(31) * theta * theta * theta * theta * theta *
                    theta) /
                       Scalar(15120);
        }

        /// fully-templated free function for quaternion log map, intended for
        /// implementation use within the class.
        ///
        /// Assumes a unit quaternion.
        ///
        /// @todo seems to be off by a factor of two in testing?
        template <typename Scalar>
        inline Eigen::Matrix<Scalar, 3, 1>
        quat_ln(Eigen::Quaternion<Scalar> const &quat) {
            // ln q = ( (phi)/(norm of vec) vec, ln(norm of quat))
            // When we assume a unit quaternion, ln(norm of quat) = 0
            // so then we just scale the vector part by phi/sin(phi) to get the
            // result (i.e., ln(qv, qw) = (phi/sin(phi)) * qv )
            Scalar vecnorm = quat.vec().norm();

            // "best for numerical stability" vs asin or acos
            Scalar phi = std::atan2(vecnorm, quat.w());

            // Here is where we compute the coefficient to scale the vector part
            // by, which is nominally phi / std::sin(phi).
            // When the angle approaches zero, we compute the coefficient
            // differently, since it gets a bit like sinc in that we want it
            // continuous but 0 is undefined.
            Scalar phiOverSin = vecnorm < 1e-4 ? cscTaylorExpansion<Scalar>(phi)
                                               : (phi / std::sin(phi));
            return quat.vec() * phiOverSin;
        }

    } // namespace ei_quat_exp_map
    using ei_quat_exp_map::quat_exp;
    using ei_quat_exp_map::quat_ln;
} // namespace util
} // namespace osvr

#endif // INCLUDED_EigenQuatExponentialMap_h_GUID_7E15BC44_BCFB_438B_902B_BA0787BEE405
