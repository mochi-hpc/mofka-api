/*
 * (C) 2023 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef MOFKA_API_PARTITION_SELECTOR_HPP
#define MOFKA_API_PARTITION_SELECTOR_HPP

#include <mofka/ForwardDcl.hpp>
#include <mofka/Metadata.hpp>
#include <mofka/Exception.hpp>
#include <mofka/Factory.hpp>

#include <functional>
#include <exception>
#include <stdexcept>
#include <optional>

namespace mofka {

/**
 * @brief The PartitionInfo structure holds information about a particular partition.
 * The information contained in such a Metadata piece is implementation-defined.
 */
class PartitionInfo : public Metadata {
    using Metadata::Metadata;
};

/**
 * @brief The PartitionSelectorInterface class provides an interface for
 * objects that decide how which target server will receive each event.
 *
 * A PartitionSelectorInterface must also provide functions to convert
 * itself into a Metadata object an back, so that its internal
 * configuration can be stored.
 */
class PartitionSelectorInterface {

    public:

    /**
     * @brief Destructor.
     */
    virtual ~PartitionSelectorInterface() = default;

    /**
     * @brief Sets the list of targets that are available to store events.
     *
     * @param targets Vector of PartitionInfo.
     */
    virtual void setPartitions(const std::vector<PartitionInfo>& targets) = 0;

    /**
     * @brief Selects a partition target to use to store the given event.
     * The returned value should be an index between 0 and N-1 where N is the
     * size of the array passed to setPartitions().
     *
     * @param metadata Metadata of the event.
     * @param requested Partition requested by the caller of push() (if provided).
     */
    virtual size_t selectPartitionFor(const Metadata& metadata,
                                      std::optional<size_t> requested) = 0;

    /**
     * @brief Convert the underlying validator implementation into a Metadata
     * object that can be stored (e.g. if the validator uses a JSON schema
     * the Metadata could contain that schema).
     */
    virtual Metadata metadata() const = 0;

    /**
     * @note A PartitionSelectorInterface class must also provide a static create
     * function with the following prototype, instanciating a shared_ptr of
     * the class from the provided Metadata:
     *
     * static std::shared_ptr<PartitionSelectorInterface> create(const Metadata&);
     */
};

class PartitionSelector {

    friend struct PythonBindingHelper;
    friend class Driver;

    public:

    PartitionSelector(const std::shared_ptr<PartitionSelectorInterface>& impl)
    : self(impl) {}

    /**
     * @brief Constructor. Will construct a valid PartitionSelector that accepts
     * any Metadata correctly formatted in JSON.
     */
    PartitionSelector();

    /**
     * @brief Copy-constructor.
     */
    inline PartitionSelector(const PartitionSelector&) = default; // LCOV_EXCL_LINE

    /**
     * @brief Move-constructor.
     */
    inline PartitionSelector(PartitionSelector&&) = default; // LCOV_EXCL_LINE

    /**
     * @brief copy-assignment operator.
     */
    inline PartitionSelector& operator=(const PartitionSelector&) = default; // LCOV_EXCL_LINE

    /**
     * @brief Move-assignment operator.
     */
    inline PartitionSelector& operator=(PartitionSelector&&) = default; // LCOV_EXCL_LINE

    /**
     * @brief Destructor.
     */
    inline ~PartitionSelector() = default; // LCOV_EXCL_LINE

    /**
     * @brief Checks for the validity of the underlying pointer.
     */
    explicit inline operator bool() const {
        return static_cast<bool>(self);
    }

    /**
     * @brief Sets the list of targets that are available to store events.
     *
     * @param targets Vector of PartitionInfo.
     */
    inline void setPartitions(const std::vector<PartitionInfo>& targets) const {
        self->setPartitions(targets);
    }

    /**
     * @brief Selects a partition target to use to store the given event.
     *
     * @param metadata Metadata of the event.
     * @param requested Partition requested by the caller of push() (if provided).
     */
    inline size_t selectPartitionFor(const Metadata& metadata,
                              std::optional<size_t> requested = std::nullopt) const {
        return self->selectPartitionFor(metadata, requested);
    }

    /**
     * @brief Convert the underlying validator implementation into a Metadata
     * object that can be stored (e.g. if the validator uses a JSON schema
     * the Metadata could contain that schema).
     */
    inline Metadata metadata() const {
        return self->metadata();
    }

    /**
     * @brief Factory function to create a PartitionSelector instance.
     * The Metadata object is expected to be a JSON object containing
     * at least a "type" field. If this field is not found, it is
     * assumed to be "default".
     *
     * @param metadata Metadata of the PartitionSelector.
     *
     * @return PartitionSelector instance.
     */
    static PartitionSelector FromMetadata(const Metadata& metadata);

    private:

    std::shared_ptr<PartitionSelectorInterface> self;
};

using PartitionSelectorFactory = Factory<PartitionSelectorInterface, const Metadata&>;

#define MOFKA_REGISTER_PARTITION_SELECTOR(__name__, __type__) \
    MOFKA_REGISTER_IMPLEMENTATION_FOR(PartitionSelectorFactory, __type__, __name__)

}

#endif
