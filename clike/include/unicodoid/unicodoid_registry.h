#ifndef UNICODOID_REGISTRY_H
#define UNICODOID_REGISTRY_H

#include "unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_registry.h
 * @brief Process-wide registry of encoding systems.
 *
 * The registry stores borrowed pointers.  Registered systems must outlive their
 * registration.  Built-in systems are static singletons.
 */

/** Initialize the registry and register the built-in systems. Idempotent. */
unicodoid_status_t unicodoid_registry_init(void);

/** Remove all registrations. */
void unicodoid_registry_cleanup(void);

/** Register a system.  Returns UNICODOID_ERR_ALREADY_EXISTS for duplicate IDs. */
unicodoid_status_t unicodoid_registry_register(const unicodoid_system_t* sys);

/** Unregister a system by ID. */
unicodoid_status_t unicodoid_registry_unregister(const char* id);

/** Find a system by ID; initializes the registry on first use. */
const unicodoid_system_t* unicodoid_registry_find_by_id(const char* id);

/** Find a system by display name. */
const unicodoid_system_t* unicodoid_registry_find_by_name(const char* name);

/** Number of registered systems. */
size_t unicodoid_registry_count(void);

/** Retrieve a system by index, or NULL when out of range. */
const unicodoid_system_t* unicodoid_registry_get_at(size_t index);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_REGISTRY_H */
