// Copyright 2019 Oath Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "blueprint.h"
#include "nearest_neighbor_distance_heap.h"

namespace vespalib::tensor { class DenseTensorView; }
namespace search::tensor { class DenseTensorAttribute; }

namespace search::queryeval {

/**
 * Blueprint for nearest neighbor search iterator.
 *
 * The search iterator matches the K nearest neighbors in a multi-dimensional vector space,
 * where the query point and document points are dense tensors of order 1.
 */
class NearestNeighborBlueprint : public ComplexLeafBlueprint {
private:
    const tensor::DenseTensorAttribute& _attr_tensor;
    std::unique_ptr<vespalib::tensor::DenseTensorView> _query_tensor;
    uint32_t _target_num_hits;
    mutable NearestNeighborDistanceHeap _distance_heap;

public:
    NearestNeighborBlueprint(const queryeval::FieldSpec& field,
                             const tensor::DenseTensorAttribute& attr_tensor,
                             std::unique_ptr<vespalib::tensor::DenseTensorView> query_tensor,
                             uint32_t target_num_hits);
    NearestNeighborBlueprint(const NearestNeighborBlueprint&) = delete;
    NearestNeighborBlueprint& operator=(const NearestNeighborBlueprint&) = delete;
    ~NearestNeighborBlueprint();
    const tensor::DenseTensorAttribute& get_attribute_tensor() const { return _attr_tensor; }
    const vespalib::tensor::DenseTensorView& get_query_tensor() const { return *_query_tensor; }
    uint32_t get_target_num_hits() const { return _target_num_hits; }

    std::unique_ptr<SearchIterator> createLeafSearch(const search::fef::TermFieldMatchDataArray& tfmda,
                                                     bool strict) const override;
    void visitMembers(vespalib::ObjectVisitor& visitor) const override;
    bool always_needs_unpack() const override;
};

}
