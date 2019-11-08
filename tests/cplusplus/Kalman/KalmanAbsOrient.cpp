/** @file
    @brief Implementation

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
// Copyright 2019 Collabora, Ltd.
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

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#if 0
template <typename T>
inline void dumpKalmanDebugOuput(const char name[], const char expr[],
                                 T const &value) {
    std::cout << "\n(Kalman Debug Output) " << name << " [" << expr << "]:\n"
              << value << std::endl;
}

#define FLEXKALMAN_DEBUG_OUTPUT(Name, Value)                                   \
    dumpKalmanDebugOuput(Name, #Value, Value)
#endif
#include "FlexKalman/AbsoluteOrientationMeasurement.h"
#include "FlexKalman/FlexibleKalmanFilter.h"
#include "FlexKalman/FlexibleUnscentedCorrect.h"
#include "FlexKalman/PoseConstantVelocity.h"
#include "FlexKalman/PoseDampedConstantVelocity.h"
// #include "unifiedvideoinertial/IMUStateMeasurements.h"

#include <catch2/catch.hpp>

#include "ContentsInvalid.h"

static const std::initializer_list<std::array<double, 5>> DATA = {
    {0.000777, 0.735083, -0.424120, 0.000000, -0.528938},
    {0.000777, 0.994757, 0.072232, 0.010660, 0.071607},
    {0.049230, 0.904147, 0.277876, 0.009694, 0.324360},
    {0.049230, 0.829623, 0.361128, 0.006575, 0.425756},
    {0.000777, 0.781396, 0.403545, 0.006223, 0.475955},
    {0.000777, 0.757074, 0.422659, 0.005805, 0.498161},
    {0.049233, 0.746140, 0.426895, 0.007090, 0.510867},
    {0.049233, 0.740573, 0.427847, 0.006742, 0.518125},
    {0.000776, 0.737885, 0.429090, 0.007459, 0.520914},
    {0.000776, 0.736579, 0.429723, 0.007710, 0.522236},
    {0.049231, 0.736150, 0.428669, 0.005341, 0.523735},
    {0.049231, 0.735447, 0.428213, 0.002545, 0.525114},
    {0.000564, 0.734909, 0.429760, 0.001144, 0.524608},
    {0.000564, 0.735422, 0.430547, -0.000215, 0.523243},
    {0.049440, 0.735881, 0.427504, -0.000824, 0.525089},
    {0.049440, 0.735445, 0.426136, -0.002629, 0.526803},
    {0.000563, 0.734822, 0.427088, -0.003253, 0.526899},
    {0.000563, 0.734807, 0.426948, -0.003074, 0.527034},
    {0.049445, 0.735370, 0.424280, -0.003685, 0.528397},
    {0.049445, 0.735055, 0.422691, -0.005200, 0.530094},
    {0.000564, 0.735211, 0.423318, -0.006070, 0.529368},
    {0.000564, 0.735126, 0.423712, -0.006471, 0.529165},
    {0.054442, 0.734356, 0.422607, -0.008642, 0.531084},
    {0.054442, 0.734319, 0.420257, -0.009948, 0.532973},
    {0.000570, 0.734690, 0.419867, -0.009785, 0.532772},
    {0.000570, 0.734596, 0.420783, -0.010798, 0.532159},
    {0.044437, 0.735354, 0.417481, -0.011954, 0.533687},
    {0.044437, 0.735864, 0.415163, -0.014254, 0.534734},
    {0.000566, 0.735074, 0.416157, -0.014792, 0.535033},
    {0.000566, 0.734525, 0.416850, -0.015171, 0.535237},
    {0.049439, 0.735620, 0.414126, -0.017460, 0.535778},
    {0.049439, 0.736052, 0.411834, -0.019798, 0.536869},
    {0.000563, 0.735417, 0.413200, -0.020901, 0.536648},
    {0.000563, 0.734961, 0.413699, -0.021123, 0.536879},
    {0.049446, 0.735123, 0.410656, -0.022029, 0.538953},
    {0.049446, 0.734563, 0.409331, -0.023665, 0.540652},
    {0.000562, 0.734204, 0.410441, -0.024693, 0.540253},
    {0.000562, 0.734206, 0.409876, -0.024023, 0.540709},
    {0.055746, 0.735040, 0.407160, -0.026315, 0.541520},
    {0.055746, 0.735168, 0.405513, -0.028850, 0.542453},
    {0.000474, 0.734304, 0.406788, -0.029662, 0.542625},
    {0.000474, 0.734378, 0.406444, -0.029313, 0.542801},
    {0.043233, 0.735421, 0.403942, -0.030010, 0.543219},
    {0.043233, 0.734619, 0.403060, -0.031812, 0.544854},
    {0.000561, 0.733246, 0.404685, -0.032631, 0.545450},
    {0.000561, 0.733518, 0.404248, -0.032331, 0.545426},
    {0.049441, 0.734176, 0.401800, -0.035112, 0.546177},
    {0.049441, 0.734113, 0.400186, -0.037773, 0.547268},
    {0.000565, 0.733974, 0.400345, -0.037848, 0.547333},
    {0.000565, 0.734581, 0.400865, -0.038963, 0.546059},
    {0.049443, 0.734284, 0.398295, -0.040439, 0.548226},
    {0.049443, 0.734224, 0.396175, -0.042787, 0.549663},
    {0.000564, 0.734072, 0.397034, -0.043684, 0.549176},
    {0.000564, 0.733868, 0.397416, -0.043972, 0.549150},
    {0.053731, 0.735268, 0.394511, -0.045409, 0.549254},
    {0.053731, 0.735302, 0.392418, -0.046640, 0.550602},
    {0.000700, 0.734523, 0.393356, -0.047116, 0.550933},
    {0.000700, 0.734612, 0.393464, -0.047318, 0.550719},
    {0.048803, 0.733838, 0.391326, -0.048347, 0.553180},
    {0.048803, 0.733835, 0.389918, -0.050331, 0.554001},
    {0.000489, 0.733756, 0.390856, -0.051381, 0.553348},
    {0.000489, 0.732924, 0.391541, -0.051517, 0.553953},
    {0.045734, 0.733468, 0.388360, -0.052453, 0.555382},
    {0.045734, 0.733928, 0.385896, -0.052881, 0.556451},
    {0.000488, 0.733209, 0.387770, -0.054521, 0.555937},
    {0.000488, 0.733085, 0.388651, -0.055467, 0.555392},
    {0.049519, 0.734510, 0.384865, -0.056593, 0.556032},
    {0.049519, 0.734751, 0.382145, -0.058445, 0.557396},
    {0.000489, 0.733668, 0.383386, -0.059034, 0.557908},
    {0.000489, 0.732808, 0.384676, -0.059866, 0.558061},
    {0.049518, 0.733384, 0.381539, -0.060358, 0.559404},
    {0.049518, 0.734184, 0.379546, -0.061424, 0.559595},
    {0.000488, 0.733050, 0.380913, -0.062123, 0.560075},
    {0.000488, 0.733043, 0.381771, -0.063136, 0.559386},
    {0.050017, 0.732183, 0.380312, -0.064666, 0.561328},
    {0.050017, 0.732030, 0.376773, -0.064712, 0.563903},
    {0.000779, 0.731666, 0.377124, -0.064835, 0.564127},
    {0.000779, 0.731559, 0.377020, -0.064626, 0.564359},
    {0.048721, 0.734326, 0.370919, -0.063187, 0.564970},
    {0.048721, 0.734883, 0.367947, -0.063456, 0.566158},
    {0.000495, 0.734131, 0.370131, -0.065440, 0.565485},
    {0.000495, 0.733238, 0.370861, -0.065626, 0.566143},
    {0.052020, 0.734332, 0.364733, -0.061167, 0.569198},
    {0.052020, 0.736918, 0.358078, -0.058003, 0.570410},
    {0.000492, 0.736228, 0.360154, -0.059870, 0.569801},
    {0.000492, 0.735652, 0.360186, -0.059507, 0.570562},
    {0.047016, 0.733523, 0.367014, -0.069606, 0.567803},
    {0.047016, 0.731369, 0.370335, -0.075101, 0.567725},
    {0.000488, 0.731114, 0.371409, -0.076171, 0.567208},
    {0.000488, 0.730230, 0.371909, -0.076047, 0.568035},
    {0.049517, 0.732196, 0.362424, -0.070621, 0.572320},
    {0.049517, 0.732782, 0.357157, -0.068637, 0.575117},
    {0.000489, 0.730090, 0.363185, -0.073608, 0.574150},
    {0.000489, 0.730562, 0.361547, -0.072090, 0.574774},
    {0.050020, 0.731062, 0.360416, -0.076104, 0.574332},
    {0.050020, 0.732234, 0.357577, -0.077570, 0.574417},
    {0.000569, 0.731447, 0.361945, -0.082112, 0.572047},
    {0.000569, 0.731039, 0.363079, -0.083163, 0.571700},
    {0.048936, 0.735483, 0.342026, -0.067328, 0.580990},
    {0.048936, 0.737302, 0.328064, -0.057375, 0.587765},
    {0.000490, 0.735058, 0.335251, -0.063817, 0.585853},
    {0.000490, 0.734533, 0.335891, -0.064206, 0.586102},
    {0.049518, 0.737661, 0.305562, -0.038123, 0.600862},
    {0.049518, 0.740145, 0.288425, -0.026480, 0.606874},
    {0.000489, 0.739116, 0.295757, -0.033524, 0.604244},
    {0.000489, 0.738500, 0.298968, -0.036566, 0.603240},
    {0.049314, 0.740149, 0.278546, -0.022998, 0.611607},
    {0.049314, 0.740413, 0.264345, -0.015356, 0.617797},
    {0.000702, 0.739409, 0.269998, -0.020628, 0.616401},
    {0.000702, 0.738227, 0.275309, -0.025512, 0.615285},
    {0.049296, 0.738351, 0.248515, -0.002918, 0.626952},
    {0.049296, 0.737698, 0.236727, 0.004925, 0.632248},
    {0.000704, 0.737607, 0.239894, 0.001828, 0.631176},
    {0.000704, 0.738416, 0.242550, -0.001123, 0.629214},
    {0.049307, 0.738768, 0.226118, 0.011234, 0.634796},
    {0.049307, 0.738991, 0.214297, 0.018977, 0.638443},
    {0.000701, 0.739007, 0.216912, 0.016427, 0.637611},
    {0.000701, 0.738454, 0.219163, 0.014404, 0.637531},
    {0.049310, 0.738703, 0.208779, 0.020036, 0.640569},
    {0.049310, 0.738238, 0.201573, 0.023257, 0.643298},
    {0.000698, 0.738378, 0.204459, 0.020417, 0.642321},
    {0.000698, 0.738559, 0.205722, 0.019105, 0.641751},
    {0.049316, 0.738302, 0.199081, 0.021033, 0.644076},
    {0.049316, 0.738361, 0.195359, 0.020231, 0.645173},
    {0.000692, 0.738540, 0.194021, 0.021392, 0.645335},
    {0.000692, 0.738385, 0.194293, 0.021123, 0.645439},
    {0.049301, 0.738345, 0.188460, 0.021845, 0.647188},
    {0.049301, 0.738760, 0.182384, 0.023047, 0.648412},
    {0.000702, 0.739115, 0.184617, 0.020790, 0.647451},
    {0.000702, 0.739268, 0.186039, 0.019343, 0.646914},
    {0.054306, 0.739417, 0.159578, 0.035693, 0.653087},
    {0.054306, 0.738679, 0.133847, 0.051311, 0.658639},
    {0.000703, 0.739467, 0.144611, 0.041207, 0.656184},
    {0.000703, 0.738437, 0.152990, 0.033528, 0.655882},
    {0.045018, 0.739368, 0.130770, 0.049867, 0.658595},
    {0.045018, 0.740035, 0.118153, 0.057352, 0.659621},
    {0.000490, 0.740616, 0.122492, 0.053313, 0.658515},
    {0.000490, 0.740542, 0.124684, 0.051292, 0.658347},
    {0.048804, 0.738569, 0.105788, 0.063325, 0.662808},
    {0.048804, 0.737924, 0.094445, 0.068484, 0.664724},
    {0.000703, 0.738189, 0.099709, 0.063642, 0.664143},
    {0.000703, 0.738515, 0.100424, 0.062926, 0.663740},
    {0.049304, 0.738613, 0.091650, 0.067300, 0.664471},
    {0.049304, 0.739321, 0.085371, 0.069399, 0.664304},
    {0.000706, 0.739540, 0.087485, 0.067419, 0.663989},
    {0.000706, 0.739827, 0.087655, 0.067192, 0.663670},
    {0.051304, 0.738968, 0.078873, 0.070893, 0.665342},
    {0.051304, 0.738596, 0.068900, 0.075662, 0.666337},
    {0.000486, 0.738673, 0.071127, 0.073615, 0.666247},
    {0.000486, 0.739217, 0.073642, 0.071288, 0.665622},
    {0.049520, 0.738301, 0.057270, 0.079811, 0.667280},
    {0.049520, 0.736954, 0.045172, 0.084429, 0.669126},
    {0.000487, 0.737463, 0.052419, 0.077836, 0.668837},
    {0.000487, 0.737782, 0.055955, 0.074594, 0.668568},
    {0.047520, 0.737444, 0.034472, 0.088096, 0.668750},
    {0.047520, 0.736812, 0.021515, 0.094534, 0.669110},
    {0.000702, 0.738079, 0.028594, 0.088157, 0.668319},
    {0.000702, 0.737086, 0.030659, 0.086134, 0.669585},
    {0.049304, 0.735826, 0.019964, 0.092115, 0.670579},
    {0.049304, 0.735293, 0.012493, 0.095464, 0.670876},
    {0.000724, 0.736359, 0.013782, 0.094344, 0.669839},
    {0.000724, 0.737008, 0.014732, 0.093483, 0.669226},
    {0.049284, 0.735419, 0.000905, 0.101424, 0.669978},
    {0.049284, 0.734191, -0.010026, 0.106628, 0.670443},
    {0.000705, 0.735168, -0.008372, 0.105213, 0.669618},
    {0.000705, 0.735639, -0.008561, 0.105401, 0.669068},
    {0.050516, 0.733141, -0.032457, 0.120676, 0.668497},
    {0.050516, 0.730339, -0.049101, 0.129979, 0.668805},
    {0.000563, 0.732876, -0.041558, 0.123531, 0.667761},
    {0.000563, 0.734156, -0.038391, 0.120845, 0.667037},
    {0.048235, 0.729325, -0.058338, 0.134958, 0.668183},
    {0.048235, 0.727013, -0.069191, 0.141254, 0.668365},
    {0.000698, 0.729084, -0.064886, 0.137769, 0.667267},
    {0.000698, 0.731348, -0.062222, 0.135872, 0.665431},
    {0.049301, 0.727736, -0.076956, 0.145486, 0.665817},
    {0.049301, 0.726296, -0.085011, 0.149350, 0.665554},
    {0.000705, 0.727244, -0.084199, 0.148835, 0.664737},
    {0.000705, 0.727474, -0.084119, 0.148771, 0.664509},
    {0.049302, 0.715414, -0.124332, 0.176726, 0.664449},
    {0.049302, 0.712305, -0.132170, 0.176160, 0.666424},
    {0.000501, 0.715151, -0.122722, 0.167802, 0.667339},
    {0.000501, 0.715921, -0.117232, 0.162551, 0.668798},
    {0.053222, 0.721075, -0.122016, 0.169947, 0.660516},
    {0.053222, 0.722452, -0.123443, 0.172547, 0.658067},
    {0.000564, 0.724924, -0.129160, 0.179310, 0.652420},
    {0.000564, 0.722517, -0.135797, 0.184721, 0.652232},
    {0.045729, 0.719417, -0.140450, 0.186568, 0.654145},
    {0.045729, 0.719639, -0.139184, 0.183493, 0.655040},
    {0.000487, 0.719889, -0.136443, 0.180862, 0.656073},
    {0.000487, 0.721005, -0.135891, 0.180857, 0.654963},
    {0.049521, 0.718683, -0.143097, 0.183895, 0.655134},
    {0.049521, 0.715596, -0.151492, 0.188036, 0.655450},
    {0.000488, 0.717121, -0.150853, 0.188153, 0.653895},
    {0.000488, 0.717109, -0.152637, 0.189945, 0.652975},
    {0.049518, 0.713510, -0.159719, 0.195249, 0.653659},
    {0.049518, 0.711883, -0.165824, 0.200963, 0.652180},
    {0.000489, 0.714911, -0.160183, 0.196802, 0.651546},
    {0.000489, 0.716962, -0.154717, 0.192292, 0.651960},
    {0.052008, 0.713326, -0.160113, 0.195246, 0.653765},
    {0.052008, 0.709543, -0.167324, 0.198280, 0.655162},
    {0.000499, 0.710392, -0.166360, 0.197749, 0.654647},
    {0.000499, 0.711317, -0.165147, 0.196998, 0.654176},
};
using State = flexkalman::pose_externalized_rotation::State;
// using Measurement = videotracker::uvbi::OrientationMeasurement;
using Measurement = flexkalman::AbsoluteOrientationMeasurement;

TEST_CASE("ConstantVel") {
    Catch::StringMaker<float>::precision = 15;
    Catch::StringMaker<double>::precision = 35;
    using ProcessModel = flexkalman::PoseConstantVelocityProcessModel;
    State state;
    ProcessModel processModel;

    state.setQuaternion(
        Eigen::Quaterniond(0.735083, -0.424120, 0.000000, -0.528938));
    size_t iteration{0};
    dumpState(state, "Initial state", iteration);
    for (auto &vals : DATA) {
        double dt = vals[0];
        INFO("Iteration " << iteration);
        {
            INFO("prediction step");
            flexkalman::predict(state, processModel, dt);
            dumpState(state, "After prediction", iteration);
            REQUIRE_FALSE(stateContentsInvalid(state));
            REQUIRE_FALSE(covarianceContentsInvalid(state));
        }
        {
            INFO("correction step");
            Eigen::Quaterniond q{vals[1], vals[2], vals[3], vals[4]};
            CAPTURE(q);
            CAPTURE(q.normalized());
            q.normalize();
            auto meas = Measurement{q, Eigen::Vector3d::Constant(0.00001)};
            REQUIRE_FALSE(covarianceContentsInvalid(meas.getCovariance(state)));
            auto correct = flexkalman::beginUnscentedCorrection(state, meas);
            REQUIRE(correct.deltaz.norm() < (EIGEN_PI * 2));
            REQUIRE(correct.stateCorrectionFinite);
            REQUIRE(correct.stateCorrection.head<6>().norm() < (EIGEN_PI * 2));
            std::cout << correct.stateCorrection.transpose() << std::endl;
            REQUIRE(correct.finishCorrection());
            dumpState(state, "After correction", iteration);
            auto diff = Eigen::AngleAxisd(state.getQuaternion() * q.inverse());
            REQUIRE(diff.angle() < EIGEN_PI);
        }
        iteration++;
    }
}

TEST_CASE("ArtificialData") {
    Catch::StringMaker<float>::precision = 15;
    Catch::StringMaker<double>::precision = 35;
    using ProcessModel = flexkalman::PoseConstantVelocityProcessModel;
    State state;
    ProcessModel processModel;

    size_t iteration{0};
    dumpState(state, "Initial state", iteration);
    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    Measurement meas = {q, Eigen::Vector3d::Constant(0.00001)};
    while (iteration < 20) {
        INFO("Iteration " << iteration);
        {
            INFO("prediction step");
            flexkalman::predict(state, processModel, 0.01);
            dumpState(state, "After prediction", iteration);
            REQUIRE_FALSE(stateContentsInvalid(state));
            REQUIRE_FALSE(covarianceContentsInvalid(state));
        }
        {
            INFO("correction step");
            q = Eigen::AngleAxisd(0.1 * iteration, Eigen::Vector3d::UnitZ());
            CAPTURE(q);
            CAPTURE(q.normalized());
            q.normalize();
            meas.setMeasurement(q);
            REQUIRE_FALSE(covarianceContentsInvalid(meas.getCovariance(state)));
            auto correct = flexkalman::beginUnscentedCorrection(state, meas);
            REQUIRE(correct.deltaz.norm() < (EIGEN_PI * 2));
            REQUIRE(correct.stateCorrectionFinite);
            REQUIRE(correct.stateCorrection.head<6>().norm() < (EIGEN_PI * 2));
            std::cout << correct.stateCorrection.transpose() << std::endl;
            REQUIRE(correct.finishCorrection());
            dumpState(state, "After correction", iteration);
            auto ori = Eigen::AngleAxisd(state.getQuaternion());
            INFO("Angle " << ori.angle() << " Axis " << ori.axis().transpose());
            auto diff = Eigen::AngleAxisd(state.getQuaternion() * q.inverse());
            REQUIRE(diff.angle() < EIGEN_PI);
        }
        iteration++;
    }
    double lastDeltaZ = 0;
    double lastDiffAngle = 0;
    // Keep going with the same measurement, should converge.
    while (iteration < 40) {
        INFO("Iteration " << iteration << " with constant measurement");
        {
            INFO("prediction step");
            flexkalman::predict(state, processModel, 0.01);
            // dumpState(state, "After prediction", iteration);
            REQUIRE_FALSE(stateContentsInvalid(state));
            REQUIRE_FALSE(covarianceContentsInvalid(state));
        }
        {
            INFO("correction step");
            auto correct = flexkalman::beginUnscentedCorrection(state, meas);
            // REQUIRE(correct.deltaz.norm() < (EIGEN_PI * 2));
            CAPTURE(correct.deltaz.transpose());
            if (iteration > 20) {
                REQUIRE(correct.deltaz.norm() < lastDeltaZ);
            }
            lastDeltaZ = correct.deltaz.norm();
            REQUIRE(correct.stateCorrectionFinite);
            CAPTURE(correct.stateCorrection.transpose());
            REQUIRE(correct.finishCorrection());
            // dumpState(state, "After correction", iteration);
            auto diff = Eigen::AngleAxisd(state.getQuaternion() * q.inverse());
            // REQUIRE(diff.angle() < EIGEN_PI);
            if (iteration > 20) {
                REQUIRE(diff.angle() < lastDiffAngle);
            }
            lastDiffAngle = diff.angle();
        }
        iteration++;
    }
}
