// Copyright (c) 2023 vesoft inc. All rights reserved.

#include <folly/RWSpinLock.h>
#include <folly/concurrency/ConcurrentHashMap.h>

#include "nebula/common/datatype/Edge.h"
#include "nebula/common/datatype/List.h"
#include "nebula/common/graph/MemGraph.h"
#include "nebula/common/module/Module.h"
#include "nebula/common/module/ModuleManager.h"
#include "nebula/common/table/RefCatalog.h"
#include "nebula/common/utils/Types.h"
#include "nebula/common/valuetype/ValueType.h"
#include "nebula/computing/ComputingAlgorithm.h"
#include "nebula/computing/ComputingContext.h"
#include "nebula/computing/VertexSubset.h"
#include "nebula/plugins/ProcedurePlugin.h"

using nebula::Edge;
using nebula::ExecutionOutcome;
using nebula::List;
using nebula::MemGraph;
using nebula::Node;
using nebula::NodeID;
using nebula::NullValue;
using nebula::ResultTable;
using nebula::Row;
using nebula::Status;
using nebula::String;
using nebula::Value;
using nebula::ValueTypeKind;
using nebula::computing::ComputingContext;
using nebula::computing::VertexSubset;
using nebula::module::ModuleManager;
using nebula::plugin::Field;
using nebula::plugin::Parameter;
using nebula::plugin::ProcContext;
using nebula::plugin::ProcContextPtr;
using nebula::plugin::Procedure;

namespace yj {

struct NetworkTopoState {
    int32_t breakerPoint{0};
    int32_t disPoint{0};
    int64_t maxTopoID{-1};
    int64_t maxBDID{-1};
    int sumIID{0};
    int sumJID{0};
    double sumQimeas{0.0};
    double sumBusQMeas{0.0};
    double sumAclineCount{0.0};
    int64_t sumLineNo{0};
    std::set<int64_t> setTopoID1;
    std::list<std::string> listTpndName;
    std::list<double> listTpndQMeas;

    // Need to be updated fields
    std::optional<int64_t> topoID{-1};
    std::optional<int64_t> point{-1};
    std::optional<int64_t> itopoID;
    std::optional<int64_t> jtopoID;

    void getResult(Row &row) const {
        row.append(topoID.has_value() ? Value(topoID.value()) : NullValue::kNullValue);
        row.append(point.has_value() ? Value(point.value()) : NullValue::kNullValue);
        row.append(itopoID.has_value() ? Value(itopoID.value()) : NullValue::kNullValue);
        row.append(jtopoID.has_value() ? Value(jtopoID.value()) : NullValue::kNullValue);
    }
};

class NetworkTopoAlgorithm : public nebula::computing::ComputingAlgorithm<NetworkTopoState> {
public:
    using Super = nebula::computing::ComputingAlgorithm<NetworkTopoState>;

    explicit NetworkTopoAlgorithm(ComputingContext *ctx) : Super(ctx) {}

    static void writeDouble(double *ptr, double val) {
        Super::write<int64_t>(reinterpret_cast<int64_t *>(ptr), val);
    }

    std::string getAclineName(const std::string &name) const {
        return name;
    }

    void run() override {
        auto *graph = this->graph();

        VertexSubset all(ctx_, graph->nodeIDs());

        VertexSubset A11 = verticesByAllLabels(all, {"Substation"});
        VertexSubset tNeutralPoint = verticesByAllLabels(all, {"neutral_point"});
        VertexSubset discreteSet = verticesByAllLabels(all, {"discrete"});

        VertexSubset disSetByFlag = discreteSet.filter([graph](NodeID s) -> bool {
            auto cond = [](const auto &v) -> bool { return v.getInt64() == 1; };
            return graph->hasProperty(s, "updateFlag", cond);
        });

        VertexSubset breakerSet = disSetByFlag.map([this, graph](NodeID s) {
            auto flagM = graph->getProperty(s, "flagM");
            auto statusM = graph->getProperty(s, "statusM");
            auto status = graph->getProperty(s, "status");
            int32_t delta = flagM.getInt64() == 1 ? statusM.getInt64() : status.getInt64();

            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (l == "discrete_breaker") {
                        return true;
                    }
                }
                return false;
            });
            for (auto t : tgts) {
                write<int32_t>(&state(t).breakerPoint, delta);
            }
            return tgts;
        });

        breakerSet.forEach([this, graph](NodeID t) {
            auto name = graph->getProperty(t, "name").getString();
            int64_t point = -1;
            static auto names = std::unordered_set<String>{
                    "四川.桃坪厂/13.8kV.2开关",
                    "四川.桃坪厂/13.8kV.3开关",
                    "阿坝.岷江电化站/110kV.151开关",
                    "阿坝.岷江电化站/10kV.1#主变低压侧901开关",
            };
            if (names.count(name)) {
                point = 1;
            } else if (name == "广元.太公电铁站/110kV.102开关") {
                point = 0;
            } else {
                point = state(t).breakerPoint;
            }
            // TODO(yee): in-place update in the memory graph
            state(t).point = point;
        });

        VertexSubset disSet = disSetByFlag.map([this, graph](NodeID s) {
            auto flagM = graph->getProperty(s, "flagM");
            auto statusM = graph->getProperty(s, "statusM");
            auto status = graph->getProperty(s, "status");
            int32_t delta = flagM.getInt64() == 1 ? statusM.getInt64() : status.getInt64();

            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (l == "discrete_dis") {
                        return true;
                    }
                }
                return false;
            });
            for (auto t : tgts) {
                write<int32_t>(&state(t).disPoint, delta);
            }
            return tgts;
        });

        disSet.forEach([this, graph](NodeID s) {
            auto name = graph->getProperty(s, "name").getString();
            int64_t point = -1;
            static auto names = std::unordered_set<String>{
                    "四川.瀑布沟厂/500kV.50126刀闸",
                    "四川.瀑布沟厂/500kV.50526刀闸",
                    "四川.红房子厂/220kV.2516刀闸",
                    "四川.红房子厂/220kV.2516刀闸",
            };
            if (names.count(name)) {
                point = 1;
            } else if (name == "广元.太公电铁站/110kV.1021刀闸") {
                point = 0;
            } else {
                point = state(s).disPoint;
            }
            // TODO(yee): in-place update in the memory graph
            state(s).point = point;
        });

        std::set<std::string> connectedSubLabels = {
                "connected_Sub_Bus",
                "connected_Sub_Load",
                "connected_Sub_Unit",
                "connected_Sub_Trans_two",
                "connected_Sub_Aclinedot",
                "connected_Sub_Trans_three",
                "connected_Sub_Compensator_P",
        };
        VertexSubset selectSub = A11.map([this, graph, &connectedSubLabels](NodeID s) {
            std::unordered_set<NodeID> tgts;
            auto f = [this, &connectedSubLabels](const Edge &e) -> bool {
                auto labels = getEdgeLabelSet(e.getEdgeID());
                for (auto &l : connectedSubLabels) {
                    for (auto &ll : labels) {
                        if (l == ll) {
                            return true;
                        }
                    }
                }
                return false;
            };
            for (auto t : graph->outNeighborIDs(s, f)) {
                auto off = graph->getProperty(t, "off");
                if (off.isInt64() && off.getInt64() == 0) {
                    tgts.emplace(t);
                }
            }
            return std::vector<NodeID>{tgts.begin(), tgts.end()};
        });
        VertexSubset cnOpenSub = selectSub.map([this, graph](NodeID s) {
            return graph->neighborIDs(s, [this](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (l == "cn_subid") {
                        return true;
                    }
                }
                return false;
            });
        });
        VertexSubset miscellaneous = selectSub.map([this, graph, connectedSubLabels](NodeID s) {
            std::unordered_set<NodeID> tgts;
            std::set<std::string> labels(connectedSubLabels.begin(), connectedSubLabels.end());
            labels.emplace("connected_Sub_Compensator_S");
            auto f = [this, &labels](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (labels.count(l)) {
                        return true;
                    }
                }
                return false;
            };
            for (auto t : graph->outNeighborIDs(s, f)) {
                auto off = graph->getProperty(t, "off");
                if (off.isInt64() && off.getInt64() == 0) {
                    tgts.emplace(t);
                }
            }
            return std::vector<NodeID>{tgts.begin(), tgts.end()};
        });
        std::set<std::string> cnLabels = {
                "connected_Bus_CN",
                "connected_Load_CN",
                "connected_Unit_CN",
                "CN_tx_two",
                "aclinedot_cn",
                "CN_tx_three",
                "connected_Compensator_P_CN",
                "connected_Compensator_S_CN",
        };
        VertexSubset cnTotal = miscellaneous.map([this, graph, &cnLabels](NodeID s) {
            auto nd = graph->getProperty(s, "nd").getInt64();
            auto tgts = graph->neighborIDs(s, [this, &cnLabels](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (cnLabels.count(l)) {
                        return true;
                    }
                }
                return false;
            });
            for (auto t : tgts) {
                writeMax<int64_t>(&state(t).maxTopoID, nd);
                writeMax<int64_t>(&state(s).maxTopoID, nd);
            }
            return tgts;
        });

        std::atomic_int sumCounter = 0;
        int sumOldCounter = 1;
        VertexSubset cnSet = cnOpenSub;
        std::set<std::string> bdLabels = {"connected_Breaker_CN", "connected_Disconnector_CN"};
        while (sumOldCounter != sumCounter) {
            sumOldCounter = sumCounter;
            VertexSubset bdSet = cnSet.map([this, graph, &bdLabels](NodeID s) {
                auto tgts = graph->neighborIDs(s, [this, &bdLabels](const Edge &e) -> bool {
                    for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                        if (bdLabels.count(l)) {
                            return true;
                        }
                    }
                    return false;
                });
                for (auto t : tgts) {
                    writeMax<int64_t>(&state(t).maxBDID, state(s).maxTopoID);
                }
                return tgts;
            });

            cnSet = bdSet.map([this, graph, &bdLabels, &sumCounter](NodeID s) {
                auto sPoint = graph->getProperty(s, "point").getInt64();
                if (sPoint != 1) return std::vector<NodeID>{};
                auto tgts = graph->neighborIDs(s, [this, &bdLabels](const Edge &e) -> bool {
                    for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                        if (bdLabels.count(l)) {
                            return true;
                        }
                    }
                    return false;
                });
                for (auto t : tgts) {
                    if (state(s).maxBDID > state(t).maxTopoID) {
                        sumCounter++;
                        // TODO(yee): data race
                        auto newTopoId = state(t).maxTopoID + state(s).maxBDID;
                        writeMax<int64_t>(&state(t).maxTopoID, newTopoId);
                    }
                }
                return tgts;
            });
        }
        std::set<std::string> buildLabels = {
                "connected_Bus_CN",
                "connected_Load_CN",
                "CN_tx_two",
                "aclinedot_cn",
                "CN_tx_three",
                "connected_Compensator_P_CN",
        };
        VertexSubset buildTP = cnTotal.map([this, graph, &buildLabels](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this, &buildLabels](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (buildLabels.count(l)) {
                        return true;
                    }
                }
                return false;
            });
            for (auto t : tgts) {
                if (state(t).maxTopoID == state(s).maxTopoID) {
                    insertNode("TopoND",
                               {
                                       {"topoid", state(s).maxTopoID},
                                       {"TOPOID", state(s).maxTopoID},
                                       {"bus_name", graph->getProperty(t, "name")},
                                       {"base_kV", graph->getProperty(t, "volt")},
                                       {"desired_volts", graph->getProperty(t, "base_value")},
                                       {"up_V", 1.1},
                                       {"lo_V", 0.9},
                                       {"Ri_vP", 0},
                                       {"Ri_vQ", 0},
                                       {"start_pt", 0.0},
                                       {"nodeSI", 0.0},
                                       {"nodeCSeverity", 1},
                                       {"ZJP", 1},
                                       {"ZJQ", 0},
                               });
                } else {
                    write<int64_t>(&state(t).maxTopoID, state(s).maxTopoID);
                }
            }
            return tgts;
        });
        // post-accum
        buildTP.forEach([this](NodeID t) { state(t).topoID = state(t).maxTopoID; });

        VertexSubset buildTPUnit = cnTotal.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (l == "connected_Unit_CN") {
                        return true;
                    }
                }
                return false;
            });
            std::vector<Node> nodes;
            for (auto t : tgts) {
                if (state(t).maxTopoID == state(s).maxTopoID) {
                    insertNode("TopoND",
                               {
                                       {"topoid", state(s).maxTopoID},
                                       {"TOPOID", state(s).maxTopoID},
                                       {"bus_name", graph->getProperty(t, "name")},
                                       {"base_kV", graph->getProperty(t, "volt")},
                                       {"desired_volts", graph->getProperty(t, "base_value")},
                                       {"up_V", 1.1},
                                       {"lo_V", 0.9},
                                       {"qUp", graph->getProperty(t, "Q_max") / 100},
                                       {"qLower", graph->getProperty(t, "Q_min") / 100},
                                       {"Ri_vP", 0},
                                       {"Ri_vQ", 0},
                                       {"start_pt", 0.0},
                                       {"nodeSI", 0.0},
                                       {"nodeCSeverity", 1},
                                       {"ZJP", 1},
                                       {"ZJQ", 0},
                               });
                } else {
                    write<int64_t>(&state(t).maxTopoID, state(s).maxTopoID);
                }
            }
            return tgts;
        });
        buildTPUnit.forEach([this](NodeID t) { state(t).topoID = state(t).maxTopoID; });

        VertexSubset totalTopoNodes = tNeutralPoint.filter([graph](NodeID t) {
            auto iOff = graph->getProperty(t, "I_off").getInt64();
            auto kOff = graph->getProperty(t, "K_off").getInt64();
            auto jOff = graph->getProperty(t, "J_off").getInt64();
            return iOff + kOff + jOff <= 1;
        });
        totalTopoNodes.forEach([this, graph](NodeID t) {
            auto midPoint = graph->getProperty(t, "middle_point").getInt64();
            insertNode("TopoND",
                       {
                               {"topoid", midPoint},
                               {"TOPOID", midPoint},
                               {"bus_name", graph->getProperty(t, "name")},
                               {"base_kV", 1},
                               {"desired_volts", 1},
                               {"up_V", 1.1},
                               {"lo_V", 0.9},
                               {"Ri_vP", 0},
                               {"Ri_vQ", 0},
                               {"start_pt", 0.0},
                               {"nodeSI", 0.0},
                               {"nodeCSeverity", 1},
                               {"ZJP", 1},
                               {"ZJQ", 0},
                       });
            insertEdge("topo_neutral", {midPoint}, {midPoint});
        });

        VertexSubset topoSub =
                cnTotal.filter([this, graph](NodeID s) {
                           auto cnID = graph->getProperty(s, "CN_id").getInt64();
                           return state(s).maxTopoID != 0 && state(s).maxTopoID != cnID;
                       })
                        .map([this, graph](NodeID s) {
                            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                                const auto &ls = getEdgeLabelSet(e.getEdgeID());
                                return ls.count("cn_subid");
                            });
                            for (auto t : tgts) {
                                auto tid = graph->getProperty(t, "id").getInt64();
                                insertEdge("topoid_subid", {state(s).maxTopoID}, {tid});
                            }
                            return tgts;
                        });

        VertexSubset checkNode3 = verticesByAllLabels(all, {"TopoND"});
        std::set<std::string> componentLabels = {
                "connected_Unit_CN",
                "connected_Load_CN",
                "connected_Bus_CN",
                "connected_Compensator_P_CN",
                "CN_tx_two",
                "CN_tx_three",
                "aclinedot_cn",
        };
        VertexSubset topoComponentNode =
                cnTotal.filter([this](NodeID s) { return state(s).maxTopoID != 0; })
                        .map([this, graph, &componentLabels](NodeID s) {
                            auto f = [this, &componentLabels](const Edge &e) -> bool {
                                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                                    if (componentLabels.count(l)) {
                                        return true;
                                    }
                                }
                                return false;
                            };
                            auto tgts = graph->neighborIDs(s, f);
                            for (auto t : tgts) {
                                auto tname = graph->getProperty(t, "typename").getString();
                                auto tid = graph->getProperty(t, "id").getInt64();
                                auto sid = state(s).maxTopoID;
                                if (tname == "Unit") {
                                    insertEdge("topo_unit", {sid}, {tid});
                                } else if (tname == "Load") {
                                    insertEdge("topo_load", {sid}, {tid});
                                } else if (tname == "Bus") {
                                    insertEdge("topo_bus", {sid}, {tid});
                                } else if (tname == "Compensator_P") {
                                    insertEdge("topo_compensatorP", {sid}, {tid});
                                } else if (tname == "two_port_transformer") {
                                    insertEdge("topo_Tx_Two", {sid}, {tid});
                                } else if (tname == "three_port_transformer") {
                                    insertEdge("topo_Tx_Three", {sid}, {tid});
                                } else if (tname == "AClinedot") {
                                    insertEdge("topo_aclinedot", {sid}, {tid});
                                }
                            }
                            return tgts;
                        });

        VertexSubset csOpenSub = cnOpenSub.map([this, graph](NodeID s) {
            auto cnID = graph->getProperty(s, "CN_id").getInt64();
            std::unordered_set<NodeID> res;
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Compensator_S_CN");
            });
            res.reserve(tgts.size());
            for (auto t : tgts) {
                auto off = graph->getProperty(t, "off").getInt64();
                if (off == 0) {
                    res.emplace(t);
                    auto iND = graph->getProperty(t, "I_nd").getInt64();
                    auto jND = graph->getProperty(t, "J_nd").getInt64();
                    if (iND == cnID) {
                        write<int>(&state(t).sumIID, state(s).maxTopoID);
                    } else if (jND == cnID) {
                        write<int>(&state(t).sumJID, state(s).maxTopoID);
                    }
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });

        VertexSubset insertLineCS =
                csOpenSub
                        .filter([this](NodeID s) {
                            auto iid = state(s).sumIID, jid = state(s).sumJID;
                            return iid != jid && iid != 0 && jid != 0;
                        })
                        .forEach([this, graph](NodeID s) {
                            auto iid = state(s).sumIID;
                            auto jid = state(s).sumJID;
                            auto sname = graph->getProperty(s, "name").getString();
                            auto csZK = graph->getProperty(s, "cs_ZK").getDouble();
                            auto volt =
                                    std::to_string(graph->getProperty(s, "volt").getDouble());
                            insertEdge("topo_connect",
                                       {iid},
                                       {jid},
                                       {
                                               {"edge_name", List({sname})},
                                               {"area", List({sname})},
                                               {"tap_bus", iid},
                                               {"z_bus", jid},
                                               {"R", List({0})},
                                               {"X", List({csZK})},
                                               {"line_Q1_list", List({200})},
                                               {"line_Q2_list", List({250})},
                                               {"line_Q3_list", List({300})},
                                               {"B_list", List({1 / csZK})},
                                               {"BIJ_list", List({1 / csZK})},
                                               {"reverse", 1},
                                               {"Decision", List({0})},
                                               {"Analysis_decision", List({0})},
                                               {"volt", volt},
                                               {"device_type", "CS传输线"},
                                               {"CSeverity", List({-1})},
                                               {"B", 1 / csZK},
                                               {"BIJ", 1 / csZK},
                                               {"line_Q1", 200},
                                               {"line_Q2", 250},
                                               {"line_Q3", 300},
                                               {"from_off", List({0})},
                                               {"to_off", List({0})},
                                       });
                            insertEdge("topo_connect",
                                       {jid},
                                       {iid},
                                       {
                                               {"edge_name", List({sname})},
                                               {"area", List({sname})},
                                               {"tap_bus", iid},
                                               {"z_bus", jid},
                                               {"R", List({0})},
                                               {"X", List({csZK})},
                                               {"line_Q1_list", List({200})},
                                               {"line_Q2_list", List({250})},
                                               {"line_Q3_list", List({300})},
                                               {"B_list", List({1 / csZK})},
                                               {"BIJ_list", List({1 / csZK})},
                                               {"reverse", -1},
                                               {"Decision", List({0})},
                                               {"Analysis_decision", List({0})},
                                               {"volt", volt},
                                               {"device_type", "CS传输线"},
                                               {"CSeverity", List({-1})},
                                               {"B", 1 / csZK},
                                               {"BIJ", 1 / csZK},
                                               {"line_Q1", 200},
                                               {"line_Q2", 250},
                                               {"line_Q3", 300},
                                               {"from_off", List({0})},
                                               {"to_off", List({0})},
                                       });
                        })
                        .forEach([this](NodeID s) {
                            state(s).itopoID = state(s).sumIID;
                            state(s).jtopoID = state(s).sumJID;
                        });
        VertexSubset aclineOpenSub =
                cnOpenSub
                        .map([this, graph](NodeID s) {
                            return graph->neighborIDs(s, [this](const Edge &e) -> bool {
                                return getEdgeLabelSet(e.getEdgeID()).count("aclinedot_cn");
                            });
                        })
                        .filter([graph](NodeID t) {
                            return graph->getProperty(t, "off").getInt64() == 0;
                        });
        VertexSubset g3 = aclineOpenSub.map([this, graph](NodeID s) {
            if (state(s).maxTopoID == 0) {
                return std::vector<NodeID>{};
            }
            auto sPimeas = graph->getProperty(s, "Pimeas").getDouble();
            auto sQimeas = graph->getProperty(s, "Qimeas").getDouble();
            bool valid = false;
            auto [b, e] = graph->outEdges(s);
            for (; b != e; ++b) {
                auto t = b.getDstID();
                if (state(t).maxTopoID != 0) {
                    valid = true;

                    auto id = graph->getProperty(*b, "id").getInt64();
                    auto ename = graph->getProperty(*b, "name").getString();
                    auto lineR = graph->getProperty(*b, "line_R").getDouble();
                    auto lineX = graph->getProperty(*b, "line_X").getDouble();
                    auto lineB = graph->getProperty(*b, "line_B").getDouble();
                    auto volt = graph->getProperty(*b, "volt").getDouble();
                    auto ih = graph->getProperty(*b, "Ih").getDouble();
                    // auto ratio = lineR / lineX;
                    auto normalLimit = sqrt(3) * volt * ih / 1000 / 100;
                    auto emerLimit = 1.1 * std::sqrt(3) * volt * ih / 1000 / 100;
                    auto ldshdLimit = 1.2 * std::sqrt(3) * volt * ih / 1000 / 100;
                    int non_reverse = 1;
                    if (normalLimit < 0.1) {
                        normalLimit = 9999;
                        emerLimit = 9999;
                        ldshdLimit = 9999;
                    }
                    if (state(s).maxTopoID < state(t).maxTopoID) {
                        non_reverse = -1;
                    }

                    auto tPimeas = graph->getProperty(t, "Pimeas").getDouble();
                    auto tQimeas = graph->getProperty(t, "Qimeas").getDouble();

                    if (lineR < 0) {
                        insertEdge("topo_connect",
                                   {state(s).maxTopoID},
                                   {state(t).maxTopoID},
                                   {
                                           {"edge_name", ename},
                                           {"area", ename},
                                           {"tap_bus", state(s).maxTopoID},
                                           {"z_bus", state(t).maxTopoID},
                                           {"flag", 0},
                                           {"R", List({-lineR})},
                                           {"X", List({lineX})},
                                           {"hB_list", List({lineB})},
                                           {"line_Q1_list", List({normalLimit})},
                                           {"line_Q2_list", List({emerLimit})},
                                           {"line_Q3_list", List({ldshdLimit})},
                                           {"control_bus", 0},
                                           {"side", 0},
                                           {"min_tap", 0},
                                           {"max_tap", 0},
                                           {"step_size", 0},
                                           {"M_P_TLPF_list", List({sPimeas / 100})},
                                           {"M_Q_TLPF_list", List({sQimeas / 100})},
                                           {"outage", 0},
                                           {"G_list",
                                            List({-lineR / (lineR * lineR + lineX * lineX)})},
                                           {"B_list",
                                            List({lineX / (lineR * lineR + lineX * lineX)})},
                                           {"BIJ_list", List({1 / lineX})},
                                           {"kcount", state(s).sumAclineCount},
                                           {"reverse", non_reverse},
                                           {"Decision", List({0})},
                                           {"Analysis_decision", List({0})},
                                           {"volt", std::to_string(volt)},
                                           {"device_type", "AC传输线"},
                                           {"CSeverity", List({-1})},
                                           {"hB", lineB},
                                           {"G", -lineR / (lineR * lineR + lineX * lineX)},
                                           {"B", lineX / (lineR * lineR + lineX * lineX)},
                                           {"BIJ", 1 / lineX},
                                           {"M_P_TLPF", sPimeas / 100},
                                           {"M_Q_TLPF", sQimeas / 100},
                                           {"line_Q1", normalLimit},
                                           {"line_Q2", emerLimit},
                                           {"line_Q3", ldshdLimit},
                                           {"key_list", List({id})},
                                           {"from_off", List({0})},
                                           {"to_off", List({0})},
                                   });
                        insertEdge("topo_connect",
                                   {state(t).maxTopoID},
                                   {state(s).maxTopoID},
                                   {
                                           {"edge_name", ename},
                                           {"area", ename},
                                           {"tap_bus", state(t).maxTopoID},
                                           {"z_bus", state(s).maxTopoID},
                                           {"flag", 0},
                                           {"R", List({-lineR})},
                                           {"X", List({lineX})},
                                           {"hB_list", List({lineB})},
                                           {"line_Q1_list", List({normalLimit})},
                                           {"line_Q2_list", List({emerLimit})},
                                           {"line_Q3_list", List({ldshdLimit})},
                                           {"control_bus", 0},
                                           {"side", 0},
                                           {"min_tap", 0},
                                           {"max_tap", 0},
                                           {"step_size", 0},
                                           {"M_P_TLPF_list", List({tPimeas / 100})},
                                           {"M_Q_TLPF_list", List({tQimeas / 100})},
                                           {"outage", 0},
                                           {"G_list",
                                            List({-lineR / (lineR * lineR + lineX * lineX)})},
                                           {"B_list",
                                            List({lineX / (lineR * lineR + lineX * lineX)})},
                                           {"BIJ_list", List({1 / lineX})},
                                           {"kcount", state(s).sumAclineCount},
                                           {"reverse", -non_reverse},
                                           {"Decision", List({0})},
                                           {"Analysis_decision", List({0})},
                                           {"volt", std::to_string(volt)},
                                           {"device_type", "AC传输线"},
                                           {"CSeverity", List({-1})},
                                           {"hB", lineB},
                                           {"G", -lineR / (lineR * lineR + lineX * lineX)},
                                           {"B", lineX / (lineR * lineR + lineX * lineX)},
                                           {"BIJ", 1 / lineX},
                                           {"M_P_TLPF", tPimeas / 100},
                                           {"M_Q_TLPF", tQimeas / 100},
                                           {"line_Q1", normalLimit},
                                           {"line_Q2", emerLimit},
                                           {"line_Q3", ldshdLimit},
                                           {"key_list", List({-id})},
                                           {"from_off", List({0})},
                                           {"to_off", List({0})},
                                   });
                    } else {
                        insertEdge("topo_connect",
                                   {state(s).maxTopoID},
                                   {state(t).maxTopoID},
                                   {
                                           {"edge_name", ename},
                                           {"area", ename},
                                           {"tap_bus", state(s).maxTopoID},
                                           {"z_bus", state(t).maxTopoID},
                                           {"flag", 0},
                                           {"R", List({lineR})},
                                           {"X", List({lineX})},
                                           {"hB_list", List({lineB})},
                                           {"line_Q1_list", List({normalLimit})},
                                           {"line_Q2_list", List({emerLimit})},
                                           {"line_Q3_list", List({ldshdLimit})},
                                           {"control_bus", 0},
                                           {"side", 0},
                                           {"min_tap", 0},
                                           {"max_tap", 0},
                                           {"step_size", 0},
                                           {"M_P_TLPF_list", List({sPimeas / 100})},
                                           {"M_Q_TLPF_list", List({sQimeas / 100})},
                                           {"outage", 0},
                                           {"G_list",
                                            List({lineR / (lineR * lineR + lineX * lineX)})},
                                           {"B_list",
                                            List({lineX / (lineR * lineR + lineX * lineX)})},
                                           {"BIJ_list", List({1 / lineX})},
                                           {"kcount", state(s).sumAclineCount},
                                           {"reverse", non_reverse},
                                           {"Decision", List({0})},
                                           {"Analysis_decision", List({0})},
                                           {"volt", std::to_string(volt)},
                                           {"device_type", "AC传输线"},
                                           {"CSeverity", List({-1})},
                                           {"hB", lineB},
                                           {"G", lineR / (lineR * lineR + lineX * lineX)},
                                           {"B", lineX / (lineR * lineR + lineX * lineX)},
                                           {"BIJ", 1 / lineX},
                                           {"M_P_TLPF", sPimeas / 100},
                                           {"M_Q_TLPF", sQimeas / 100},
                                           {"line_Q1", normalLimit},
                                           {"line_Q2", emerLimit},
                                           {"line_Q3", ldshdLimit},
                                           {"key_list", List({id})},
                                           {"from_off", List({0})},
                                           {"to_off", List({0})},
                                   });
                        insertEdge("topo_connect",
                                   {state(t).maxTopoID},
                                   {state(s).maxTopoID},
                                   {
                                           {"edge_name", ename},
                                           {"area", ename},
                                           {"tap_bus", state(t).maxTopoID},
                                           {"z_bus", state(s).maxTopoID},
                                           {"flag", 0},
                                           {"R", List({lineR})},
                                           {"X", List({lineX})},
                                           {"hB_list", List({lineB})},
                                           {"line_Q1_list", List({normalLimit})},
                                           {"line_Q2_list", List({emerLimit})},
                                           {"line_Q3_list", List({ldshdLimit})},
                                           {"control_bus", 0},
                                           {"side", 0},
                                           {"min_tap", 0},
                                           {"max_tap", 0},
                                           {"step_size", 0},
                                           {"M_P_TLPF_list", List({tPimeas / 100})},
                                           {"M_Q_TLPF_list", List({tQimeas / 100})},
                                           {"outage", 0},
                                           {"G_list",
                                            List({lineR / (lineR * lineR + lineX * lineX)})},
                                           {"B_list",
                                            List({lineX / (lineR * lineR + lineX * lineX)})},
                                           {"BIJ_list", List({1 / lineX})},
                                           {"kcount", state(s).sumAclineCount},
                                           {"reverse", -non_reverse},
                                           {"Decision", List({0})},
                                           {"Analysis_decision", List({0})},
                                           {"volt", std::to_string(volt)},
                                           {"device_type", "AC传输线"},
                                           {"CSeverity", List({-1})},
                                           {"hB", lineB},
                                           {"G", lineR / (lineR * lineR + lineX * lineX)},
                                           {"B", lineX / (lineR * lineR + lineX * lineX)},
                                           {"BIJ", 1 / lineX},
                                           {"M_P_TLPF", tPimeas / 100},
                                           {"M_Q_TLPF", tQimeas / 100},
                                           {"line_Q1", normalLimit},
                                           {"line_Q2", emerLimit},
                                           {"line_Q3", ldshdLimit},
                                           {"key_list", List({-id})},
                                           {"from_off", List({0})},
                                           {"to_off", List({0})},
                                   });
                    }
                }
            }
            return valid ? std::vector<NodeID>{s} : std::vector<NodeID>{};
        });

        ///////////////////////// Insert for two_port transformer ID //////////////////////

        VertexSubset x1 =
                cnOpenSub
                        .map([this, graph](NodeID s) {
                            return graph->neighborIDs(s, [this](const Edge &e) -> bool {
                                return getEdgeLabelSet(e.getEdgeID()).count("CN_tx_two");
                            });
                        })
                        .filter([graph](NodeID t) {
                            return graph->getProperty(t, "off").getInt64() == 0;
                        });

        VertexSubset x2 = x1.map([this, graph](NodeID s) {
            if (state(s).maxTopoID == 0) {
                return std::vector<NodeID>{};
            }
            auto sid = graph->getProperty(s, "id").getInt64();
            auto sname = graph->getProperty(s, "name").getString();
            auto svolt = graph->getProperty(s, "volt").getDouble();
            auto rstar = graph->getProperty(s, "Rstar").getDouble();
            auto xstar = graph->getProperty(s, "Xstar").getDouble();
            auto itapL = graph->getProperty(s, "itapL").getDouble();
            auto itapH = graph->getProperty(s, "itapH").getDouble();
            auto itapC = graph->getProperty(s, "itapC").getDouble();
            auto sPimeas = graph->getProperty(s, "Pimeas").getDouble();
            auto sQimeas = graph->getProperty(s, "Qimeas").getDouble();
            auto st = graph->getProperty(s, "t").getDouble();
            auto ss = graph->getProperty(s, "S").getDouble();
            double ratio = rstar / xstar;
            UNUSED(ratio);
            auto tgts = graph->outNeighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("txI_txJ_transformerline");
            });
            for (auto t : tgts) {
                if (state(t).maxTopoID != 0) {
                    auto off = graph->getProperty(t, "off").getInt64();
                    if (off == 0) {
                        auto tt = graph->getProperty(t, "t").getDouble();
                        auto tvolt = graph->getProperty(t, "volt").getDouble();
                        auto tid = graph->getProperty(t, "id").getInt64();

                        auto tPimeas = graph->getProperty(t, "Pimeas").getDouble();
                        auto tQimeas = graph->getProperty(t, "Qimeas").getDouble();

                        double tapRatio = st / tt;
                        if (tapRatio < 1.0) {
                            tapRatio = 1.0;
                        }
                        double normalLimit = ss / 100;
                        double emerLimit = 1.1 * normalLimit;
                        double ldshdLimit = 1.2 * normalLimit;

                        int non_reverse = 1;
                        if (normalLimit < 0.1) {
                            normalLimit = 9999;
                            emerLimit = 9999;
                            ldshdLimit = 9999;
                        }
                        if (state(s).maxTopoID < state(t).maxTopoID) {
                            non_reverse = -1;
                        }

                        if (rstar < 0) {
                            insertEdge(
                                    "topo_connect",
                                    {state(s).maxTopoID},
                                    {state(t).maxTopoID},
                                    {
                                            {"edge_name", sname},
                                            {"area", sname},
                                            {"tap_bus", state(s).maxTopoID},
                                            {"z_bus", state(t).maxTopoID},
                                            {"flag", 1},
                                            {"R", List({-rstar})},
                                            {"X", List({xstar})},
                                            {"hB_list", List({0})},
                                            {"line_Q1_list", List({normalLimit})},
                                            {"line_Q2_list", List({emerLimit})},
                                            {"line_Q3_list", List({ldshdLimit})},
                                            {"transformer_final_turns_ratio_list",
                                             List({tapRatio})},
                                            {"transformer_final_angle", List({0})},
                                            {"min_tap", itapL},
                                            {"max_tap", itapH},
                                            {"step_size", itapC},
                                            {"M_P_TLPF_list", List({sPimeas / 100})},
                                            {"M_Q_TLPF_list", List({sQimeas / 100})},
                                            {"outage", 0},
                                            {"G_list",
                                             List({-rstar / (rstar * rstar + xstar * xstar)})},
                                            {"B_list",
                                             List({xstar / (rstar * rstar + xstar * xstar)})},
                                            {"BIJ_list", List({1 / xstar})},
                                            {"kcount", 1},
                                            {"reverse", non_reverse},
                                            {"Decision", List({0})},
                                            {"Analysis_decision", List({0})},
                                            {"volt",
                                             std::to_string(svolt) + "-" +
                                                     std::to_string(tvolt)},
                                            {"device_type", "双绕组变压器"},
                                            {"CSeverity", List({-1})},
                                            {"hB", 0},
                                            {"G", -rstar / (rstar * rstar + xstar * xstar)},
                                            {"B", xstar / (rstar * rstar + xstar * xstar)},
                                            {"BIJ", 1 / xstar},
                                            {"transformer_final_turns_ratio", tapRatio},
                                            {"M_P_TLPF", sPimeas / 100},
                                            {"M_Q_TLPF", sQimeas / 100},
                                            {"line_Q1", normalLimit},
                                            {"line_Q2", emerLimit},
                                            {"line_Q3", ldshdLimit},
                                            {"key_list", List({sid})},
                                            {"from_off", List({0})},
                                            {"to_off", List({0})},
                                    });
                            insertEdge(
                                    "topo_connect",
                                    {state(t).maxTopoID},
                                    {state(s).maxTopoID},
                                    {
                                            {"edge_name", sname},
                                            {"area", sname},
                                            {"tap_bus", state(t).maxTopoID},
                                            {"z_bus", state(s).maxTopoID},
                                            {"flag", 1},
                                            {"R", List({-rstar})},
                                            {"X", List({xstar})},
                                            {"hB_list", List({0})},
                                            {"line_Q1_list", List({normalLimit})},
                                            {"line_Q2_list", List({emerLimit})},
                                            {"line_Q3_list", List({ldshdLimit})},
                                            {"transformer_final_turns_ratio_list",
                                             List({-tapRatio})},
                                            {"transformer_final_angle", List({0})},
                                            {"min_tap", itapL},
                                            {"max_tap", itapH},
                                            {"step_size", itapC},
                                            {"M_P_TLPF_list", List({tPimeas / 100})},
                                            {"M_Q_TLPF_list", List({tQimeas / 100})},
                                            {"outage", 0},
                                            {"G_list",
                                             List({-rstar / (rstar * rstar + xstar * xstar)})},
                                            {"B_list",
                                             List({xstar / (rstar * rstar + xstar * xstar)})},
                                            {"BIJ_list", List({1 / xstar})},
                                            {"kcount", 1},
                                            {"reverse", -non_reverse},
                                            {"Decision", List({0})},
                                            {"Analysis_decision", List({0})},
                                            {"volt",
                                             std::to_string(tvolt) + "-" +
                                                     std::to_string(svolt)},
                                            {"device_type", "双绕组变压器"},
                                            {"CSeverity", List({-1})},
                                            {"hB", 0},
                                            {"G", -rstar / (rstar * rstar + xstar * xstar)},
                                            {"B", xstar / (rstar * rstar + xstar * xstar)},
                                            {"BIJ", 1 / xstar},
                                            {"transformer_final_turns_ratio", -tapRatio},
                                            {"M_P_TLPF", tPimeas / 100},
                                            {"M_Q_TLPF", tQimeas / 100},
                                            {"line_Q1", normalLimit},
                                            {"line_Q2", emerLimit},
                                            {"line_Q3", ldshdLimit},
                                            {"key_list", List({tid})},
                                            {"from_off", List({0})},
                                            {"to_off", List({0})},
                                    });
                        } else {
                            insertEdge(
                                    "topo_connect",
                                    {state(s).maxTopoID},
                                    {state(t).maxTopoID},
                                    {
                                            {"edge_name", sname},
                                            {"area", sname},
                                            {"tap_bus", state(s).maxTopoID},
                                            {"z_bus", state(t).maxTopoID},
                                            {"flag", 1},
                                            {"R", List({rstar})},
                                            {"X", List({xstar})},
                                            {"hB_list", List({0})},
                                            {"line_Q1_list", List({normalLimit})},
                                            {"line_Q2_list", List({emerLimit})},
                                            {"line_Q3_list", List({ldshdLimit})},
                                            {"transformer_final_turns_ratio_list",
                                             List({tapRatio})},
                                            {"transformer_final_angle", List({0})},
                                            {"min_tap", itapL},
                                            {"max_tap", itapH},
                                            {"step_size", itapC},
                                            {"M_P_TLPF_list", List({sPimeas / 100})},
                                            {"M_Q_TLPF_list", List({sQimeas / 100})},
                                            {"outage", 0},
                                            {"G_list",
                                             List({rstar / (rstar * rstar + xstar * xstar)})},
                                            {"B_list",
                                             List({xstar / (rstar * rstar + xstar * xstar)})},
                                            {"BIJ_list", List({1 / xstar})},
                                            {"kcount", 1},
                                            {"reverse", non_reverse},
                                            {"Decision", List({0})},
                                            {"Analysis_decision", List({0})},
                                            {"volt",
                                             std::to_string(svolt) + "-" +
                                                     std::to_string(tvolt)},
                                            {"device_type", "双绕组变压器"},
                                            {"CSeverity", List({-1})},
                                            {"hB", 0},
                                            {"G", rstar / (rstar * rstar + xstar * xstar)},
                                            {"B", xstar / (rstar * rstar + xstar * xstar)},
                                            {"BIJ", 1 / xstar},
                                            {"transformer_final_turns_ratio", tapRatio},
                                            {"M_P_TLPF", sPimeas / 100},
                                            {"M_Q_TLPF", sQimeas / 100},
                                            {"line_Q1", normalLimit},
                                            {"line_Q2", emerLimit},
                                            {"line_Q3", ldshdLimit},
                                            {"key_list", List({sid})},
                                            {"from_off", List({0})},
                                            {"to_off", List({0})},
                                    });
                            insertEdge(
                                    "topo_connect",
                                    {state(t).maxTopoID},
                                    {state(s).maxTopoID},
                                    {
                                            {"edge_name", sname},
                                            {"area", sname},
                                            {"tap_bus", state(t).maxTopoID},
                                            {"z_bus", state(s).maxTopoID},
                                            {"flag", 1},
                                            {"R", List({rstar})},
                                            {"X", List({xstar})},
                                            {"hB_list", List({0})},
                                            {"line_Q1_list", List({normalLimit})},
                                            {"line_Q2_list", List({emerLimit})},
                                            {"line_Q3_list", List({ldshdLimit})},
                                            {"transformer_final_turns_ratio_list",
                                             List({-tapRatio})},
                                            {"transformer_final_angle", List({0})},
                                            {"min_tap", itapL},
                                            {"max_tap", itapH},
                                            {"step_size", itapC},
                                            {"M_P_TLPF_list", List({tPimeas / 100})},
                                            {"M_Q_TLPF_list", List({tQimeas / 100})},
                                            {"outage", 0},
                                            {"G_list",
                                             List({rstar / (rstar * rstar + xstar * xstar)})},
                                            {"B_list",
                                             List({xstar / (rstar * rstar + xstar * xstar)})},
                                            {"BIJ_list", List({1 / xstar})},
                                            {"kcount", 1},
                                            {"reverse", -non_reverse},
                                            {"Decision", List({0})},
                                            {"Analysis_decision", List({0})},
                                            {"volt",
                                             std::to_string(tvolt) + "-" +
                                                     std::to_string(svolt)},
                                            {"device_type", "双绕组变压器"},
                                            {"CSeverity", List({-1})},
                                            {"hB", 0},
                                            {"G", rstar / (rstar * rstar + xstar * xstar)},
                                            {"B", xstar / (rstar * rstar + xstar * xstar)},
                                            {"BIJ", 1 / xstar},
                                            {"transformer_final_turns_ratio", -tapRatio},
                                            {"M_P_TLPF", tPimeas / 100},
                                            {"M_Q_TLPF", tQimeas / 100},
                                            {"line_Q1", normalLimit},
                                            {"line_Q2", emerLimit},
                                            {"line_Q3", ldshdLimit},
                                            {"key_list", List({tid})},
                                            {"from_off", List({0})},
                                            {"to_off", List({0})},
                                    });
                        }
                    }
                }
            }

            return std::vector<NodeID>{};
        });

        //////////////////////// Insert for three_port transformer ID //////////////////////

        VertexSubset y1 =
                cnOpenSub
                        .map([this, graph](NodeID s) {
                            return graph->neighborIDs(s, [this](const Edge &e) -> bool {
                                return getEdgeLabelSet(e.getEdgeID()).count("CN_tx_three");
                            });
                        })
                        .filter([graph](NodeID t) {
                            return graph->getProperty(t, "off").getInt64() == 0;
                        });
        VertexSubset y2 = y1.map([this, graph](NodeID s) {
            if (state(s).maxTopoID == 0) {
                return std::vector<NodeID>{};
            }

            auto rstar = graph->getProperty(s, "Rstar").getDouble();
            auto xstar = graph->getProperty(s, "Xstar").getDouble();
            auto st = graph->getProperty(s, "t").getDouble();
            auto ss = graph->getProperty(s, "S").getDouble();
            auto sname = graph->getProperty(s, "name").getString();
            auto itapL = graph->getProperty(s, "itapL").getDouble();
            auto itapH = graph->getProperty(s, "itapH").getDouble();
            auto itapC = graph->getProperty(s, "itapC").getDouble();
            auto sPimeas = graph->getProperty(s, "Pimeas").getDouble();
            auto sQimeas = graph->getProperty(s, "Qimeas").getDouble();
            auto svolt = graph->getProperty(s, "volt").getDouble();
            auto sid = graph->getProperty(s, "id").getInt64();

            double ratio = rstar / xstar;
            UNUSED(ratio);
            double tapRatioThree = st;
            if (tapRatioThree < 1.0) {
                tapRatioThree = 1.0;
            }
            double normalLimit = ss / 100;
            double emerLimit = 1.1 * normalLimit;
            double ldshdLimit = 1.2 * normalLimit;
            if (normalLimit < 0.1) {
                normalLimit = 9999;
                emerLimit = 9999;
                ldshdLimit = 9999;
            }

            std::unordered_set<NodeID> tgts;
            auto f = [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("neutral_three");
            };
            for (auto t : graph->neighborIDs(s, f)) {
                auto iOff = graph->getProperty(t, "I_off").getInt64();
                auto kOff = graph->getProperty(t, "K_off").getInt64();
                auto jOff = graph->getProperty(t, "J_off").getInt64();
                if (iOff + kOff + jOff <= 1) {
                    tgts.emplace(t);
                    auto tMiddlePoint = graph->getProperty(t, "middle_point").getInt64();
                    if (rstar < 0) {
                        insertEdge("topo_connect",
                                   {state(s).maxTopoID},
                                   {tMiddlePoint},
                                   {
                                           {"edge_name", sname},
                                           {"area", sname},
                                           {"tap_bus", state(s).maxTopoID},
                                           {"z_bus", tMiddlePoint},
                                           {"flag", 1},
                                           {"R", List({-rstar})},
                                           {"X", List({xstar})},
                                           {"hB_list", List({0})},
                                           {"line_Q1_list", List({normalLimit})},
                                           {"line_Q2_list", List({emerLimit})},
                                           {"line_Q3_list", List({ldshdLimit})},
                                           {"transformer_final_turns_ratio_list",
                                            List({tapRatioThree})},
                                           {"transformer_final_angle", List({0})},
                                           {"min_tap", itapL},
                                           {"max_tap", itapH},
                                           {"step_size", itapC},
                                           {"M_P_TLPF_list", List({sPimeas / 100})},
                                           {"M_Q_TLPF_list", List({sQimeas / 100})},
                                           {"outage", 0},
                                           {"G_list",
                                            List({-rstar / (rstar * rstar + xstar * xstar)})},
                                           {"B_list",
                                            List({xstar / (rstar * rstar + xstar * xstar)})},
                                           {"BIJ_list", List({1 / xstar})},
                                           {"kcount", 1},
                                           {"reverse", 1},
                                           {"Decision", List({0})},
                                           {"Analysis_decision", List({0})},
                                           {"volt", std::to_string(svolt)},
                                           {"device_type", "三绕组变压器"},
                                           {"CSeverity", List({-1})},
                                           {"hB", 0},
                                           {"G", -rstar / (rstar * rstar + xstar * xstar)},
                                           {"B", xstar / (rstar * rstar + xstar * xstar)},
                                           {"BIJ", 1 / xstar},
                                           {"transformer_final_turns_ratio", tapRatioThree},
                                           {"M_P_TLPF", sPimeas / 100},
                                           {"M_Q_TLPF", sQimeas / 100},
                                           {"line_Q1", normalLimit},
                                           {"line_Q2", emerLimit},
                                           {"line_Q3", ldshdLimit},
                                           {"key_list", List({sid})},
                                           {"from_off", List({0})},
                                           {"to_off", List({0})},
                                   });
                        insertEdge("topo_connect",
                                   {tMiddlePoint},
                                   {state(s).maxTopoID},
                                   {
                                           {"edge_name", sname},
                                           {"area", sname},
                                           {"tap_bus", tMiddlePoint},
                                           {"z_bus", state(s).maxTopoID},
                                           {"flag", 1},
                                           {"R", List({-rstar})},
                                           {"X", List({xstar})},
                                           {"hB_list", List({0})},
                                           {"line_Q1_list", List({normalLimit})},
                                           {"line_Q2_list", List({emerLimit})},
                                           {"line_Q3_list", List({ldshdLimit})},
                                           {"transformer_final_turns_ratio_list",
                                            List({-tapRatioThree})},
                                           {"transformer_final_angle", List({0})},
                                           {"min_tap", itapL},
                                           {"max_tap", itapH},
                                           {"step_size", itapC},
                                           {"outage", 0},
                                           {"G_list",
                                            List({-rstar / (rstar * rstar + xstar * xstar)})},
                                           {"B_list",
                                            List({xstar / (rstar * rstar + xstar * xstar)})},
                                           {"BIJ_list", List({1 / xstar})},
                                           {"kcount", 1},
                                           {"reverse", -1},
                                           {"Decision", List({0})},
                                           {"Analysis_decision", List({0})},
                                           {"volt", std::to_string(svolt)},
                                           {"device_type", "三绕组变压器"},
                                           {"CSeverity", List({-1})},
                                           {"hB", 0},
                                           {"G", -rstar / (rstar * rstar + xstar * xstar)},
                                           {"B", xstar / (rstar * rstar + xstar * xstar)},
                                           {"BIJ", 1 / xstar},
                                           {"transformer_final_turns_ratio", -tapRatioThree},
                                           {"line_Q1", normalLimit},
                                           {"line_Q2", emerLimit},
                                           {"line_Q3", ldshdLimit},
                                           {"key_list", List({-sid})},
                                           {"from_off", List({0})},
                                           {"to_off", List({0})},
                                   });
                    } else {
                        insertEdge("topo_connect",
                                   {state(s).maxTopoID},
                                   {tMiddlePoint},
                                   {
                                           {"edge_name", sname},
                                           {"area", sname},
                                           {"tap_bus", state(s).maxTopoID},
                                           {"z_bus", tMiddlePoint},
                                           {"flag", 1},
                                           {"R", List({rstar})},
                                           {"X", List({xstar})},
                                           {"hB_list", List({0})},
                                           {"line_Q1_list", List({normalLimit})},
                                           {"line_Q2_list", List({emerLimit})},
                                           {"line_Q3_list", List({ldshdLimit})},
                                           {"transformer_final_turns_ratio_list",
                                            List({tapRatioThree})},
                                           {"transformer_final_angle", List({0})},
                                           {"min_tap", itapL},
                                           {"max_tap", itapH},
                                           {"step_size", itapC},
                                           {"M_P_TLPF_list", List({sPimeas / 100})},
                                           {"M_Q_TLPF_list", List({sQimeas / 100})},
                                           {"outage", 0},
                                           {"G_list",
                                            List({rstar / (rstar * rstar + xstar * xstar)})},
                                           {"B_list",
                                            List({xstar / (rstar * rstar + xstar * xstar)})},
                                           {"BIJ_list", List({1 / xstar})},
                                           {"kcount", 1},
                                           {"reverse", 1},
                                           {"Decision", List({0})},
                                           {"Analysis_decision", List({0})},
                                           {"volt", std::to_string(svolt)},
                                           {"device_type", "三绕组变压器"},
                                           {"CSeverity", List({-1})},
                                           {"hB", 0},
                                           {"G", rstar / (rstar * rstar + xstar * xstar)},
                                           {"B", xstar / (rstar * rstar + xstar * xstar)},
                                           {"BIJ", 1 / xstar},
                                           {"transformer_final_turns_ratio", tapRatioThree},
                                           {"M_P_TLPF", sPimeas / 100},
                                           {"M_Q_TLPF", sQimeas / 100},
                                           {"line_Q1", normalLimit},
                                           {"line_Q2", emerLimit},
                                           {"line_Q3", ldshdLimit},
                                           {"key_list", List({sid})},
                                           {"from_off", List({0})},
                                           {"to_off", List({0})},
                                   });
                        insertEdge("topo_connect",
                                   {tMiddlePoint},
                                   {state(s).maxTopoID},
                                   {
                                           {"edge_name", sname},
                                           {"area", sname},
                                           {"tap_bus", tMiddlePoint},
                                           {"z_bus", state(s).maxTopoID},
                                           {"flag", 1},
                                           {"R", List({rstar})},
                                           {"X", List({xstar})},
                                           {"hB_list", List({0})},
                                           {"line_Q1_list", List({normalLimit})},
                                           {"line_Q2_list", List({emerLimit})},
                                           {"line_Q3_list", List({ldshdLimit})},
                                           {"transformer_final_turns_ratio_list",
                                            List({-tapRatioThree})},
                                           {"transformer_final_angle", List({0})},
                                           {"min_tap", itapL},
                                           {"max_tap", itapH},
                                           {"step_size", itapC},
                                           {"outage", 0},
                                           {"G_list",
                                            List({rstar / (rstar * rstar + xstar * xstar)})},
                                           {"B_list",
                                            List({xstar / (rstar * rstar + xstar * xstar)})},
                                           {"BIJ_list", List({1 / xstar})},
                                           {"kcount", 1},
                                           {"reverse", -1},
                                           {"Decision", List({0})},
                                           {"Analysis_decision", List({0})},
                                           {"volt", std::to_string(svolt)},
                                           {"device_type", "三绕组变压器"},
                                           {"CSeverity", List({-1})},
                                           {"hB", 0},
                                           {"G", rstar / (rstar * rstar + xstar * xstar)},
                                           {"B", xstar / (rstar * rstar + xstar * xstar)},
                                           {"BIJ", 1 / xstar},
                                           {"transformer_final_turns_ratio", -tapRatioThree},
                                           {"line_Q1", normalLimit},
                                           {"line_Q2", emerLimit},
                                           {"line_Q3", ldshdLimit},
                                           {"key_list", List({-sid})},
                                           {"from_off", List({0})},
                                           {"to_off", List({0})},
                                   });
                    }
                }
            }

            return std::vector<NodeID>{tgts.begin(), tgts.end()};
        });

        //========================= set Frm_To_Cp =========================
        VertexSubset vTPND = verticesByAllLabels(all, {"TopoND"});
        VertexSubset vCP1 =
                vTPND.map([this, graph](NodeID s) {
                         return graph->neighborIDs(s, [this](const Edge &e) -> bool {
                             return getEdgeLabelSet(e.getEdgeID()).count("topo_compensatorP");
                         });
                     }).filter([this](NodeID t) { return getNodeLabelSet(t).count("C_P"); });
        VertexSubset vCN1 = vCP1.map([this, graph](NodeID s) {
            auto qimeas = graph->getProperty(s, "Qimeas").getDouble();
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Compensator_P_CN");
            });
            std::unordered_set<NodeID> res;
            for (auto t : tgts) {
                if (getNodeLabelSet(t).count("CN")) {
                    res.insert(t);
                    writeDouble(&state(t).sumQimeas, qimeas);
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });
        const std::set<std::string> connCNLabels = {
                "connected_Breaker_CN",
                "connected_Disconnector_CN",
        };
        VertexSubset vBD = vCN1.map([this, graph, &connCNLabels](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this, &connCNLabels](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (connCNLabels.count(l)) {
                        return true;
                    }
                }
                return false;
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vCN2 = vBD.map([this, graph, &connCNLabels](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this, &connCNLabels](const Edge &e) -> bool {
                for (auto &l : getEdgeLabelSet(e.getEdgeID())) {
                    if (connCNLabels.count(l)) {
                        return true;
                    }
                }
                return false;
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vD4 = vCN2.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Disconnector_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vCN8 = vD4.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Disconnector_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vCN9 = vCN2.merge(vCN8);
        VertexSubset vACLineDot1 = vCN9.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) {
                return getEdgeLabelSet(e.getEdgeID()).count("aclinedot_cn");
            });
            for (auto t : tgts) {
                if (getNodeLabelSet(t).count("ACline_dot")) {
                    writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
                }
            }
            return tgts;
        });
        std::set<std::string> aclineLabels = {
                "aclinedot_aclinedot",
                "aclinedot_aclinedot_reverse",
        };
        VertexSubset vACLineDot2 = vACLineDot1.map([this, graph, &aclineLabels](NodeID s) {
            auto topoID = graph->getProperty(s, "topoID").getInt64();
            std::unordered_set<NodeID> tgts;
            auto [b, e] = graph->outEdges(s);
            for (; b != e; ++b) {
                for (auto &l : getEdgeLabelSet(*b)) {
                    if (aclineLabels.count(l)) {
                        auto t = b.getDstID();
                        if (getNodeLabelSet(t).count("ACline_dot")) {
                            writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
                            tgts.emplace(t);

                            folly::RWSpinLock::WriteHolder holder(spinLock);
                            state(t).setTopoID1.emplace(topoID);
                            edgeList.push_back(b.getEdge());
                        }
                    }
                }
            }
            return std::vector<NodeID>{tgts.begin(), tgts.end()};
        });

        VertexSubset vD1 = vCN1.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Disconnector_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vCN3 = vD1.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Disconnector_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vB1 = vCN3.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Breaker_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vCN4 = vB1.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Breaker_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vD2 = vCN4.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Disconnector_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vCN5 = vD2.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Disconnector_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vB2 = vCN1.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Breaker_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vCN6 = vB2.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Breaker_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vD3 = vCN6.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Disconnector_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vCN7 = vD3.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Disconnector_CN");
            });
            for (auto t : tgts) {
                writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
            }
            return tgts;
        });
        VertexSubset vBus1 = vCN5.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Bus_CN");
            });
            for (auto t : tgts) {
                if (getNodeLabelSet(t).count("BUS")) {
                    writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
                }
            }
            return tgts;
        });
        VertexSubset vBus2 = vCN7.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("connected_Bus_CN");
            });
            for (auto t : tgts) {
                if (getNodeLabelSet(t).count("BUS")) {
                    writeDouble(&state(t).sumQimeas, state(s).sumQimeas);
                }
            }
            return tgts;
        });
        VertexSubset vTPND3 = vBus1.filter([graph](NodeID s) {
                                       return graph->getProperty(s, "volt").getDouble() > 400;
                                   }).map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("topo_bus");
            });
            std::unordered_set<NodeID> res;
            res.reserve(tgts.size());
            for (auto t : tgts) {
                if (getNodeLabelSet(t).count("TopoND")) {
                    writeDouble(&state(t).sumBusQMeas, std::abs(state(s).sumQimeas / 100));
                    res.emplace(t);
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });
        VertexSubset vTPND4 = vBus2.filter([graph](NodeID s) {
                                       return graph->getProperty(s, "volt").getDouble() > 400;
                                   }).map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) -> bool {
                return getEdgeLabelSet(e.getEdgeID()).count("topo_bus");
            });
            std::unordered_set<NodeID> res;
            res.reserve(tgts.size());
            for (auto t : tgts) {
                if (getNodeLabelSet(t).count("TopoND")) {
                    writeDouble(&state(t).sumBusQMeas, std::abs(state(s).sumQimeas / 100));
                    res.emplace(t);
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });

        VertexSubset vTPND1 = vACLineDot1.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) {
                return getEdgeLabelSet(e.getEdgeID()).count("topo_aclinedot");
            });
            std::unordered_set<NodeID> res;
            res.reserve(tgts.size());
            for (auto t : tgts) {
                if (getNodeLabelSet(t).count("TopoND")) {
                    writeAdd<int64_t>(&state(t).sumLineNo, 1);
                    res.emplace(t);
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });

        vTPND1 = vACLineDot1.map([this, graph](NodeID s) {
            std::string sname(graph->getProperty(s, "name").getString());
            std::unordered_set<NodeID> res;

            auto fn = [&, this](const auto &b) {
                if (getEdgeLabelSet(*b).count("topo_aclinedot")) {
                    auto t = b.getDstID();
                    if (getNodeLabelSet(t).count("TopoND")) {
                        res.emplace(t);
                        folly::RWSpinLock::WriteHolder wh(spinLock);
                        for (auto i : state(t).setTopoID1) {
                            state(t).setTopoID1.emplace(i);
                        }
                        std::string name = getAclineName(sname);
                        state(t).listTpndName.push_back(name);
                        state(t).listTpndQMeas.push_back(std::abs(state(s).sumQimeas / 100));
                        topoAclineDotEdge1.push_back(b.getEdge());
                    }
                }
            };

            for (auto [b, e] = graph->outEdges(s); b != e; ++b) {
                fn(b);
            }

            for (auto [b, e] = graph->inEdges(s); b != e; ++b) {
                fn(b);
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });

        VertexSubset vTPND2 = vACLineDot2.map([this, graph](NodeID s) {
            auto tgts = graph->neighborIDs(s, [this](const Edge &e) {
                return getEdgeLabelSet(e.getEdgeID()).count("topo_aclinedot");
            });
            std::unordered_set<NodeID> res;
            res.reserve(tgts.size());
            for (auto t : tgts) {
                if (getNodeLabelSet(t).count("TopoND")) {
                    writeAdd<int64_t>(&state(t).sumLineNo, 1);
                    res.emplace(t);
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });

        vTPND2 = vACLineDot2.map([this, graph](NodeID s) {
            std::string sname(graph->getProperty(s, "name").getString());
            std::unordered_set<NodeID> res;
            auto fn = [&, this](const auto &b) {
                if (getEdgeLabelSet(*b).count("topo_aclinedot")) {
                    auto tid = b.getDstID();
                    if (getNodeLabelSet(tid).count("TopoND")) {
                        res.emplace(tid);
                        auto &t = state(tid);
                        folly::RWSpinLock::WriteHolder wh(spinLock);
                        for (auto i : t.setTopoID1) {
                            t.setTopoID1.emplace(i);
                        }
                        std::string name = getAclineName(sname);
                        t.listTpndName.push_back(name);
                        t.listTpndQMeas.push_back(t.sumBusQMeas / t.sumLineNo);
                        topoAclineDotEdge2.push_back(b.getEdge());
                    }
                }
            };
            for (auto [b, e] = graph->outEdges(s); b != e; ++b) {
                fn(b);
            }
            for (auto [b, e] = graph->inEdges(s); b != e; ++b) {
                fn(b);
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });

        vTPND1.forEach([this, graph](NodeID s) {
            auto [b, e] = graph->outEdges(s);
            for (; b != e; ++b) {
                if (getEdgeLabelSet(*b).count("topo_connect")) {
                    auto t = b.getDstID();
                    if (getNodeLabelSet(t).count("TopoND")) {
                        auto edge = b.getEdge();
                        edge.setProperty("from_CP_list", emptyList);
                        edge.setProperty("to_CP_list", emptyList);
                        updateEdge(edge);
                    }
                }
            }
        });

        vTPND2.forEach([this, graph](NodeID s) {
            auto [b, e] = graph->outEdges(s);
            for (; b != e; ++b) {
                if (getEdgeLabelSet(*b).count("topo_connect")) {
                    auto t = b.getDstID();
                    if (getNodeLabelSet(t).count("TopoND")) {
                        auto edge = b.getEdge();
                        edge.setProperty("from_CP_list", emptyList);
                        edge.setProperty("to_CP_list", emptyList);
                        updateEdge(edge);
                    }
                }
            }
        });

        VertexSubset vTPND1ConnectedTPND2 = vTPND1.map([this, graph](NodeID s) {
            std::unordered_set<NodeID> res;
            for (auto [b, e] = graph->outEdges(s); b != e; ++b) {
                if (getEdgeLabelSet(*b).count("topo_connect")) {
                    auto t = b.getDstID();
                    if (getNodeLabelSet(t).count("TopoND")) {
                        res.emplace(t);

                        folly::RWSpinLock::WriteHolder wh(spinLock);
                        topoConnectEdge1.push_back(b.getEdge());
                    }
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });
        VertexSubset vTPND2ConnectedTPND1 = vTPND2.map([this, graph](NodeID s) {
            std::unordered_set<NodeID> res;
            for (auto [b, e] = graph->outEdges(s); b != e; ++b) {
                if (getEdgeLabelSet(*b).count("topo_connect")) {
                    auto t = b.getDstID();
                    if (getNodeLabelSet(t).count("TopoND")) {
                        res.emplace(t);

                        folly::RWSpinLock::WriteHolder wh(spinLock);
                        topoConnectEdge2.push_back(b.getEdge());
                    }
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });
        VertexSubset vTopoSet = vTPND.map([this, graph](NodeID s) {
            std::unordered_set<NodeID> res;
            for (auto [b, e] = graph->outEdges(s); b != e; ++b) {
                Edge edge = b.getEdge();
                if (getEdgeLabelSet(*b).count("topo_connect")) {
                    auto t = b.getDstID();
                    if (getNodeLabelSet(t).count("TopoND")) {
                        res.emplace(t);
                        edge.setProperty("from_CP", 0);
                        edge.setProperty("to_CP", 0);
                        updateEdge(edge);

                        // clang-format off
                        /*
                  double from_CP = 0,
                  double to_CP = 0,
                  FOREACH i in range [0, e.from_CP_list.size()-1] DO
                    from_CP = from_CP + e.from_CP_list.get(i)//,
                  END,
                  e.from_CP = from_CP,
                  FOREACH i in range [0, e.to_CP_list.size()-1] DO
                    to_CP = to_CP + e.to_CP_list.get(i)//,
                  END,
                  e.to_CP = to_CP;
                         */
                        // clang-format on
                    }
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });

        VertexSubset tTest = vTPND.map([this, graph](NodeID s) {
            std::unordered_set<NodeID> res;
            for (auto [b, e] = graph->outEdges(s); b != e; ++b) {
                if (getEdgeLabelSet(*b).count("topo_connect")) {
                    auto t = b.getDstID();
                    if (getNodeLabelSet(t).count("TopoND")) {
                        res.emplace(t);
                    }
                }
            }
            return std::vector<NodeID>{res.begin(), res.end()};
        });
    }  // end of run

    std::string name() const override {
        return "yj.network_topo";
    }

private:
    folly::RWSpinLock spinLock;
    std::list<nebula::Edge> edgeList;
    std::list<nebula::Edge> topoAclineDotEdge1;
    std::list<nebula::Edge> topoAclineDotEdge2;
    std::list<nebula::Edge> topoConnectEdge1;
    std::list<nebula::Edge> topoConnectEdge2;
    nebula::List emptyList;
};

}  // namespace yj


static constexpr const char *kColumnNames[] = {
        "topoID",
        "point",
        "itopoID",
        "jtopoID",
};

static folly::Future<ExecutionOutcome> networkTopoProcedure(ProcContextPtr pctx,
                                                            std::vector<Value> args) {
    ExecutionOutcome outcome;
    outcome.status = Status::OK();

    if (!pctx->rctx()) {
        return outcome;
    }

    if (args.size() < 1u || !args[0].isString()) {
        return outcome;
    }

    auto engine = pctx->computingEngine();

    const auto &ref = args[0].getRef();
    auto memGraph = pctx->refCatalog()->getGraph(ref.entryID());

    auto ctx = std::make_unique<ComputingContext>(engine, memGraph.get(), pctx->rctx());
    auto algo = std::make_unique<yj::NetworkTopoAlgorithm>(ctx.get());
    algo->run();

    ResultTable table;
    std::vector<std::string> colNames{std::begin(kColumnNames), std::end(kColumnNames)};
    table.setColumnNames(std::move(colNames));

    algo->getResult(&table);

    outcome.result.emplace(std::move(table));
    return outcome;
}

Procedure declareNetworkTopoProcedure() {
    Procedure proc;
    proc.name = "network_topo";
    proc.comment = "network topology processing algorithm";
    proc.func = &networkTopoProcedure;
    proc.params = {
            Parameter{
                    std::make_shared<nebula::StringValueType>(),
                    "graphName",
                    "graph name",
            },
    };
    for (auto &name : kColumnNames) {
        proc.fields.emplace_back(Field{
                std::make_shared<nebula::StringValueType>(),
                name,
        });
    }
    return proc;
}
